/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	//kprintf("resched\n");
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
	//kprintf("\n I am inside resched() and currpid is =%d", currpid);

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			//kprintf("\n executing the previous one \n ");
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/

  // Lab3. TODO: change the page directories as a process is ctx out
	//kprintf(" \n the old pdbr is =%lx and 0x%08d", (unsigned long)ptold->pdbr, ptold->pdbr);
	//kprintf("\n the new pdbr is =%lu and 0x%08d", (unsigned long)ptnew->pdbr, ptnew->pdbr);
/*	if(ptnew->nonvirtual==1)
	{
		kprintf("\n the process name is = %s", ptnew->prname);
		ptnew->pdbr=(unsigned long)pagedir_null/NBPG;
	}*/
	unsigned long temp= ptnew->pdbr << 12; 
	write_cr3(temp);
	/*kprintf("\n the old process name =%s and the name of the new process =%s", ptold->prname, ptnew->prname);
	kprintf("\n the new temp is =%32b and 0x%08d", temp, temp);*/
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
