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

Une fois que le linker a terminé sa phase de relogement des sympboles, chaque symbole est associé à exactement une définition de symbole (c'est-à-dire une entre dans la table des symboles de ses modules objets sources).

Le linker connaît désormais la taille des sections de code et de données de ses modules objets en entrée. 

Il est alors rêt à entamer la `phase de relogement`, dans laquelle les modules d'entrée sont fusionnés et  où les adresses `runtime` de chaque symbole sont assignées.

Cette phase se déroule en deux étapes: 

1. **Relogement des sections et des définitions de symboles**: durant cette étape, le linker agrège toutes les sections d'un même type en une seule section du même type. Par exemple, toutes les sections `.data` sont agrégées en une seule section `.data` correspondant au fichier exécutable. Le linker assigne alors une adresse `runtime` à chaque section agrégée (exemple `.data`), à chaque section provenant des fichiers en entrée (conceptuellement, `.data.a.o`, `.data.b.o`, bien que ces sections ne soient pas nommées ainsi), et à chaque symbole défini dans les modules en entrée. Au sortir de cette étape, chaque instruction et chaque variable globale du programme a une adresse `runtime` unique.

2. **Relogement des références aux symboles au sein des sections**: durant cette étape, le linker modifie chaque référence à un symbole dans le corps des sections de code et de données, de telle sorte qu'elles pointent vers la bonne adresse `runtime`. Pour ce faire, le linker s'appuie sur des `structures` nommées `entrées de relogement` ou `relocation entries`.

### Relocation entries

Lorsque que l'assembleur génère un module objet, il ne connaît pas encore l'emplacement final où son code et de ses données seront stockés. Il ne connaît pas non plus l'emplacement des fonctions et des variables globales définies par d'autres modules. 

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
- `offset`: offset de la section pour la référence à modifier
- `symbol`: indique le symbole vers lequel la référence modifiée devrait pointer (ex: une fonction)
- `type`: indique au linker comment modifier la référence (ex: `R_X86_64_PC32`)
- `addend`: une constante signée utilisée par certains `type` de relocation pour moduler la valeur de la référence modifiée

#### Types de relogement

Le format `ELF` définit 32 types de relogement. Les plus courants sont:

- `R_X86_64_PC32`: Relogement d'une référence qui utilise une adresse relative de 32 bits relative au `PC` (`Program Counter`). Lorsque le `CPU` exécute une instruction utilisant une adresse relative au `PC`, il calcule l'`effective address` (c'est-à-dire la cible de l'instruction `call`) en ajoutant la valeur 32-bits encodée dans l'instruction à la valeur courante du `PC` -> il s'agit toujours de l'adresse de l'instruction suivante.

- `R_X86_64_32`: Reloge une référence qui utilise une adresse absolue en 32 bits. Avec une adresse absolue, le `CPU` utilise directement la valeur 32 bits encodée dans l'instruction en guise d'`effective address`, sans autre modification.

> Ces deux types de relogement correspondent au `small code model x86-64`, qui pose l'hypothèse que la taille totale du code et de la donnée présents dans l'exécutable n'excèdent pas 2Go, et par conséquent qu'ils peuvent être atteints durant l'exécution en utilisant des adresses `PC-relative` de 32 bits.
>
> C'est le comportement par défaut des compilateurs tels que `gcc`. Les programmes plus grands que 2Go doivent être compilés avec l'option `-mcmodel=medium` ou `-mcmodel=large`.










Votre intuition d'une table agrégée est fausse : les relocations ne sont jamais agrégées. Seul le contenu des sections (.text, .data) l'est.

Sur quoi boucle s

s parcourt les sections d'entrée, celles de chaque .o : la .text de main.o, la .text de sum.o, la .data de main.o, etc. Ce sont vos « sections intermédiaires » (la notation .text.fichier_b est la vôtre, ça n'existe pas dans l'ELF : chaque .o a simplement sa propre section nommée .text).

Le livre est elliptique : sa boucle est en fait imbriquée dans un parcours de tous les modules.

foreach module m {
  foreach section s de m {                  // .text de m, .data de m...
    foreach relocation entry r de .rela.s de m {
      ...
    }
  }
}

Sur quoi boucle r

Pour une section s donnée, r ne parcourt que les entrées de la table de relocations de cette section dans ce .o : .rela.text de main.o pour s = .text de main.o, et ainsi de suite. Deux s différents ne bouclent donc jamais sur les mêmes r.

main.o   .text  ──▶ .rela.text de main.o   (entrées : array, sum)
main.o   .data  ──▶ .rela.data de main.o   (si elle existe)
sum.o    .text  ──▶ .rela.text de sum.o    (absente ici)

Pourquoi r.offset est cohérent

Dans une entrée, r.offset est l'offset dans la section d'entrée à laquelle elle s'applique, donc dans la .text de ce .o. C'est pourquoi la formule du livre écrit ADDR(s) + r.offset : ADDR(s) est l'adresse finale de cette section d'entrée (calculée en phase 1), pas celle de l'agrégat. Si les relocations étaient fusionnées, les offsets deviendraient ambigus (offset 0x11 de quel fichier ?).

Ce qui est agrégé dans l'exécutable final

Les octets des sections de même type sont concaténés dans .text, .data, etc. Les relocations, elles, sont lues, appliquées, puis jetées. Il ne reste dans l'exécutable que les relocations de chargement dynamique (.rela.dyn, .rela.plt).

Le livre mélange les deux niveaux, ce qui prête à confusion : s désigne dans l'algorithme un morceau d'entrée, même s'il décrit ce qui deviendra une partie de l'agrégat.

✻ Sautéed for 8s · done 11:47 PM

❯ je pensais avoir compris que le linker agrégeait tout. En fait, ce n'est qu'à moitié vrai, il agrège tout, mais il garde en même temps les fichiers individuels ouverts et travaille en parallèle le fichier global et les fichiers individuels ? C'est pour ça que pour chaque module, et pour chaque section, et pour chaque relocation, il essaie de trouver l'adresse finale

Oui, c'est l'idée, avec une nuance sur « en parallèle » : c'est séquentiel, en deux passe

Le modèleLe linker garde en mémoire chaque .o séparément (ses sections, ses symboles, ses relocatigat » n'est pas un fichier construit d'abord : c'est surtout un plan de mise en page.Passe 1 : mise en page (phase 1)Pour chaque .o et chaque section d'entrée, le linker décide où elle ira dans l'agrégat :main.o .text → agrégat .text, offset 0x00   → ADDR = 0x401000sum.o  .text → agrégat .text, offset 0x30   → ADDR = 0x401030main.o .data → agrégat .data, offset 0x00   → ADDR = 0x404000Il en déduit ADDR(symbole) = ADDR(section d'entrée) + st_value. À ce stade, aucun octet da table.Passe 2 : copie et patch (phase 2)Il recopie les octets de chaque section d'entrée à sa place dans l'agrégat, et pour chaquit l'adresse calculée dans le trou. C'est ici que la triple boucle intervient : module,section, relocation.

Pourquoi il faut garder les modules distincts

Chaque relocation est exprimée relativement à son .o :
- r.offset est un offset dans la section de ce .o,
- l'index du symbole r.symbol pointe dans la .symtab de ce .o (qui peut être UND, donc ilinition dans un autre .o via la table globale des symboles).

Sans le .o d'origine, ces informations n'ont plus de sens.

Ce que vous aviez juste et ce qui est à ajuster

- Juste : il agrège les octets, et il garde les fichiers individuels pour interpréter leurs relocations.
- À ajuster : ce n'est pas « il essaie de trouver l'adresse finale » à chaque relocation. L'adresse finale est déjà connue quand la passe 2 commence. La boucle ne fait que la lire dans la table et l'écrire dans le trou. Le travail de calcul des adresses est entièrement fait en passe 1.
(Le livre utilise R_X86_64_PC32 ; les linkers modernes émettent PLT32 pour les call, et le calcul est le même.)

3. Comment se calcule l'addend ?

L'addend est calculé par l'assembleur, pas par le linker. C'est une constante qui corrige l'écart entre :
- l'adresse du trou (refaddr), à laquelle le CPU ne se réfère pas,
- et la valeur de %rip utilisée par le CPU pour calculer le déplacement.

En x86-64, un déplacement PC-relatif est calculé par rapport à l'adresse de l'instruction suivante, c'est-à-dire la fin du champ de 4 octets du trou (si l'instruction se termine par ce champ) :

