/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_elf.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 15:38:27 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 14:50:23 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_elf.h"

#include <stdio.h>
#include <string.h>

#include "resource_collector.h"
#include "xalloc.h"

typedef struct nm_args nm_args_t;

extern nm_args_t options;

elf_ei_t read_elf_ident(void* addr) {
    char* ident = (char*)addr;
    return (elf_ei_t){
        .mag0 = ident[0],
        .mag1 = ident[1],
        .mag2 = ident[2],
        .mag3 = ident[3],
        .class = ident[4],
        .data = ident[5],
        .version = ident[6],
        .osabi = ident[7],
        .abiversion = ident[8],
    };
}

nm_sym_out_t** read_elf32(void* addr) {
    (void)addr;
    nm_sym_out_t** ret_array = nullptr;
    return ret_array;
}

static int is_external(const Elf64_Sym* s) {
    unsigned char binding = ELF64_ST_BIND(s->st_info);

    return binding == STB_GLOBAL || binding == STB_WEAK ||
           binding == STB_GNU_UNIQUE || s->st_shndx == SHN_UNDEF ||
           s->st_shndx == SHN_COMMON;
}

static int is_undef(const Elf64_Sym* s) {
    return s->st_shndx == SHN_UNDEF;
}

static void get_symbol_sections(Elf64_Shdr* shdr, Elf64_Half shnum,
                                elf_elem_str_t* sections,
                                elf_elem_str_t* symbols,
                                elf_elem_str_t* dynamic) {
    for (Elf64_Half i = 0; i < shnum; ++i) {
        Elf64_Shdr* s = &shdr[i];

        if (s == sections->strings || s->sh_link == shnum) {
            continue;

        } else if (s->sh_type == SHT_SYMTAB) {
            /* in .symtab, read .strtab index */
            (*symbols).elems = s;
            (*symbols).strings = &shdr[s->sh_link];

        } else if (s->sh_type == SHT_DYNSYM) {
            /* in .dynsym, read .dynstr index */
            (*dynamic).elems = s;
            (*dynamic).strings = &shdr[s->sh_link];
        }
    }
}

/* section name -> letter, as bfd's coff_section_type() */
static int section_type_from_name(const char* name) {
    static const struct {
        const char* prefix;
        int type;
    } table[] = {
        {".bss", 'b'},     {".code", 't'},  {".data", 'd'},
        {"*DEBUG*", 'N'},  {".debug", 'N'}, {".drectve", 'i'},
        {".edata", 'e'},   {".fini", 't'},  {".idata", 'i'},
        {".init", 't'},    {".pdata", 'p'}, {".rdata", 'r'},
        {".rodata", 'r'},  {".sbss", 's'},  {".scommon", 'c'},
        {".sdata", 'g'},   {".text", 't'},  {"vars", 'd'},
        {"zerovars", 'b'},
    };

    for (size_t i = 0; i < sizeof(table) / sizeof(*table); ++i) {
        size_t len = strlen(table[i].prefix);

        /* ".text" and ".text.foo" match, ".textfoo" does not */
        if (strncmp(name, table[i].prefix, len) == 0 &&
            (name[len] == '\0' || strchr(".$0123456789", name[len]))) {
            return table[i].type;
        }
    }
    return '?';
}

static bool is_debug_section(const Elf64_Shdr* section, const char* name) {
    if (section->sh_flags & SHF_ALLOC) {
        return false;
    }
    return strncmp(name, ".debug", 6) == 0 ||
           strncmp(name, ".zdebug", 7) == 0 ||
           strncmp(name, ".gnu.debuglto_.debug_", 21) == 0 ||
           strncmp(name, ".gnu.linkonce.wi.", 17) == 0 ||
           strncmp(name, ".line", 5) == 0 || strncmp(name, ".stab", 5) == 0 ||
           strcmp(name, ".gdb_index") == 0;
}

