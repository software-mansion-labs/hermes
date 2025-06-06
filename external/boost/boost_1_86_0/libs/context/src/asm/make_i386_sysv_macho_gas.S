/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|   EDI   |   ESI   |   EBX    |   EBP   |   EIP   |    to   |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |    10   |    11   |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |                                                                      |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   data  |                                                                      |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.text
.globl _hoost_make_fcontext
.align 2
_hoost_make_fcontext:
    /* first arg of hoost_make_fcontext() == top of context-stack */
    movl  0x4(%esp), %eax

    /* reserve space for first argument of context-function
       eax might already point to a 16byte border */
    leal  -0x8(%eax), %eax

    /* shift address in EAX to lower 16 byte boundary */
    andl  $-16, %eax

    /* reserve space for context-data on context-stack, and align the stack */
    leal  -0x34(%eax), %eax

    /* third arg of hoost_make_fcontext() == address of context-function */
    /* stored in EBX */
    movl  0xc(%esp), %ecx
    movl  %ecx, 0x10(%eax)

    /* save MMX control- and status-word */
    stmxcsr (%eax)
    /* save x87 control-word */
    fnstcw  0x4(%eax)

    /* compute abs address of label trampoline */
    call  1f
    /* address of trampoline 1 */
1:  popl  %ecx
    /* compute abs address of label trampoline */
    addl  $trampoline-1b, %ecx
    /* save address of trampoline as return address */
    /* will be entered after calling hoost_jump_fcontext() first time */
    movl  %ecx, 0x18(%eax)

    /* compute abs address of label finish */
    call  2f
    /* address of label 2 */
2:  popl  %ecx
    /* compute abs address of label finish */
    addl  $finish-2b, %ecx
    /* save address of finish as return-address for context-function */
    /* will be entered after context-function returns */
    movl  %ecx, 0x14(%eax)

    ret /* return pointer to context-data */

trampoline:
    /* move transport_t for entering context-function */
    movl  %eax, (%esp)
    movl  %edx, 0x4(%esp)
    pushl %ebp
    /* jump to context-function */
    jmp *%ebx

finish:
    /* exit code is zero */
    xorl  %eax, %eax
    movl  %eax, (%esp)
    /* exit application */
    call  __exit
    hlt
