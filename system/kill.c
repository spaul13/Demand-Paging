/* kill.c - kill */

#include <xinu.h>
void evict_frame (pid32 pid);
syscall kill_bsm(pid32 pid);
/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	//kprintf("KILL : %d\n", pid);
	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}
	
	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

  // Lab3 TODO. Free frames as a process gets killed.
	/*bsm_lookup(currpid,address,&bs,&pg_offset_bs);
	for(int i=0;i<3072;i++)
	{
		if(frmtab[i].fr_pid==currpid) //need not to write backf
		{
			write_bs((char*)((FRAME0+i)*NBPG),bs,frmtab[i].fr_vpno); //how to get page_numth page of the backing store to write
		}
	}
			
	write_bs((char*)((FRAME0+new_frame)*NBPG),bs,pg_offset_bs); // write the page pointed by the FRAME0+new_frame to the bs*/
	
	kill_bsm(pid);
	deallocate_bs(proctab[pid].b_store); //releasing the backing store
	evict_frame(pid);


	freestk(prptr->prstkbase, prptr->prstklen);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
void evict_frame(pid32 pid)
{
	int i = 0;
	for(i = 0; i < NFRAMES; i++)
		{
		if(frmtab[i].fr_pid == pid)
			{
		  	/*if(frm_tab[i].fr_type == FR_PAGE)
				update_frm_fifo(i);*/
			if(frmtab[i].fr_type==FR_TBL)
				kprintf("\n frame holding process page directory or table");
			else if(frmtab[i].fr_type==FR_PAGE)
				kprintf("\n frame for the pages");
			hook_ptable_delete(i);
		   	frmtab[i].fr_dirty = 0;
		  	frmtab[i].fr_pid = -1;
		  	frmtab[i].fr_refcnt = 0;
		  	frmtab[i].fr_status = FRM_UNMAPPED;
		  	//frm_tab[i].fr_type = FR_PAGE;
		  	frmtab[i].fr_vpno = 4096;
		  	//frm_tab[i].next_frame = -1;
			}
		}
}
syscall kill_bsm(pid32 pid)
{
    for(int i = 0; i < (MAX_ID+1); i++)
		{
		if(bsmtab[i].bs_pid == pid)
			{
			bsmtab[i].bs_npages = 0;
			bsmtab[i].bs_sem = 0;
			bsmtab[i].bs_status = BSM_UNMAPPED;
			bsmtab[i].bs_vpno = 4096;
			}
		}
		//kprintf("\n the bsm's are successfully killed");
		return OK;
	}


