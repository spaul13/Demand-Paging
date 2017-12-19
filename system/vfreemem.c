/* vfreemem.c - vfreemem */

#include <xinu.h>


syscall	vfreemem(
	  char		*blkaddr,	
	  uint32	nbytes		// Size of block in bytes	Pointer to memory blkaddr	
	)
{
				/* Saved interrupt mask		*/
		
	struct procent * pptr;
	struct virtualblock *first_ptr,  *block;// *q,*p, 
	//unsigned top;
	
	
	pptr= &proctab[currpid];
	
	if (nbytes==0)// || (unsigned)blkaddr>((unsigned)((pptr->vpnpages)*4096)+(4096*4096)) || ((unsigned)blkaddr)<((unsigned) NBPG*4096) )
	{	
	//	kprintf("\n I am returning from the first SYSERR");
		return(SYSERR);
	}
	
	nbytes = (unsigned)roundmb(nbytes);
	
	first_ptr = pptr->vmemlist;
	
	//q=curr//
	//p=next
	
	block=(struct virtualblock *)blkaddr;	
	
	//kprintf("\n In vfreemem block= %d, first_ptr->mnext->vaddr=%d, nbytes %d", block, first_ptr->mnext->vaddr, nbytes);
	block->vlen = nbytes; 
	//p=block->mnext;
	block->vaddr = (unsigned)block;
	
	
//	kprintf("\n block + len %d and initial length %d",block->vaddr + block->vlen, (pptr->vmemlist)->mnext->vlen);
	
	if( (block->vaddr + block->vlen) < first_ptr->mnext->vaddr)
	{
		//kprintf("\n here in if %d", block);
		block->mnext = first_ptr->mnext;
		first_ptr->mnext = block;
	}
	
	else if (((block->vaddr + block->vlen) > first_ptr->mnext->vaddr) && (block->vaddr <=first_ptr->mnext->vaddr ))
	{
	//	kprintf("\n else here ");
		first_ptr->mnext->vaddr = block->vaddr;
		first_ptr->mnext->vlen+= (first_ptr->mnext->vaddr  - block->vaddr)*4;
		
	}
	else if((block->vaddr + block->vlen) == first_ptr->mnext->vaddr)
	{
		//kprintf("\n else if here");
		first_ptr->mnext->vaddr = block->vaddr;
		first_ptr->mnext->vlen+= nbytes;
	}
	else
	{
		
		return(SYSERR);
	}
  	    
	
	return OK;
}









































/*struct	memblk	*p, *q, *block;
	unsigned top;

	if (nbytes==0)
		return(SYSERR);
	block = (struct memblk*)(blkaddr);
	nbytes = (unsigned)roundmb(nbytes);

	for( p=proctab[currpid].vmemlist.mnext,q= &proctab[currpid].vmemlist;
	     p != (struct memblk *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	if (((top=q->mlength+(unsigned)q)>(unsigned)block && q!= &proctab[currpid].vmemlist) ||
	    (p!=NULL && (nbytes+(unsigned)block) > (unsigned)p )) {
		return(SYSERR);
	}
	if ( q!= &proctab[currpid].vmemlist && top == (unsigned)block )
			q->mlength += nbytes;
	else {
		block->mlength = nbytes;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlength + (unsigned)q ) == (unsigned)p) {
		q->mlength += p->mlength;
		q->mnext = p->mnext;
	}
	//kprintf("To be implemented!\n");
	return(OK);
}*/









































