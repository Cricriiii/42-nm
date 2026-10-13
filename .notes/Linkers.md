# Linking

## Fichiers objets

Il y a 3 types de fichiers objets:
 - `relogeables`: contiennent du code binaire et des données sous une forme qui peut être combinée avec d'autres fichiers objets relogeables, à la compilation ou à l'exécution.
 - `exécutables`: contiennent du code binaire et des données sous une forme qui peut être directement copiée en mémoire et exécutée
 - `partagés`: type spécial d'objet relogeable qui peut être chargé en mémoire et liée dynamiquement, au chargement ou à l'exécution
 
Les `compilers` et les `assemblers` génèrent des fichiers objets `relogeables` (ceci incluant les fichiers objets partagés). Les `linkers` génèrent des objets `exécutables`.

Sous Linux, le format historique `a.out` a été supplanté par le format `ELF` (pour Executable and Linkable Format).

## ELF Format

|Section|Description|
|:---:|---|
|ELF HEADER|Commence à l'adresse `0x0` dans le cas d'un fichier `relogeable`. L'entête débute par une séquence de 16 octets décrivant la taille d'un `word` et l'`endianness` du système qui a généré le fichier. La suite contient des informations qui permettront au `linker` de parser et interpréter le fichier objet: la taille de l'entête ELF, le type de l'objet (`relogeable`, `exécutable`, `partagé`), le type de la machine (ex: `x86-64`), le décalage de la `section header table`.|
|.text|Code machine du programme compilé.|
|.rodata|Données en lecture seule comme les `string` et les `jump table` des `switch`.|
|.data|Variables C `global` et `static` initialisées. Les variables `local` n'y apparaissent pas.|
|.bss|Variables C `global` et `static` non initialisées ainsi que les variables `global` ou `static` initialiées à 0. Cette section n'occupe aucun véritable espace dans le fichier objet, et sert à peine de `placeholder`. Les formats de fichiers objets distinguent les variables initialisées et non initialisées pour des questions d'optimisation de taille. Les variables non initialisées n'ont aucune place à occuper et sont allouées à l'exécution avec une valeur initiale de 0. L'acronyme `bss` provient d'une directive IBM de 1957, `block started by symbol`.|
|.symtab|Table des informations à propos des fonctions et des variables `global` qui sont définies et référencées dans le programme. **De nombreux programmeurs pensent à tort qu'un programme doit être compilé avec l'option `-g` pour obtenir cette table, alors qu'en réalité, tout objet `relogeable` a son symbole dans `.symtab` (à moins que le programmeur ait volontairement supprimé la table avec la commande `STRIP`)**. A l'inverse de la table des symboles à l'intérieur du compilateur, les variables `local` n'apparaissent pas dans cette section.|
|.rel.text|Liste des emplacements dans la section `.text` qui seront à modifier par le `linker` lorsqu'il liera le fichier objet concerné avec tous les autres. En principe, toute instruction qui `call` une fonction externe ou fait référence à une variable `global` doit être modifiée. Autrement, les instructions qui `call` des fonctions `local` n'ont pas besoin d'être modifiées. A noter également, les informations de relogement ne sont pas nécessaires pour les objets `executables` et sont donc omise, à moins que l'utilisateur ait explicitement demandé au `linker` de les inclure.|
|.rel.data|Informations de relogement de toute variable `global` qui référencée ou définie dans le programme. En principe, toute variabie `global` initialisée, dont la valeur initiale est l'adresse d'une variable `global` ou d'une fonction `extern` est à modifier.|
|.debug|Une table des symboles de débogage, avec des entrées relatives aux variables `locales`, daux `typedef`, aux variables `global` définies et référencées dans le programme, ainsi que le code C source. Cette table n'existe qu'avec l'option `-g`.|
|.line|La correspondance entre les lignes du code C source et les codes d'instruction machine de la section `.text`. Cette table n'existe qu'avec l'option `-g`.|
|.strtab|Une table de `string` correspondant aux tables des symboles des sections `.symtab` et `.debug` et pour les noms des titres de sections. Cette section est une séquence de `null-terminated strings`, vers le début desquelles pointent chaque entrée des tables `.symtab` et `.debug`.|
|Section header table|Décrit les sections des fichiers objets.|
||

### Symboles du linker

Dans le contexte d'un `linker`, il existe trois types de symboles associés à un module `m`: 
    - Les symboles `global` définis par `m` et qui peuvent être référencés par les autres modules. Il s'agit des symboles relatifs aux fonctions C `nonstatic` et aux variables `global`.
    - Les symboles `global` référencés par `m` et définis par les autres modules. Ce sont les symboles `external` qui correspondent à des fonctions C `nonstatic` et des varibales `global` définies dans d'autres modules.
    - Les symboles `local` qui sont définis et référencés exclusivement par le module `m`. Ils correspondent aux fonctions C `static` et aux variables `global` qui sont définies avec l'attribut `static`. Ces symboles sont uniquement visibles dans le module `m`.

> Il faut faire la différence entre les syumboles `local` au sens du `linker` et les variables `local` d'un programme.
> La table des symboles `.symtab` ne contient aucun symbole relatif à une variable `local` et `nonstatic` d'un programme. Ces dernières sont générées à l'exécution sur la `stack` et sont sans intérêt pour le `linker`.

### Cas des variables `local` et `static`

En ce qui concerne les variables `local` mais `static` d'une fonction ne sont pas gérées sur la `stack`. Le compilateur alloue un espace dans `.data` ou `.bss` pour chaque définition et crée un symbole de liaison `local` avec un nom unique.

