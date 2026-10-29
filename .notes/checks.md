Ok, il s'agit de valider le fichier ELF en entrée, pas le code source. L'objectif est que ft_nm ne plante jamais sur un fichier corrompu, tronqué ou qui n'est pas de l'ELF. Chaque offset et chaque taille lus dans le fichier sont à vérifier avant d'être utilisés.

1. Le fichier lui-même

- open et fstat doivent réussir. Sinon, message du type nm: 'x': No such file.
- Le fichier doit être régulier. Un répertoire doit donner une erreur.
- La taille doit être au moins sizeof(Elf64_Ehdr) (ou Elf32_Ehdr). Sinon, c'est file format not recognized.
- mmap doit réussir. Garde la taille du fichier : toutes les vérifications suivantes se font par rapport à elle.

2. L'en-tête (e_ident et Ehdr)

- Magic : e_ident[0..3] == 0x7f 'E' 'L' 'F'.
- EI_CLASS vaut ELFCLASS32 ou ELFCLASS64. Toute autre valeur est invalide.
- EI_DATA vaut ELFDATA2LSB ou ELFDATA2MSB. Si l'endianness du fichier diffère de celle de ta machine, il faut byte-swapper tous les champs lus.
- EI_VERSION == EV_CURRENT et e_version == EV_CURRENT.
- e_ehsize == sizeof(Ehdr) et e_shentsize == sizeof(Shdr).
- e_type doit être plausible (ET_REL, ET_EXEC, ET_DYN…).

3. La table des sections

- Calcule e_shoff + e_shnum * e_shentsize sans overflow et vérifie qu'il est <= taille du fichier.
- Cas étendus : si e_shnum == 0 et e_shoff != 0, le vrai nombre de sections est dans sh_size de la section 0. Si e_shstrndx == SHN_XINDEX (0xffff), l'index est dans sh_link de la section 0.
- e_shstrndx doit être < e_shnum.
- Pour chaque section, sauf SHT_NOBITS, sh_offset + sh_size <= taille du fichier, toujours sans overflow.
- sh_link et sh_info doivent être des index valides quand ils sont utilisés.

4. La table des symboles

- Trouve la section SHT_SYMTAB. Si elle n'existe pas, c'est no symbols.
- sh_entsize == sizeof(Sym) et sh_size % sh_entsize == 0.
- sh_link doit pointer vers une section de type SHT_STRTAB, elle-même dans les bornes du fichier.
- Pour chaque symbole :
  - st_name < taille de strtab.
  - La chaîne est terminée par un \0 dans strtab. Utilise memchr borné, pas strlen.
  - st_shndx est soit un index < e_shnum, soit une valeur spéciale (SHN_UNDEF, SHN_ABS, SHN_COMMON, SHN_XINDEX).

5. Les noms de sections

Le calcul des lettres (N pour .debug*, etc.) utilise shstrtab. Il faut donc vérifier aussi que sh_name est dans les bornes et que la chaîne est terminée.

6. Comportement attendu

- En cas d'erreur, affiche un message sur stderr (nm: file: file format not recognized), puis passe au fichier suivant.
- Sans argument, utilise a.out.
- Avec plusieurs fichiers, affiche l'en-tête \nfile: avant chaque liste.
- Code de retour non nul si au moins une erreur est survenue.
- Aucun segfault, aucune lecture hors du mmap, aucun munmap oublié.

Comment tester

head -c 40 a.out > trunc.o            # tronqué dans l'Ehdr
head -c 2000 a.out > trunc2.o         # tronqué dans les sections
printf '\x7fELF' > magic_only.o
cp a.out bad.o && printf '\xff\xff\xff\xff' | dd of=bad.o bs=1 seek=40 conv=notrunc  # e_shoff cassé
gcc -m32 -c x.c -o x32.o              # 32 bits
echo hello > text.o                   # pas de l'ELF
Compare avec nm sur chacun de ces fichiers. Un petit fuzzer qui corrompt des octets au hasard, lancé sous valgrind ou -fsanitize=address, trouve vite les oublis de vérification.

Si tu veux, je regarde ton ft_nm/source pour repérer les endroits où ces vérifications manquent.