/* initialize.c - nulluser, sysinit, sizmem */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>
#include <lab3.h>
#include <paging.h>
extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
extern	void xdone(void);	/* System "shutdown" procedure		*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/

/* Lab3. initializes data structures and necessary set ups for paging */
static	void initialize_paging();

//reading the control registers
unsigned long read_cr0(void);
unsigned long read_cr2(void);
unsigned long read_cr3(void) ;
unsigned long read_cr4(void);

int sum,ptime;
syscall init_frm();
syscall init_bsm();
syscall init_frame_fifo();	

//writing the control registers
void write_cr0(unsigned long n);
void write_cr2(unsigned long n);
void write_cr3(unsigned long n);
void write_cr4(unsigned long n);

struct fr_map frmtab[NFRAMES];
struct bs_map bsmtab[MAX_ID +1];
struct check recheck[NFRAMES];
void dump32(unsigned long n);
uint32 returning_frame;

//pd_t* pagedir_null;
uint32 fifo_head;
uint32 count;
int reclaim;

/* Declarations of major kernel variables */

struct	procent	proctab[NPROC];	/* Process table			*/
struct	sentry	semtab[NSEM];	/* Semaphore table			*/
struct	memblk	memlist;	/* List of free memory blocks		*/

/* Lab3. frames metadata handling */
frame_md_t frame_md;


/* Active system status */

int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/
unsigned long tmp;
pt_t* global_page_table[4];
int replacement;
		
