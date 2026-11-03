*This project has been created as part of the 42 curriculum by cgajean.*

# ft_nm

A C implementation of the `nm` command from GNU binutils.

## Description

`nm` lists the symbols of object files. For each symbol it prints its value,
a letter for its type, and its name:

```
0000000000001139 T main
                 U printf@GLIBC_2.2.5
0000000000004010 D global_counter
0000000000004018 b local_buffer
```

The goal is to get the same output as `nm` by parsing the files by hand,
without `libelf` or `libbfd`. Supported inputs:

- ELF files, 32 and 64 bits, little endian: object files (`.o`), executables
  and shared libraries (`.so`)
- ar archives (`.a`), each member being listed under its name

The symbol letter is deduced from the binding and type of the symbol and from
the section it belongs to: `T`/`t`, `D`/`d`, `B`/`b`, `R`/`r`, `U`, `W`/`w`,
`V`/`v`, `C`, `A`, `N`, `i`, `u`. Lowercase means local, uppercase means global.

Symbols are sorted by name using the current locale, like `nm` does. Error
messages use the same wording as `nm` (missing file, directory, file format
not recognized, truncated file, no symbols), and an invalid file does not
stop the processing of the next ones. All offsets and sizes read from a file
are checked against its size, so a corrupted file is rejected instead of
crashing the program.

Supported options:

| Option | Long form | Effect |
|---|---|---|
| `-a` | `--debug-syms` | Display debugger-only symbols |
| `-g` | `--extern-only` | Display only external symbols |
| `-u` | `--undefined-only` | Display only undefined symbols |
| `-p` | `--no-sort` | Do not sort the symbols |
| `-r` | `--reverse-sort` | Reverse the sort order |
| `-h` | `--help` | Display the usage |

If no file is given, `a.out` is read.

## Instructions

### Requirements

- Linux, `make` and a C23 compiler (gcc 13+ or clang 18+)
- For the 32-bit build: the 32-bit glibc headers (`glibc-devel.i686` on
  Fedora, `gcc-multilib` on Debian/Ubuntu)
- Optional: `valgrind` for the memory tests, `doxygen` for the documentation,
  `docker` for the development container

### Compilation

The sources are in the `ft_nm/` directory:

```sh
cd ft_nm
make          # ./ft_nm (64 bits)
make x86      # ./ft_nm32 (32 bits)
make re
make clean
make fclean
```

The root Makefile forwards these targets to `ft_nm/` (`make local`,
`make lre`, `make clean`, `make fclean`).

### Docker

The `Dockerfile` sets up a Fedora image with all the dependencies, including
the 32-bit libraries. From the root of the repository:

```sh
make
```

This builds the image and opens a shell in the container. `ft_nm/` is mounted
on `/ft_nm`, so changes made on the host are visible inside.

### Usage

```sh
./ft_nm [option(s)] [file(s)]

./ft_nm /usr/bin/ls
./ft_nm -g -r libft.a
./ft_nm -u main.o utils.o
```

### Tests

The test targets run `nm` and `ft_nm` on the same files and compare the
outputs and the exit codes. Files go in `T_PROG`, options in `T_ARGS`:

```sh
make test_all  T_PROG="/usr/bin/ls /usr/lib64/libc.so.6"
make test_each T_PROG="main.o libft.a" T_ARGS="-g"
make test_all_valgrind  T_PROG="..."
make test_each_valgrind T_PROG="..."
make test_clean
```

`test_all` runs both programs once on all the files, `test_each` gives a
result per file. The `_valgrind` versions also run `ft_nm` under Valgrind to
check for leaks and open file descriptors.

To test many files at once:

```sh
make test_each T_PROG="$(find /usr/lib64 -maxdepth 1 -name '*.so*' -type f | head -n 50 | tr '\n' ' ')"
```

Outputs, diff and Valgrind report are written to `ft_nm/.test_results/`.

### Documentation

```sh
make doc        # HTML documentation in ft_nm/.docs
make docopen    # open it in the browser
make docclean
```

`make help` lists all the targets.

## Repository layout

```
Dockerfile               development container
Makefile                 Docker targets, forwards the rest to ft_nm/
ft_nm/
    Makefile             build, tests, documentation
    Doxyfile
    include/
    source/
        main.c           entry point, loop over the files
        options/         command line parsing
        file_manip/      opening and mapping of the files
        elf/             ELF checks, symbol reading, type letters
        ar/              archive parsing
        print/           sort, filters, output, error messages
    libs/                allocation wrappers, resource collector
    tests/               comparison scripts against nm
```

## Technical choices

- Files are mapped with `mmap` and read in place. Archive members are read
  directly from the mapping, without copies.
- 32-bit and 64-bit ELF structures are converted to a common format, so the
  type deduction and the output code are written once.
- Inside an archive a member can start at any even offset, so ELF structures
  are copied with `memcpy` before being read, to avoid unaligned accesses.
- A resource collector frees all allocations and mappings at exit, error
  paths included.

## Resources

- `man 1 nm`
- `man 5 elf` and `/usr/include/elf.h`
- `/usr/include/ar.h` for the archive format
- [System V ABI, ELF chapters](https://refspecs.linuxfoundation.org/elf/gabi4+/contents.html)
- [ELF specification v1.2](https://refspecs.linuxfoundation.org/elf/elf.pdf)
- [Oracle Linker and Libraries Guide](https://docs.oracle.com/cd/E23824_01/html/819-0690/index.html)
- Bryant and O'Hallaron, *Computer Systems: A Programmer's Perspective*,
  chapter 7 (Linking)
- John R. Levine, *Linkers and Loaders*
- [binutils source code](https://sourceware.org/git/?p=binutils-gdb.git;a=tree;f=binutils),
  mostly `nm.c` and the way `bfd` classifies symbols
- [Doxygen manual](https://www.doxygen.nl/manual/)
- [Valgrind manual](https://valgrind.org/docs/manual/manual.html)