Exemple:

```c
int f() {
    static int x = 0;
    return x;
}

int g() {
    static int x = 1;
    return x;
}
```
Dans ce cas le compilateur exporte deux symboles locaux avec des noms différents. Par exemple `x.1` et `x.2`.

### ELF `.symtab` et les `table entries`

Ces symboles font partie de la table des symboles. Cette table contient des entrées sous la forme suivante: 

```c
typedef struct {
    int     name;       /* String table offset */
    char    type:4,     /* Function or data */
            binding:4;  /* Local or global */
    char    reserved;   /* Unused */
    short   section;    /* Section header index */
    long    value;      /* Section offset or absolute address */
    long    size;       /* Object size in bytes */
} Elf64_Symbol;
```

Les champs se comprennent ainsi:
- `name`: offset en octet dans `.strtab`, vers le nom `null-terminated` du symbole.
- `value`: adresse du symbole. Dans le cas d'un objet `relogeable`, `value` est l'offset à partir du début de la section où l'objet est défini. Dans le cas d'un objet `exécutable`, c'est la valeur absolue à l'exécution.
- `size`: taille en octets de l'objet.
- `type`: type de l'objet: `data` ou `function`.
- `binding`: indique si le symbole est `local` ou `global`.
- `section`: assignation à une section du fichier objet. Il s'agit d'un indice dans la `section header table`.

La table des symboles peut aussi contenir des entrées pour les sections individuelles et le chemin du fichier source original:

```sh
// main.c
int count = 5;
int main() { return count; }

gcc -c main.c
readelf -s main.o

Sortie typique (simplifiée) :

Num:  Value  Size  Type     Bind    Ndx  Name
  0:  0      0     NOTYPE   LOCAL   UND
  1:  0      0     FILE     LOCAL   ABS  main.c       ← chemin du fichier source
  2:  0      0     SECTION  LOCAL   1    (.text)      ← symbole de section
  3:  0      0     SECTION  LOCAL   2    (.data)
  4:  0      0     SECTION  LOCAL   3    (.bss)
  5:  0      4     OBJECT   GLOBAL  2    count        ← vrai symbole (variable)
  6:  0      11    FUNC     GLOBAL  1    main         ← vrai symbole (fonction)
```

### Pseudosections sans entrée dans la `section header table`:
- `ABS`: symboles qui ne doivent pas être relogés.
- `UNDEF`: symboles indéfinis, c'est-à-dire des ssymboles référencés dans le module courant mais définis ailleurs.
- `COMMON`: représente les objets non initialisés et non encore alloués. Dans ce cas, le champ `value` précise l'alignement nécessaire, et `size` précise la taille minimum.

> Ces pseudosections n'existent que dans les fichiers objets `relogeables`, pas dans les fichiers `exécutables`.

### Distinction entre `COMMON` et `.bss`

La distinction est subtile:

- `COMMON`: variables `global` non initialisées.
- `.bss`: variables `static` non initialisées, et variables `global` ou `static` initialisées à 0.


### Practice problem 7.1

Soit le code suivant, remplir le tableau pour le fichier objet `swap.o`.

```c
/* m.c */
void swap();

int buf[2] = {1, 2};

int main() {
    swap();
    return 0;
}

/* swap.c */
extern int buf[];

int* bufp0 = &buf[0];
int* bufp1;

void swap() {
    int temp;

    bufp1 = &buf[1];
    temp = *bufp0;
    *bufp0 = *bufp1;
    *bufp1 = temp;
}
```

|Symbol|`.symtab` entry?|Symbol type|Module where defined|Section|
|:---:|:---:|:---:|:---:|:---:|
|buf|yes|extern|`m.o`|`.data`|
|bufp0|yes|global|`swap.o`|`.data`|
|bufp1|yes|global|`swap.o`|`COMMON`|
|swap|yes|global|`swap.o`|`.text`|
|temp|no|-|||


## Résolution des symboles

### Symboles locaux
Le linker n'autorise qu'une seule définition par symbole.
La résolution des symboles pour les références à des symboles locaux définis dans le même module que la référence est direct, en raison de la règle de la définition unique.
Le compilateur s'assure également que les variables locales `static` ont un nom unique, puisqu'elles obtiennent un symbole.

### Symboles globaux
La résolution des symboles globaux est en revanche plus technique.

Lorsque le compilateur tombe sur un symbole (une variable ou un nom de fonction) qui n'est pas localement défini, il émet l'hypothèse que la définition est réalisée dans un autre module. Il génère pour cela une entrée dans la table des symboles et délègue le soin de la résoudre au linker. En fin de compte, si le linker ne parvient pas à trouver de définition pour le symbole, il émet une erreur.

Exemple:

```c
/* linkererror.c */
void foo(void);

int main(void) {
    foo();
    return 0;
}
```

Donne l'erreur suivante au moment de la laison:

```sh
linux> gcc -Wall -Og -o linkerror linkerror.c
/tmp/ccSz5uti.o: In function ‘main’:
/tmp/ccSz5uti.o(.text+0x7): undefined reference to ‘foo’
```

La difficulté provient du fait que différents modules peuvent définir chacun des symboles globaux portant le même nom.
Le linker de Linux choisit entre deux stratégies pouvant conduire à des bogues incompréhensibless: lever une erreur ou choisir une définition et abandonner les autres.

### Règle de résolution des symboles dupliqués

Historiquement, le compilateur exportait chaque symbole `global` vers l'assembleur en tant que symbole `strong` ou `weak`. Les `fonctions` et les variables `global` initialisées étaient assorties de l'étiquette `strong`, et les variables `global` non initialisées de l'étiquette `weak`.

