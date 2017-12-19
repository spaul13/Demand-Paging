/* srpolicy.c - srpolicy */

#include <xinu.h>
#include <string.h>
#include <lab3.h>
#include <paging.h>

int32 currpolicy;
unsigned long inaddr;
struct fifo_node frame_fifo[NFRAMES];
//uint32 fifo_head;
/*------------------------------------------------------------------------
 *  srplicy  -  Set the page replacement policy.
 *------------------------------------------------------------------------
 */
syscall srpolicy(int policy)
{
	switch (policy) {
	case FIFO:
		/* LAB3 TODO */
		currpolicy = FIFO;
		return OK;

	case GCA:
		/* LAB3TODO - Bonus Problem */
    currpolicy = GCA;
    return SYSERR;

	default:
		return SYSERR;
	}
}



syscall get_frm(uint32* avail)
{
  
  sid32 fsem = semcreate(1); 
  uint32 i = 0;
  *avail = -1;
  uint32 frame_number;
  for(i = 0; i < NFRAMES; i++)
  	{
  	if(frmtab[i].fr_status == FRM_UNMAPPED) //first checking from the IPT if any one is available or not
  		{
  		*avail = i;
		return OK;
  		}
  	}

  if(currpolicy == FIFO)
  	{
	wait(fsem);
  	frame_number = remove_frm_fifo();
	free_frm(frame_number);
	*avail = frame_number;
	signal(fsem);
	return OK;
  	}
  
  if(currpolicy == GCA)
	{
	wait(fsem);
	frame_number = get_frm_GCA();
	free_frm(frame_number);
	*avail = frame_number;
	signal(fsem);
	return OK;
	}
  panic("\n ERROR: the currpolicy doesn't match");
  return SYSERR;
}

uint32 remove_frm_fifo()
{
	uint32 frame_number;
	frame_number = fifo_head;
	//kprintf("\n ==========================\n Popping the Frame_no = %d \n ====================== \n ", frame_number);
	
	fifo_head = frame_fifo[fifo_head].next_frame;

	frame_fifo[frame_number].next_frame = -1;		

	return(frame_number);

}

syscall free_frm(uint32 i)
{
	unsigned long v_addr;	
	unsigned int pt_offset, pd_offset;
	unsigned long pdbr;
	pd_t *pd_entry; 
	pt_t *pt_entry;
	bsd_t bs_store;
	uint32 page_num;
	pid32 frame_pid;
	frmtab[i].replace=1;
	if(i>NFRAMES) panic("\n invalid frame-no");

	if(frmtab[i].fr_type == FR_PAGE)
	{
		unsigned long address=(unsigned long) ((frmtab[i].fr_vpno)*4096);
		// kprintf("\n the associated vp no is = %d", frmtab[i].fr_vpno); 
		v_addr = frmtab[i].fr_vpno;
		if(frmtab[i].fr_pid == currpid)
		{
			//asm volatile("invlpg (%-1)" ::"r" (a) : "memory"); //invalidate TLB
			//unsigned long tmp = a;
			inaddr=(unsigned long) ((frmtab[i].fr_vpno)*4096);
		//	kprintf("\n -------- I am invalidating TLB------------ and fr_vpno is = %d\n", v_addr);
 			asm("invlpg inaddr");
		}

		frame_pid = frmtab[i].fr_pid;
		hook_pswap_out(frame_pid, v_addr, i); //hook log for page swapping 
		pdbr = proctab[frame_pid].pdbr;	
		
		pd_offset = ((address)>>22);     //gives offset page directory
		pt_offset = ((address >> 12) & 0x000003ff);		
		/*pd_offset = v_addr/1024;
		pt_offset = v_addr&1023;*/
		
		pd_entry = (pd_t *)(pdbr*NBPG);
		pt_entry = (pt_t *) (pd_entry[pd_offset].pd_base*NBPG);
		
		//pt_entry = (pt_t *)((i+FRAME0)*NBPG);

		/*frmtab[i].fr_refcnt=frmtab[i].fr_refcnt-1;

		if(frmtab[i].fr_refcnt==0)
		{
			pd_entry[pd_offset].pd_pres=0; //pd is not present
		}*/

		if(pt_entry[pt_offset].pt_dirty==1) //we can also use the backing store map to get the page_num and bs_store
		{
			//write_bs((char * )((i+FRAME0)*NBPG) , bs_store, page_num);		
			//write_bs((char * )((i+FRAME0)*NBPG) , bs_store, page_num);
		/*	bs_store = proctab[frmtab[i].fr_pid].b_store;
			page_num = frmtab[i].fr_vpno - proctab[frame_pid].vpno;*/

					
		//	kprintf("\n --------------- the pid  =%d and the vaddr =%u", frmtab[i].fr_pid, address);
			if((bsm_lookup(frmtab[i].fr_pid,address,&bs_store,&page_num)==SYSERR))
			{
				kprintf("\n ERROR: backing store map is not found");
				panic("\n print not finding bs_store and page_num");
			}
			else
			{
				//kprintf("\n ---popping -- Frame: %d, frame_ address : %d , bs_store: %d, pg_num: %d\n", i, (i+FRAME0)*NBPG, bs_store, page_num);	
				if(open_bs(bs_store)==SYSERR)
					kprintf("\n unable to open the backing store");
				write_bs((char * )((i+FRAME0)*NBPG) , bs_store, page_num);
				recheck[i].replace=1;
				recheck[i].bsn=bs_store;
				recheck[i].pagenum=page_num;
				//kprintf("====== \n the writing addresses are = %u ===============", (i+FRAME0)*NBPG);
				frmtab[i].fr_vprev = page_num;
				close_bs(bs_store);
			}
	
		}
		if(--(frmtab[pd_entry[pd_offset].pd_base-FRAME0].fr_refcnt) == 0)
		{
			frmtab[pd_entry[pd_offset].pd_base-FRAME0].fr_pid = -1;
			frmtab[pd_entry[pd_offset].pd_base-FRAME0].fr_status = FRM_UNMAPPED;
			frmtab[pd_entry[pd_offset].pd_base-FRAME0].fr_type = FR_PAGE;
			frmtab[pd_entry[pd_offset].pd_base-FRAME0].fr_vpno = 4096;
			pd_entry[pd_offset].pd_pres = 0;
		}


	//	kprintf("\n pt_entry[%d] = %d, freeing pt_offset: %d\n====================\n ", pt_offset, pt_entry[pt_offset], pt_offset);
		pt_entry[pt_offset].pt_pres = 0;
		//pt_entry[pt_offset].pt_base = 0;
	}

		return OK;
}




