/*
       Copyright Claudio Jeker 2024
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/*
 * typedef void*	fcontext_t;
 *
 * struct transfer_t {
 *	fcontext_t	fctx;
 *	void	*	data;
 * };
 *
 * transfer_t hoost_jump_fcontext(fcontext_t const to, void *vp);
 */
#define CC64FSZ 176
#define BIAS 2047
#define SP 128
#define I7 136

.file "jump_sparc64_sysv_elf_gas.S"
.text
.align  4
.global hoost_jump_fcontext
.type   hoost_jump_fcontext, %function
hoost_jump_fcontext:
	# prepare stack
	save	%sp, -CC64FSZ, %sp

	# store framepointer and return address in slots reserved
	# for arguments
	stx %fp, [%sp + BIAS + SP]
	stx %i7, [%sp + BIAS + I7]
	mov %sp, %o0
	# force flush register windows to stack and with that save context
	flushw
	# get SP (pointing to new context-data) from %i0 param
	mov %i0, %sp
	# load framepointer and return address from context
	ldx [%sp + BIAS + SP], %fp
	ldx [%sp + BIAS + I7], %i7

	ret
	 restore %o0, %g0, %o0
	# restore old %sp (pointing to old context-data) in %o0
	# *data stored in %o1 was not modified
.size	hoost_jump_fcontext,.-hoost_jump_fcontext
# Mark that we don't need executable stack.
.section .note.GNU-stack,"",%progbits
