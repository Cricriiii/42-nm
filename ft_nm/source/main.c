/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/10 00:22:15 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/15 15:27:11 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define _GNU_SOURCE

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cvector.h"
#include "ft_nm.h"

struct global_args_t {
    bool debug_syms;  /* -a option*/
    bool extern_only; /* -g option*/
    bool help;        /* -h option*/
    bool no_sort;     /* -p option*/
    bool rsort;       /* -r option*/
    bool undef_only;  /* -u option*/
    cvector(char*) filename;

} global_args = {};

static const char* opt_string = "aghpru";

const struct option opts[] = {{"debug-syms", no_argument, nullptr, 'a'},
                              {"extern-only", no_argument, nullptr, 'g'},
                              {"help", no_argument, NULL, 'h'},
                              {"no-sort", no_argument, nullptr, 'p'},
                              {"reverse-sort", no_argument, nullptr, 'r'},
                              {"undefined-only", no_argument, nullptr, 'u'},
                              {nullptr, 0, nullptr, 0}};

void test_32() {
    printf("32\n");
};
void test_64() {
    printf("64\n");
}
#define test(x) _Generic((x), Elf32_Ehdr*: test_32, Elf64_Ehdr*: test_64)();

int main(int argc, char** argv, char** envp) {
    (void)argc;
    (void)argv;
    (void)envp;

    int long_index;
    int opt;
    while ((opt = getopt_long(argc, argv, opt_string, opts, &long_index)) !=
           -1) {
        switch (opt) {
            case 'a':
                global_args.debug_syms = true;
                break;

            case 'g':
                global_args.extern_only = true;
                break;

            case 'h':
                global_args.help = true;
                break;

            case 'p':
                global_args.no_sort = true;
                break;

            case 'r':
                global_args.rsort = true;
                break;

            case 'u':
                global_args.undef_only = true;
                break;

            default:
                cvector_push_back(global_args.filename, optarg);
        }
    }

    for (int i = optind; i < argc; ++i) {
        cvector_push_back(global_args.filename, argv[i]);
    }

    if (cvector_empty(global_args.filename)) {
        cvector_push_back(global_args.filename, "a.out");
    }

    // printf(
    //     "debug_syms = %d\n"
    //     "extern_only = %d\n"
    //     "help = %d\n"
    //     "no_sort = %d\n"
    //     "rsort = %d\n"
    //     "undef_only = %d\n",
    //     global_args.debug_syms, global_args.extern_only, global_args.help,
    //     global_args.no_sort, global_args.rsort, global_args.undef_only);

    printf("Files:\n");
    for (cvector_iterator(char*) it = cvector_begin(global_args.filename);
         it != cvector_end(global_args.filename); ++it) {
        printf("  %s\n", *it);
    }

    /* --------------------------------------------------------------------------
     */

    int file_fd = open(global_args.filename[0], O_RDONLY);
    if (file_fd == -1) {
        perror("open");
    }

    struct stat st = {};
    int file_stats = fstat(file_fd, &st);
    if (file_stats == -1) {
        perror("fstat");
    }

    void* mapped = nullptr;

    if (file_fd != -1) {
        mapped = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, file_fd, 0);
        close(file_fd);
        file_fd = -1;
    }

    /* Ehdr Elf header */
    Elf64_Ehdr* e = (Elf64_Ehdr*)mapped;
    if (e) {
        ehdr_interpret(e);
    }
    
    /* Phdr Program header */    
    if (e->e_phoff) {
        // Elf64_Half e_ph_size = e->e_phentsize;
        Elf64_Half e_ph_num = e->e_phnum;
        Elf64_Phdr* p = (Elf64_Phdr*)(mapped + e->e_phoff);
        for (auto ptr = p; ptr - p < e_ph_num; ++ptr) {
            phdr_interpret(ptr);
        }
    }

    // Elf64_Sym* str;
    
    /* Shdr Section header */
    if (e->e_shoff) {
        // Elf64_Half e_sh_size = e->e_shentsize;
        Elf64_Half e_sh_num = e->e_shnum;
        Elf64_Shdr* s = (Elf64_Shdr*)(mapped + e->e_shoff);
        for (auto ptr = s; ptr - s < e_sh_num; ++ptr) {
            shdr_interpret(ptr);
        }
    }
    
    munmap(mapped, st.st_size);

    return (0);
}
