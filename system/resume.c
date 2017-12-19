/* resume.c - resume */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  resume  -  Unsuspend a process, making it ready
 *------------------------------------------------------------------------
 */
pri16	resume(
	  pid32		pid		/* ID of process to unsuspend	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	pri16	prio;			/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16)SYSERR;
	}
	prptr = &proctab[pid];
	if (prptr->prstate != PR_SUSP) {
		restore(mask);
		return (pri16)SYSERR;
	}
	prio = prptr->prprio;		/* Record priority to return	*/
	ready(pid);
	/*if(prptr->nonvirtual==0)
	{
		if(prptr->vmemlist.mnext==NULL)
			kprintf("\n I am initializing the next here");
		prptr->vmemlist.mnext=(struct memblk*)(4096*4096);
		prptr->vmemlist.mnext->mlength=prptr->vmemlist.mlength;
		kprintf("\n the mnext is = %lu", (unsigned long)(prptr->vmemlist.mnext));
	}*/
	restore(mask);
	return prio;
}
