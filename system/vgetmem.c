/* vgetmem.c - vgetmem */

#include <xinu.h>

char  	*vgetmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{	
	struct	virtualblock	*p, *q, *leftover;
	struct procent *pptr;
	pptr= &proctab[currpid];
	head->mnext = (struct virtualblock*)NULL;
	pptr->vmemlist->mnext = (struct virtualblock*)head;
	kprintf("\nInside VGETMEM(): for process %d", currpid);
	kprintf("\nInside VGETMEM(): initial bytes aksed %d and pptr->vmemlist is %d and (pptr->vmemlist)->mnext is %d for process %d, addr %d", nbytes, pptr->vmemlist, (pptr->vmemlist)->mnext, currpid, (((pptr->vmemlist)->mnext))->mnext->vaddr);
	
	kprintf("\n Inside VGETMEM(): pptr->vmemlist is %d and vmemlist->mnext is %d, addr %d, len %d", pptr->vmemlist, (pptr->vmemlist)->mnext,(struct virtualblock *)((pptr->vmemlist)->mnext)->vaddr, (pptr->vmemlist)->mnext->vlen);
	
	kprintf("\n Inside VGETMEM(): ADDRESS %d, len %d",     ((pptr->vmemlist)->mnext)->vaddr     , (pptr->vmemlist)->mnext->vlen);
	kprintf("\n Inside VGETMEM(): vmemlist %d, vmemlist->mnext %d, store %d", pptr->vmemlist, pptr->vmemlist->mnext, pptr->b_store);
//}
	
	p = pptr->vmemlist->mnext;
	
	//kprintf("\nInside VGETMEM(): p->vaddr %d, p->vlen %d, *p->vaddr %d", p->vaddr, p->vlen, (&(unsigned int)(p->vaddr)));
	if (nbytes==0 )//|| (pptr->vmemlist)->mnext == (struct mblock *) NULL) 
	{
		//kprintf("\nInside VGETMEM(): Error!");
		return( (char *)SYSERR);
	}
	
	nbytes = (unsigned int) roundmb(nbytes);
	//kprintf("\nInside VGETMEM(): bytes asked : %d", nbytes);
	
	
	//q = pptr->vmemlist;
	//p = q->mnext;
	
	for ( (q = (pptr->vmemlist), p = q->mnext); (p != (struct virtualblock *) NULL); (q=p,p=p->mnext) )
	{
	//	kprintf("\n inside VGETMEM() : inside for %d", p->vlen);
		if ( p->vlen == nbytes) 
		{
	//		kprintf("\nInside VGETMEM(): got the block when requested is equal to block %d", p->vlen);
			q->mnext = p->mnext;
			return( (char *)(p->vaddr) );
		} 
		
		else if ( p->vlen > nbytes ) 
		{
			leftover = (struct virtualblock*)getmem(sizeof(struct virtualblock*));//(struct mblock *)( (unsigned)p + nbytes );
			leftover->vaddr = (p->vaddr + nbytes);
			leftover->vlen = (p->vlen - nbytes);
			leftover->mnext = p->mnext;
			q->mnext = leftover;
			
			//leftover->mlen = p->mlen - nbytes;
		//	kprintf("\nInside VGETMEM(): got the block greater %d, leftover len %d, vmemlist %d, vmemlist->mnext %d, leftover %d, leftover->vaddr %d", p->vlen, leftover->vlen, pptr->vmemlist, pptr->vmemlist->mnext, leftover, leftover->vaddr);
			return( (char *)(p->vaddr) );
		}
		
		
	}
		
	return( (char *)SYSERR );
	}


















































