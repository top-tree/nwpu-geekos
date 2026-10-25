/*
 * User program entry point function
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2004, Iulian Neamtiu <neamtiu@cs.umd.edu>
 * $Revision: 1.10 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

/*
 * Trampoline enters with a far return frame (EIP, CS) on the stack.
 * Keep the entry sequence in top-level assembly so the compiler cannot
 * add a prologue or epilogue around that frame.
 */
__asm__(
    ".text\n"
    ".globl _Entry\n"
    "_Entry:\n"
    "pushl $0\n"
    "pushl $0\n"
    "call main\n"
    "addl $8, %esp\n"
    "lret\n"
);
