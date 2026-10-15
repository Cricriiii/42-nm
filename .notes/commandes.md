readelf -h a.out    # regarde la ligne "Section header string table index"
readelf -S a.out    # toutes les sections, avec les vrais index


┌───────────────────────────────────────┬───────────────────┬─────────────────────────────────────────┐
│                 Champ                 │  C'est un index…  │              …dans quoi ?               │
├───────────────────────────────────────┼───────────────────┼─────────────────────────────────────────┤
│ e_shstrndx                            │ de section header │ le tableau s[] des section headers      │
├───────────────────────────────────────┼───────────────────┼─────────────────────────────────────────┤
│ sh_offset (de cette section)          │ en octets         │ depuis le début du fichier              │
├───────────────────────────────────────┼───────────────────┼─────────────────────────────────────────┤
│ sh_name (de n'importe quelle section) │ en octets         │ depuis le début de la section .shstrtab │
└───────────────────────────────────────┴───────────────────┴─────────────────────────────────────────┘