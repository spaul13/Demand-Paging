/* paging.h */

#ifndef __PAGING_H_
#define __PAGING_H_

/* Structure for a page directory entry */

typedef struct {
	unsigned int pd_pres	: 1;		/* page table present?		*/
	unsigned int pd_write : 1;		/* page is writable?		*/
	unsigned int pd_user	: 1;		/* is use level protection?	*/
	unsigned int pd_pwt	: 1;		/* write through cachine for pt? */
	unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
	unsigned int pd_acc	: 1;		/* page table was accessed?	*/
	unsigned int pd_mbz	: 1;		/* must be zero			*/
	unsigned int pd_fmb	: 1;		/* four MB pages?		*/
	unsigned int pd_global: 1;		/* global (ignored)		*/
	unsigned int pd_avail : 3;		/* for programmer's use		*/
	unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

pd_t* pagedir_null; 

/* Structure for a page table entry */

typedef struct {
	unsigned int pt_pres	: 1;		/* page is present?		*/
	unsigned int pt_write : 1;		/* page is writable?		*/
	unsigned int pt_user	: 1;		/* is use level protection?	*/
	unsigned int pt_pwt	: 1;		/* write through for this page? */
	unsigned int pt_pcd	: 1;		/* cache disable for this page? */
	unsigned int pt_acc	: 1;		/* page was accessed?		*/
	unsigned int pt_dirty : 1;		/* page was written?		*/
	unsigned int pt_mbz	: 1;		/* must be zero			*/
	unsigned int pt_global: 1;		/* should be zero in 586	*/
	unsigned int pt_avail : 3;		/* for programmer's use		*/
	unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct {
  unsigned int pg_offset : 12;		
  unsigned int pt_offset : 10;		
  unsigned int pd_offset : 10;		
} vaddr;

//extern	struct	vaddr vraddr[];

struct bs_map{
  int bs_status;			/* MAPPED or UNMAPPED		*/
  pid32 bs_pid;				/* process id using this slot   */
  uint32 bs_vpno;			/* starting virtual page number */
  uint32 bs_npages;			/* number of pages in the store */
  int bs_sem;				/* semaphore mechanism ?	*/
  int ispriv;              /* PRIVATE HEAP */
};

struct fr_map{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  pid32 fr_pid;				/* process id using this frame  */
  uint32 fr_vpno;			/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/

  int fr_dirty;
  int fr_type;
  uint32 nxt_frm;
  uint32 fr_vprev; //previous pagenum while reading
    int replace;     
  };

struct fifo_node
{
	int frm_id;
	int next_frame;
};
struct check
{
int pagenum;
bsd_t bsn;
int replace;
};
 
/*struct fifo_list        //for the fifo page replacement policy implementation
{
	int frm_no;
	struct fifo_list* frm_next;
	struct fifo_list* frm_prev;
};*/




#define PAGEDIRSIZE	1024
#define PAGETABSIZE	1024

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/

#ifndef NFRAMES
#define NFRAMES		 50	/* number of frames		*/
#endif

#define MAP_SHARED 1
#define MAP_PRIVATE 2

#define FIFO 3
#define GCA 4

#define MAX_ID		7		/* You get 8 mappings, 0 - 7 */
#define MIN_ID		0

extern int32 currpolicy;
extern struct fr_map frmtab[NFRAMES];
extern struct bs_map bsmtab[MAX_ID +1];
extern struct fifo_node frame_fifo[NFRAMES];
extern int sum,ptime;
extern struct check recheck[NFRAMES];
extern unsigned long inaddr;
extern int reclaim;
extern int replacement;
extern uint32 returning_frame;

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1
extern uint32 count;
extern uint32 fifo_head;
extern sid32 pfsem;
extern int page_faulting;

#endif // __PAGING_H_
