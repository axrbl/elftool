/**
 * Copyright (c) 2024 libryan@outlook.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ELF32_H__
#define __ELF32_H__

#include <stdint.h>


typedef struct {
    uint8_t  ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf32_hdr_t;

#define PT_LOAD 1
typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} program_hdr_t;


#endif /*__ELF32_H__*/
