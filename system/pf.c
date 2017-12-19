#include <xinu.h>
#include <string.h>
#include <lab3.h>
#include <paging.h>


struct fr_map frmtab[NFRAMES];
struct bs_map bsmtab[MAX_ID +1];
sid32 pfsem;
//int page_faulting, ptime;


syscall pf()
{

	//vaddr* vaddress;
	wait(pfsem);
	if((proctab[currpid].vmemlist->mnext==NULL) && (proctab[currpid].nonvirtual==0))
	{
		//kprintf("\n I am initializing the next here for pid = %d", currpid);
		proctab[currpid].vmemlist->mnext=(struct virtualblock*)(4096*4096);
	}	
	if(proctab[currpid].nonvirtual == 1)
	{
		//kprintf("\n non-virtual processes \n and name of it = %s", proctab[currpid].prname);
		return OK;
	}

	unsigned long pdbr, address;
	pd_t* pd_entry;
	struct procent * pptr;
	//uint32 new_pt, curr_frame_vpno;
	uint32 new_frame, pg_offset_bs;	
	bsd_t bs;
	pt_t* pt_entry;
	uint32 pd_offset, pt_offset;
	//ptime=clktime; 
	//unsigned long temp;	
	page_faulting=1;
	get_faults();
	//kprintf("\n the total number of page-faults = %d \n", count);

	page_faulting=0;
	address= read_cr2();
	//vaddress=((vaddr* ) address);
	//kprintf("\n the page %32b , page pt_offset = %10b pd_offset = %10b \n", address, vaddress->pt_offset, vaddress->pd_offset);
	pptr = &proctab[currpid];
	pdbr = pptr->pdbr;
//	pdbr=pdbr<<22;
	//kprintf("\n the pdbr value = %32b and 0x%08x address in bits = 0x%08x and in bits = %32b  and in integer = %d \n ",  pdbr, pdbr, address, address, address);
	if((address < (unsigned long) (4096*4096)))
	{
		//kprintf("\n Invalid address access and number of virtual heap pages =%d \n and I am killing the process", pptr->vpnpages);
		//kprintf("\n In same condition, the faulted address is = %lu and the address in bits = 0x%08x and in bits = %32b  and in integer = %d \n", address, address, address, address);
		kprintf("\n -----Invalid Address Access-----\n I am killing currpid and currpid is= %d \n", currpid);
		kill(currpid);
		return SYSERR;
	} 
	
	
	//curr_frame_vpno = (uint32)(address/4096);
	pd_offset = ((address)>>22);     //gives offset page directory
	pt_offset = ((address >> 12) & 0x000003ff);//(((address)>>12)&1023); //gives offset in page table 

	//kprintf("\n ---------------------------\n for pid = %d page fault handler is getting called with address = %u and 0x%08x and pdbr = %d\n -------------------------\n ", currpid, address, address, pdbr);
	//pd_t * pdir = (pd_t*) (pdbr*NBPG + sizeof(pd_t) * (pd_offset));
	pd_entry= (pd_t *) (pdbr*NBPG);

	/*kprintf("\n the pdir is = %32b and 0x%08x and in decimal = %d", (unsigned long)pdir, (unsigned long)pdir, (unsigned long)pdir); 
	kprintf("\n the pd_entry is = %32b and 0x%08x and in decimal = %d", (unsigned long)&pd_entry[pd_offset], (unsigned long)&pd_entry[pd_offset], &pd_entry[pd_offset]); */
	int frame_no;
    	if(pd_entry[pd_offset].pd_pres == 0)
	{
		//kprintf("page table not present\n");
		get_frm(&frame_no);
		//kprintf("pd the allocated frame_no from FIFO is = %d\n", frame_no);
		frmtab[frame_no].fr_vpno = (unsigned int) address/4096;
		pd_entry[pd_offset].pd_pres = 1;
		pd_entry[pd_offset].pd_write = 1;
		pt_entry = create_a_page_table(currpid, frame_no, FR_TBL);
		//hook_ptable_create(FRAME0+frame_no);
		pd_entry[pd_offset].pd_base = ((unsigned int)pt_entry/NBPG);
		//pptr->pdbr=pd_entry[pd_offset].pd_base;
		//new_pt=pd_entry[pd_offset].pd_base-FRAME0;
		//kprintf("\n in page-directory not present, page table created %d and the frmae_no is =%d \n ", pd_entry[pd_offset].pd_base, frame_no);
		if(pt_entry==NULL)
		{
			kill(currpid);
    			return SYSERR;
		}
	}
	//kprintf("\n I am here");
	pt_entry = (pt_t*)((pd_entry[pd_offset].pd_base)*NBPG);
	//pt_t* pagetable=(pt_t*)((pd_entry[pd_offset].pd_base)*NBPG);
	if(pt_entry[pt_offset].pt_pres == 0)
	{
		get_frm(&new_frame);
		/*reclaim++;		
		if(reclaim == NFRAMES)	
		{
			reclaim= 8;
			replacement=1;
			kprintf("\n frames are getting replaced ");
		}
		new_frame=reclaim;
		frmtab[new_frame].replace=replacement;	
		
		if(frmtab[new_frame].replace==1)
		{
			if(free_frm(new_frame) == SYSERR)
				kprintf("\n its not freeing the frame \n");
		}*/

			
		//kprintf("------------------the allocated frame_no is %d\n", new_frame);

		pt_entry[pt_offset].pt_pres = 1;
		pt_entry[pt_offset].pt_write = 1;
		pt_entry[pt_offset].pt_base = (FRAME0 + new_frame);

		frmtab[pd_entry[pd_offset].pd_base - FRAME0].fr_refcnt++;
		frmtab[new_frame].fr_status = FRM_MAPPED;
		frmtab[new_frame].fr_pid = currpid;
		frmtab[new_frame].fr_vpno = address/NBPG;
		frmtab[new_frame].fr_type=FR_PAGE;

		//frmtab[new_frame].fr_vpno = (proctab[currpid].b_store*MAX_PAGES_PER_BS)+4096;
		//uint32 curr_frame_vpno = address/4096;
	//	 kprintf("In pfint before bsm and address is = %d the vp_no is = %d  and currpid = %d \n", address, curr_frame_vpno, currpid);
		bsm_lookup(currpid,address,&bs,&pg_offset_bs);
		

	//	kprintf("Reading from -- bs: %d, pg_offset_bs: %d and While writing the pagenum : %d \n", bs, pg_offset_bs, frmtab[new_frame].fr_vprev);
		/*if(recheck[new_frame].replace == 1)
		{
			if ((recheck[new_frame].bsn == bs)&&(recheck[new_frame].pagenum == pg_offset_bs))
			{*/
		if(open_bs(bs)==SYSERR)
				kprintf("\n unable to open the backing store");
		read_bs((char*) ((FRAME0+new_frame)*NBPG), bs, pg_offset_bs);
		recheck[new_frame].replace = 0;
		close_bs(bs);
		vaddr* vaddress= ((vaddr *) address);
		hook_pfault(currpid, vaddress, frmtab[new_frame].fr_vpno, new_frame);

		//}
		//}
		//read_bs((char*)((FRAME0+new_frame)*NBPG), bs, pg_offset_bs);
		//read_bs((char*)((FRAME0+new_frame)*NBPG), bs, pg_offset_bs);

		frmtab[new_frame].fr_vprev=pg_offset_bs;

		if(currpolicy == FIFO){
			//kprintf("\n ========= \n pushing frame no. %d\n ===============", new_frame);
			insert_frm_fifo(new_frame);	//insert_frm_fifo(new_frame);
		}

	}

	//kprintf("In pfint after read_bs\n");		
	/*pdbr = (unsigned long)pd_entry/NBPG;
	temp = pdbr<<12;
	kprintf("\n value of the pd_entry : %lx and in %32b \n PDBR value : %d , temp %lx", (unsigned long)pd_entry, (unsigned long)pd_entry, pdbr, temp);*/
	//write_cr3(temp);

	/*vaddr* vaddress= ((vaddr *) address);
	hook_pfault(currpid, vaddress, frmtab[new_frame].fr_vpno, new_frame);*/
	signal(pfsem);
	/*sum=sum+(clktime-ptime);
	kprintf("\n total fault processing time = %d", sum);*/
	//kprintf("\n *vaddress = %d,vaddress = %32b and %d", *vaddress, vaddress, vaddress);
	//kprintf("\n after hooks currpid =%d , address =%d, vp_no= %d and new_frame = %d", currpid, vaddress, frmtab[new_frame].fr_vpno, new_frame); 
	return OK;
}







