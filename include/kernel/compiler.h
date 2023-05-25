#ifndef _COMPILER_H
#define _COMPILER_H
/*
 * __used is intended to be used in cases where a reference to an identifier 
 * may be invisible to the compiler (e.g. an inline assembly operand not listed
 * in the asm()'s operands), preventing the compiler from eliminating the
 * variable or function.
 */
#define __used         		__attribute__((__used__))
#define __aligned(a) 		__attribute__((__aligned__(a)))
/* Clang can replace some vars with new automatic ones that go in .data;
 * mark all explicit-segment vars 'used' to prevent that. */
#define __section(s)      	__used __attribute__((__section__(s)))
#define __text_section(s) __attribute__((__section__(s)))

#define __init            __text_section(".init.text")
#define __initdata        __section(".init.data")

/* Some constant macros are used in both assembler and
 * C code.  Therefore we cannot annotate them always with
 * 'UL' and other type specifiers unilaterally.  We
 * use the following macros to deal with this.
 *
 * Similarly, _AT() will cast an expression with a type in C, but
 * leave it unchanged in asm.
 */

#ifdef __ASSEMBLY__
#define _AC(X,Y)	X
#define _AT(T,X)	X
#else
#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#define _AT(T,X)	((T)(X))
#endif


#endif 