/* section flags -> letter, as bfd's decode_section_type() */
static int section_type_from_flags(const Elf64_Shdr* section,
                                   const char* name) {
    bool has_contents = section->sh_type != SHT_NOBITS;
    bool readonly = !(section->sh_flags & SHF_WRITE);

    if (section->sh_flags & SHF_EXECINSTR) {
        return 't';
    }
    if ((section->sh_flags & SHF_ALLOC) && has_contents) {
        return readonly ? 'r' : 'd';
    }
    if (!has_contents) {
        return 'b';
    }
    if (is_debug_section(section, name)) {
        return 'N';
    }
    if (readonly) {
        return 'n';
    }
    return '?';
}

int get_symbol_type(const Elf64_Shdr* shdr, Elf64_Half shnum,
                    const char* shstrtab, const Elf64_Sym* s) {
    unsigned char binding = ELF64_ST_BIND(s->st_info);
    unsigned char type = ELF64_ST_TYPE(s->st_info);

    if (s->st_shndx == SHN_COMMON) {
        return 'C';
    }
    if (s->st_shndx == SHN_UNDEF) {
        if (binding == STB_WEAK) {
            return type == STT_OBJECT ? 'v' : 'w';
        }
        return 'U';
    }
    if (type == STT_GNU_IFUNC) {
        return 'i';
    }
    if (binding == STB_WEAK) {
        return type == STT_OBJECT ? 'V' : 'W';
    }
    if (binding == STB_GNU_UNIQUE) {
        return 'u';
    }
    if (binding != STB_LOCAL && binding != STB_GLOBAL) {
        return '?';
    }

    int c;

    /* bfd treats any index it can't resolve as an absolute symbol */
    if (s->st_shndx == SHN_ABS || s->st_shndx >= shnum) {
        c = 'a';
    } else {
        const Elf64_Shdr* section = &shdr[s->st_shndx];
        const char* name = shstrtab + section->sh_name;

        c = section_type_from_name(name);
        if (c == '?') {
            c = section_type_from_flags(section, name);
        }
    }

    if (binding == STB_GLOBAL && c >= 'a' && c <= 'z') {
        c -= 'a' - 'A';
    }
    return c;
}

nm_sym_out_t** read_elf64(void* addr) {
    nm_sym_out_t** ret_array = nullptr;

    /* sections start */
    Elf64_Shdr* shdr_start = shdr64(addr + ehdr64(addr)->e_shoff);

    /* section header array */
    Elf64_Half shnum = ehdr64(addr)->e_shnum;

    elf_elem_str_t sections = {.strings = &shdr_start[ehdr64(addr)->e_shstrndx]};
    elf_elem_str_t symbols = {};
    elf_elem_str_t dynamic = {};
    get_symbol_sections(shdr_start, shnum, &sections, &symbols, &dynamic);

    if (symbols.elems) {
        size_t n_sym = symbols.elems->sh_size / symbols.elems->sh_entsize;

        ret_array = xcalloc(n_sym + 1, sizeof(nm_sym_out_t*));
        store_ptr_resource(ret_array);

        Elf64_Sym* symbol_tab = sym64(addr + symbols.elems->sh_offset);
        char* string_tab = addr + symbols.strings->sh_offset;

        size_t cur_sym_idx = 0;

        for (size_t i = 0; i < n_sym; ++i) {
            Elf64_Sym* s = &symbol_tab[i];

            nm_sym_out_t* nm_sym = xcalloc(1, sizeof(nm_sym_out_t));
            store_ptr_resource(nm_sym);
            nm_sym->name = string_tab + s->st_name;
            nm_sym->value = s->st_value;
            nm_sym->type = get_symbol_type(shdr_start, shnum, addr + symbols.elems->sh_offset, s);
            nm_sym->external = is_external(s);
            nm_sym->undef = is_undef(s);

            ret_array[cur_sym_idx++] = nm_sym;
        }
    }

    return ret_array;
}
