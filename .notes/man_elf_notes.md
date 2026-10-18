# man 5 elf

## Introduction
Un exécutable au format ELF contient un header ELF suivi d'une `program header table` ou une `section header table`, ou les deux.

    1/  Le header ELF est toujours à l'offset 0.
    2/  Le header ELF définit les offset de:
            - la `program header table`
            - la `section header table`
        Ces deux tables définissent le reste des particularités du fichier.     

## Types
Les types suivants sont utilisés. `N` prend la valeur `32` ou `64` selon l'architecture.

```c
    ElfN_Addr;       // Unsigned program address, uintN_t
    ElfN_Off;        // Unsigned file offset, uintN_t
    ElfN_Section;    // Unsigned section index, uint16_t
    ElfN_Versym;     // Unsigned version symbol information, uint16_t
    Elf_Byte;        // unsigned char
    ElfN_Half;       // uint16_t
    ElfN_Sword;      // int32_t
    ElfN_Word;       // uint32_t
    ElfN_Sxword;     // int64_t
    ElfN_Xword;      // uint64_t
```

## ELF header (Ehdr)

La structure du header ELF est la suivante (N= 32 ou 64):

```c
    #define EI_NIDENT 16

    typedef struct {
        unsigned char e_ident[EI_NIDENT];
        uint16_t      e_type;
        uint16_t      e_machine;
        uint32_t      e_version;
        ElfN_Addr     e_entry;
        ElfN_Off      e_shoff;=
        ElfN_Off      e_shoff;
        uint32_t      e_flags;
        uint16_t      e_ehsize;
        uint16_t      e_phentsize;
        uint16_t      e_phnum;
        uint16_t      e_shentsize;
        uint16_t      e_shnum;
        uint16_t      e_shstrndx;
    } ElfN_Ehdr;
```

Avec:
- `e_ident`:        Tableau d'octets indiquant comment interpréter le fichier. Il contient le `magic number`, l'endianness, etc. Tout est défini par macros.
- `e_type`:         Identifie le type du fichier objet: relogeable, exécutable, partagé, etc.
- `e_machine`:      Indique l'architecture requise pour le fichier: AMD x86-64, Intel 80860, etc.
- `e_version`:      Identifie la version du fichier.
- `e_entry`:        Spécifie l'adresse virtuelle à laquelle le système démarre le processus. Sans valeur, contient 0.
- `e_phoff`:        Contient l'offset de la `program header table` en octets. Si absente, contient 0.
- `e_shoff`:        Contient l'offset de la `section header table` en octets. Si absente, contient 0.
- `e_ehsize`:       Contient la taille en octets du `ELF header`.
- `e_phentisize`:   Contient la taille en octets des entrées de la `program header table`.
- `e_phnum`:        Contient le nombre d'entrées dans la `program header table`. Par conséquent, `e_phnum * e_phentisize` donne la taille de la `program header table`. Sans `program header table`, contient 0. 
- `e_shentsize`:    Contient la taille en octets des entrées de la `section header table`.
- `e_shnum`:        Ce membre contient le nombre dentrées dans la `section header table`. Par conséquent, `e_shnum * e_shentsize` donne la taille de la `section header table`. Si le fichier n'a pas de `section header table`, contient 0.
- `e_shstrndx`:     Contient l'indice de l'entrée dans la `section header table` associé à la section `section name string table`. Sans `section name string table`, vaut `SHN_UNDEF`.

Notes:

- Si `e_phnum >= PN_XNUM (=0xffff)`, contient 0xffff. Le vrai nombre d'entrées est alors stocké dans `sh_info` de l'entrée initiale de la `section header table`. Sinon `sh_info` vaut 0.
- Si `e_shnum >= SHN_LORESERVE (=0xff00)`, contient 0x0. Le vrai nombre d'entrées est alors stocké dans `sh_size` de l'entrée initiale de la `section header table`. Sinon, `sh_size` vaut 0.
- Si l'indice de la `section name string table` est plus grand ou égal à `SHN_LORESERV (=0xff00)`, `e_shstrndx` vaut `SHN_XINDEX (0xffff)` et le vrai indice est situé dans le membre `sh_link` de l'entrée initiale de la `section header table`. Sinon, `sh_link` vaut 0.

