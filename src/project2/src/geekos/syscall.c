/*
 * System call handlers
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003,2004 David Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.59 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/syscall.h>
#include <geekos/errno.h>
#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/elf.h>
#include <geekos/malloc.h>
#include <geekos/screen.h>
#include <geekos/keyboard.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/timer.h>
#include <geekos/vfs.h>

static int Copy_User_String(ulong_t userAddr, ulong_t length, ulong_t maxLength,
    char **pStr)
{
    char *str;

    if (length > maxLength) {
        return EINVALID;
    }
    str = Malloc(length + 1);
    if (str == 0) {
        return ENOMEM;
    }
    if (!Copy_From_User(str, userAddr, length)) {
        Free(str);
        return EINVALID;
    }
    str[length] = '\0';
    *pStr = str;
    return 0;
}

/*
 * Null system call.
 * Does nothing except immediately return control back
 * to the interrupted user program.
 * Params:
 *  state - processor registers from user mode
 *
 * Returns:
 *   always returns the value 0 (zero)
 */
static int Sys_Null(struct Interrupt_State* state)
{
    return 0;
}

/*
 * Exit system call.
 * The interrupted user process is terminated.
 * Params:
 *   state->ebx - process exit code
 * Returns:
 *   Never returns to user mode!
 */
static int Sys_Exit(struct Interrupt_State* state)
{
    Exit(state->ebx);
}

/*
 * Print a string to the console.
 * Params:
 *   state->ebx - user pointer of string to be printed
 *   state->ecx - number of characters to print
 * Returns: 0 if successful, -1 if not
 */
static int Sys_PrintString(struct Interrupt_State* state)
{
    char *buf;
    int rc;

    if (state->ecx == 0) {
        return 0;
    }
    rc = Copy_User_String(state->ebx, state->ecx, 1023, &buf);
    if (rc < 0) {
        return rc;
    }
    Put_Buf(buf, state->ecx);
    Free(buf);
    return 0;
}

/*
 * Get a single key press from the console.
 * Suspends the user process until a key press is available.
 * Params:
 *   state - processor registers from user mode
 * Returns: the key code
 */
static int Sys_GetKey(struct Interrupt_State* state)
{
    return Wait_For_Key();
}

/*
 * Set the current text attributes.
 * Params:
 *   state->ebx - character attributes to use
 * Returns: always returns 0
 */
static int Sys_SetAttr(struct Interrupt_State* state)
{
    Set_Current_Attr((uchar_t)state->ebx);
    return 0;
}

/*
 * Get the current cursor position.
 * Params:
 *   state->ebx - pointer to user int where row value should be stored
 *   state->ecx - pointer to user int where column value should be stored
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_GetCursor(struct Interrupt_State* state)
{
    int row, col;

    Get_Cursor(&row, &col);
    if (!Copy_To_User(state->ebx, &row, sizeof(row)) ||
        !Copy_To_User(state->ecx, &col, sizeof(col))) {
        return -1;
    }
    return 0;
}

/*
 * Set the current cursor position.
 * Params:
 *   state->ebx - new row value
 *   state->ecx - new column value
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_PutCursor(struct Interrupt_State* state)
{
    if (!Put_Cursor(state->ebx, state->ecx)) {
        return -1;
    }
    return 0;
}

/*
 * Create a new user process.
 * Params:
 *   state->ebx - user address of name of executable
 *   state->ecx - length of executable name
 *   state->edx - user address of command string
 *   state->esi - length of command string
 * Returns: pid of process if successful, error code (< 0) otherwise
 */
static int Sys_Spawn(struct Interrupt_State* state)
{
    int rc;
    char *program = 0;
    char *command = 0;
    struct Kernel_Thread *process;

    rc = Copy_User_String(state->ebx, state->ecx, 1023, &program);
    if (rc < 0) {
        goto done;
    }
    rc = Copy_User_String(state->edx, state->esi, 1023, &command);
    if (rc < 0) {
        goto done;
    }

    Enable_Interrupts();
    rc = Spawn(program, command, &process);
    Disable_Interrupts();

done:
    if (program != 0) {
        Free(program);
    }
    if (command != 0) {
        Free(command);
    }
    return rc;
}

/*
 * Wait for a process to exit.
 * Params:
 *   state->ebx - pid of process to wait for
 * Returns: the exit code of the process,
 *   or error code (< 0) on error
 */
static int Sys_Wait(struct Interrupt_State* state)
{
    struct Kernel_Thread *kthread = Lookup_Thread(state->ebx);
    int exitCode;

    if (kthread == 0) {
        return -1;
    }
    Enable_Interrupts();
    exitCode = Join(kthread);
    Disable_Interrupts();
    return exitCode;
}

/*
 * Get pid (process id) of current thread.
 * Params:
 *   state - processor registers from user mode
 * Returns: the pid of the current thread
 */
static int Sys_GetPID(struct Interrupt_State* state)
{
    return g_currentThread->pid;
}


/*
 * Global table of system call handler functions.
 */
const Syscall g_syscallTable[] = {
    Sys_Null,
    Sys_Exit,
    Sys_PrintString,
    Sys_GetKey,
    Sys_SetAttr,
    Sys_GetCursor,
    Sys_PutCursor,
    Sys_Spawn,
    Sys_Wait,
    Sys_GetPID,
};

/*
 * Number of system calls implemented.
 */
const int g_numSyscalls = sizeof(g_syscallTable) / sizeof(Syscall);
