#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <elf.h>

#include "bootloader.h"

#define ROUND(x) (((((uintptr_t)(x))-1)|0xFFF) + 1)

#define PLACEHOLDER_ENTRY 0x44444444
#define PLACEHOLDER_TEXT_OFF 0x33333333
#define PLACEHOLDER_TEXT_SIZE 0x22222222

uint32_t *find_32_placeholder(uint32_t placeholder, void *bootloader_start, size_t bl_len) {
    return memmem(bootloader_start, bl_len, &placeholder, sizeof(uint32_t)); // TODO: recode
}

int main(int argc, char **argv) {
    assert(argc == 2);
    FILE *fp = fopen(argv[1], "rb");

    assert(fp);

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *elf = malloc(size);
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)elf;

    fread(elf, 1, size, fp);
    fclose(fp);

    printf("elf size: %zx\n", size);

    uintptr_t ph_offset         = ROUND(size);
    uintptr_t bootloader_offset = ROUND(ph_offset + ((hdr->e_phnum + 1) * hdr->e_phentsize));
    size = ROUND(bootloader_offset + bootloader_len);

    printf(
        "new size: %zx\n"
        "phoff   : %zx\n"
        "bloff   : %zx\n",
        size, ph_offset, bootloader_offset);

    elf = realloc(elf, size); // TODO: memset added bytes
    assert(elf);
    hdr = (Elf64_Ehdr *)elf;

    /* FIND .text SECTION */
    Elf64_Shdr  *text_hdr = NULL;

    {
        char *sh_name = elf + ((Elf64_Shdr *)(elf + hdr->e_shoff + (hdr->e_shstrndx * hdr->e_shentsize)))->sh_offset;

        for (size_t i = 0; i < hdr->e_shnum; i++) {
            Elf64_Shdr *shdr = (Elf64_Shdr *)(elf + hdr->e_shoff + (i * hdr->e_shentsize));
            if (strcmp(".text", sh_name + shdr->sh_name) == 0)
                text_hdr = shdr;
        }

        printf(".text vaddr/size: %p/%zu\n", text_hdr->sh_addr, text_hdr->sh_size);
    }

    /* ENCRYPT .text */
    text_hdr->sh_flags |= SHF_WRITE;
    for (size_t i = 0; i < text_hdr->sh_size; i++) {
        elf[text_hdr->sh_offset + i] ^= 0x58;
    }

    /* COPY OLD PHs */
    memcpy(elf + ph_offset, elf + hdr->e_phoff, hdr->e_phnum * hdr->e_phentsize);
    hdr->e_phoff = ph_offset;

    uintptr_t available_vaddr = 0;
    Elf64_Phdr *phtbl_hdr = NULL;

    /* FIX PT_PHDR / FIND FREE VADDR / PATCH .text PH */
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
    {
        /* it will store the ph and the bootloader because too lazy to do 2 phs. */
        Elf64_Phdr *phdr = (Elf64_Phdr *)(elf + hdr->e_phoff + (hdr->e_phentsize * hdr->e_phnum));
        phdr->p_type    = PT_LOAD;
        phdr->p_offset  = ph_offset;
        phdr->p_vaddr   = available_vaddr;
        phdr->p_paddr   = available_vaddr;
        phdr->p_filesz  = size - ph_offset;
        phdr->p_memsz   = size - ph_offset;
        phdr->p_flags   = PF_X|PF_W|PF_R;
        phdr->p_align   = 8;
        hdr->e_phnum++;
    }

    /* ADD THE BOOTLOADER */
    memcpy(elf + bootloader_offset, bootloader, bootloader_len); // TODO: encrypt/decrypt .text

    uintptr_t old_entry = hdr->e_entry;
    hdr->e_entry = available_vaddr + (bootloader_offset - ph_offset); // update the entrypoint

    /* FIND & CHANGE PLACEHOLDERS */
    uint32_t *plch = find_32_placeholder(PLACEHOLDER_ENTRY, elf+bootloader_offset, bootloader_len);
    *plch = -(intptr_t)(hdr->e_entry - old_entry + (((char *)plch) - (elf + bootloader_offset)) + 4); // TODO: better readability

    plch = find_32_placeholder(PLACEHOLDER_TEXT_OFF, elf+bootloader_offset, bootloader_len);
    *plch = hdr->e_entry - text_hdr->sh_addr;

    plch = find_32_placeholder(PLACEHOLDER_TEXT_SIZE, elf+bootloader_offset, bootloader_len);
    *plch = text_hdr->sh_size;

    /* write */
    fp = fopen("packed", "wb");
    fwrite(elf, 1, size, fp);
    fclose(fp);

    return 0;
}