/*struct	virtualblock	*p, *q, *leftover;
	struct procent *pptr;
	pptr= &proctab[currpid];
	head->mnext = (struct virtualblock*)NULL;
	pptr->vmemlist->mnext = (struct virtualblock*)head;
	kprintf("\nInside VGETMEM(): for process %d", currpid);
	kprintf("\nInside VGETMEM(): initial bytes aksed %d and pptr->vmemlist is %d and (pptr->vmemlist)->mnext is %d for process %d, addr %d", nbytes, pptr->vmemlist, (pptr->vmemlist)->mnext, currpid, (((pptr->vmemlist)->mnext))->mnext->vaddr);
	
	kprintf("\n Inside VGETMEM(): pptr->vmemlist is %d and vmemlist->mnext is %d, addr %d, len %d", pptr->vmemlist, (pptr->vmemlist)->mnext,(struct virtualblock *)((pptr->vmemlist)->mnext)->vaddr, (pptr->vmemlist)->mnext->vlen);
	
	kprintf("\n Inside VGETMEM(): ADDRESS %d, len %d",     ((pptr->vmemlist)->mnext)->vaddr     , (pptr->vmemlist)->mnext->vlen);
	kprintf("\n Inside VGETMEM(): vmemlist %d, vmemlist->mnext %d, store %d", pptr->vmemlist, pptr->vmemlist->mnext, pptr->b_store);
//}
	
	p = pptr->vmemlist->mnext;
	
	//kprintf("\nInside VGETMEM(): p->vaddr %d, p->vlen %d, *p->vaddr %d", p->vaddr, p->vlen, (&(unsigned int)(p->vaddr)));
	if (nbytes==0 )//|| (pptr->vmemlist)->mnext == (struct mblock *) NULL) 
	{
		kprintf("\nInside VGETMEM(): Error!");
		return( (char *)SYSERR);
	}
	
	nbytes = (unsigned int) roundmb(nbytes);
	kprintf("\nInside VGETMEM(): bytes asked : %d", nbytes);
	
	
	//q = pptr->vmemlist;
	//p = q->mnext;
	
	for ( (q = (pptr->vmemlist), p = q->mnext); (p != (struct virtualblock *) NULL); (q=p,p=p->mnext) )
	{
		kprintf("\n inside VGETMEM() : inside for %d", p->vlen);
		if ( p->vlen == nbytes) 
		{
			kprintf("\nInside VGETMEM(): got the block when requested is equal to block %d", p->vlen);
			q->mnext = p->mnext;
			return( (char *)(p->vaddr) );
		} 
		
		else if ( p->vlen > nbytes ) 
		{
			leftover = (struct virtualblock*)getmem(sizeof(struct virtualblock*));//(struct mblock *)( (unsigned)p + nbytes );
			leftover->vaddr = (p->vaddr + nbytes);
			leftover->vlen = (p->vlen - nbytes);
			leftover->mnext = p->mnext;
			q->mnext = leftover;
			
			//leftover->mlen = p->mlen - nbytes;
			kprintf("\nInside VGETMEM(): got the block greater %d, leftover len %d, vmemlist %d, vmemlist->mnext %d, leftover %d, leftover->vaddr %d", p->vlen, leftover->vlen, pptr->vmemlist, pptr->vmemlist->mnext, leftover, leftover->vaddr);
			return( (char *)(p->vaddr) );
		}
		
		
	}
		
	return( (char *)SYSERR );
	}*/































/*for (q= &proctab[currpid].vmemlist,p=proctab[currpid].vmemlist.mnext ;
	     p != (struct memblk *) NULL ;
	     q=p,p=p->mnext)
		if ( p->mlength == nbytes) {
			q->mnext = p->mnext;
			return( (char *)p );
		} else if ( p->mlength > nbytes ) {
			leftover = (struct memblk *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlength = p->mlength - nbytes;
			return( (char *)p );
		}
	return( (char *)SYSERR );

}







*/

















