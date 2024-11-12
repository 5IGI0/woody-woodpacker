#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>

#include <elf.h>
#include "pack.h"

int main(int argc, char **argv) {
    int     ifd      = 0;
    char    *elf     = NULL;
    size_t  elf_size = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        return 1;
    }

    ifd = open(argv[1], O_RDONLY);
    if (ifd < 0) {
        perror(argv[1]);
        return 1;
    }

    elf_size = lseek(ifd, 0, SEEK_END);
    lseek(ifd, 0, SEEK_SET);

    if (elf_size < sizeof(Elf64_Ehdr)) {
        close(ifd);
        write(2, "Invalid file (unsupported format or truncated elf header.)\n", 59);
        return 1;
    }

    if ((elf = malloc(elf_size)) == NULL) {
        close(ifd);
        perror("malloc()");
        return 1;
    }

    Elf64_Ehdr *hdr = (Elf64_Ehdr *)elf;

    if (read(ifd, elf, elf_size) < 0) {
        close(ifd);
        perror(argv[1]);
        return 1;
    }
    close(ifd);

    if ( /* check ELF */
        hdr->e_ident[EI_MAG0] != ELFMAG0 ||
        hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 ||
        hdr->e_ident[EI_MAG3] != ELFMAG3
    ) {
        free(elf);
        write(2, "unsupported format.\n", 20);
        return 1;
    }

    if (
        // hdr->e_ident[EI_CLASS] != ELFCLASS64 ||
        hdr->e_ident[EI_DATA] != ELFDATA2LSB ||
        (hdr->e_machine != EM_X86_64 && hdr->e_machine != EM_386)) {
        free(elf);
        write(2, "unsupported architecture.\n", 26);
        return 1;
    }

    int ret = 0;
    if (hdr->e_machine == EM_X86_64)
        ret = pack_elf64(elf, elf_size);
    else
        ret = pack_elf32(elf, elf_size);

    free(elf);
    return ret != 0;
}