Les linkers de Linux opèrent selon ces règles:
- Règle 1: Plusieurs symboles `strong` portant le même nom sont interdits
- Règle 2: En présence d'un symbole `strong` et de plusieurs symboles `weak` portant le même nom, choisir le symbole `strong`
- Règle 3: En l'absence d'un symbole `strong` et de plusieurs symboles `weak` portant le même nom, choisir n'importe lequel des symboles `weak`

Cette règle est toujours en vigueur, mais depuis `gcc10`, les critères de déclaration par défaut ont changé.
Désormais, le comportement par défaut de `gcc` correspond à l'option implicite `-f-no-common`, tandis que le comportement historique correspondait à `-f-common`.

>Désormais, lorsque `gcc` (`-f-no-common` implicite) rencontre un symbole `global` non initialisé, il considère de facto que le module possède le symbole. Il réserve alors 4 octets dans `.bss` et émet un `GLOBAL`. Le symbole devient donc un symbole `strong` et toute nouvelle définition (obligatoirement `strong`) produira une erreur.
>
> Historiquement, `gcc` (`-f-common` implicite) rencontrait un symbole `global` non initialisé, il considérait ne pas être en mesure de déterminer le propriétaire du symbole. Il émettait un symbole `COMMON` sans section ni adressse (juste une taille et un alignement).

### Practice problem 7.2

Question A:
```c
/* Module 1 */
int main{}

/* Module 2 */
int main;
int p2() {}
```
```
(a) REF(main.1) -> DEF(main.1)
(b) REF(main.2) -> DEF(main.1)

car symbole STRONG (1) contre WEAK (2)
```

Question B:
```c
/* Module 1 */
int main{}

/* Module 2 */
int main = 1;
int p2() {}
```
```
(a) REF(main.1) -> DEF(ERREUR)
(b) REF(main.2) -> DEF(ERREUR)

car deux symboles STRONG
```

Question C:
```c
/* Module 1 */
int x;
int main{}

/* Module 2 */
double x = 1.0;
int p2() {}
```
```
(a) REF(main.1) -> DEF(main.2)
(b) REF(main.2) -> DEF(main.2)

car symbole WEAK (1) contre STRONG(2). 

Situation problématique, parce que le type double l'emporte et risque de corrompre la mémoire de la déclaration INT.
```

## Laison des librairies statiques

Durant la phase de résolution des symboles, le linker scanne les fichiers objets relogeables et les archives de gauche à droite dans l'ordre séquentiel dans lequel ils apparaissent dans la ligne de commande.

Durant cette opération, le linker maintient un ensemble `E` de fichiers objets relogeables qui seront à fusionner pour former le fichier exécutable, un ensemble `U` de symbols non résolus (c'est-à-dire les symboles auquel il est fait référence mais non encore définis), et un ensemble `D` de symboles qui ont été définis dans les fichiers d'entrée précédents.

### L'algorithme

Au départ, `E`, `U` et `D` sont vides. Ensuite, pour chaque nouveau fichier d'entrée `f` de la ligne de commande, le linker détermine si `f` est un fichier objet ou une archive.

- S'il s'agit d'un fichier objet, le linker ajoute `f` à `E` et met à jour `U` et `D` pour refléter les définitions de symboles et les références dans `f`. Puis il passe au fichier suivant.
- Si s'agit d'une archive, le linker tente de faire correspondre les symboles non résolus dans `U` avec les symboles définis par les membres de l'archive. Si un membre `m` de l'archive définit un symbole qui résout une référence dans `U`, alors `m` est ajouté à `E` et le linker met à jour `U` et `D` pour refléter les définitions de symboles et les références dans `m`. Ce processus itère sur tous les fichiers objets membres de l'archive jusqu'à ce qu'un point d'arrêt soit atteint et que `U` et `D` n'évoluent plus. A ce moment-là, tout fichier objet membre non contenu dans `E` (c'est-à-dire en excès par rapport aux besoins du programme) est simplement rejeté.
- Si `U` n'est pas vide lorsque le linker a fini de scanner les fichiers d'entrée de la ligne de commande, il affiche une erreur et se termine. Sinon, il fusionne et reloge les fichiers objets dans `E` afin de construire le fichier exécutable en sortie.

### Ordre d'apparition des librairies

Cet algorithme implique deux choses concernant le fichier `f` en cours:
- Si l'on est en présence d'un fichier objet, celui-ci est chargé intégralement dans `E`, avec tous ses symboles locaux comme globaux, résolus comme non résolus.
- Si l'on est en présence d'une archive, uniquement les symboles requis pour résoudre `U` sont chargés, et les autres sont rejetés.

Or, l'ordre d'apparition dans fichiers dans la commande est important puisque la lecture et le traitement se fait de gauche à droite.

```sh
$ gcc file.o libx.a liby.a   # file.o dépend de libx.a, qui dépend de liby.a
```

> La règle veut que les librairies soient placées après tout le reste, puisqu'elles permettent de résoudre les symboles des fichiers objets, de sorte que tous les symboles puissent être résolus.

### Point d'attention concernant l'ordre des librairies en cas de dépendance circulaire

Dans le cas d'une `référence circulaire` entre deux librairies, `libx.a` <-> `liby.a`, le traitement de `libx.a` va permettre de résoudre les symboles contenus dans `U` à cet instant. Vient le tour de `liby.a`, qui permet de résoudre les symboles présents dans `U` à cet instant... tout en créant de nouveaux besoins auquel `libx.a` est censé répondre. Or, `libx.a` est déjà passé.

