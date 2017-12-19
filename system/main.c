#include <xinu.h>

extern void page_policy_test(void);
extern void gentle1();
extern void gentle2();
extern void gentle3();
extern void hard();
extern void custom();

process	main(void)
{
  srpolicy(FIFO);
  kprintf("\n I am after srpolicy in main");

  /* Start the network */
  /* DO NOT REMOVE OR COMMENT BELOW */
#ifndef QEMU
  netstart();
#endif

  /*  TODO. Please ensure that your paging is activated 
      by checking the values from the control register.
  */

  /* Initialize the page server */
  /* DO NOT REMOVE OR COMMENT THIS CALL */
  psinit();
  kprintf("\n I am after psinit entering into page_policy_test in main");


  //page_policy_test();
//	gentle1();
/*	gentle2();*/
//	gentle3();
/*	custom(1);
	hard(1);*/
	fifo_policy_test(1);
//	gca_policy_test(1);

  return OK;
}
