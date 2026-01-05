/* context.S - Context switch routine */
    .text
    .globl context_switch
context_switch:
    /* Arguments: context_switch(old_ctx, new_ctx) */
    mov 4(%esp), %eax      /* old_ctx */
    mov 8(%esp), %edx      /* new_ctx */

    lea 1f, %ecx           /* address to resume after switch */
    mov %ecx, 8(%eax)      /* old_ctx->eip */
    mov %esp, 0(%eax)      /* old_ctx->esp */
    mov %ebp, 4(%eax)      /* old_ctx->ebp */

    mov 0(%edx), %esp      /* new esp */
    mov 4(%edx), %ebp      /* new ebp */
    jmp *8(%edx)           /* jump to new eip */
1:
    ret

/* Mark stack as non-executable for tools that honor .note.GNU-stack */
.section .note.GNU-stack,"",@progbits