> Les librairies peuvent être répétées pour résoudre cette dépendance circulaire:

```sh
$ gcc file.o libx.a liby.a libx.a   # dépendance circulaire résolue entre libx.a et liby.a
```

### Practice problem 7.3

Soit les listes de dépendances suivantes, quelle est la déclaration de compilation minimale ? 

```
A. p.o → libx.a
B. p.o → libx.a → liby.a
C. p.o → libx.a → liby.a and liby.a → libx.a → p.o
```

Réponse: 
```sh
# A
$ gcc p.o libx.a

# B
$ gcc p.o libx.a liby.a

# C
$ gcc p.o libx.a liby.a libx.a
$ gcc p.o -Wl,--start-group libx.a liby.a -Wl,--end-group
```

## Relogement

Une fois que le linker a terminé la phase de relogement des sympoles, chaque symbole est associé à exactement une définition (c'est-à-dire une entrée dans la table des symboles de ses modules objets sources).

Le linker connaît désormais la taille des sections de code et de données de ses modules objets sources. 

Il est alors prêt à entamer la `phase de relogement`, dans laquelle les modules d'entrée sont fusionnés et les adresses `runtime` de chaque symbole sont assignées.

Cette phase se déroule en deux étapes: 

1. **Relogement des sections et des définitions de symboles**: durant cette étape, le linker agrège toutes les sections d'un même type en une seule section du même type. Par exemple, toutes les sections `.data` sont agrégées en une seule section `.data` correspondant au fichier exécutable. Le linker assigne alors une adresse `runtime` à chaque section agrégée (exemple `.data`), à chaque section provenant des fichiers en entrée (conceptuellement, `.data.a.o`, `.data.b.o`, bien que ces sections ne soient pas nommées ainsi), et à chaque symbole défini dans les modules en entrée. Au sortir de cette étape, chaque instruction et chaque variable globale du programme a une adresse `runtime` unique.

2. **Relogement des références aux symboles au sein des sections**: durant cette étape, le linker associe chaque référence à un symbole dans le corps des sections de code et de données, de telle sorte qu'elles pointent vers la bonne adresse `runtime`. Pour ce faire, le linker s'appuie sur des `structures` nommées `entrées de relogement` ou `relocation entries`.

### Relocation entries

Lorsque que l'assembleur génère un module objet, il ne connaît pas encore l'emplacement final où son code et ses données seront stockés. Il ne connaît pas non plus l'emplacement des fonctions et des variables globales définies par d'autres modules. 

Dès que l'assembleur rencontre une référence à un objet dont l'emplacement final est inconnu, il génère une `entrée de relogement` qui lui permet d'indiquer au linker comment modifier la référence lorsqu'il devra fusionner les fichiers objets en un exécutable.

Ces `relocation entries` sont placées dans `.rel.text` pour le code, et `.rel.data` pour les données.

#### Structure des entrées de relogement au format `ELF`

```c
typedef struct {
    long    offset;     /* Décalage de la référence à reloger */
    long    type:32,    /* Type de relogement */   
            symbol:32;  /* Indice de la table des symboles */   
    long    addend;     /* Constante faisant partie de l'expression du relogement */
} Elf64_Rela;
```
Avec:
- `offset`: offset dans la section pour la référence à modifier
- `symbol`: indique le symbole vers lequel la référence modifiée devrait pointer (ex: une fonction)
- `type`: indique au linker comment modifier la référence (ex: `R_X86_64_PC32`)
- `addend`: une constante signée utilisée par certains `type` de relocation pour moduler la valeur de la référence modifiée

#### Types de relogement

Le format `ELF` définit 32 types de relogement. Les plus courants et simples sont:

- `R_X86_64_PC32`: Relogement d'une référence qui utilise une adresse de 32 bits relative au `PC` (`Program Counter`). Lorsque le `CPU` exécute une instruction utilisant une adresse relative au `PC`, il calcule l'`effective address` (c'est-à-dire la cible de l'instruction `call`) en ajoutant la valeur 32-bits encodée dans l'instruction à la valeur courante du `PC` -> il s'agit toujours de l'adresse de l'instruction suivante.

- `R_X86_64_32`: Reloge une référence qui utilise une adresse absolue en 32 bits. Avec une adresse absolue, le `CPU` utilise directement la valeur 32 bits encodée dans l'instruction en guise d'`effective address`, sans autre modification.

> Ces deux types de relogement correspondent au `small code model x86-64`, qui pose l'hypothèse que la taille totale du code et de la donnée présents dans l'exécutable n'excèdent pas 2Go, et par conséquent qu'ils peuvent être atteints durant l'exécution en utilisant des adresses `PC-relative` de 32 bits.
>
> C'est le comportement par défaut des compilateurs tels que `gcc`. Les programmes plus grands que 2Go doivent être compilés avec l'option `-mcmodel=medium` ou `-mcmodel=large`.


## Relogement des références aux symboles

### Algorithme de relogement

```
foreach section s {
  foreach relocation entry r {
    refptr = s + r.offset; /* ptr to reference to be relocated */

    /* Relocate a PC-relative reference */
    if (r.type == R_X86_64_PC32) {
      refaddr = ADDR(s) + r.offset; /* ref’s run-time address */
      *refptr = (unsigned) (ADDR(r.symbol) + r.addend - refaddr);
    }

    /* Relocate an absolute reference */
    if (r.type == R_X86_64_32)
      *refptr = (unsigned) (ADDR(r.symbol) + r.addend);
  }
}
```

#### Sur quoi boucle s

s parcourt les sections d'entrée, celles de chaque .o : la .text de main.o, la .text de sum.o, la .data de main.o, etc. Ce sont vos « sections intermédiaires » (la notation .text.fichier_b est la vôtre, ça n'existe pas dans l'ELF : chaque .o a simplement sa propre section nommée .text).

Le livre est elliptique : sa boucle est en fait imbriquée dans un parcours de tous les modules.

```
foreach module m {
  foreach section s de m {                  // .text de m, .data de m...
    foreach relocation entry r de .rela.s de m {
      ...
    }
  }
}
```

#### Sur quoi boucle r

Pour une section `s` donnée, `r` ne parcourt que les entrées de la table de relocations de cette section dans ce .o : `.rela.text` de `main.o` pour `s = .text de main.o`, et ainsi de suite. Deux `s` différents ne bouclent donc jamais sur les mêmes `r`.

`main.o`   `.text`  -> `.rela.text` de `main.o`   (entrées : `array`, `sum`)
`main.o`   `.data`  -> `.rela.data` de `main.o`   (si elle existe)
`sum.o`    `.text`  -> `.rela.text` de `sum.o`    (absente ici)

Dans une entrée, `r.offset` est l'offset dans la section d'entrée à laquelle elle s'applique, donc dans la `.text` de ce `.o`. C'est pourquoi le livre écrit: `ADDR(s) + r.offset` : `ADDR(s)` est l'adresse finale de cette section d'entrée (calculée en phase 1), pas celle de l'agrégat. Si les relocations étaient fusionnées, les offsets deviendraient ambigus (offset 0x11 de quel fichier ?).

#### Ce qui est agrégé dans l'exécutable final

Les octets des sections de même type sont concaténés dans `.text`, `.data`, etc. Les relocations, elles, sont lues, appliquées, puis jetées. Il ne reste dans l'exécutable que les relocations de chargement dynamique (.rela.dyn, .rela.plt).

Le livre mélange les deux niveaux, ce qui prête à confusion : `s` désigne dans l'algorithme un morceau d'entrée, même s'il décrit ce qui deviendra une partie de l'agrégat.

### Le modèle
Le linker garde en mémoire chaque .o séparément (ses sections, ses symboles, ses relocations) comme un plan de mise en page.

- **Passe 1 : mise en page (phase 1)**
Pour chaque .o et chaque section d'entrée, le linker décide où elle ira dans l'agrégat :
  - main.o .text -> agrégat .text, offset 0x00 -> ADDR = 0x401000
  - sum.o  .text -> agrégat .text, offset 0x30 -> ADDR = 0x401030
  - main.o .data -> agrégat .data, offset 0x00 -> ADDR = 0x404000

Il en déduit `ADDR(symbole) = ADDR(section d'entrée) + st_value`. À ce stade, aucun octet da table.

- **Passe 2 : copie et patch**
Il recopie les octets de chaque section d'entrée à sa place dans l'agrégat, et pour chaque l'adresse calculée dans le trou.
C'est ici que la triple boucle intervient : module, section, relocation.

#### Pourquoi il faut garder les modules distincts

Chaque relocation est exprimée relativement à son `.o` :
- `r.offset` est un offset dans la section de ce `.o`,
- l'index du symbole `r.symbol` pointe dans la `.symtab` de ce `.o` (qui peut être `UND`, donc défini dans un autre `.o` via la table globale des symboles).

Sans le `.o` d'origine, ces informations n'ont plus de sens.

Le linker agrège les octets et garde les fichiers individuels pour interpréter les relocations. 
L'adresse finale étant déjà connue quand la passe 2 commence, la boucle la lit dans la table et l'écrit dans le trou.

Le travail de calcul des adresses est entièrement fait en passe 1.

> Note: le livre utilise `R_X86_64_PC32`, mais les linkers modernes émettent `PLT32` pour les `call`. Le calcul ne change pas.

### Comment se calcule `addend` ?

L'addend est calculé par l'assembleur. C'est une constante qui corrige l'écart entre :
- l'adresse du trou (`refaddr`), à laquelle le `CPU` ne se réfère pas,
- la valeur de `%rip` utilisée par le `CPU` pour calculer le déplacement.

En `x86-64`, un déplacement `PC-relatif` est calculé par rapport à l'adresse de l'instruction suivante, c'est-à-dire la fin du champ de 4 octets du trou (si l'instruction se termine par ce champ) :