/*struct	virtualblock	*p, *q, *leftover;
	struct procent *pptr;
	pptr= &proctab[currpid];
	head->mnext = (struct virtualblock*)NULL;
	pptr->vmemlist->mnext = (struct virtualblock*)head;
	kprintf("\nInside VGETMEM(): for process %d", currpid);
	kprintf("\nInside VGETMEM(): initial bytes aksed %d and pptr->vmemlist is %d and (pptr->vmemlist)->mnext is %d for process %d, addr %d", nbytes, pptr->vmemlist, (pptr->vmemlist)->mnext, currpid, (((pptr->vmemlist)->mnext))->mnext->vaddr);
	
	kprintf("\n Inside VGETMEM(): pptr->vmemlist is %d and vmemlist->mnext is %d, addr %d, len %d", pptr->vmemlist, (pptr->vmemlist)->mnext,(struct virtualblock *)((pptr->vmemlist)->mnext)->vaddr, (pptr->vmemlist)->mnext->vlen);
	
	kprintf("\n Inside VGETMEM(): ADDRESS %d, len %d",     ((pptr->vmemlist)->mnext)->vaddr     , (pptr->vmemlist)->mnext->vlen);
	kprintf("\n Inside VGETMEM(): vmemlist %d, vmemlist->mnext %d, store %d", pptr->vmemlist, pptr->vmemlist->mnext, pptr->b_store);
	
	p = pptr->vmemlist->mnext;
	
	//kprintf("\nInside VGETMEM(): p->vaddr %d, p->vlen %d, *p->vaddr %d", p->vaddr, p->vlen, (&(unsigned int)(p->vaddr)));
	if (nbytes==0 )//|| (pptr->vmemlist)->mnext == (struct mblock *) NULL) 
	{
		kprintf("\nInside VGETMEM(): Error!");
		return( (char *)SYSERR);
	}
	
	nbytes = (unsigned int) roundmb(nbytes);
	kprintf("\nInside VGETMEM(): bytes asked : %d", nbytes);
	
	
	//q = pptr->vmemlist;
	//p = q->mnext;
	
	for ( (q = (pptr->vmemlist), p = q->mnext); (p != (struct virtualblock *) NULL); (q=p,p=p->mnext) )
	{
		kprintf("\n inside VGETMEM() : inside for %d", p->vlen);
		if ( p->vlen == nbytes) 
		{
			kprintf("\nInside VGETMEM(): got the block when requested is equal to block %d", p->vlen);
			q->mnext = p->mnext;
			return( (char *)(p->vaddr) );
		} 
		
		else if ( p->vlen > nbytes ) 
		{
			leftover = (struct virtualblock*)getmem(sizeof(struct virtualblock*));//(struct mblock *)( (unsigned)p + nbytes );
			leftover->vaddr = (p->vaddr + nbytes);
			leftover->vlen = (p->vlen - nbytes);
			leftover->mnext = p->mnext;
			q->mnext = leftover;
			
			//leftover->mlen = p->mlen - nbytes;
			kprintf("\nInside VGETMEM(): got the block greater %d, leftover len %d, vmemlist %d, vmemlist->mnext %d, leftover %d, leftover->vaddr %d", p->vlen, leftover->vlen, pptr->vmemlist, pptr->vmemlist->mnext, leftover, leftover->vaddr);
			return( (char *)(p->vaddr) );
		}
		
		
	}
		
	return( (char *)SYSERR );
}*/































/*	struct virtualblock *p; //leftover;
	struct procent *pptr;	
	kprintf("\n ---------- I am inside the vgetmem() ----------------------------- \n");
	pptr= &proctab[currpid];
    //kprintf("\nInside VGETMEM(): for process %d", currpid);
    kprintf("\nInside VGETMEM(): initial bytes aksed %d and pptr->vmemlist is %d and (pptr->vmemlist)->mnext is %d for process %d, addr %d", nbytes, pptr->vmemlist, (pptr->vmemlist)->mnext, currpid, (((pptr->vmemlist)->mnext))->mnext->vaddr);
       
    p = pptr->vmemlist;
   
//    kprintf("\nInside VGETMEM(): p->vaddr %d, p->vlen %d, *p->vaddr %d", p->vaddr, p->vlen, (&(unsigned int)(p->vaddr)));
    if ((nbytes==0 )|| ((pptr->vmemlist)->mnext == (struct virtualblock *) NULL) )
    {
        kprintf("\nInside VGETMEM(): Error!");
        return( (char *)SYSERR);
    }
   
    nbytes = (unsigned int) roundmb(nbytes);
    //kprintf("\nInside VGETMEM(): bytes asked : %d", nbytes);
   
   
    //q = pptr->vmemlist;
    //p = q->mnext;
   
    
	while(p!=(struct virtualblock*)NULL)//for ( (q = (pptr->vmemlist), p = q->mnext); (p != (struct virtualblock *) NULL); (q=p,p=p->mnext) )
        {
        	kprintf("\n inside VGETMEM() : inside for %d", p->vlen);
        	if ( p->vlen == nbytes)
        {
            kprintf("\nInside VGETMEM(): got the block when requested is equal to block %d", p->vlen);
	    p=p->mnext;
            //q->mnext = p->mnext;
            return( (char *)(p->vaddr) );
        }
       
        else if ( p->vlen > nbytes )
        {
        	p->mnext=  (struct virtualblock*)( (unsigned)p + nbytes ); 
		p->vlen=p->vlen-nbytes;
		leftover = (struct virtualblock*)getmem(sizeof(struct virtualblock*) (unsigned)p + nbytes );
            leftover->vaddr = (p->vaddr + nbytes);
            leftover->vlen = (p->vlen - nbytes);
            leftover->mnext = p->mnext;
            p->mnext = leftover;
            return( (char *)(p->vaddr) );
        }
       	else{
		p=p->mnext;}
       
    }
   
       
    return( (char *)SYSERR );
}*/
	
  	
	
	


