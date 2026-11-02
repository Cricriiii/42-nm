# Audit de ft_nm

Date : 2026-11-02 · Branche `archive` · mis à jour après correction des points 1.2, 1.3, 1.4, 2, 3 et 4
Chemins relatifs à `ft_nm/` (les sources sont dans `ft_nm/ft_nm/`). Les numéros de ligne sont ceux du code à la date de la mise à jour.

**Légende :** ✅ corrigé et testé · ⚠️ corrigé en partie · ⛔ volontairement pas reproduit · ❔ non traité

**Méthode.** Lecture complète de `main.c`, `ar/`, `elf/`, `file_manip/`, `options/`, `print/display_file.c` et `print/print_symbols.c`. Je n'ai pas relu `libs/source/resource_collector.c`, `libs/source/xalloc.c` ni `print.c` en entier. Les corrections ont été vérifiées avec un binaire compilé en `-fsanitize=address,undefined` et par comparaison de la sortie avec `nm` (voir §7).

---

## 1. Bugs

| # | Problème | État | Où c'est corrigé |
|---|---|---|---|
| 1.1 | Dépassement de `buf` pour les symboles `U` | ✅ | `source/print/print_symbols.c:86-93` : le tampon et les `sprintf` sont remplacés par un `printf` direct (`%*s` pour les indéfinis, `%.*lx` sinon). Vérifié d'abord par la trace ASan que tu as collée (écriture de 29 octets dans un buffer de 28). |
| 1.2 | Fichier vide ou répertoire : le programme s'arrêtait | ✅ | `source/file_manip/file_manip.c:61-81` (`map_file`) : répertoire → `print_warning_directory()` (`source/print/print.c:103`), taille 0 → refus silencieux, échec de `mmap` → `print_perror()`. `source/main.c:61-65` : `continue` au lieu de `exit`. |
| 1.3 | Lectures mal alignées (archives) | ✅ | `source/elf/elf_to_generic.c` : `memcpy` aux lignes 26, 62 (en-tête ELF), 79, 113 (section), 130, 160 (symbole). |
| 1.4 | Archives (liste ci-dessous) | ✅ sauf une ligne | `source/ar/read_ar.c` |

### 1.4 en détail

| Défaut | État | Correction |
|---|---|---|
| Alignement sur 2 octets absent | ✅ | `read_ar.c:218` : `ofst += ofst & 1;` |
| `ar_size` négatif → boucle infinie | ✅ | `_parse_ar_size`, `read_ar.c:50` : chiffres et espaces seulement ; sinon « malformed archive » (`read_ar.c:178-179`) |
| En-tête complet lu avant de vérifier qu'il tient | ✅ | `read_ar.c:171-173` (en-tête) et `read_ar.c:182-184` (membre) : `AR_TRUNCATED_RET` |
| Signature de fin d'en-tête non vérifiée | ✅ | `read_ar.c:177` (`ar_is_fmag`) |
| Nom non initialisé | ✅ | `read_ar.c:120-121` : `xcalloc(len + 1)` puis `memcpy`, libéré après usage |
| `strlen` sur un champ non terminé, tableau de taille variable | ✅ | `read_ar.c:110-117` : `memchr` borné à 16 octets ou à la taille de la table, et plus de tableau de taille variable |
| Nom `/123` sans table, ou indice hors table | ✅ | `read_ar.c:99-105` : « malformed archive » |
| Taille du membre = fin de l'archive | ✅ | `read_ar.c:119` : `cur_memory.size = member_size` |
| Erreur de format perdue (`retval` réécrit, no-op) | ✅ | Un membre non ELF est signalé sous son nom (`read_ar.c:124-133`) et ignoré, sans changer le code de retour (comme `nm`) |
| Codes de retour négatifs (254, 255) | ✅ | `read_ar.c:227-235` : 1 en cas d'erreur, 0 sinon |
| Table `/SYM64/` | ✅ | `read_ar.c:33` |
| Archives « thin » (`!<thin>`), format BSD (`__.SYMDEF`) | ❔ | Non traité |