syscall bsm_lookup(pid32 pid, long vaddr, bsd_t* store, int* pageth)
{
	int i = 0;
	for (i = 0; i < MAX_ID+1; i++)
	{
		if(bsmtab[i].bs_pid == pid)
			{
				if(((vaddr/NBPG) >= bsmtab[i].bs_vpno) && (vaddr/NBPG < (bsmtab[i].bs_vpno + bsmtab[i].bs_npages)))
				{
					*store = i; //store value
					*pageth = (vaddr/NBPG) - bsmtab[i].bs_vpno; //page offset
					//kprintf("\n from bsm lookup the pageth is = %d \n", *pageth);
					if ((*pageth < 0)  || (*pageth > MAX_PAGES_PER_BS)) panic("\n wrong pageoffset in backingstore ");
					return OK;
				}
			}
	}
	return SYSERR;
}


syscall bsm_map(pid32 pid, int vpno, int source, int npages)
{
	bsmtab[source].bs_npages = npages;
	bsmtab[source].bs_pid = pid;
	bsmtab[source].bs_sem = 1;
	bsmtab[source].bs_status = BSM_MAPPED;
	bsmtab[source].bs_vpno = vpno; //must be the base virtual page no
	proctab[pid].vpno=vpno;
	proctab[pid].b_store=source;
	proctab[pid].vpnpages=npages;
	
	return OK;
}



//if(frmtab[new_frame].replace == 1){
			/*if(frmtab[new_frame].fr_vprev != pg_offset_bs)
				panic("\n the pagenum are not the same");*/

/*int store,pageth;
if ((bsm_lookup(currpid, address, &store, &pageth) == SYSERR) || (store == -1) || (pageth == -1)) 
	{
		kprintf("%\nlu : Not Legal Address. This is not mapped\n", address);
		kill(currpid);
		return (SYSERR);
	}
*/

/*int loopbreaker=0;
		for(int i=7;i<3072;i++)
		{
			if((frmtab[i].fr_status == FRM_UNMAPPED) && (loopbreaker==0))
			{
				frame_no=i;
				kprintf("\n Inside pf handler in pd for now I am getting a free frame at %d", i);
				loopbreaker=1 ;
			}
		}*/

//kprintf("\n the pt_entry is = %32b and 0x%08x and in decimal = %d", (unsigned long)pt_entry, (unsigned long)pt_entry, (unsigned long)pt_entry); 
//kprintf("\n the pagetable is = %32b and 0x%08x and in decimal = %d", (unsigned long)&pagetable[pt_offset], (unsigned long)&pagetable[pt_offset], &pagetable[pt_offset]); 
//pt_entry = ((pt_t *)0xFFC00000) + (0x400 * pdindex); //from resources