```
valeur voulue dans le trou = cible - (adresse du trou + 4)
                           = cible + (-4) - adresse du trou
                                     addend
```

Donc `addend` = -4 pour un `call` ou un `lea` dont le déplacement est le dernier champ de l'instruction.

Si des octets suivent le déplacement dans l'instruction (par exemple `cmpl $1, var(%rip)` avec un immédiat de 1 octet après), l'addend vaut -5, et ainsi de suite : `addend = -(nombre d'octets entre le début du trou et la fin de l'instruction)`.                            

### Où sont les relocation entries ?

Dans chaque `.o`. Il y a une section de relocations par section à patcher :

```sh
main.o
 ├─ .text         # octets du code
 ├─ .rela.text    # relocations qui s'appliquent à .text
 ├─ .data
 ├─ .rela.data    # (présente seulement s'il y a des références à patcher dans .data)
 └─ .symtab
```

Dans l'algorithme du livre, `foreach section s` parcourt donc les sections de chaque `.o`, et `foreach relocation entry r` parcourt la `.rela.<s>` correspondante. Un `sum.o` sans référence extérieure n'a pas de `.rela.text`.

### Relogement des références PC-relatif (exemple)

```ss
1  000000000000000 <main>:
2     0:	48 83 ec 08          	sub    $0x8,%rsp
3     4:	be 02 00 00 00       	mov    $0x2,%esi
4     9:	bf 00 00 00 00       	mov    $0x0,%edi
5  			a: R_X86_64_32	array
6     e:	e8 00 00 00 00       	callq  13 <main+0x13>
7  			f: R_X86_64_PC32	sum-0x4
8    13:	48 83 c4 08          	add    $0x8,%rsp
9    17:	c3                   	retq   
```