syscall insert_frm_fifo(int frame_num)
{
	int nxt_frm = -1;
	int curr_frm = -1;
	
	if(fifo_head == -1)
		{
		fifo_head = frame_num;
	//	kprintf("Updated fifo_head for the first time\n");
		return OK;
		}
	else
		{
		nxt_frm = frame_fifo[fifo_head].next_frame;
		curr_frm = fifo_head;
		}
	
	while(nxt_frm != -1)
		{
		//kprintf("In while loop for to update fifo\n");
		curr_frm = nxt_frm;
		nxt_frm = frame_fifo[nxt_frm].next_frame;
		}
	
	frame_fifo[curr_frm].next_frame = frame_num;
	frame_fifo[frame_num].next_frame = -1;
	
	//kprintf(" After updating fifo\n");
	return OK;
	
}

uint32 get_frm_GCA()
{
	
	unsigned int pd_offset, pt_offset;
	unsigned long pdbr;
	pd_t* pd_entry;
	pt_t* pt_entry;	
	int i=0;

	while(TRUE)
	{
	//kprintf("\n I am iterating for %d times", i);
	i++;
	for(uint32 frame_no = returning_frame; frame_no < NFRAMES; frame_no ++)
	{
		uint32 vpno = frmtab[frame_no].fr_vpno;
		unsigned long address = (unsigned long)vpno * 4096;
		pd_offset = ((address)>>22);     //gives offset page directory
		pt_offset = ((address >> 12) & 0x000003ff);		
		
		pdbr = proctab[frmtab[frame_no].fr_pid].pdbr; //proctab[currpid].pdbr;
		pd_entry = (pd_t *)(pdbr*NBPG);
		pt_entry = (pt_t *) (pd_entry[pd_offset].pd_base*NBPG);
	
		if ( frmtab[frame_no].fr_type == FR_PAGE)
		{
			if ( pd_entry[pd_offset].pd_pres )
			{
				if ((pt_entry[pt_offset].pt_acc ==1 ) && (pt_entry[pt_offset].pt_dirty ==1 ))
				{
					//kprintf("\n --------------[%d] both bits are 1, making dirty 0", frame_no);
					pt_entry[pt_offset].pt_dirty = 0;
				}
				else if ((pt_entry[pt_offset].pt_acc ==0 ) && (pt_entry[pt_offset].pt_dirty ==1 ))
				{
					//kprintf("\n ------- [%d] access bit is 1, making 0", frame_no);
					pt_entry[pt_offset].pt_dirty = 0;
				}
				else if ((pt_entry[pt_offset].pt_acc ==1 ) && (pt_entry[pt_offset].pt_dirty == 0))
				{
					//kprintf("\n ----------- [%d] dirty bit is 1, making 0", frame_no);
					pt_entry[pt_offset].pt_acc = 0;
				}	
				else
				{
					//kprintf("\n getting both 0s and using this frame %d", frame_no);
					returning_frame = frame_no+1;
					if (returning_frame >= NFRAMES) returning_frame =0;
					uint32 frame_number = frame_no;
					hook_pswap_out(frmtab[frame_no].fr_pid, vpno, frame_no); //checking hook log for correct implementation
					return frame_number;
				}
			}
		}
	}
	}

	panic("\n frame freeing is not done ");
	return SYSERR;

}
	















/*			bs_store = proctab[frmtab[i].fr_pid].b_store;
			page_num = frmtab[i].fr_vpno - proctab[frame_pid].vpno;

			kprintf("Freeing -- Frame: %d, bs_store: %d, pg_num: %d\n", i+FRAME0, bs_store, page_num);
			if(open_bs(bs_store)==SYSERR)
				kprintf("\n unable to open the backing store");
			write_bs((char * )((i+FRAME0)*NBPG) , bs_store, page_num);
			close_bs(bs_store);*/