/*	struct procent * pptr;
	struct virtualblock *first_ptr,  *block;// *q,*p, 
	//unsigned top;
	
	
	pptr= &proctab[currpid];
	
	if (nbytes==0 || (unsigned)blkaddr>((unsigned)((pptr->vpnpages)*4096)+(4096*4096)) || ((unsigned)blkaddr)<((unsigned) NBPG*4096) )
	{	
		kprintf("\n I am returning from the first SYSERR");
		return(SYSERR);
	}
	
	nbytes = (unsigned)roundmb(nbytes);
	
	first_ptr = pptr->vmemlist;
	
	//q=curr//
	//p=next
	
	block=(struct virtualblock *)blkaddr;	
	
	kprintf("\n In vfreemem block= %d, first_ptr->mnext->vaddr=%d, nbytes %d", block, first_ptr->mnext->vaddr, nbytes);
	block->vlen = nbytes; 
	//p=block->mnext;
	block->vaddr = (unsigned)block;
	
	
	kprintf("\n block + len %d and initial length %d",block->vaddr + block->vlen, (pptr->vmemlist)->mnext->vlen);
	
	if( (block->vaddr + block->vlen) < first_ptr->mnext->vaddr)
	{
		kprintf("\n here in if %d", block);
		block->mnext = first_ptr->mnext;
		first_ptr->mnext = block;
	}
	
	else if (((block->vaddr + block->vlen) > first_ptr->mnext->vaddr) && (block->vaddr <=first_ptr->mnext->vaddr ))
	{
		kprintf("\n else here ");
		first_ptr->mnext->vaddr = block->vaddr;
		first_ptr->mnext->vlen+= (first_ptr->mnext->vaddr  - block->vaddr)*4;
		
	}
	else if((block->vaddr + block->vlen) == first_ptr->mnext->vaddr)
	{
		kprintf("\n else if here");
		first_ptr->mnext->vaddr = block->vaddr;
		first_ptr->mnext->vlen+= nbytes;
	}
	else
	{
		
		return(SYSERR);
	}
  	    
	
	return OK;
}*/
	




















/*for( p=proctab[currpid].vmemlist->mnext,q= &proctab[currpid].vmemlist;
	     p != (struct virtualblock *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	if (((top=q->vlen+(unsigned)q)>(unsigned)block && q!= &proctab[currpid].vmemlist) ||
	    (p!=NULL && (nbytes+(unsigned)block) > (unsigned)p )) {
		return(SYSERR);
	}
	if ( q!= &memlist && top == (unsigned)block )
			q->vlen += nbytes;
	else {
		block->vlen = nbytes;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->vlen + (unsigned)q ) == (unsigned)p) {
		q->vlen += p->vlen;
		q->mnext = p->mnext;
	}


return OK;
}*/







/*struct procent * pptr;
	struct virtualblock *first_ptr,  *block;// *q,*p, 
	//unsigned top;
	
	pptr= &proctab[currpid];
	
	if (nbytes==0 || (unsigned)blkaddr>((unsigned)((pptr->vpnpages)*4096)+(4096*4096)) || ((unsigned)blkaddr)<((unsigned) NBPG*4096) )
	{
	return(SYSERR);
	}
	
	nbytes = (unsigned)roundmb(nbytes);
	
	first_ptr = pptr->vmemlist;
	
	//q=curr//
	//p=next
	
	block=(struct virtualblock *)blkaddr;	
	
	kprintf("\n here here %d, %d, nbytes %d", block, first_ptr->mnext->vaddr, nbytes);
	block->vlen = nbytes; 
	//p=block->mnext;
	block->vaddr = (unsigned)block;
	
	
		//kprintf("\n block + len %d and initia length %d",block->vaddr + block->vlen, (pptr->vmemlist)->mnext->vlen);
	
	if( (block->vaddr + block->vlen) < first_ptr->mnext->vaddr)
	{
		kprintf("\n here %d", block);
		block->mnext = first_ptr->mnext;
		first_ptr->mnext = block;
	}
	
	else if (((block->vaddr + block->vlen) > first_ptr->mnext->vaddr) && (block->vaddr <=first_ptr->mnext->vaddr ))
	{
		//kprintf("\n else here ");
		first_ptr->mnext->vaddr = block->vaddr;
		first_ptr->mnext->vlen+= (first_ptr->mnext->vaddr  - block->vaddr)*4;
		
	}
	else if((block->vaddr + block->vlen) == first_ptr->mnext->vaddr)
	{
		first_ptr->mnext->vaddr = block->vaddr;
		first_ptr->mnext->vlen+= nbytes;
	}
	else
	{
		return(SYSERR);
	}*/
	
	
	
