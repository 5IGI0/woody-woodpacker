#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <elf.h>

#include "ft.h"
#include "bootloader.h"
#include "pack.h"

static inline uintptr_t prepare_program_headers(char *elf, size_t size, size_t output_size, uintptr_t ph_offset, const Elf64_Shdr *text_hdr) {
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)elf;

    ASSERT_OFFSET(hdr->e_phoff, hdr->e_phnum, hdr->e_phentsize, size, 0);

    /* COPY OLD PHs */
    ft_memcpy(elf + ph_offset, elf + hdr->e_phoff, hdr->e_phnum * hdr->e_phentsize);// TODO: remove
    hdr->e_phoff = ph_offset;

    /* FIX PT_PHDR / FIND FREE VADDR / PATCH .text PH */
    uintptr_t available_vaddr = 0;
    Elf64_Phdr *phtbl_hdr = NULL;

    for (size_t i = 0; i < hdr->e_phnum; i++) {
        Elf64_Phdr *phdr = (Elf64_Phdr *)(elf  + hdr->e_phoff + (i * hdr->e_phentsize));

        if (phdr->p_type == PT_LOAD) {
            if ((phdr->p_vaddr + phdr->p_memsz) > available_vaddr) {
                /*  if the ph is a loadable segment and virtual addr * memsize > available_vaddr
                    then update it */
                available_vaddr = (phdr->p_vaddr + phdr->p_memsz);
            }
            
            if (phdr->p_vaddr <= text_hdr->sh_addr && (phdr->p_vaddr+phdr->p_memsz) > text_hdr->sh_addr) {
                /*  if the PH contains the .text, then we need to make it writable
                    so the bootloader can decrypt its content */
                phdr->p_flags |= PF_W;
            }
        } else if (phdr->p_type == PT_PHDR) {
            /*  if the ph is the ph table's header, then we update the offset/size
                (and we will update the vaddr once we found it) */
            phdr->p_offset  = ph_offset;
            phdr->p_filesz  = hdr->e_phentsize * (hdr->e_phnum + 1);
            phdr->p_memsz   = hdr->e_phentsize * (hdr->e_phnum + 1);
            phtbl_hdr       = phdr;
        }
    }

    /* UPDATE PT_PHDR VADDR */
    available_vaddr = ROUND(available_vaddr);
    phtbl_hdr->p_vaddr = available_vaddr;
    phtbl_hdr->p_paddr = available_vaddr;

    /* ADD NEW SEGMENT */
    if (available_vaddr == 0)
        write(2, "invalid program headers.\n", 25);
    else {
        /* it will store the ph and the bootloader because too lazy to do 2 phs. */
        Elf64_Phdr *phdr = (Elf64_Phdr *)(elf + hdr->e_phoff + (hdr->e_phentsize * hdr->e_phnum));
        phdr->p_type    = PT_LOAD;
        phdr->p_offset  = ph_offset;
        phdr->p_vaddr   = available_vaddr;
        phdr->p_paddr   = available_vaddr;
        phdr->p_filesz  = output_size - ph_offset;
        phdr->p_memsz   = output_size - ph_offset;
        phdr->p_flags   = PF_X|PF_W|PF_R;
        phdr->p_align   = 8;
        hdr->e_phnum++;
    }

    return available_vaddr;
}

static const Elf64_Shdr *find_text(const char *elf, size_t size) {
    const Elf64_Ehdr *hdr       = (Elf64_Ehdr *)elf;
    const Elf64_Shdr *shdr      = (Elf64_Shdr *)(elf + hdr->e_shoff + (hdr->e_shentsize * hdr->e_shstrndx));
    const char       *sec_names = NULL;
    size_t           sec_namesz = 0;

    if (hdr->e_shstrndx == 0 || hdr->e_shstrndx >= hdr->e_shnum) {
        write(2, "the ELF doesn't contain string table.\n", 38);
        return NULL;
    }

    ASSERT_OFFSET(hdr->e_shoff, hdr->e_shnum, hdr->e_shentsize, size, NULL);
    ASSERT_OFFSET(shdr->sh_offset, 1, shdr->sh_size, size, NULL);

    sec_names  = elf + shdr->sh_offset;
    sec_namesz = shdr->sh_size;

    /*  if the name table is smaller than ".text\0"
        then it's obviously not present. */
    if (sec_namesz < sizeof(".text"))
        return NULL;

    for (size_t i = 0; i < hdr->e_shnum; i++) {
        shdr = (const Elf64_Shdr *)(elf + hdr->e_shoff + (i * hdr->e_shentsize));

        if (shdr->sh_name > (sec_namesz - sizeof(".text")))
            continue;
        if (ft_memcmp(".text", sec_names+shdr->sh_name, sizeof(".text")) == 0)
            return shdr;
    }

    write(2, ".text not found.\n", 17);
    return NULL;
}

