
1. Rejet : « file format not recognized », code de retour 1

- Fichier trop court : 3 ou 16 octets (moins que e_ident).
- Mauvais magic (\x7fELF).
- EI_VERSION différent de 1.
- e_shnum == 0 avec e_shoff != 0.
- e_phoff énorme, hors du fichier.
- .symtab avec sh_entsize ≠ sizeof(ElfN_Sym) (0 ou 8 testés).
- .symtab avec sh_link hors des sections (99 testé).

2. Pas de rejet, mais « no symbols » (message sur stderr)

Le fichier est accepté comme ELF, et il n'a simplement rien à afficher. Le code de retour mesuré est 0.
- e_shoff énorme (table hors du fichier).
- e_shnum trop grand (255 testé).
- e_shentsize invalide (16 testé).
- e_shstrndx invalide : message warning: ... corrupt string table index.
- EI_CLASS ou EI_DATA invalides (0 et 3 testés).
- Fichiers tronqués : 52, 63, 64 octets, tronqué d'un octet, tronqué de moitié. Je n'ai pas mesuré le code de retour de ces cas.
- Symbole avec st_shndx = SHN_XINDEX sans table SYMTAB_SHNDX.
- Section .symtab avec sh_type bogus (99 testé).

3. Acceptation, avec avertissement, la sortie continue

- Section qui dépasse la fin du fichier (.symtab ou .strtab) : warning: ... section extending past end of file. Pour une .symtab, il y a « no symbols », et pour une .strtab, les noms sont <corrupt>.
- sh_link de .symtab pointant sur une section non-chaîne (0 ou .text) : attempt to load strings from a non-string section, les noms sont <corrupt>.
- .strtab de taille 0 : noms <corrupt>, sans message.
- st_name hors de la table de chaînes : invalid string offset N >= taille.
- .strtab sans \0 final : string table [10] is corrupt, mais les noms s'affichent.

4. Acceptation silencieuse, comportement normal

- e_type inconnu (9 testé).
- e_machine inconnu.
- e_version ≠ 1 (le champ du header, pas EI_VERSION).
- e_ehsize faux (16 testé).
- sh_size de .symtab non multiple de sh_entsize.
- st_shndx hors plage (999 testé).
- e_phnum énorme : un simple warning sur l'alignement des segments.

Pour ft_nm

1. Le rejet est étroit. BFD ne rejette que ce qui empêche d'identifier ou de parcourir le fichier. La plupart des incohérences sont tolérées et ne donnent qu'un avertissement ou « no symbols ». Comme ton test_each compare stdout, tu peux te limiter à ces cas : si tu rejettes plus que nm, tu perds de la sortie qu'il aurait affichée.
2. Ton read_elfN ne doit jamais lire hors du mapping. Quand un champ est incohérent, nm n'affiche rien ou affiche <corrupt>, et ne plante pas. C'est ce que tes vérifications doivent garantir.
3. « no symbols » n'est pas un rejet : le code de retour est 0 et le message est sur stderr.

Les scripts de test sont dans /tmp/claude-1000/-home-fox-Work-ft-nm/b7a193cf-fb1c-49d5-a73f-7eb5f4f5b237/scratchpad/r.sh et r2.sh. Ils partent d'un t.o du même dossier. Je peux en tirer un jeu de fichiers cassés dans .tests/ si tu veux automatiser la comparaison.