/*intmask	mask;				mask = disable();
	if(proctab[currpid].vmemlist.mnext==NULL)
	{
		kprintf("\n I am initializing it on vgetmem()");
		proctab[currpid].vmemlist.mnext=(struct memblk*)(4096*4096)//+proctab[currpid].vmemlist.mlength);
	}
	struct	memblk	*prev, *curr, *leftover;
	struct procent *pptr;
	pptr=&proctab[currpid];
	kprintf("\n I am entering into vgetmem with pid = %d and nbytes = %d", currpid, nbytes);
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	
	prev = &(pptr->vmemlist);
	curr = pptr->vmemlist.mnext;
	kprintf("\n the prev = %p and the curr is =%p and ", prev, curr);
	while (curr != NULL) {		
		if (curr->mlength == nbytes) 			{	
			prev->mnext = curr->mnext;
			pptr->vmemlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { 
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			pptr->vmemlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);
		} else {		
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
*/






  /* pptr= &proctab[currpid];
    //kprintf("\nInside VGETMEM(): for process %d", currpid);
    kprintf("\nInside VGETMEM(): initial bytes aksed %d and pptr->vmemlist is %d and (pptr->vmemlist)->mnext is %d for process %d, addr %d", nbytes, pptr->vmemlist, (pptr->vmemlist)->mnext, currpid, (((pptr->vmemlist)->mnext))->mnext->vaddr);
       
    p = pptr->vmemlist;
   
//    kprintf("\nInside VGETMEM(): p->vaddr %d, p->vlen %d, *p->vaddr %d", p->vaddr, p->vlen, (&(unsigned int)(p->vaddr)));
    if ((nbytes==0 )|| ((pptr->vmemlist)->mnext == (struct virtualblock *) NULL) )
    {
        kprintf("\nInside VGETMEM(): Error!");
        return( (char *)SYSERR);
    }
   
    nbytes = (unsigned int) roundmb(nbytes);
    //kprintf("\nInside VGETMEM(): bytes asked : %d", nbytes);
   
   
    //q = pptr->vmemlist;
    //p = q->mnext;
   
    for ( (q = (pptr->vmemlist), p = q->mnext); (p != (struct virtualblock *) NULL); (q=p,p=p->mnext) )
    {
        kprintf("\n inside VGETMEM() : inside for %d", p->vlen);
        if ( p->vlen == nbytes)
        {
            kprintf("\nInside VGETMEM(): got the block when requested is equal to block %d", p->vlen);
            q->mnext = p->mnext;
            return( (char *)(p->vaddr) );
        }
       
        else if ( p->vlen > nbytes )
        {
            leftover = (struct virtualblock*)getmem(sizeof(struct virtualblock*));//(struct mblock *)( (unsigned)p + nbytes );
            leftover->vaddr = (p->vaddr + nbytes);
            leftover->vlen = (p->vlen - nbytes);
            leftover->mnext = p->mnext;
            q->mnext = leftover;
            return( (char *)(p->vaddr) );
        }
       
       
    }
   
       
    return( (char *)SYSERR );
}*/



/*intmask	mask;			
	struct	virtualblock	*prev, *curr, *leftover;
	struct procent *pptr;
	pptr= &proctab[currpid];
	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	

	prev = pptr->vmemlist;
	curr = pptr->vmemlist->mnext;
	while (curr != NULL) {			

		if (curr->vlen == nbytes) {	
			prev->mnext = curr->mnext;
			pptr->vmemlist->vlen -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->vlen > nbytes) { 
			leftover = (struct virtualblock *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->vlen = curr->vlen - nbytes;
			pptr->vmemlist->vlen -= nbytes;
			restore(mask);
			return (char *)(curr);
		} else {			
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
}*/

