/**
 * Copyright (c) 2024 libryan@outlook.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "elf32.h"

#define debug           true/*false*/
#define dbg(fmt...)     if (debug) printf(fmt)

typedef struct {
    elf32_hdr_t header;
    uint8_t     data[0];
} elf32_img_t;


static elf32_img_t* input(const char* in_file)
{
    elf32_img_t* elf = NULL;

    FILE *fp = fopen(in_file, "rb");
    if (NULL == fp) {
        perror("Failed to open input file");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    elf = (elf32_img_t*) malloc(size);
    if (NULL == elf) {
        perror("Failed to allocate memory for input buffer");
        fclose(fp);
        return NULL;
    }

    if (size != fread(elf, 1, size, fp)) {
        perror("Failed to read input file");
        free(elf);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return elf;
}


static elf32_img_t* strip(const elf32_img_t* in, uint32_t* psz)
{
    const program_hdr_t* phdr = (const program_hdr_t*) ((uint8_t*) in + in->header.phoff);
    uint32_t phnum = in->header.phnum;
    uint32_t aoff = 0;
    uint32_t size = sizeof(in->header);

    // Calculate the size needed for the new ELF image
    for (uint32_t i = 0; i < phnum; i++) {
        if (phdr[i].type == PT_LOAD) {
            size += (phdr[i].filesz + sizeof(program_hdr_t));
            aoff++;
        }
    }

    elf32_img_t* out = (elf32_img_t*) malloc(size);
    if (NULL == out) {
        perror("Failed to allocate memory for output buffer");
        return NULL;
    }

    // Copy and adjust the new ELF header
    memcpy((void *) out, in, sizeof(in->header));
    out->header.ehsize = sizeof(in->header);
    out->header.phnum = aoff;
    out->header.phentsize = sizeof(program_hdr_t);
    out->header.phoff = size - aoff * sizeof(program_hdr_t);
    out->header.shoff = size;
    out->header.shnum = 0;
    out->header.shentsize = 0;
    out->header.shstrndx = 0;

    // Write new program headers and associated sections
    aoff = out->header.ehsize;
    program_hdr_t* ophdr = (program_hdr_t*) ((uint8_t*) out + out->header.phoff);
    for (uint32_t i = 0; i < phnum; i++) {
        if (phdr[i].type == PT_LOAD) {
            // Copy and fix new out sections' offset
            memcpy((void*) ophdr, (const void*) &phdr[i], sizeof(program_hdr_t));
            ophdr->offset = aoff;

            // Copy section's content in file offset to new out
            memcpy((void*) ((uint8_t*) out + aoff), (void*) ((uint8_t*) in + phdr[i].offset), phdr[i].filesz);

            // Next out
            aoff += phdr[i].filesz;
            ophdr++;
        }
    }

    aoff += (out->header.phentsize * out->header.phnum);
    if (aoff != size) {
        printf("Convert error: %u - %u!\n", aoff, size);
        free(out);
        return NULL;
    }

    *psz = size;
    return out;
}


static uint32_t output(const uint8_t* raw, uint32_t size, const char* out_file)
{
    FILE *fp = fopen(out_file, "wb");
    if (NULL == fp) {
        perror("Failed to open output file");
        return 0;
    }

    uint32_t written = fwrite(raw, 1, size, fp);
    fclose(fp);

    return written;
}


int main(int argc, const char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.elf output.elf\n", argv[0]);
        return -1;
    }

    const char* in_file = argv[1];
    const char* out_file = argv[2];

    elf32_img_t* origin = input(in_file);
    if (NULL == origin) {
        return -2;
    }

    uint32_t size;
    elf32_img_t* stripped = strip(origin, &size);
    if (NULL == stripped) {
        free(origin);
        return -3;
    }

    if (output((uint8_t*) stripped, size, out_file) != size) {
        free(stripped);
        free(origin);
        return -4;
    }

    printf("Stripped %s -> %s, %u bytes written.\n", in_file, out_file, size);
    free(stripped);
    free(origin);

    return 0;
}