Inutilisés:
    - `e_flags`: Contient des flags spécifiques au processeur. Il n'en existe pas encore.


## Program header (Phdr)

La `program header table` est un tableau de structures, chacune décrivant un segment ou une information dont le système a besoin pour préparer l'exécution du programme.

Un segment contient une ou plusieurs sections.

Les `program header` n'ont de sens que pour les exécutables et les fichiers partagés.

Un fichier indique la taille de son propre `program header` via les membres `e_phentsize` et `e_phnum` du `ELF header`.

Le `program header` est décrit par les types `Elf32_Phdr` et `Elf64_Phdr`, selon l'architecture.

```c
    typedef struct {
        uint32_t   p_type;
        Elf32_Off  p_offset;
        Elf32_Addr p_vaddr;
        Elf32_Addr p_paddr;
        uint32_t   p_filesz;
        uint32_t   p_memsz;
        uint32_t   p_flags;
        uint32_t   p_align;
    } Elf32_Phdr;

    typedef struct {
        uint32_t   p_type;
        uint32_t   p_flags;
        Elf64_Off  p_offset;
        Elf64_Addr p_vaddr;
        Elf64_Addr p_paddr;
        uint64_t   p_filesz;
        uint64_t   p_memsz;
        uint64_t   p_align;
    } Elf64_Phdr;
```

La principale différence est l'emplacement du membre `p_flags`.

On a:
- `p_type`:    Indique le type de segment décrit par l'entrée, ou comment interpréter les informations de l'entrée. Elle fonctionne par macros.
- `p_offset`:  Contient l'offset, à partir du début du fichier, du premier octet du segment.
- `p_vaddr`:   Contient l'adresse virtuelle à laquelle se situe le premier octet du segment.   
- `p_paddr`:   Si le système emploie l'adressage physique, contient l'adresse physique du segment, sinon 0.
- `p_filesz`:  Contient le nombre d'octets dans l'image fichier du segment. Peut être 0.
- `p_memsz`:   Contient le nombre d'octets dans l'image mémoire du segment. Peut être 0.
- `p_flags`:   Contient un masque binaire relatif au segment: exécutable, inscriptible, lisible.
- `p_align`:   Contient la valeur à laquelle les segments sont alignés en mémoire et dans le fichier. C'est l'égalité entre la congruence de `p_vaddr` et `p_offset` par rapport à la `page size`. Si 0, signifie qu'aucun alignement est nécessaire. Sinon, `p_align` doit être positif, entier, puissance de 2 et `p_vaddr = p_offset % p_align`.


## Section header (Shdr)

La `section header table` permet de localiser toutes les sections du fichier.

C'est un tableau de structures `Elf32_Shdr` ou `Elf64_Shdr` selon l'architecture.

Le membre `e_shoff` du `ELF header` indique l'offset depuis le début du fichier de la `section header table`. `e_shnum` contient le nombre d'entrées que le `section header` contient. `e_shentsize` contient la taille de chaque entrée.

Un `section header table entry` est un sous-ensemble de ce tableau. Certains indices sont réservés: l'entrée initiale et les indices compris entre `SHN_LORESERVE` et `SHN_HIRESERVE`. L'`entrée initiale` est utilisée dans les extensions ELF pour `e_phnum`, `sh_num` et `e_shstrndx`. Dans les autres cas, chaque membre dans l'entrée initiale est égal à 0.

Un fichier objet n'a pas de sections pour une série d'indices à consulter dans le `man`.

La `section header` est représentée par les structures suivantes, sans réelles différences l'une avec l'autre:

```c
    typedef struct {
        uint32_t   sh_name;
        uint32_t   sh_type;
        uint32_t   sh_flags;
        Elf32_Addr sh_addr;
        Elf32_Off  sh_offset;
        uint32_t   sh_size;
        uint32_t   sh_link;
        uint32_t   sh_info;
        uint32_t   sh_addralign;
        uint32_t   sh_entsize;
    } Elf32_Shdr;

    typedef struct {
        uint32_t   sh_name;
        uint32_t   sh_type;
        uint64_t   sh_flags;
        Elf64_Addr sh_addr;
        Elf64_Off  sh_offset;
        uint64_t   sh_size;
        uint32_t   sh_link;
        uint32_t   sh_info;
        uint64_t   sh_addralign;
        uint64_t   sh_entsize;
    } Elf64_Shdr;
```

On a:

- `sh_name`:        Contient le nom de la section. C'est un indice dans la section `section header string table`, qui donne l'emplacement d'une `null-terminated string`.
- `sh_type`:        Catégorise le contenu de la section et son sens. Ce sont des macros.
- `sh_flags`:       Les sections gèrent des flags de 1 bit correspondant à des attributs variés. Si le bit est à 1, la valeur pour la section est `on`, sinon `off`. Ce sont des macros.
- `sh_addr`:        Contient le premier octet d'une section, si elle apparaît dans l'image mémoire d'un processus.
- `sh_offset`:      Contient l'offset depuis le début du fichier jusqu'au premier octet de la section. Une section, `SHT_NOBITS`, n'occupe aucun espace dans le fichier, et son membre `sh_offset` repère son emplacement conceptuel.
- `sh_size`:        Contient la taille de la section, en octets. S'il s'agit de `SHT_NOBITS`, une valeur peut être présente bien qu'en réalité aucun espace ne soit occupé.
- `sh_link`:        Contient un lien vers un indice de la `section header table`, dont l'interprétation dépend du type de la section.
- `sh_info`:        Contient des informations supplémentaires, dont l'interprétation dépend de la section.
- `sh_addralign`:   Certaines sections ont des contraintes d'alignement. Si une section contient un `double word`, le système doit assurer un alignement sur un `double word` pour toute la section. Ceci dit, on doit avoir `sh_addr % sh_addralign = 0`. Seulement 0 ou des puissances positives de 2 sont autorisées. Une valeur de 0 ou 1 signifie que la section n'a pas de contrainte d'alignement.
- `sh_entsize`:     Certaines sections contiennent une table d'entrées de taille fixe, telles qu'une `symbol table`. Dans une telle section, ce membre indique la taille en octet de chaque entrée. En l'absence d'une table d'entrées de taille fixe, contient 0.

**Voir man.**
Plusieurs sections contiennent des informations sur le programme et le contrôle.


## String and symbol tables

Les sections `string table` contiennent des `C-strings`, que le fichier objet utilise pour représenter les symboles et les noms des sections.

On fait références à ces `strings` via un indice dans la `string table section`.

Le premier octet, qui est [0], est défini pour contenir un `null byte`, tout comme le dernier octet de la section qui permet de s'assurer de la bonne complétion des `strings` de la section.

La `symbol table` d'un fichier objet contient les informations nécessaires pour loger et reloger les définitions et références symboliques d'un programme.

Un indice dans la `symbol table` est un sous-ensemble de ce tableau.

Les structures sont les suivantes. Entre 32 et 64 bits, seul l'ordre change:

```c
    typedef struct {
        uint32_t      st_name;
        Elf32_Addr    st_value;
        uint32_t      st_size;
        unsigned char st_info;
        unsigned char st_other;
        uint16_t      st_shndx;
    } Elf32_Sym;

    typedef struct {
        uint32_t      st_name;
        unsigned char st_info;
        unsigned char st_other;
        uint16_t      st_shndx;
        Elf64_Addr    st_value;
        uint64_t      st_size;
    } Elf64_Sym;
```

On a:

- `st_name`:    Contient un indice dans la `symbol string table`. Le symbole peut ne pas avoir de nom.
- `st_value`:   Donne la valeur du symbole.
- `st_size`:    Plusieurs symboles ont une taille qui leur est associée. Contient 0 si le symbole n'a pas de taille ou qu'elle est inconnue.
- `st_info`:    Spécifie le type du symbole et ses attributs de binding. Ce sont des macros.
- `st_other`:   Définit la visibilité du symbole. Ce sont des macros.
- `st_shndx`:   Chaque entrée de la `symbol table` est défini relativement à une section. Contient l'indice de la `section header` correspondante.


## Relocation entries (Rel & Rela)

Le relogement consiste à connecter des références symboliques à des définitions symboliques.

Les fichiers relogeables doivent contenir les informations qui décrivent comment modifier le contenu de leurs sections, et par conséquent permettre aux exécutables et aux fichiers partagés d'avoir les bonnes informations liées à l'image d'un processus. Les `relocation entries` sont ces données.

Structures des relogements qui n'ont pas besoin d'un `addend`:

```c
    typedef struct {
        Elf32_Addr r_offset;
        uint32_t   r_info;
    } Elf32_Rel;

    typedef struct {
        Elf64_Addr r_offset;
        uint64_t   r_info;
    } Elf64_Rel;
```

Structures des relogements qui ont besoin d'un `addend`:

```c
    typedef struct {
        Elf32_Addr r_offset;
        uint32_t   r_info;
        int32_t    r_addend;
    } Elf32_Rela;

    typedef struct {
        Elf64_Addr r_offset;
        uint64_t   r_info;
        int64_t    r_addend;
    } Elf64_Rela;
```

On a:
- `r_offset`:   Indique l'emplacement où appliquer le relogement. Pour un fichier relogeable, c'est l'offset en octets à partir du début de la section jusqu'à l'unité de stockage du relogement. Pour un fichier exécutable ou un fichier partagé, c'est l'adresse virtuelle de l'unité de stockage du relogement.
- `r_info`:     Donne à la fois l'index de la `symbol table`par rapport auquel la relocalisation doit être effectuée et le type de relocalisation à appliquer. Lorsque le texte fait référence au type de relogement d'une `relocation entry` ou à son index dans la table des symboles, il s'agit du résultat de l'application, respectivement, de `ELF[32|64]_R_TYPE` ou `ELF[32|64]_R_SYM` au membre `r_info` de l'entrée.`
- `r_addend`:   Ce membre spécifie une constante `addend` utilisée pour le calcul de la valeur à stocker dans le champ du relogement.


## Dynamic tags (Dyn)

La section `.dynamic` contient une série de structures qui contiennent des informations relatives au `dynamic linking`.

```c
    typedef struct {
        Elf32_Sword    d_tag;           // Contrôle l'interprétation de d_un
        union {
            Elf32_Word d_val;
            Elf32_Addr d_ptr;
        } d_un;
    } Elf32_Dyn;
    extern Elf32_Dyn _DYNAMIC[];

    typedef struct {
        Elf64_Sxword    d_tag;           // Contrôle l'interprétation de d_un
        union {
            Elf64_Xword d_val;
            Elf64_Addr  d_ptr;
        } d_un;
    } Elf64_Dyn;
    extern Elf64_Dyn _DYNAMIC[];
```

On a:

- `d_tag`:      Ce sont des macros.
- `d_val`:      Représente des valeurs entières avec des interprétations variées.
- `d_ptr`:      Représente les adresses vituelles du programme. En interprétant ces adresses, la vraie adresse doit être calculée à partir de la valeur du fichier original et de l'adresse de base de la mémoire. Les fichiers ne contiennent pas de `relocation entries` pour corriger ces adresses.
- `_DYNAMIC`:   Tableau contenant toutes les structures dynamiques de la section `.dynamic`. Il est automatiquement peuplé par le linker.

## Notes (Nhdr)