valeur voulue dans le trou = cible - (adresse du trou + 4)
                           = cible + (-4) - adresse du trou
1. Où sont les relocation entries ?

Oui, dans chaque .o. Il y a une section de relocations par section à patcher :

main.o
 ├─ .text         ← octets du code
 ├─ .rela.text    ← relocations qui s'appliquent à .text
 ├─ .data
 ├─ .rela.data    ← (présente seulement s'il y a des références à patcher dans .data)
 └─ .symtab

Dans l'algorithme du livre, foreach section s parcourt donc les sections de chaque .o, et foreach relocation entry r parcourt la .rela.<s> correspondante. Un sum.o sans référence extérieure n'a pas de
.rela.text.

2. Exemple qui déroule la boucle

Avec main.c et sum.c du dernier exemple, sur main.o (code compilé sans PIE, -fno-pie pour simplifier, et en ne gardant que la relocation PC-relative) :

main:
  0: sub  $8,%rsp
  4: mov  $2,%esi
  9: lea  array(%rip),%rdi     ← 48 8d 3d 00 00 00 00   (trou aux octets 0xc..0xf)
 10: call sum                  ← e8 00 00 00 00         (trou aux octets 0x11..0x14)

(Offsets approximatifs : vérifiez avec objdump -dr main.o.) readelf -r main.o donne :

Offset  Type           Symbol  Addend
0xc     R_X86_64_PC32  array   -4
0x11    R_X86_64_PLT32 sum     -4

Supposons que la phase 1 ait donné :

ADDR(.text de main.o) = 0x401000     ADDR(array) = 0x404000
ADDR(sum)             = 0x401030

La boucle fait, pour s = .text, avec r = 2ᵉ entrée (sum) :

refptr  = s + r.offset                 = pointeur sur le trou (octet 0x11 de .text)
refaddr = ADDR(s) + r.offset           = 0x401000 + 0x11 = 0x401011
*refptr = ADDR(sum) + r.addend - refaddr
        = 0x401030 + (-4) - 0x401011   = 0x1b

Le trou devient 1b 00 00 00, donc e8 1b 00 00 00.

Pour la 1ʳᵉ entrée : refaddr = 0x40100c, *refptr = 0x404000 - 4 - 0x40100c = 0x2ff0.

(Le livre utilise R_X86_64_PC32 ; les linkers modernes émettent PLT32 pour les call, et le calcul est le même.)

3. Comment se calcule l'addend ?

L'addend est calculé par l'assembleur, pas par le linker. C'est une constante qui corrige l'écart entre :
- l'adresse du trou (refaddr), à laquelle le CPU ne se réfère pas,
- et la valeur de %rip utilisée par le CPU pour calculer le déplacement.

En x86-64, un déplacement PC-relatif est calculé par rapport à l'adresse de l'instruction suivante, c'est-à-dire la fin du champ de 4 octets du trou (si l'instruction se termine par ce champ) :