Soit la `relocation entry` correspante:

```
r.offset = 0xf
r.symbol = sum
r.type = R_X86_64_PC32
r.addend = -4
```

En supposant que le linker a déterminé que : 

```
ADDR(s) = ADDR(.text) = 0x4004d0
```
et 
```
ADDR(s.symbol) = ADDR(sum) = 0x4004e8
```

Alors, en suivant l'algorithme, on obtient l'adresse `runtime` de la référence ligne 7 : 

```
refaddr = ADDR(s) + r.offset
        = 0x4004d0 + 0xf
        = 0x4004df
```

Cela permet de mettre à jour la référence de sorte qu'elle pointe vers `sum` au runtime, ligne 8:
```
*refptr = (unsigned) (ADDR(r.symbol)  + r.addend  - refaddr)
        = (unsigned) (0x4004e8       + (-4)      - 0x4004df)
        = (unsigned) (0x5)
```

Dans le fichier exécutable en sortie, l'instruction `call` contient la relocation suivante: 

```
4004de:	e8 05 00 00 00       	callq  4004e8 <sum>
```

### Relogement des références absolues (exemple)

Elles sont directes.

Par exemple, la ligne 4 contient l'instruction `mov` copie l'adresse de `array` (une valeur immédiate de 32 bits) dans `edi`.

```
9:	bf 00 00 00 00       	mov    $0x0,%edi
```

L'instruction `mov` commence à l'offset `0x9` et contient l'opcode de 1 octet `0xbf`, suivi d'une emplacement de 4 octets (32 bits) servant à référencer `array`.

La `relocation entry` correspondante est la suivante:

```
r.offset = 0xa
r.symbol = array
r.type = R_X86_64_32
r.addend = 0
```

Ces champs indiquent au linker qu'il doit modifier lar éférence absolue qui démarre à `0xa` de telle sorte qu'elle pointe à l'exécution sur le premier octet de `array`. 

En supposant que le linker a déterminé que : 

```
ADDR(r.symbol) = ADDR(array) = 0x601018
```

Le linker met à jour la référence via l'algorithme:

```
*refptr = (unsigned) (ADDR(r.symbol)  + r.addend)
        = (unsigned) (0x601018       + (0))
        = (unsigned) (0x601018)
```

La ligne correspondante dans l'exécutable prend donc la forme :

```sh
4004d9: bf 18 10 60 00      mov   $0x601018,%edi  # %edi = &array
```

### Practice Problem 7.4

A. What is the hex address of the relocated reference to sum in line 5?

> 0x4004df

B. What is the hex value of the relocated reference to sum in line 5?

> 0x05

### Practice Problem 7.5

> Consider the call to function swap in object file `m.o` (Figure 7.5).
>```sh
> 9:  e8 00 00 00 00      callq e <main+0xe>    #swap()
>```
> with the following relocation entry:
> ```
> r.offset = 0xa
> r.symbol = swap
> r.type = R_X86_64_PC32
> r.addend = -4
> ```
>
> Now suppose that the linker relocates `.text` in `m.o` to address `0x4004d0` and `swap` to address `0x4004e8`. Then what is the value of the relocated reference to swap in the `callq` instruction?

Réponse:

```
Soit

ADDR(s) = ADDR(.text) = 0x4004d0

et

ADDR(r.symbol) = ADDR(swap) = 0x4004e8

On a:

refaddr = ADDR(s) + r.offset
        = (ADDR(.text) + r.offset)
        = (0x4004d0 + 0xa)
        = (0x4004da)

*refptr = (unsigned) (ADDR(r.symbol) + r.addend - refaddr)
        = (unsigned) (0x4004e8 + (-4) - 0x4004da)
        = (unsigned) (0xa)
```

> 4004d9:  e8 0a 00 00 00      callq e 0x4004e8 <swap>    #swap()




























































Votre intuition est bonne : tout ce schéma décrit l'espace d'adressage virtuel d'un processus. La question à se poser n'est donc pas « où le noyau se trouve-t-il en RAM ? » mais « pourquoi le noyau apparaît-il dans l'espace virtuel de chaque processus ? ».

1. Le noyau est mappé dans l'espace virtuel de chaque processus

Chaque processus a ses propres tables de pages. Linux y ajoute, dans la partie haute, les mêmes entrées pour tous les processus : elles pointent vers les mêmes pages physiques, qui contiennent le code et les données du noyau.

- La partie basse (code, tas, pile…) est propre à chaque processus.
- La partie haute (le noyau) est identique partout et partagée physiquement.

La raison est pratique. Lors d'un appel système, d'une interruption ou d'un défaut de page, le CPU passe en mode noyau sans changer de tables de pages (pas de changement de CR3, pas de vidage du TLB). Le code du noyau est déjà présent à des adresses virtuelles valides. Il peut aussi lire directement les tampons de l'utilisateur, par exemple pour un read(fd, buf, n), puisque les deux zones sont dans le même espace.

2. Pourquoi c'est « invisible au code utilisateur »