Fichiers annexes : `include/nm_ar.h:20` (`AR_TRUNCATED_RET`), `source/elf/elf_format.c:152-154` (refus d'un fichier de moins de `EI_NIDENT` octets), `source/main.c:73` (`mapped.size >= SARMAG` avant de tester la signature).

---

## 2. Lectures hors mapping sur des ELF corrompus — ✅

`source/elf/read_elfN.c` ne lit plus rien que le fichier ne contient. `read_elfN` reçoit maintenant la taille (`include/nm_elf.h:136`, `source/print/display_file.c:48`).

| Valeur lue | État | Correction |
|---|---|---|
| `sh_entsize` du `.symtab` (division par zéro) | ✅ | `read_elfN.c:99` : un `.symtab` dont `sh_entsize` n'est pas la taille d'un symbole est ignoré. Le nombre de symboles se calcule avec `sizeof` du symbole. |
| `sh_offset` + `sh_size` du `.symtab` | ✅ | `section_in_file`, `read_elfN.c:35`, appelé ligne 100 |
| `sh_link` | ✅ | `read_elfN.c` (dans `get_symbol_sections`) : testé contre `shnum` avant lecture |
| `e_shstrndx` | ✅ | Plus d'adresse arbitraire : une table des noms invalide donne `<corrupt>` (voir ci-dessous) |
| `st_name`, `sh_name` | ✅ | `elf_str_at`, `read_elfN.c:50-60` : table dans le fichier, offset dans la table, `\0` avant la fin ; sinon `<corrupt>` (`read_elfN.c:25`). Utilisé lignes 206 et 211, et `source/elf/type_deduction.c:160` |
| `st_shndx` d'un `STT_SECTION` | ✅ | `read_elfN.c:201-207` : hors table → `<corrupt>` |
| `st_shndx` pour la valeur `ET_REL` | ✅ | `read_elfN.c:217-218` : ajout de `st_shndx < shnum` |
| Retours ignorés de `Ehdr_to_generic` | ✅ | Branche morte supprimée (`== -1` n'arrivait jamais) |
| Numérotation étendue des sections (`e_shnum == 0`) | ✅ | `read_elfN.c:153-162` : nombre de sections lu dans la section 0 |
| `SHN_XINDEX` sur `e_shstrndx` | ✅ | `read_elfN.c:159` : indice lu dans `sh_link` de la section 0 |
| `SHN_XINDEX` sur `st_shndx` d'un symbole | ❔ | Non géré |

`get_symbol_type` prend aussi la taille du fichier (`source/elf/type_deduction.c:110`).

---

## 3. Écarts de comportement avec `nm`

| Écart | État | Détail |
|---|---|---|
| Code de retour = dernier fichier seulement | ✅ | `source/main.c:70-80` : 1 dès qu'un fichier échoue |
| Code 127 pour `EMFILE`/`ENFILE` | ✅ | Supprimé (`source/main.c:51-55`) |
| Symboles `N` masqués sans `-a` | ✅ | `source/print/print_symbols.c:80` : plus de test sur `'N'` |
| Symboles `a` masqués sans `-a` | ✅ | Seuls les `STT_FILE` sont masqués, par la propriété `file` (`include/nm_elf.h:115`, `source/elf/read_elfN.c:195`, `source/print/print_symbols.c:80`). Vérifié sur un `.o` construit à la main. |
| Valeur d'un symbole commun (`C`) : `nm` affiche `st_size` | ✅ | `source/elf/read_elfN.c:217-220`. Trouvé par la série de tests sur `libcrypto.a`. |
| `sh_addr` ajouté à la valeur d'un symbole `ET_REL` | ✅ | `read_elfN.c:216-223`. Tu m'avais dit l'avoir retiré, le code est pourtant dans le fichier. À vérifier avec `git diff`. |
| Message d'un fichier inexistant | ✅ | `source/print/print.c:91-93` : `prog: 'fichier': No such file` |
| Archive rejetée dès le premier membre : en-tête affiché à tort | ✅ | `source/ar/read_ar.c:164-190` et `221-223` : en-tête affiché après validation du premier membre |
| Objets LTO | ⛔ | `nm` passe par `liblto_plugin.so`. Pas un bug de `ft_nm`. |
| Tri des noms identiques | ✅ | Testé avec des noms dupliqués : sortie identique à `nm` |
| Archive avec `ar_size` négatif / mauvaise signature de fin d'en-tête | ⛔ | `nm` les tolère (comportement permissif de BFD). `ft_nm` les rejette : « malformed archive », code 1. |
| Fichier illisible (permission refusée) | ❔ | Pas pu comparer avec un fichier non vide |

---

## 4. Qualité et robustesse

| Point | État | Détail |
|---|---|---|
| `MAP_SHARED` | ✅ | `source/file_manip/file_manip.c:77` : `MAP_PRIVATE` |
| `nm_args_t` dupliqué | ✅ | Un seul `extern nm_args_t options;` dans `include/options.h:26` |
| Code mort | ✅ | `read_elf_ident` / `elf_ei_t` et la variable `dynamic` supprimés (`source/elf/read_elfN.c`, `include/nm_elf.h`) |
| `arch >> 2` illisible | ✅ | Macro `arch_hex_width`, `include/nm_elf.h:51`, utilisée `print_symbols.c:86` |
| Commentaire obsolète « Suppressed… » | ✅ | Supprimé |
| `arch_global` global | ❔ | Fonctionne parce que tout est séquentiel. Pas modifié. |
| Fuites | ✅ | Valgrind sur `crt1.o`, `crtavr64du32.o`, `odd.a`, `ab.o` : aucune erreur ni fuite. `make test_all_valgrind` échoue tant que le Makefile compile avec ASan (incompatible avec Valgrind), j'ai donc utilisé un binaire sans sanitizer. |

---

## 5. État du projet

- Tout ce qui est marqué ✅ a été testé (voir §7).
- Reste à décider : archives « thin » et format BSD (§1.4), `SHN_XINDEX` sur les symboles (§2), fichier illisible (§3), `arch_global` (§4).
- À vérifier par toi : `make test_all_valgrind` ne marche plus avec `-fsanitize` dans le Makefile.

---

## 6. Rappel des commandes

```sh
# Compiler un binaire avec sanitizers (déjà dans ton Makefile)
make lre
# Comparer avec nm
make test_all T_PROG=/usr/lib/x86_64-linux-gnu/crt1.o
make test_all T_PROG=/usr/lib/x86_64-linux-gnu/libc.a T_ARGS=-a
```

---

## 7. Résultats des tests

**Série complète** : tous les fichiers ELF et archives de `/usr/lib`, `/usr/bin` et `/usr/libexec` (identifiés par `file`), avec les options par défaut, `-a`, `-g`, `-u`, `-r` et `-p`, comparés à `nm` sur la sortie standard, la sortie d'erreur et le code de retour. Binaire compilé avec `-fsanitize=address,undefined`.

- **62 526 exécutions, 11 écarts**, tous sur deux fichiers :
  - `python.o` (objet LTO) : écart attendu, voir §3 (plugin de `nm`). Pas un bug de `ft_nm`.
  - `libcrypto.a` : **bug trouvé par ce test, corrigé** (voir ci-dessous).
- Aucune erreur ASan ni UBSan sur l'ensemble.

**Bug trouvé par la série : symboles communs (`C`).** `nm` affiche la taille (`st_size`) d'un symbole commun, pas `st_value` (qui est l'alignement). Sur `libcrypto.a`, `OPENSSL_ia32cap_P` valait `0x4` chez `ft_nm` et `0x28` chez `nm`. Corrigé dans `source/elf/read_elfN.c:217-220`. Le petit `.o` de test, qui avait un symbole commun de 4 octets aligné sur 4, ne le révélait pas.

**Après la correction** : nouvelle série sur tous les `.o` et `.a` de `/usr/lib` et `/usr/bin` avec les options par défaut, `-a` et `-u` : **9 300 exécutions, 3 écarts**, uniquement `python.o` (LTO).

**Autres vérifications faites pendant les corrections :**
- Fuzz des en-têtes ELF : 4 500 fichiers dérivés de `crt1.o`, `crtavr64du32.o` et `python.o`. Aucune erreur de sanitizer, aucun crash.
- Fuzz de `.symtab` et `.strtab` : 6 000 exécutions (avec et sans `-a`). Même résultat.
- Archives fabriquées : membre de taille impaire, nom long, membre non ELF, archive vide, archive sans membre ELF, archive tronquée. Sortie et code de retour identiques à `nm`.
- Valgrind (binaire sans sanitizer) : aucune erreur ni fuite sur `crt1.o`, `crtavr64du32.o`, `odd.a`, `ab.o`.

**Écarts connus qui ne sont pas des bugs :** voir les lignes ⛔ du §3 (LTO, archives avec `ar_size` négatif).

