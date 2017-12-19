/* vcreate.c - vcreate */

#include <xinu.h>

local	int newpid();
struct virtualblock * head;

#define	roundew(x)	( (x+3)& ~0x3)

/*----------------------------------------------------------------------------
 *  vcreate  -  Creates a new XINU process. The process's heap will be private
 *  and reside in virtual memory.
 *----------------------------------------------------------------------------
 */
pid32	vcreate(
	  void		*funcaddr,	/* Address of the function	*/
	  uint32	ssize,		/* Stack size in words		*/
		uint32	hsize,		/* Heap size in num of pages */
	  pri16		priority,	/* Process priority > 0		*/
	  char		*name,		/* Name (for debugging)		*/
	  uint32	nargs,		/* Number of args that follow	*/
	  ...
	)
{
	uint32		savsp, *pushsp;
	//intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/

	//mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundew(ssize);
	if (((saddr = (uint32 *)getstk(ssize)) ==
	    (uint32 *)SYSERR ) ||
	    (pid=newpid()) == SYSERR || priority < 1 ) {
		//restore(mask);
		return SYSERR;
	}
//	sid32 vcsem=semcreate(1);
//	wait(vcsem);
  // Lab3 TODO. set up page directory, allocate bs etc.
	//if(proctab[pid].nonvirtual!=1){
//	kprintf("\n I am inside vcreate.");
	prptr = &proctab[pid];
	prptr->nonvirtual=0; //for all virtual processes
	pd_t* page_dir = create_page_dir(pid);
	if( page_dir == NULL)
	{
		return(SYSERR);
	}
	//kprintf("\nInside CREATE(): New Page Directory Allocated %d", (unsigned long)page_dir/NBPG);
	prptr->pdbr = (unsigned long)page_dir/NBPG;
	//kprintf("\n the value of the pdbr is= %32b and 0x%08x \n", prptr->pdbr,prptr->pdbr);
	int check = 0;
	bsd_t bs;
	if(hsize<=MAX_PAGES_PER_BS )
	{
		bs=allocate_bs(hsize);
		kprintf("\n backing store %d is allocated", bs);
		if ( bs == SYSERR ) panic("no backing store found for the process \n");
		bsm_map(pid,4096,bs,hsize);
		check=1;
	}
	int p = 0;
	int heapsize=hsize;
	if (check != 1)
	{
	while(heapsize > 0)
	{
		if(heapsize>MAX_PAGES_PER_BS)
		{
		bs=allocate_bs(MAX_PAGES_PER_BS);
		kprintf("\n now heapsize >200 backing store %d is allocated", bs);
		if ( bs == SYSERR ) panic("no backing store found for the process \n");
		//uint32 vp =(4096 + bs*MAX_PAGES_PER_BS);
		bsm_map(pid,4096 + p*MAX_PAGES_PER_BS, bs, MAX_PAGES_PER_BS);
		}
		else{
			bs=allocate_bs(heapsize);
			kprintf("\n backing store %d is allocated", bs);
			if ( bs == SYSERR ) panic("no backing store found for the process \n");
			bsm_map(pid,4096 + p*MAX_PAGES_PER_BS,bs,heapsize);
		}

	heapsize = heapsize - MAX_PAGES_PER_BS;	
	//kprintf("\n heapsize is = %d", heapsize);
	p++;
	}
	}
	//}
	//struct memblk *mptr;
	if(prptr->nonvirtual==0)
	{
	prptr->vmemlist = (struct virtualblock*)getmem(sizeof(struct virtualblock));
	head = (struct virtualblock*)getmem(sizeof(struct virtualblock));
//	kprintf("\n the head and pptr->memlist is=%u", (unsigned long)head);
	
	if(head == (struct virtualblock*)NULL)
	{
		return(SYSERR);
	}
	if(prptr->vmemlist == (struct virtualblock*)NULL)
	{
		return(SYSERR);
	}
	
	//pptr->store = 4096*4096;
	head->vaddr = 4096*4096;//NBPG);
	head->vlen = hsize*4096;
	/*head->mnext = (struct virtualblock*)NULL;
	prptr->vmemlist->mnext = (struct virtualblock*)head;*/
	
	
	/*head->mnext = (struct virtualblock*)NULL;
	prptr->vmemlist->mnext = (struct virtualblock*)head;*/
	//kprintf("\n the current pid is = %d", pid);
	/*mptr = (struct memblk*)((2048*4096) + ((200*4096)*bs));
   	// kprintf("\n Inside VCREATE(): Starting address of BS %d is %d", bs_no, mptr);
    	mptr->mlength = 4096*hsize;
   // kprintf("\n Inside VCREATE(): Heap size of BS %d is %d", bs_no, mptr->mlen);
	mptr->mnext = 0;*/
	}

//	proctab[pid].vmemlist.mnext=(struct memblk *)(4096*4096);
//	proctab[pid].vmemlist.mnext->mlength=hsize*NBPG;
	/*if(prptr->nonvirtual == 0)
	{
		proctab[pid].vmemlist.mlength=4096*hsize;
		//proctab[pid].vmemlist.mnext = (struct memblk*)(4096*4096);
	}*/
	//proctab[pid].vmemlist->mnext->vlen=4096*hsize;

	//proctab[pid].vmemlist->mnext=(struct virtualblock *)(0x01000000);
	//proctab[pid].vmemlist->mnext=(4096+(MAX_PAGES_PER_BS*bs))*4096 //as private heap starting from 4096 and NBPG 4096
	prcount++;
	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = priority;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;
	/* Initialize stack as if the process was called		*/

	*saddr = STACKMAGIC;
	savsp = (uint32)saddr;

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
		*--saddr = *a--;	/*   onto created process' stack*/
	*--saddr = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registerss, and an old SP	*/

	*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	*--saddr = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	*--saddr = 0;			/* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
	//signal(vcsem);

	
	return pid;
}

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}
