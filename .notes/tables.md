Voici le schéma, avec les vraies valeurs de ton a.out.

ELF HEADER
  e_shoff    = 0x3658  ──────────────┐
  e_shnum    = 30                    │
  e_shstrndx = 29  ────────────────┐ │
                                   │ │
                                   │ ▼
SECTION HEADER TABLE (à l'offset 0x3658, 30 entrées de 64 octets)
 ┌────┬──────────┬─────────┬──────────┬────────┬─────────┐
 │ i  │ sh_name  │ sh_type │ sh_offset│sh_link │ sh_info │
 ├────┼──────────┼─────────┼──────────┼────────┼─────────┤
 │ .. │   ...    │   ...   │   ...    │        │         │
 │  5 │ ".dynsym"│ DYNSYM  │  0x3d8   │   6 ───┼────┐    │
 │  6 │ ".dynstr"│ STRTAB  │  0x468   │   0    │    │    │  ◄─┐
 │ .. │   ...    │   ...   │   ...    │        │    │    │    │
 │ 27 │ ".symtab"│ SYMTAB  │  0x3030  │  28 ───┼──┐ │    │    │
 │ 28 │ ".strtab"│ STRTAB  │  0x3378  │   0    │  │ │    │  ◄─┼─┐
 │ 29 │".shstrtab"│ STRTAB │  0x3541  │   0    │  │ │    │    │ │
 └────┴──────────┴─────────┴──────────┴────────┴──┼─┼────┘    │ │
        ▲                          ▲              │ └─────────┘ │
        │                          │              └─────────────┘
        │                          └── e_shstrndx (29) pointe ici ─┐
        │                                                          │
        └── sh_name = offset dans la table 29 ◄────────────────────┘

Les trois chaînes de liens :

1. NOMS DE SECTIONS
   shdr[i].sh_name ──► octets de la section 29 (.shstrtab)   ex: "…\0.symtab\0…"
                       (index 29 vient de e_shstrndx)

2. NOMS DES SYMBOLES DE .symtab
   Elf64_Sym.st_name ──► octets de la section 28 (.strtab)   ex: "…\0main\0…"
                         (index 28 vient de shdr[27].sh_link)

3. NOMS DES SYMBOLES DE .dynsym
   Elf64_Sym.st_name ──► octets de la section 6 (.dynstr)    ex: "…\0printf\0…"
                         (index 6 vient de shdr[5].sh_link)

Lecture d'un symbole, pas à pas :

symtab = shdr[27]      (trouvée car sh_type == SHT_SYMTAB)
strtab = shdr[28]      (= shdr[symtab.sh_link])
n      = 0x348 / 0x18 = 35 symboles

symbole k :
  sym  = base + 0x3030 + k*24          ← symtab.sh_offset
  nom  = base + 0x3378 + sym.st_name   ← strtab.sh_offset

Il y a deux niveaux de pointeurs :
- sh_link donne un numéro de section, c'est-à-dire un index dans la section header table.
- st_name et sh_name donnent un offset en octets à l'intérieur de la table de chaînes.