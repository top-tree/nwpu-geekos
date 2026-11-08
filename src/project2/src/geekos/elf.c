/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.29 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h>  /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/elf.h>


/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */
int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat)
{
    elfHeader *elf_header = (elfHeader*)exeFileData;
    programHeader *program_header;
    int cnt = 0;

    exeFormat->entryAddr = elf_header->entry;
    for (int i = 0; i < elf_header->phnum; i++) {
        program_header = (programHeader*)(exeFileData + elf_header->phoff + i * elf_header->phentsize);
        if (program_header->type != 1) {
            continue;
        }
        exeFormat->segmentList[cnt].offsetInFile = program_header->offset;
        exeFormat->segmentList[cnt].lengthInFile = program_header->fileSize;
        exeFormat->segmentList[cnt].startAddress = program_header->vaddr;
        exeFormat->segmentList[cnt].sizeInMemory = program_header->memSize;
        exeFormat->segmentList[cnt].protFlags = program_header->flags;
        cnt++;
    }
    exeFormat->numSegments = cnt;
    return 0;
}

