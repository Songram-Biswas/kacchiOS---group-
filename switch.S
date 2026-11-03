/* switch.S - x86 Context Switch */
.global context_switch

/* void context_switch(uint32_t* old_esp, uint32_t new_esp); */
context_switch:
    pushal

    movl 36(%esp), %eax       /* eax = old_esp pointer */
    testl %eax, %eax
    jz .load_new
    movl %esp, (%eax)         /* *old_esp = %esp */

.load_new:
    movl 40(%esp), %esp       /* %esp = new_esp */

    popal
    ret