valeur voulue dans le trou = cible - (adresse du trou + 4)
                           = cible + (-4) - adresse du trou
                                       ↑
                                     addend

Donc addend = -4 pour un call ou un lea dont le déplacement est le dernier champ de l'instruction. Si des octets suivent le déplacement dans l'instruction (par exemple cmpl $1, var(%rip) avec un immédiat de 1 octet après), l'addend vaut -5, et ainsi de suite : addend = -(nombre d'octets entre le début du trou et la fin de l'instruction).

Pour une relocation absolue (hors du sujet ici), l'addend sert à autre chose : par exemple &array[1] donne addend = +4.

✻ Churned for 15s · done 11:36 PM

※ recap: Vous étudiez la phase de relocalisation du linker (CS:APP) pour comprendre le travrs ELF. Je viens d'expliquer où sont les relocation entries et comment l'addend secalcule. Prochaine étape : vérifier l'exemple avec `readelf -r main.o`. (disable recaps in /config)

❯ je n'ai pas compris ce que font les deux boucles. Est-ce que s prend la valeur de chaque  ou bien les sections intermédiaires (.text, .text.fichier_b, .text.fichier_c, .data,.data.fichier_b, .data_fichier_c) ? Et la boucle sur les r, elle boucle sur quelles relocation entries ? Il n'y a qu'une seule table de relocation entries à ce moment, enfin deux, non ? .rel.text et .rel.data, tout est agrégé ? Donc chaque s boucle sur les mêmes r ?

