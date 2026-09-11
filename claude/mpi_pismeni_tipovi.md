# MPI — PISMENI deo: tipovi zadataka i obrasci rešavanja

Analizirano 19 rokova (2020–2025). MPI zadatak je **najbolje bodovan** na pismenom (često
40–45 poena).

| Gradivni blok | Pojavljivanja |
|---|---|
| **1. Distribucija matrice + množenje** | **15×** |
| **2. Paralelni fajl I/O** | **12×** |
| 3. MINLOC/MAXLOC („rezultat u procesu koji ima min/max") | 10× |
| 4. Kreiranje komunikatora | 10× |
| 5. Izvedeni tipovi podataka | 7× |

**Bitno:** to nisu odvojeni zadaci nego **blokovi koji se kombinuju**. Tipičan zadatak =
distribucija matrice (izvedeni tip) + računanje + MAXLOC + upis u fajl. Zato ih uči kao
gradivne delove, ne kao zasebne zadatke.

---

# 0. Kostur i rečnik zahteva

```c
#include <mpi.h>
int main(int argc, char **argv) {
    int rank, p;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /* ... */

    MPI_Finalize();
    return 0;
}
```
Pokretanje (traži se eksplicitno u nekim rokovima):
```
mpicc program.c -o program -lm
mpirun -np 16 ./program
```

## Rečnik: formulacija → šta MORAŠ upotrebiti

| Formulacija u zadatku | Obavezno |
|---|---|
| „koristiti **grupne operacije**" | `MPI_Bcast/Scatter/Gather/Allgather/Reduce` — **ne** `Send/Recv` |
| „koristiti **Point-to-Point** operacije" | `MPI_Send`/`MPI_Recv` — obrnuto |
| „slanje se obavlja **odjednom**" / „jednim `MPI_Send`" | **izvedeni tip podatka** |
| „šalje se **samo 1 izvedeni tip** podatka" | jedan `MPI_Type_*` koji pokriva **ceo** blok |
| „funkcije za **kreiranje novih komunikatora**" | `MPI_Comm_split` |
| „rezultat u procesu koji **sadrži minimum/maksimum**" | `MPI_MINLOC` / `MPI_MAXLOC` |
| „redosled **ne može se predvideti**" | **deljeni** pokazivač: `MPI_File_read_shared` |
| „redosled od **prvog do poslednjeg** procesa" | `MPI_File_write_ordered` |
| „**eksplicitni pomeraj**" | `MPI_File_write_at` / `read_at` |
| „**pojedinačni** pokazivači" | `MPI_File_write` (individual pointer) |
| „voditi računa o **efikasnosti** učitavanja" | kolektivne verzije (`_all`) + `MPI_File_set_view` |

---

# TIP 1 — Distribucija matrice (15×)

**Ceo zadatak stoji na jednom pitanju: kako je matrica podeljena?** Računanje posle toga je
trivijalno. Postoje četiri šeme i za svaku postoji gotov obrazac.

## 1a — Blok susednih VRSTA (najlakše)

*„Matrica A podeljena je u blokove od po n/p vrsta"*

Vrste su u memoriji **susedne** (row-major) → nije potreban izvedeni tip:
```c
int rowsPer = n / p;
double *local = malloc(rowsPer * n * sizeof(double));

MPI_Scatter(A, rowsPer * n, MPI_DOUBLE,
            local, rowsPer * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

## 1b — CIKLIČNE vrste ili kolone (`l mod p = i`)

*„proces P_i dobija kolone sa indeksima l, l mod p = i, tj. i, i+p, i+2p, …"*

Ovo je **najčešća** šema i traži izvedeni tip u dva koraka.

**Kolona** matrice `m×n` (row-major) = `m` elemenata sa razmakom `n`:
```c
MPI_Datatype colType, colResized, cyclicCols;

// 1) jedna kolona: m blokova od po 1 elementa, razmak n
MPI_Type_vector(m, 1, n, MPI_DOUBLE, &colType);
// 2) "skupi" tip na 1 element da bi susedne kolone bile 1 double razmaknute
MPI_Type_create_resized(colType, 0, sizeof(double), &colResized);
MPI_Type_commit(&colResized);
// 3) n/p kolona sa razmakom p  ->  ciklicna raspodela
MPI_Type_vector(n/p, 1, p, colResized, &cyclicCols);
MPI_Type_create_resized(cyclicCols, 0, sizeof(double), &cyclicCols);
MPI_Type_commit(&cyclicCols);

MPI_Scatter(A, 1, cyclicCols, localCols, (n/p)*m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

**`MPI_Type_create_resized` je ključ.** Bez njega `MPI_Scatter` pomera pokazivač za **ceo
opseg** tipa (cela kolona = do kraja matrice), pa drugi proces dobija smeće. Resize kaže
MPI-ju: „sledeći primerak počinje **1 double** dalje, ne ceo tip dalje".

**Ciklične vrste** su isto, samo bez unutrašnjeg razmaka:
```c
MPI_Type_vector(m/p, n, p*n, MPI_DOUBLE, &cyclicRows);   // m/p vrsta, po n elem., razmak p*n
MPI_Type_create_resized(cyclicRows, 0, n*sizeof(double), &cyclicRows);
MPI_Type_commit(&cyclicRows);
```

## 1c — Blokovi `k×k` u mreži procesa `q×q`

*„Broj procesa je p i uređeni su kao matrica q×q (q²=p)"*

```c
int q = (int)sqrt(p);
int k = n / q;
int myRow = rank / q, myCol = rank % q;

MPI_Datatype blockType;
MPI_Type_vector(k, k, n, MPI_DOUBLE, &blockType);          // k vrsta po k elem., razmak n
MPI_Type_create_resized(blockType, 0, k * sizeof(double), &blockType);
MPI_Type_commit(&blockType);

int *counts = malloc(p*sizeof(int)), *displs = malloc(p*sizeof(int));
for (int i = 0; i < p; i++) {
    counts[i] = 1;
    displs[i] = (i/q) * k * q + (i%q);      // pozicija bloka u jedinicama "resized" tipa
}
MPI_Scatterv(A, counts, displs, blockType,
             localBlock, k*k, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```
**Zašto `Scatterv` a ne `Scatter`:** blokovi nisu ravnomerno razmaknuti u linearnoj memoriji
(posle `q` blokova preskačeš `k` vrsta), pa moraš zadati **displacements** ručno.

## 1d — Nejednaki blokovi (`P_i` dobija `2^i` vrsta)

Kad blokovi **nisu jednake veličine**, `Scatterv` je jedini izbor:
```c
int *counts = malloc(p*sizeof(int)), *displs = malloc(p*sizeof(int));
int off = 0;
for (int i = 0; i < p; i++) {
    counts[i] = (1 << i) * n;        // 2^i vrsta puta n elemenata
    displs[i] = off;
    off += counts[i];
}
MPI_Scatterv(A, counts, displs, MPI_DOUBLE,
             local, counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

## Posle distribucije — računanje i skupljanje

**Množenje matrica podeljenih po vrstama (A) i kolonama (B):**
```c
// svaki proces racuna deo C od svojih vrsta A i svojih kolona B
for (int i = 0; i < rowsPer; i++)
    for (int j = 0; j < colsPer; j++) {
        double s = 0;
        for (int t = 0; t < n; t++) s += Arows[i*n+t] * Bcols[j*n+t];
        Cpart[i*colsPer + j] = s;
    }
MPI_Gather(Cpart, rowsPer*colsPer, MPI_DOUBLE, C, ..., 0, MPI_COMM_WORLD);
```

**Matrica × vektor sa podelom po KOLONAMA — pazi:** svaki proces daje **delimičnu sumu**
celog rezultujućeg vektora, ne komad vektora. Skupljanje ide **redukcijom**, ne `Gather`-om:
```c
for (int j = 0; j < colsPer; j++)
    for (int i = 0; i < m; i++)
        partial[i] += Acols[j*m + i] * bLocal[j];

MPI_Reduce(partial, c, m, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
```
**To je zamka koja se najviše naplaćuje:** podela po **vrstama** → `Gather`; podela po
**kolonama** → `Reduce(MPI_SUM)`.

## 1e — „obezbediti da svaki proces ima sve blokove iz iste vrste/kolone"

To je `Allgather` **unutar komunikatora vrste/kolone** (v. TIP 4):
```c
MPI_Allgather(myBlockA, k*k, MPI_DOUBLE, rowBlocksA, k*k, MPI_DOUBLE, rowComm);
MPI_Allgather(myBlockB, k*k, MPI_DOUBLE, colBlocksB, k*k, MPI_DOUBLE, colComm);
```

---

# TIP 2 — MINLOC / MAXLOC (10×)

*„Rezultat se prikazuje u procesu koji sadrži minimum svih vrednosti u matrici A"*
*„Naći zaposlenog s najmanjom prosečnom platom"*
*„Rezultat upisati u proces koji je učitao najveći element niza"*

Obična redukcija daje **vrednost**, ali ne i **ko je ima**. Za to služe `MPI_MINLOC` i
`MPI_MAXLOC` sa **parnim tipovima**:

```c
struct { double val; int rank; } in, out;

in.val = mojLokalniMin;
in.rank = rank;

MPI_Allreduce(&in, &out, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
// out.val  = globalni minimum
// out.rank = ID procesa koji ga ima

int root = out.rank;                         // sada znas gde da skupis rezultat
MPI_Gather(local, cnt, MPI_DOUBLE, result, cnt, MPI_DOUBLE, root, MPI_COMM_WORLD);
if (rank == root) { /* prikaz */ }
```

**Parni tipovi:** `MPI_DOUBLE_INT`, `MPI_FLOAT_INT`, `MPI_2INT`, `MPI_LONG_INT`.
Struktura mora biti **tačno** `{ vrednost; int; }` — MPI očekuje taj raspored.

**Zašto `Allreduce` a ne `Reduce`:** rezultat mora znati **svaki** proces, jer svi moraju
znati kome da pošalju podatke.

**Varijanta „koji zaposleni ima najmanju platu"** — isto, samo je `in.rank` zamenjen
**indeksom zaposlenog**, pa `out.rank` daje globalni indeks:
```c
in.val = najmanjaPlataKodMene;
in.rank = globalniIndeksTogZaposlenog;
MPI_Reduce(&in, &out, 1, MPI_FLOAT_INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
```

---

# TIP 3 — Izvedeni tipovi podataka (7×)

## 3a — Struktura (student, zaposleni)

*„Za svakog studenta se pamte: broj indeksa (int), ime (string), prezime (string), prosečna
ocena (float)"*

```c
typedef struct {
    int   indeks;
    char  ime[30];
    char  prezime[30];
    float prosek;
} Student;

MPI_Datatype studentType;
int          blocklen[4] = {1, 30, 30, 1};
MPI_Datatype types[4]    = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT};
MPI_Aint     disp[4], base;

Student s;
MPI_Get_address(&s,          &base);
MPI_Get_address(&s.indeks,   &disp[0]);
MPI_Get_address(&s.ime,      &disp[1]);
MPI_Get_address(&s.prezime,  &disp[2]);
MPI_Get_address(&s.prosek,   &disp[3]);
for (int i = 0; i < 4; i++) disp[i] -= base;

MPI_Type_create_struct(4, blocklen, disp, types, &studentType);
MPI_Type_commit(&studentType);

MPI_Bcast(studenti, n, studentType, 0, MPI_COMM_WORLD);   // "odjednom svim procesima"
```
**Obavezno `MPI_Get_address`, ne ručno računanje pomeraja** — kompajler ubacuje padding
između polja i ručni proračun je pogrešan.

## 3b — Nepravilan raspored („šahovska šema", dijagonale)

Kad označeni elementi nisu pravilan blok, koristi se `MPI_Type_indexed`:
```c
MPI_Datatype patternType;
int blocklens[CNT], displs[CNT];
int c = 0;
for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
        if ((i + j) % 2 == 0) {           // uslov iz slike
            blocklens[c] = 1;
            displs[c]    = i*n + j;        // pomeraj u ELEMENTIMA od pocetka matrice
            c++;
        }
MPI_Type_indexed(c, blocklens, displs, MPI_INT, &patternType);
MPI_Type_commit(&patternType);

if (rank == 0) {
    MPI_Send(A, 1, patternType, 1, 0, MPI_COMM_WORLD);   // JEDNIM pozivom
    MPI_Send(A, 1, patternType, 3, 0, MPI_COMM_WORLD);
    MPI_Send(A, 1, patternType, 5, 0, MPI_COMM_WORLD);
}
```
Prijemna strana može primiti kao `c` običnih `MPI_INT` — tipovi se ne moraju poklapati, samo
ukupan broj osnovnih elemenata.

## Kada koji tip

| Raspored | Funkcija |
|---|---|
| pravilan: `k` blokova od `b` elemenata sa stalnim razmakom | `MPI_Type_vector` |
| nepravilan: proizvoljni pomeraji | `MPI_Type_indexed` |
| polja različitih tipova (struktura) | `MPI_Type_create_struct` |
| podmatrica u većoj matrici | `MPI_Type_create_subarray` |
| **uvek uz Scatter/Gather nad blokovima** | + `MPI_Type_create_resized` |

**Nikad ne zaboravi `MPI_Type_commit`** pre upotrebe (i `MPI_Type_free` na kraju).

---

# TIP 4 — Kreiranje komunikatora (10×)

Sve preko `MPI_Comm_split(stari, color, key, &novi)`: procesi sa **istim `color`** ulaze u
isti novi komunikator, a `key` određuje njihov redosled (rank) u njemu.

## Vrste i kolone mreže `q×q`
```c
int q = (int)sqrt(p);
int myRow = rank / q, myCol = rank % q;

MPI_Comm rowComm, colComm;
MPI_Comm_split(MPI_COMM_WORLD, myRow, myCol, &rowComm);   // isti red -> ista boja
MPI_Comm_split(MPI_COMM_WORLD, myCol, myRow, &colComm);   // ista kolona -> ista boja
```

## Dijagonala (komunikator DIAG)
```c
int color = (myRow == myCol) ? 1 : MPI_UNDEFINED;
MPI_Comm diagComm;
MPI_Comm_split(MPI_COMM_WORLD, color, rank, &diagComm);

if (diagComm != MPI_COMM_NULL) {         // procesi van dijagonale dobijaju NULL
    int drank, dsize;
    MPI_Comm_rank(diagComm, &drank);
    MPI_Comm_size(diagComm, &dsize);
    MPI_Bcast(podaci, n, studentType, 0, diagComm);
}
```
**`MPI_UNDEFINED` je ključ** — procesi koji ne pripadaju grupi dobiju `MPI_COMM_NULL` i
**moraju** biti isključeni `if`-om, inače program pukne.

## Procesi sa ID deljivim sa x
```c
int color = (rank % x == 0) ? 1 : MPI_UNDEFINED;
MPI_Comm_split(MPI_COMM_WORLD, color, rank, &divComm);
```

---

# TIP 5 — Paralelni fajl I/O (12×)

Postoje **tri načina** pozicioniranja i zadatak uvek eksplicitno kaže koji hoće.

| Formulacija | Funkcija | Ponašanje |
|---|---|---|
| „**pojedinačni pokazivači**", redosled od prvog do poslednjeg | `MPI_File_write_ordered` | kolektivno, po rangu |
| „**eksplicitni pomeraj**" | `MPI_File_write_at` / `MPI_File_read_at` | svaki proces zna svoju poziciju |
| „redosled se **ne može predvideti**" | `MPI_File_read_shared` / `write_shared` | deljeni pokazivač, ko stigne |

## Otvaranje i zatvaranje
```c
MPI_File fh;
MPI_File_open(MPI_COMM_WORLD, "file1.dat",
              MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
/* ... */
MPI_File_close(&fh);
```
Za čitanje: `MPI_MODE_RDONLY`.

## 5a — Upis redom od prvog do poslednjeg procesa
```c
int cnt = M*(M+1)/2;
MPI_File_write_ordered(fh, buf, cnt, MPI_INT, MPI_STATUS_IGNORE);
```
Jedna linija — `ordered` sama garantuje redosled po rangu. (Ručna alternativa: svaki proces
računa `offset = rank * cnt * sizeof(int)` i koristi `write_at`.)

## 5b — Prvi proces piše 0..K−1, drugi od K sa pomerajem K…
```c
int *buf = malloc(K * sizeof(int));
for (int i = 0; i < K; i++) buf[i] = rank*K + i;

MPI_Offset off = (MPI_Offset)rank * K * sizeof(int);
MPI_File_write_at_all(fh, off, buf, K, MPI_INT, MPI_STATUS_IGNORE);
```
`_at_all` je **kolektivna** varijanta — brža, i pokriva zahtev „voditi računa o efikasnosti".

## 5c — Nepredvidiv redosled čitanja (deljeni pokazivač)
```c
MPI_File_read_shared(fh, buf, cnt, MPI_INT, MPI_STATUS_IGNORE);
```
Svaki poziv pomera **zajednički** pokazivač — koji proces dobije koji deo nije određeno.
Upravo to zadatak i traži kad kaže „pozicija sa koje svaki pojedinačan proces čita podatke
ne može se predvideti".

## 5d — Isprepletani (interleaved) blokovi po slici

Kad slika pokazuje da procesi upisuju **naizmenične blokove**, koristi se **file view**:
```c
MPI_Datatype fileType;
MPI_Type_vector(numBlocks, blockSize, p * blockSize, MPI_INT, &fileType);
MPI_Type_commit(&fileType);

MPI_Offset disp = (MPI_Offset)rank * blockSize * sizeof(int);
MPI_File_set_view(fh, disp, MPI_INT, fileType, "native", MPI_INFO_NULL);

MPI_File_write_all(fh, buf, numBlocks * blockSize, MPI_INT, MPI_STATUS_IGNORE);
```
**`MPI_File_set_view` je odgovor na svaku „sliku sa isprepletanim blokovima".** Posle
postavljanja pogleda proces „vidi" samo svoje blokove kao neprekidan niz i piše ih jednim
kolektivnim pozivom — to je i najefikasnije, što zadatak izričito traži.

## 5e — Round-robin, od poslednjeg do prvog procesa
```c
int chunk = myCount / 10;
for (int t = 0; t < 10; t++) {
    MPI_Offset off = (MPI_Offset)(t * p + (p - 1 - rank)) * chunk * sizeof(int);
    MPI_File_write_at_all(fh, off, buf + t*chunk, chunk, MPI_INT, MPI_STATUS_IGNORE);
}
```
Obrnuti redosled se dobija sa `p - 1 - rank` u proračunu pomeraja.

## 5f — Efikasno čitanje cikličnih kolona iz fajla

*„Vektor b učitati iz fajla tako da P_i učita elemente i, i+p, i+2p…"*
```c
MPI_Datatype cyclicType;
MPI_Type_vector(n/p, 1, p, MPI_DOUBLE, &cyclicType);
MPI_Type_commit(&cyclicType);

MPI_File_set_view(fh, rank * sizeof(double), MPI_DOUBLE, cyclicType,
                  "native", MPI_INFO_NULL);
MPI_File_read_all(fh, bLocal, n/p, MPI_DOUBLE, MPI_STATUS_IGNORE);
```
Isti izvedeni tip radi i za distribuciju iz memorije (TIP 1b) i za čitanje iz fajla — samo se
jednom koristi u `Scatter`, drugi put u `set_view`.

---

# Sklapanje tipičnog zadatka

Zadatak od 45 poena obično traži sve odjednom. Redosled rada:

1. **`MPI_Init`, rank, size**, izračunaj `q = √p`, `myRow`, `myCol`
2. **Kreiraj komunikatore** (`rowComm`, `colComm`, ili `diagComm`)
3. **Napravi izvedeni tip** za blok/kolonu/strukturu (+ `resized` + `commit`)
4. **Distribuiraj** (`Scatter`/`Scatterv`/`Bcast`, ili `Send`/`Recv` ako traže P2P)
5. **Lokalno računanje**
6. **Ako se traži „proces koji ima min/max"** → `MPI_Allreduce` sa `MINLOC/MAXLOC`
7. **Skupi rezultat** — `Gather` (podela po vrstama) ili `Reduce(SUM)` (podela po kolonama)
8. **Upiši u fajl** ako se traži (`set_view` + `write_all`)
9. **`MPI_Type_free`, `MPI_Comm_free`, `MPI_Finalize`**
10. **Nacrtaj primer** za konkretno `n` i `p` — traži se u pola rokova

---

# Kontrolna lista

- Traže li **grupne** ili **Point-to-Point** operacije? (pola poena pada na tome)
- „Šalje se odjednom / jednim `MPI_Send`" → **izvedeni tip**, ne petlja
- Uz svaki `Type_vector` za blokove ide **`Type_create_resized`** — inače Scatter promašuje
- **`MPI_Type_commit`** pre upotrebe
- Podela po **kolonama** → skupljanje je **`Reduce(SUM)`**, ne `Gather`
- „Rezultat u procesu koji ima min/max" → **`Allreduce` + `MINLOC/MAXLOC`**, ne obična redukcija
- Kod `Comm_split` sa `MPI_UNDEFINED` — proveri `!= MPI_COMM_NULL` pre upotrebe
- Fajl: „ne može se predvideti" → **shared**; „redosled od prvog do poslednjeg" → **ordered**;
  „eksplicitni pomeraj" → **at**
- Slika sa isprepletanim blokovima → **`MPI_File_set_view`**
- „Voditi računa o efikasnosti" → kolektivne `_all` verzije
- Ako se traži — napiši i **komandnu liniju** (`mpicc`, `mpirun -np p`) i **primer podele**