bool8   PAGE_SERVER_STATUS;    /* Indicate the status of the page server */
sid32   bs_init_sem;
int page_faulting;

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{
	struct	memblk	*memptr;	/* Ptr to memory block		*/
	uint32	free_mem;		/* Total amount of free memory	*/

	/* Initialize the system */

	sysinit();

  // Lab3
	initialize_paging();

	kprintf("\n\r%s\n\n\r", VERSION);

	/* Output Xinu memory layout */
	free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=NULL;memptr = memptr->mnext) {
	    kprintf("           [0x%08X to 0x%08X]\r\n",
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Create the RDS process */

	rdstab[0].rd_comproc = create(rdsprocess, RD_STACK, RD_PRIO,
					"rdsproc", 1, &rdstab[0]);
	if(rdstab[0].rd_comproc == SYSERR) {
		panic("Cannot create remote disk process");
	}
	resume(rdstab[0].rd_comproc);

	/* Enable interrupts */

	enable();

	/* Create a process to execute function main() */

	resume (
	   create((void *)main, INITSTK, INITPRIO, "Main process", 0,
           NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {
		;		/* Do nothing */
	}

}

/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	int32	i;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Ptr to semaphore table entry	*/

	/* Platform Specific Initialization */

	platinit();

	/* Initialize the interrupt vectors */

	initevec();

	/* Initialize free memory list */

	meminit();

	/* Initialize system variables */

	/* Count the Null process as the first process in the system */

	prcount = 1;

	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
	}

	/* Initialize the Null process entry */

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->prprio = 0;
	strncpy(prptr->prname, "prnull", 7);
	prptr->prstkbase = getstk(NULLSTK);
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	currpid = NULLPROC;

	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readylist = newqueue();

	/* Initialize the real time clock */

	clkinit();

#ifndef QEMU
	for (i = 0; i < NDEVS; i++) {
		init(i);
	}
#else
    init(QEMUCONSOLE);
    init(QEMURDISK);
#endif

	PAGE_SERVER_STATUS = PAGE_SERVER_INACTIVE;
	bs_init_sem = semcreate(1);

	return;
}



static void initialize_paging()
{
	/* LAB3 TODO */
	kprintf("\n the currpid is = %d", currpid);
	reclaim=7;
	returning_frame = 0;
	page_faulting=0;
	sum=0;
	ptime=0;
	pfsem = semcreate(1);
	init_check();
	replacement=0;
	init_bsm();
	init_frm();
	init_frame_fifo();
	count=0;		
	int error=init_global_pt(); //global_pt will call the create page table on fly for the null process
	if(error==SYSERR){
		kprintf("\n its giving SYSERR while assigning the global page table");}
	pd_t* page_directory=create_page_dir(currpid); // create page directory for the null process and also setting the CR3 value for null
	if(page_directory==NULL)
	{
		kprintf("SYSERR");
	}

	struct procent *ptptr;
	ptptr=&proctab[currpid];
	//ptptr->pdbr=readcr3 >> 12;
	kprintf("\n the pdbr value obtained from the function =%d %d", (unsigned int)page_directory, ((unsigned int)page_directory)/NBPG);
	unsigned long pdbr = ((unsigned long)page_directory)/NBPG; //(unsigned int)page_directory/NBPG;1029
	ptptr->pdbr=pdbr;
	unsigned long temp;
	temp = pdbr<<12;
	kprintf("\n taking 1024 from the initialize paging function, PDBR value : %32b, %d , temp %32b	%d", pdbr, pdbr, temp, temp);
	write_cr3(temp);

	set_evec(14, (unsigned long)page_fault_isr);
	//enable paging
	temp =  read_cr0();
	kprintf("\n calling temp from initialize paging\n \n ");
	dump32(temp); 
	kprintf("\n the value of temp after reading =%32b & %d", temp, temp);
  	//temp = temp | ( 0x1 << 31 );
	temp =temp | 0x80000001;
	kprintf("\n calling temp from initialize paging after ORing it \n \n ");
	dump32(temp); 
	kprintf("\n the value of temp after reading =%32b & %d temp1", temp, temp);
	write_cr0(temp);
	kprintf("\n paging is enabled \n");
	/*asm("pushl %eax");
  	asm("movl %cr0, %eax");
  	asm("movl %eax, tmp");
  	asm("popl %eax");
	temp = tmp;
	temp = temp | 0x80000001;
	tmp = temp;
	//write_cr0(temp); writing the cr0 value with the temp 
  	asm("pushl %eax");
  	asm("movl tmp, %eax");		
  	asm("movl %eax, %cr0");
	asm("popl %eax"); */    
	return;
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}

//creating on-fly page table

pt_t* create_a_page_table(pid32 pid, int frame_no, int type)
{
	// int get_frame_num;
	pt_t* page_table;
	
	int num_entry_pt;
	
	//gives base address of page table
	page_table = (pt_t*) ((FRAME0+frame_no)*NBPG);
	hook_ptable_create(FRAME0+frame_no);
	//kprintf("Creating ptable at frame %d, address %lu \n", 
	//	(FRAME0+frame_no), (unsigned long) page_table); 	
	frmtab[frame_no].fr_pid=pid;
	frmtab[frame_no].fr_type=type;
	frmtab[frame_no].fr_status=FRM_MAPPED;
	for( num_entry_pt=0; num_entry_pt<1024 ; num_entry_pt++)
	{
		page_table[num_entry_pt].pt_pres = 0;					   	
		page_table[num_entry_pt].pt_write = 0;		
		page_table[num_entry_pt].pt_user = 0;
       	     	page_table[num_entry_pt].pt_pwt= 0;		
	    	page_table[num_entry_pt].pt_pcd= 0;	
	    	page_table[num_entry_pt].pt_acc= 0;		
	    	page_table[num_entry_pt].pt_dirty = 0;		
	    	page_table[num_entry_pt].pt_mbz= 0;	
	    	page_table[num_entry_pt].pt_global =0;	
	    	page_table[num_entry_pt].pt_avail = 0;		
	    	page_table[num_entry_pt].pt_base = 0; 	
	}
	
	return page_table;
}

//creating a global page table

syscall init_global_pt()
{
	int num_global_pt, num_entry_pt;
	pt_t* page_table;
	
		
	for( num_global_pt=0; num_global_pt<4; num_global_pt++)
	{
		page_table = create_a_page_table(0, num_global_pt+1, FR_TBL); //null process initializes everything
		//kprintf("\n Gloabl PT %d and address is %lu and page no is %d \n", num_global_pt, (unsigned long)page_table, ((unsigned int)page_table/NBPG));
		if( page_table == NULL )
		{
			return(SYSERR);
		}
		
		
		for( num_entry_pt=0; num_entry_pt<1024; num_entry_pt++)
		{
			page_table[num_entry_pt].pt_pres = 1;					    
			page_table[num_entry_pt].pt_write = 1;		
	    		page_table[num_entry_pt].pt_user = 0;		
           		page_table[num_entry_pt].pt_pwt= 0;		
	    		page_table[num_entry_pt].pt_pcd= 0;	
	    		page_table[num_entry_pt].pt_acc= 0;		
	    		page_table[num_entry_pt].pt_dirty = 0;		
	    		page_table[num_entry_pt].pt_mbz= 0;	
	    		page_table[num_entry_pt].pt_global =0;	
	    		page_table[num_entry_pt].pt_avail = 0;		
	
			page_table[num_entry_pt].pt_base = (1024*num_global_pt)+num_entry_pt;
		}

		global_page_table[num_global_pt] = page_table;
		//kprintf("\n address of ptable is %lx and the global_page_table[%d] = %d",
		//	(unsigned long) page_table, num_global_pt, global_page_table[num_global_pt]);
	}

	page_table = create_a_page_table(0, 5, FR_TBL);
	for( num_entry_pt=0; num_entry_pt<1024; num_entry_pt++)
	{
		page_table[num_entry_pt].pt_pres = 1;					    
		page_table[num_entry_pt].pt_write = 1;		
		page_table[num_entry_pt].pt_user = 0;		
           	page_table[num_entry_pt].pt_pwt= 0;		
	    	page_table[num_entry_pt].pt_pcd= 0;	
	    	page_table[num_entry_pt].pt_acc= 0;		
	    	page_table[num_entry_pt].pt_dirty = 0;		
	    	page_table[num_entry_pt].pt_mbz= 0;	
	    	page_table[num_entry_pt].pt_global =0;	
	    	page_table[num_entry_pt].pt_avail = 0;		
		page_table[num_entry_pt].pt_base = (0x90000000+NBPG*num_entry_pt)>>12;
	}

	return OK;
}

//page directory setup or function
pd_t* create_page_dir(pid32 pid)
{
	pd_t* page_dir;
	//kprintf("\n the incoming pid is =%d and the name of this process is = %s", pid, proctab[pid].prname);
	//int get_frame_num; //for not having replacement policy as of now
	uint32 frame_no; 
	int num_entry_pd;
	//int loopbreaker=0;
	get_frm(&frame_no);
	/*for(int i=6;i<3072;i++)
		{
			if((frmtab[i].fr_status==FRM_UNMAPPED) && (loopbreaker==0))
			{
				frame_no=i;
				kprintf("\n Inside pf handler in pd for now I am getting a free frame at %d", i);
				loopbreaker=1 ;
			}
		}*/

	
	//set_frm_tab_upon_frame_allocation(pid, frame_no, FR_DIR);	//set frame table values
	if(pid==0){
		kprintf("Creating page directory for NULL process\n");		
		frame_no=0;
	}

	//gives base address of page table
	page_dir = (pd_t*)((FRAME0+frame_no)*NBPG);
	frmtab[frame_no].fr_pid=pid;
	frmtab[frame_no].fr_status=FRM_MAPPED;
	frmtab[frame_no].fr_type=FR_TBL;
	if(pid==0)
	{
		pagedir_null=(pd_t*)((FRAME0+frame_no)*NBPG);

	}
	
	//make entry into page directory for global page tables
	for( num_entry_pd=0; num_entry_pd<4; num_entry_pd++)
	{
		page_dir[num_entry_pd].pd_pres	= 1;		/* page table present?		*/
		page_dir[num_entry_pd].pd_write = 1;		/* page is writable?		*/
		page_dir[num_entry_pd].pd_user	= 0;		/* is use level protection?	*/
		page_dir[num_entry_pd].pd_pwt	= 0;		/* write through cachine for pt?*/
		page_dir[num_entry_pd].pd_pcd	= 0;		/* cache disable for this pt?	*/
		page_dir[num_entry_pd].pd_acc	= 0;		/* page table was accessed?	*/
		page_dir[num_entry_pd].pd_mbz	= 0;		/* must be zero			*/
		page_dir[num_entry_pd].pd_fmb	= 0;		/* four MB pages?		*/
		page_dir[num_entry_pd].pd_global= 0;		/* global (ignored)		*/
		page_dir[num_entry_pd].pd_avail = 0;		/* for programmer's use		*/
		page_dir[num_entry_pd].pd_base	= 1025+num_entry_pd;//((unsigned int)global_page_table[num_entry_pd])/NBPG;		/* location of page table?	*/
		//kprintf("\nGlobal pt %d has page DIRectory address %d and the global page table address= %d", num_entry_pd, page_dir[num_entry_pd].pd_base, (unsigned long)global_page_table[num_entry_pd]);
	}
		
	
	//for page tables that are not global page table and hence initialized
	//for( num_entry_pd=4; num_entry_pd<(NBPG/4); num_entry_pd++)
	num_entry_pd=576;
	{
		page_dir[num_entry_pd].pd_pres	= 1;		/* page table present?		*/
		page_dir[num_entry_pd].pd_write = 1;		/* page is writable?		*/
		page_dir[num_entry_pd].pd_user	= 0;		/* is use level protection?	*/
		page_dir[num_entry_pd].pd_pwt	= 0;		/* write through cachine for pt?*/
		page_dir[num_entry_pd].pd_pcd	= 0;		/* cache disable for this pt?	*/
		page_dir[num_entry_pd].pd_acc	= 0;		/* page table was accessed?	*/
		page_dir[num_entry_pd].pd_mbz	= 0;		/* must be zero			*/
		page_dir[num_entry_pd].pd_fmb	= 0;		/* four MB pages?		*/
		page_dir[num_entry_pd].pd_global= 0;		/* global (ignored)		*/
		page_dir[num_entry_pd].pd_avail = 0;		/* for programmer's use		*/
		page_dir[num_entry_pd].pd_base	= 1029;		/* location of page table?	*/	
		//kprintf("\nGlobal pt %d has page DIRectory address %d and the global page table address= %d", num_entry_pd, page_dir[num_entry_pd].pd_base);
	}
	
	return page_dir;
}

syscall init_bsm()
{
    for(int i = 0; i < (MAX_ID+1); i++)
		{
		bsmtab[i].bs_npages = 0;
		bsmtab[i].bs_pid = -1;
		bsmtab[i].bs_sem = 0;
		bsmtab[i].bs_status = BSM_UNMAPPED;
		bsmtab[i].bs_vpno = 4096;
		}

	return OK;
}


syscall init_frm()
{
	int i;
	
	//fr_list_ptr = NULL;
	//nru_head = NULL;

	for( i=0; i<NFRAMES; i++)
	{
		frmtab[i].fr_status = FRM_UNMAPPED;					
		frmtab[i].fr_pid = -1;								
		frmtab[i].fr_vpno = -1;								
		frmtab[i].fr_refcnt = 0;							
		frmtab[i].fr_dirty = 0; //frame is not dirty
		frmtab[i].fr_vprev=-1;
		frmtab[i].replace = 0;
		}
	
	return OK;
}

syscall init_frame_fifo()
{
	int i = 0;
	for(i = 0; i < NFRAMES; i++)
	{
		frame_fifo[i].frm_id = i;
		frame_fifo[i].next_frame = -1;
	}
	fifo_head=-1;
	return OK;
}

syscall init_check()
{
	for(int i = 0; i < NFRAMES; i++)
	{
		recheck[i].replace=0;
	}
return OK;
}

unsigned long read_cr0(void) {

  unsigned long local_tmp;

  
  asm("pushl %eax");
  asm("movl %cr0, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;
  kprintf("\n reading the cr0 register");
  kprintf("\n calling temp from read_cr0 \n \n");
  dump32(tmp);

 
  return local_tmp;
}

unsigned long read_cr2(void) {

  unsigned long local_tmp;

  
  asm("pushl %eax");
  asm("movl %cr2, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  
  return local_tmp;
}
unsigned long read_cr3(void) {

    unsigned long local_tmp;

  
  asm("pushl %eax");
  asm("movl %cr3, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  
  return local_tmp;
}


/*-------------------------------------------------------------------------
 * read_cr4 - read CR4
 *-------------------------------------------------------------------------
 */

unsigned long read_cr4(void) {

   unsigned long local_tmp;

 
  asm("pushl %eax");
  asm("movl %cr4, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

 
  return local_tmp;
}


/*-------------------------------------------------------------------------
 * write_cr0 - write CR0
 *-------------------------------------------------------------------------
 */

void write_cr0(unsigned long n) {

 
  kprintf("\n before writing cr0");
  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");		
  asm("movl %eax, %cr0");
  asm("popl %eax");
  kprintf("\n wrote cr0");
 
}


/*-------------------------------------------------------------------------
 * write_cr3 - write CR3
 *-------------------------------------------------------------------------
 */

void write_cr3(unsigned long n) {


  
  
  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");               
  asm("movl %eax, %cr3");
  asm("popl %eax");
  //kprintf("\n wrote cr3 register");

 }


/*-------------------------------------------------------------------------
 * write_cr4 - write CR4
 *-------------------------------------------------------------------------
 */

void write_cr4(unsigned long n) {


  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");               
  asm("movl %eax, %cr4");
  asm("popl %eax");

  

}

void dump32(unsigned long n) {
  int i;

  for(i = 31; i>=0; i--) {
    kprintf("%02d ",i);
  }

  kprintf("\n");

  for(i=31;i>=0;i--)
    kprintf("%d  ", (n&(1<<i)) >> i);

  kprintf("\n");
}