Chaque entrée de table de pages contient un bit U/S (User/Supervisor). Les pages du noyau sont marquées supervisor only. Si un programme en mode utilisateur (ring 3) tente d'y accéder, le MMU lève une faute de protection et le processus reçoit un segfault. Ces pages sont donc bien mappées, mais inaccessibles depuis le code utilisateur.

3. La simplification du « 2^48 »

En x86-64 avec des adresses virtuelles sur 48 bits, une adresse doit être canonique : les bits 63 à 47 doivent tous être égaux. Il existe donc deux zones valides :

0x0000000000000000 – 0x00007FFFFFFFFFFF   → espace utilisateur (128 Tio)
        [ trou non canonique, inutilisable ]
0xFFFF800000000000 – 0xFFFFFFFFFFFFFFFF   → espace noyau (128 Tio)

Le noyau ne commence donc pas réellement à l'adresse 2^48. Il occupe la « moitié haute » de l'espace, en adresses canoniques. Le livre montre seulement qu'il se trouve au-dessus de la plus grande adresse utilisateur légale. Avec la pagination à 5 niveaux (adresses sur 57 bits), le principe reste le même, seules les bornes changent.

4. Une nuance moderne : KPTI

Depuis la faille Meltdown (2018), Linux active souvent le KPTI (Kernel Page-Table Isolation). Chaque processus a alors deux jeux de tables de pages :
- en mode utilisateur, le noyau est presque entièrement retiré du mappage, sauf un petit « trampoline » pour entrer dans le noyau ;
- en mode noyau, on bascule vers des tables où tout le noyau est mappé.

Le modèle du livre reste valable sur le plan conceptuel, mais cette isolation est aujourd'hui plus stricte que le simple bit U/S.

Le chapitre 9.7 (Figure 9.26, « The virtual memory of a Linux process ») reprend ce point en détail. Il montre notamment que la zone noyau contient une partie identique pour tous les processus et une partie propre à chacun : ses structures, sa pile noyau, etc.















Les deux approches chargent une bibliothèque dynamique. Ce qui les distingue, c'est le moment où le choix de la bibliothèque est fait.

Votre cas habituel : liaison dynamique au chargement

gcc -o prog main.c -L. -lvector

- Le .h ne fournit que les déclarations, pour que le compilateur sache comment appeler addvec.
- Au moment de l'édition de liens, ld vérifie que addvec existe dans libvector.so. Il écrit ensuite dans l'exécutable une dépendance (DT_NEEDED: libvector.so) et crée une entrée PLT/GOT.
- Au lancement, ld-linux.so charge obligatoirement libvector.so avant main. Si le fichier est absent, le programme ne démarre pas du tout (error while loading shared libraries).

Le nom de la bibliothèque et le nom des fonctions sont donc figés au moment de la compilation. C'est ce qui se passe avec printf et la libc, et c'est le bon choix dans 95 % des cas.

dlopen/dlsym : liaison pendant l'exécution

On ne fait pas -lvector : l'exécutable ignore tout de libvector.so. C'est le programme lui-même qui décide, pendant qu'il tourne, quel fichier ouvrir et quel symbole chercher. Le chemin et le nom du symbole ne sont que des chaînes de caractères, qui peuvent venir d'un fichier de configuration, d'un répertoire, de l'utilisateur…

Cela sert quand on ne peut pas ou ne veut pas connaître la bibliothèque au moment de compiler :

1. Plugins. Le programme parcourt un dossier plugins/, fait dlopen sur chaque .so trouvé, puis dlsym(h, "plugin_init"). Des tiers peuvent écrire des plugins des années après la compilation du programme. Exemples : modules Apache/nginx, filtres GIMP, plugins VLC/GStreamer, modules PAM.
2. Choix d'une implémentation à l'exécution. Les chargeurs OpenGL/Vulkan, par exemple, ouvrent le pilote NVIDIA, AMD ou Intel selon le matériel présent. La glibc elle-même utilise dlopen pour NSS (libnss_files.so, libnss_ldap.so…) selon /etc/nsswitch.conf.
3. Dépendances optionnelles. « Si libcuda.so est présente, j'utilise le GPU, sinon je calcule sur le CPU. » Avec -lcuda, le programme refuserait de démarrer sur une machine sans CUDA.
4. Interpréteurs et FFI. Quand Python fait import numpy, il charge _multiarray_umath.so avec dlopen. Avec ctypes.CDLL("libfoo.so"), c'est encore dlopen/dlsym : l'interpréteur a été compilé bien avant votre module.
5. Rechargement à chaud. Un serveur ou un moteur de jeu peut faire dlclose, recompiler le .so, puis refaire dlopen sans redémarrer. Le livre cite d'ailleurs l'exemple des serveurs web qui mettent à jour leurs fonctions sans s'arrêter.
6. Démarrage plus rapide. On ne charge une grosse bibliothèque que si la fonctionnalité qui l'utilise est effectivement demandée.

En résumé

┌──────────────────────┬──────────────────────────────────────┬───────────────────────────────────────┐
│                      │            -lvector + .h             │             dlopen/dlsym              │
├──────────────────────┼──────────────────────────────────────┼───────────────────────────────────────┤
│ Bibliothèque connue  │ à la compilation                     │ à l'exécution                         │
├──────────────────────┼──────────────────────────────────────┼───────────────────────────────────────┤
│ Bibliothèque absente │ le programme ne démarre pas          │ vous gérez l'erreur (dlerror)         │
├──────────────────────┼──────────────────────────────────────┼───────────────────────────────────────┤
│ Appel                │ addvec(...) directement (via la PLT) │ via un pointeur de fonction           │
├──────────────────────┼──────────────────────────────────────┼───────────────────────────────────────┤
│ Usage typique        │ dépendances normales                 │ plugins, pilotes, extensions, options │
└──────────────────────┴──────────────────────────────────────┴───────────────────────────────────────┘