Votre intuition d'une table agrégée est fausse : les relocations ne sont jamais agrégées. Seul le contenu des sections (.text, .data) l'est.

Sur quoi boucle s

s parcourt les sections d'entrée, celles de chaque .o : la .text de main.o, la .text de s. Ce sont vos « sections intermédiaires » (la notation .text.fichier_b est la vôtre, çan'existe pas dans l'ELF : chaque .o a simplement sa propre section nommée .text).

Le livre est elliptique : sa boucle est en fait imbriquée dans un parcours de tous les modules.

foreach module m {
  foreach section s de m {                  // .text de m, .data de m...
    foreach relocation entry r de .rela.s de m {
      ...
    }
  }
}

Sur quoi boucle r

Pour une section s donnée, r ne parcourt que les entrées de la table de relocations de cea.text de main.o pour s = .text de main.o, et ainsi de suite. Deux s différents nebouclent donc jamais sur les mêmes r.

main.o   .text  ──▶ .rela.text de main.o   (entrées : array, sum)
main.o   .data  ──▶ .rela.data de main.o   (si elle existe)
sum.o    .text  ──▶ .rela.text de sum.o    (absente ici)

Pourquoi r.offset est cohérent

Dans une entrée, r.offset est l'offset dans la section d'entrée à laquelle elle s'appliquo. C'est pourquoi la formule du livre écrit ADDR(s) + r.offset : ADDR(s) est l'adressefinale de cette section d'entrée (calculée en phase 1), pas celle de l'agrégat. Si les re les offsets deviendraient ambigus (offset 0x11 de quel fichier ?).

Ce qui est agrégé dans l'exécutable final

Les octets des sections de même type sont concaténés dans .text, .data, etc. Les relocatiuées, puis jetées. Il ne reste dans l'exécutable que les relocations de chargementdynamique (.rela.dyn, .rela.plt).

Le livre mélange les deux niveaux, ce qui prête à confusion : s désigne dans l'algorithme un morceau d'entrée, même s'il décrit ce qui deviendra une partie de l'agrégat.

✻ Sautéed for 8s · done 11:47 PM

❯ je pensais avoir compris que le linker agrégeait tout. En fait, ce n'est qu'à moitié vrai, il agrège tout, mais il garde en même temps les fichiers individuels ouverts et travaille en parallèle le fichier global et les fichiers individuels ? C'est pour ça que pour chaque module, et pour chaque section, et pour chaque relocation, il essaie de trouver l'adresse finale

Oui, c'est l'idée, avec une nuance sur « en parallèle » : c'est séquentiel, en deux passes.

Le modèle

Le linker garde en mémoire chaque .o séparément (ses sections, ses symboles, ses relocations) jusqu'à la fin. L'« agrégat » n'est pas un fichier construit d'abord : c'est surtout un plan de mise en page.

Passe 1 : mise en page (phase 1)
Pour chaque .o et chaque section d'entrée, le linker décide où elle ira dans l'agrégat :

main.o .text → agrégat .text, offset 0x00   → ADDR = 0x401000
sum.o  .text → agrégat .text, offset 0x30   → ADDR = 0x401030
main.o .data → agrégat .data, offset 0x00   → ADDR = 0x404000

Il en déduit ADDR(symbole) = ADDR(section d'entrée) + st_value. À ce stade, aucun octet de code n'a changé, c'est de la table.

Passe 2 : copie et patch (phase 2)
Il recopie les octets de chaque section d'entrée à sa place dans l'agrégat, et pour chaquit l'adresse calculée dans le trou. C'est ici que la triple boucle intervient : module,section, relocation.

Pourquoi il faut garder les modules distincts

Chaque relocation est exprimée relativement à son .o :
- r.offset est un offset dans la section de ce .o,
- l'index du symbole r.symbol pointe dans la .symtab de ce .o (qui peut être UND, donc ilinition dans un autre .o via la table globale des symboles).

Sans le .o d'origine, ces informations n'ont plus de sens.

Ce que vous aviez juste et ce qui est à ajuster

- Juste : il agrège les octets, et il garde les fichiers individuels pour interpréter leu
- À ajuster : ce n'est pas « il essaie de trouver l'adresse finale » à chaque relocation.nnue quand la passe 2 commence. La boucle ne fait que la lire dans la table et l'écriredans le trou. Le travail de calcul des adresses est entièrement fait en passe 1.