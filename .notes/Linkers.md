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

