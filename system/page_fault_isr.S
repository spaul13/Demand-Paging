
.text
 .globl page_fault_isr

page_fault_isr:
        pushal
 	pushfl
	cli
        call pf
	popfl
        popal
        add $0x4, %esp
	iret