static inline void setup_bootloader(char *elf, uintptr_t bootloader_vaddr, uintptr_t bootloader_offset, Elf64_Shdr *text_hdr) {
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)elf;

    /* ENCRYPT .text */
    text_hdr->sh_flags |= SHF_WRITE;
    for (size_t i = 0; i < text_hdr->sh_size; i++) {
        elf[text_hdr->sh_offset + i] ^= 0x58;
    }

    /* ADD THE BOOTLOADER */
    ft_memcpy(elf + bootloader_offset, bootloader_64, bootloader_64_len);
    uintptr_t old_entry = hdr->e_entry;
    hdr->e_entry = bootloader_vaddr;

    /* FIND & CHANGE PLACEHOLDERS */
    uint32_t *plch = find_32_placeholder(PLACEHOLDER_ENTRY, elf+bootloader_offset, bootloader_64_len);
    *plch = -(intptr_t)(hdr->e_entry - old_entry + (((char *)plch) - (elf + bootloader_offset)) + 4); // TODO: better readability
    plch = find_32_placeholder(PLACEHOLDER_TEXT_OFF, elf+bootloader_offset, bootloader_64_len);
    *plch = hdr->e_entry - text_hdr->sh_addr;
    plch = find_32_placeholder(PLACEHOLDER_TEXT_SIZE, elf+bootloader_offset, bootloader_64_len);
    *plch = text_hdr->sh_size;
}

int pack_elf(const char *input_elf, size_t input_size) {
    char             *elf;
    size_t           elf_size;
    Elf64_Shdr       *text_hdr;
    Elf64_Ehdr       *hdr = (Elf64_Ehdr *)input_elf;

    /* CHECK THAT AN ENTRYPOINT IS PRESENT */
    if (hdr->e_entry == 0) {
        write(2, "cannot pack elf that don't have an entrypoint.\n", 47);
        return 1;
    }

    /* COMPUTE OUTPUT ELF SIZE */
    uintptr_t ph_offset         = ROUND(input_size);
    uintptr_t bootloader_offset = ROUND(ph_offset + ((hdr->e_phnum + 1) * hdr->e_phentsize));
    elf_size = ROUND(bootloader_offset + bootloader_64_len);

    if ((elf = malloc(elf_size)) == NULL) {
        perror("malloc()");
        return -1;
    }
    hdr = (Elf64_Ehdr *)elf;
    ft_memcpy(elf, input_elf, input_size);
    ft_memset(elf+input_size, 0, elf_size-input_size);

    if ((text_hdr = (Elf64_Shdr *)find_text(input_elf, input_size)) == NULL) {
        free(elf);
        return -1;
    }

    uintptr_t available_vaddr = prepare_program_headers(elf, input_size, elf_size, ph_offset, text_hdr);
    if (available_vaddr == 0) {
        free(elf);
        return -1;
    }

    setup_bootloader(elf, available_vaddr + (bootloader_offset - ph_offset), bootloader_offset, text_hdr);

    /* write */
    int ofd = open("woody", O_CREAT|O_WRONLY|O_TRUNC, 00711);
    
    if (ofd < 0) {
        free(elf);
        perror("woody");
        return 1;
    }

    if (write(ofd, elf, elf_size) < 0) {
        free(elf);
        perror("woody");
        return 1;
    }

    close(ofd);
    free(elf);
    return 0;
}