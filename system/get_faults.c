/* get_faults.c - get_faults */
#include <xinu.h>

/*---------------------------------------------------------------------------
 *  get_faults  -  Return the number of times the page fault handler has been
 *  called.
 *---------------------------------------------------------------------------
 */
uint32 get_faults() {
	/* LAB3 TODO */
  if(page_faulting==1)
  	count+=1;
  return count;	
}