Pour votre propre libvector.so, utilisée par votre propre programme, il n'y a aucune raison de passer par dlopen : le .h et -lvector suffisent. Le livre montre dlopen sur ce petit exemple uniquement pour présenter l'interface. Son intérêt réel apparaît quand le programme qui charge la bibliothèque et la bibliothèque chargée sont écrits indépendamment l'un de l'autre.

Le flag -rdynamic sert justement à ce scénario de plugins. Il exporte les symboles de l'exécutable pour que les .so chargés puissent, eux aussi, appeler des fonctions du programme hôte.














Dans l'ensemble c'est juste. Quelques corrections :

1. La GOT ne contient pas de code, seulement des adresses. Au départ, GOT[4] contient l'adresse de la 2e instruction de PLT[2]. C'est ce « renvoi » que tu décris, mais c'est une simple adresse, pas un code de démarrage.
2. Le linker n'est pas appelé à chaque appel. Le programme appelle PLT[2], l'entrée propre à addvec (ce n'est pas une entrée générique). PLT[2] fait jmp *GOT[4], c'est-à-dire qu'il saute à l'adresse stockée dans GOT[4]. Il n'« appelle » pas l'entrée de la GOT. Après le premier appel, ce saut mène directement à addvec et le linker n'intervient plus du tout.
3. .dynamic n'est pas du code, c'est une section de données de l'exécutable. Elle sert de fiche technique au linker : quelles bibliothèques charger, et où se trouvent la table des symboles et la table de relocation. GOT[0] contient son adresse. Le linker, lui, est le code de ld-linux.so.
4. Ce que fait PLT[0] : il empile GOT[1], qui identifie le programme pour le linker, puis saute à l'adresse contenue dans GOT[2]. Cette adresse est le point d'entrée du linker, _dl_runtime_resolve.
5. Le « travail » que tu ne comprends pas bien :
   - avec l'indice empilé, le linker lit l'entrée correspondante de .rela.plt, qui lui donne le nom du symbole (addvec) et la case de la GOT à remplir (GOT[4]) ;
   - il cherche addvec dans les tables de symboles des bibliothèques chargées ;
   - il écrit l'adresse trouvée dans GOT[4] ;
   - il saute à addvec, si bien que le premier appel aboutit lui aussi.

Détail de vocabulaire : la GOT est dans la section .got.plt et non dans .data, mais les deux font partie du segment de données, modifiable. La PLT est dans la section .plt, qui fait partie du segment de code.

C'est l'éditeur de liens statique (ld) qui crée la PLT et la GOT, au moment de produire l'exécutable. Il y écrit :
- tout le code de la PLT ;
- les valeurs initiales de la GOT : GOT[0] reçoit l'adresse de .dynamic, et GOT[3] et suivantes reçoivent les adresses des pushq de la PLT ;
- la table .rela.plt, que le linker dynamique utilisera.

Deux cases restent vides jusqu'au lancement du programme : GOT[1] et GOT[2]. Le linker dynamique les remplit au chargement, puisque son adresse n'est pas connue avant.




























Pas tout à fait. Ce n'est pas le compilateur qui fait ce travail mais l'éditeur de liens. Et il ne lit pas les deux noms comme malloc : il fait deux redirections en sens opposés.

Avec -Wl,--wrap,malloc :
- un appel à malloc (par exemple dans int.c) est relié à __wrap_malloc, ton wrapper ;
- un appel à __real_malloc (dans le wrapper) est relié à malloc, celui de la libc.

Résultat : int.c → __wrap_malloc → __real_malloc, qui est en réalité le malloc de la libc.

Le compilateur ne voit rien de tout ça. Il compile int.c avec un appel à malloc tout à fait normal, et mymalloc.c avec un appel à une fonction __real_malloc qui n'est définie nulle part. Les redirections n'ont lieu qu'à l'édition des liens, dont la commande finale ressemble à ceci :

gcc -Wl,--wrap,malloc -Wl,--wrap,free -o intl int.o mymalloc.o

✻ Crunched for 10s · done 6:33 PM

❯ Quel est le mécanisme ? Je comprends qu'en écrivant -Wl,--wrap,malloc, je lie toutes les écritures de malloc() à __wrap_malloc, mais pourquoi __real_malloc est-il bien interprété ? C'est une convention du compilateur ? __wrap et __real sont déjà prévus ?

Oui, c'est une convention, mais celle de l'éditeur de liens (ld) et non du compilateur. Les préfixes __wrap_ et __real_ sont codés en dur dans l'option --wrap.

Le mécanisme :

1. Compilation. gcc -c mymalloc.c produit mymalloc.o. Le compilateur ne connaît pas __real_malloc : il le traite comme une fonction externe quelconque et l'inscrit dans la table des symboles comme symbole non défini, comme il le ferait pour printf.
2. Édition des liens. Avec --wrap malloc, ld applique deux règles chaque fois qu'il doit résoudre un symbole non défini :
   - si le nom est malloc, il le relie à __wrap_malloc ;
   - si le nom est __real_malloc, il le relie à malloc.

Une seule option active donc les deux règles. __real_malloc n'existe nulle part en tant que fonction : c'est juste un nom réservé que ld reconnaît et redirige.
