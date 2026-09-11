# OpenMP — PISMENI deo: tipovi zadataka i obrasci rešavanja

Analizirano 19 rokova (2020–2025).

| Tip | Pojavljivanja |
|---|---|
| **1. Analiza + transformacija + paralelizacija petlje** | **15×** |
| 2. Traženje max/min sa i bez `critical` | 2× |
| 3. `task` direktiva — koliko taskova, redosled | 1× |
| 4. Množenje matrica | 1× |

**Praktično: ako izađeš na pismeni, skoro sigurno dobijaš TIP 1.** Ostalo je usputno.

---

# TIP 1 — Analiza i transformacija petlje (15×)

## Format zadatka (uvek isti)

> a) Da li je moguće izvršiti paralelizaciju date petlje bez modifikacija? Obrazložiti.
> b) Ukoliko postoje zavisnosti, objasniti ih (i o kom tipu zavisnosti je reč).
> c) Transformisati petlju tako da paralelizacija bude moguća.
> d) Razvijanjem petlje za konkretne vrednosti, testirati sekvencijalno i paralelno rešenje i
>    pokazati da je korektno paralelizovana.
>
> **Napomena: zadaci koji daju odgovor samo na stavku c) neće biti bodovani.**

**Ta napomena je ključna** — ako samo napišeš paralelan kod bez analize zavisnosti (a, b) i bez
provere (d), dobijaš **0**. Vreme rasporedi na sve četiri stavke.

---

## KORAK 1 — Popiši svaku naredbu i klasifikuj je

Petlja uvek ima **2–3 naredbe**. Za svaku odredi kojoj od pet kategorija pripada:

| Kategorija | Kako izgleda | Rešenje |
|---|---|---|
| **A. Akumulator** | `prod *= x[i]`, `g += z[i]`, `x = x + a[i][j]` — promenljiva se akumulira i **NE koristi** se dalje u telu | `reduction` |
| **B. Indukciona** | `d = d + m`, `t += k`, `a *= 4` — ali se **KORISTI** u telu (kao indeks ili operand) | **zatvorena formula** preko `i` |
| **C. Nezavisna** | `b[i][j] = a[i-2][j]` — piše u jedan niz, čita iz **drugog** | nema zavisnosti, ništa ne radi |
| **D. Zavisnost na nizu** | `a[i] = … a[i±k]` — **isti** niz i piše i čita | v. KORAK 2 |
| **E. Prefiks (scan)** | `rok[i] = rok[i-1] + a` gde je `a` i samo akumulator | najteže, v. dole |

**Prvo pitanje kod akumulatora:** *koristi li se ta promenljiva još negde u telu petlje?*
- **NE** → obična `reduction`, gotovo
- **DA** → to je **indukciona** promenljiva, treba zatvorena formula

*Zašto:* `reduction` daje samo **finalnu** vrednost posle petlje. Ako telo koristi
**međuvrednost** iz iteracije `i`, reduction ne pomaže.

---

## KORAK 2 — Za zavisnost na nizu: RAW ili WAR?

Ovo je **srce zadatka**. Pravilo:

> Element koji se **čita** biće upisan u iteraciji čiji je indeks jednak tom indeksu.
> Pitaj se: **da li je ta iteracija već izvršena?**
> - **JESTE** → element je već upisan → **RAW (prava zavisnost)** → teško
> - **NIJE** → element se piše **kasnije** → **WAR (anti-zavisnost)** → lako

**Obavezno pogledaj smer petlje** — isti izraz daje suprotan odgovor:

| Petlja | Čita se | Zaključak |
|---|---|---|
| `for(i=0; i<N; i++)` (raste) | `a[i+1]` | piše se **kasnije** → **WAR** |
| `for(i=0; i<N; i++)` (raste) | `a[i-1]` | pisano **ranije** → **RAW** |
| `for(i=N-2; i>=0; i--)` (opada) | `a[i+1]` | pisano **ranije** → **RAW** |
| `for(i=N-2; i>=0; i--)` (opada) | `a[i-1]` | piše se **kasnije** → **WAR** |

**Ovo je zamka koju profesorka koristi najviše** — 2023_jan (`a[i]=…-a[i+1]`, i opada → RAW)
i 2024_jun (`a[i]=…-a[i-1]`, i opada → WAR) izgledaju skoro identično, a rešenja su
potpuno različita.

---

## KORAK 3 — Primeni odgovarajući obrazac

### Obrazac A — `reduction` (akumulator koji se ne koristi u telu)
```c
#pragma omp parallel for reduction(*:prod)
for (i = 0; i < N; i++) {
    ...
    prod = prod * x[i];
}
```
Radi za `+ - * && || & | ^`. Za deljenje (`d = d / z[i]`) **nema** reduction operatora —
akumuliraj proizvod pa podeli jednom:
```c
double p = 1.0;
#pragma omp parallel for reduction(*:p)
for (i = 0; i < N; i++) p *= z[i];
d = d0 / p;
```

### Obrazac B — zatvorena formula (indukciona promenljiva)

Izrazi vrednost promenljive **direktno preko `i`**:

| Kod | Vrednost u iteraciji `i` | Posle petlje |
|---|---|---|
| `d = d + m` (petlja po m od 0) | `d0 + m(m−1)/2` | `d0 + n(n−1)/2` |
| `a *= 4` | `a0 · 4^i` | `a0 · 4^N` |
| `t += k` u **ugnježdenoj** petlji | `t0 + k·(i·M + j)` | `t0 + k·M·M` |

**Za ugnježdene petlje** ključno je izračunati **linearni redni broj iteracije**:
`n = i·M + j`. To je najčešća greška — studenti pišu `t0 + k·j` i gube spoljnu petlju.

### Obrazac C — kopiranje niza (WAR / anti-zavisnost)

Anti-zavisnost postoji samo zato što upis **uništava** vrednost koju neko još treba da
pročita. Ako sačuvaš **originalni** niz, zavisnost nestaje:
```c
double *a_old = malloc(N * sizeof(double));
memcpy(a_old, a, N * sizeof(double));

#pragma omp parallel for
for (i = 0; i < N; i++)
    a[i] = a_old[i+1] + b[i] * c[offset + N-i-1];

free(a_old);
```
**WAR je uvek rešiva ovako.** RAW nije — tu vrednost stvarno moraš prvo izračunati.

### Obrazac D — podela petlje (loop distribution)

Kad su u telu **dve naredbe** i zavisnost ide **između njih**, razdvoj ih u dve petlje.
Redosled dve nove petlje bira se prema tipu zavisnosti:

**RAW** (`y[i] = y[i] + x[i-1];  x[i] = x[i] + z[i];`) — `y` treba **novu** vrednost `x`:
```c
#pragma omp parallel for
for (i = 1; i < N; i++) x[i] = x[i] + z[i];      // PRVO x

#pragma omp parallel for
for (i = 1; i < N; i++) y[i] = y[i] + x[i-1];    // PA y
```

**WAR** (`y[i] = y[i] + x[i+1];  x[i] = x[i] + z[i];`) — `y` treba **staru** vrednost `x`:
```c
#pragma omp parallel for
for (i = 1; i < N; i++) y[i] = y[i] + x[i+1];    // PRVO y (cita staro x)

#pragma omp parallel for
for (i = 1; i < N; i++) x[i] = x[i] + z[i];      // PA x
```

Isto važi kad je jedna naredba zavisna a druga nije (`c = foo(c)` pored `a[i] += …`):
nezavisni deo paralelno, zavisni sekvencijalno.

### Obrazac E — paralelizuj UNUTRAŠNJU petlju (RAW po spoljnoj)

Kad je zavisnost po spoljnoj petlji (`b[i][j] = 4*b[i-2][j]`), a unutrašnja `j` je čista:
```c
#pragma omp parallel for reduction(+:x)      // ako ima akumulatora, ide OVDE
for (i = 2; i < N; i++)                       // ne, ovo ne valja - i nosi zavisnost
```
Ispravno — spoljna ostaje sekvencijalna:
```c
for (i = 2; i < N; i++) {
    #pragma omp parallel for reduction(+:x)
    for (j = 0; j < M; j++) {
        x = x + a[i][j];
        b[i][j] = 4 * b[i-2][j];
    }
}
```
**Mana:** N−2 fork-join para. Ako se traži bolje rešenje, izvedi **zatvorenu formulu** za
lanac razmaka 2:
```
b[i][j] = 4·b[i-2][j]  ⟹  b[i][j] = 4^(⌊i/2⌋) · b[i mod 2][j]
```
pa je i spoljna petlja paralelizabilna.

### Obrazac F — podela opsega (zavisnost samo u delu iteracija)
```c
int a = 6;
for (i = 0; i < N; i++) {
    if (i >= N/2) b[i] = b[i-1];    // RAW samo za drugu polovinu
    b[i]++;
    a *= 4;
}
```
Prva polovina je čista, druga je lanac `b[i] = b[i-1] + 1`. Razdvoj i izvedi formulu za lanac:
```c
#pragma omp parallel for
for (i = 0; i < N/2; i++) b[i]++;              // nezavisno

int base = b[N/2 - 1];                          // vec ima +1
#pragma omp parallel for
for (i = N/2; i < N; i++)
    b[i] = base + (i - (N/2 - 1));              // zatvorena formula lanca

a = 6 * pow(4, N);
```

### Obrazac G — prefiks suma / scan (najteži, 1×)
```c
for (i = N-1; i > 1; i--) {
    a += okt[i] + okt2[i];      // a je prefiks suma
    rok[i] = rok[i-1] + a;      // rok je opet prefiks suma
}
```
Ovde `a` **jeste** akumulator ali se **koristi** u telu → indukciona, a njena vrednost u
iteraciji `i` je **prefiks suma** (nema zatvorene formule kao kod `+k`).

**Odgovor na a):** nije paralelizabilno bez transformacije — dve ulančane prave (RAW)
zavisnosti.

**Rešenje:** dvostepeno, preko prefiks sume:
```c
// 1) izracunaj a[i] za svako i - paralelni scan (OpenMP 5.0)
#pragma omp parallel for reduction(inscan, +:a)
for (i = N-1; i > 1; i--) {
    a += okt[i] + okt2[i];
    #pragma omp scan inclusive(a)
    A[i] = a;
}
// 2) rok je opet scan nad (rok_pocetno + A) - isti postupak
```
Ako `inscan` nije dozvoljen, navedi da se radi **dvoprolaznim** algoritmom (svaka nit
izračuna svoju lokalnu sumu → sekvencijalni scan po nitima → svaka nit doda svoj ofset).

### Obrazac H — trka na `c[j]` u ugnježdenoj petlji
```c
for (i = 0; i < M; i++)
    for (j = 0; j < M; j++) { c[j] += e[t];  t += k; }
```
Dva problema odjednom: `t` je indukciona (indeks!), a `c[j]` se akumulira **preko `i`** — ako
paralelizuješ **spoljnu** petlju, dve niti pišu isti `c[j]` → **trka**.

Paralelizuj **unutrašnju** (svaki `j` je poseban element, nema trke) + zatvorena formula za t:
```c
for (i = 0; i < M; i++) {
    #pragma omp parallel for
    for (j = 0; j < M; j++)
        c[j] += e[3 + k * (i*M + j)];       // t izrazeno preko i i j
}
t = 3 + k * M * M;                           // vrednost posle petlje
```
Ili, ako je dozvoljen OpenMP 4.5, redukcija nad nizom pa i spoljna ide paralelno:
```c
#pragma omp parallel for private(j) reduction(+:c[:M])
for (i = 0; i < M; i++)
    for (j = 0; j < M; j++) c[j] += e[3 + k*(i*M + j)];
```

---

## KORAK 4 — stavka d) testiranje (ne preskači, nosi poene)

Traži se da **razviješ petlju za konkretne vrednosti** i pokažeš da sekvencijalno i paralelno
daju isto. Napiši:

```c
// razvijanje za N = 6, i = 4:
//   a[4] = b[4]*c[pom+1] - a[3]
//   a[3] = b[3]*c[pom+2] - a[2]   ...
```
pa kod:
```c
int main() {
    int N = 6;
    double a1[6], a2[6], b[6], c[20];
    /* ista inicijalizacija za oba */

    sekvencijalno(a1, b, c, N);
    paralelno   (a2, b, c, N);

    for (int i = 0; i < N; i++)
        if (fabs(a1[i] - a2[i]) > 1e-9) { printf("GRESKA na %d\n", i); return 1; }
    printf("OK\n");
    return 0;
}
```
Zadatak često traži da se prikažu **i vrednosti skalarnih promenljivih** (`d`, `prod`, `x`,
`a`) posle petlje — zato uvek napiši i njihovu zatvorenu formulu, ne samo niz.

---

## Tabela svih 15 rokova (isti zadatak, drugi brojevi)

| Rok | Petlja | Obrasci |
|---|---|---|
| 2020_sep | `b[m]=b[m+1]*d; d=d+m;` | **C** (WAR na b) + **B** (d se KORISTI) |
| 2022_jan / 2022_dec | `a[i]=a[i+1]+…; prod*=x[i];` | **C** (WAR) + **A** |
| 2022_jun | `y[i]=y[i]+x[i-1]; x[i]=x[i]+z[i]; g+=z[i-1];` | **D-RAW** (x pa y) + **A** |
| 2022_jun2 | `y[i]=y[i]+x[i+1]; x[i]=x[i]+z[i]; g+=z[i-1];` | **D-WAR** (y pa x) + **A** |
| 2022_sep | `x=x+a[i][j]; b[i][j]=a[i-2][j];` | **samo A** — nema zavisnosti! |
| 2022_okt | `x=x+a[i][j]; b[i][j]=4*b[i-2][j];` | **A** + **E/RAW razmak 2** |
| 2023_jan / 2023_sep | `d=d/z[i]; a[i]=…-a[i+1];` (i **opada**) | **A** (deljenje!) + **RAW** |
| 2023_okt2 b | `a+=okt[i]+okt2[i]; rok[i]=rok[i-1]+a;` | **G** (dvostruki scan) |
| 2024_jun | `d=d*z[i]; a[i]=…-a[i-1];` (i **opada**) | **A** + **C (WAR)** |
| 2024_okt / 2025_jan | `c[j]+=e[t]; t+=k;` | **H** (t indeks + trka na c) |
| 2025_jun | `if(i>=N/2) b[i]=b[i-1]; b[i]++; a*=4;` | **F** (podela opsega) + **A** |
| kolokvijum | `h+=a[i][j]; a[i][j]=a[i-2][j];` | **A** + **RAW razmak 2** |

**Obrati pažnju na 2022_sep** — jedini gde je tačan odgovor na a) **„DA, moguće je"**
(`b` se piše, `a` samo čita — različiti nizovi). Dovoljno je dodati `reduction(+:x)` i
`private(j)`. Ne izmišljaj zavisnost koje nema.

---

# TIP 2 — Max/min sa i bez `critical` (2×)

Formulacija: *„Da li je moguće paralelizovati petlju kojom se traži maksimalna vrednost…?
Napisati kod sa i bez `critical`. Razmotriti oba slučaja. Dati primer podele iteracija po
nitima."*

**Odgovor na „da li je moguće":** DA — `max` je **asocijativna i komutativna** operacija, pa
redosled poređenja ne utiče na rezultat.

**Bez `critical`** (`reduction`, OpenMP 3.1+):
```c
double max_val = A[0];
#pragma omp parallel for reduction(max: max_val)
for (int i = 0; i < N; i++)
    if (A[i] > max_val) max_val = A[i];
```

**Sa `critical`** (radi i na starijim standardima):
```c
double max_global = -INFINITY;

#pragma omp parallel
{
    double max_local = -INFINITY;          // privatni maksimum

    #pragma omp for
    for (int i = 0; i < N; i++)
        if (A[i] > max_local) max_local = A[i];

    #pragma omp critical                    // SAMO JEDNOM po niti
    if (max_local > max_global) max_global = max_local;
}
```
**Obavezno napomeni zašto `critical` ide izvan petlje:** da je unutar `for` petlje, svaka
iteracija bi ulazila u kritičnu sekciju → potpuna serijalizacija → nema ubrzanja.

**Za matricu** ista stvar, samo dvostruka petlja (`collapse(2)` ili `private(j)`).

**„Primer podele iteracija po nitima"** — nacrtaj za konkretan slučaj, npr. N=100, 4 niti,
`schedule(static)`: nit0 → 0–24, nit1 → 25–49, nit2 → 50–74, nit3 → 75–99.

---

# TIP 3 — `task` direktiva (1×)

```c
omp_set_num_threads(3);
#pragma omp parallel
{
    #pragma omp task
        printf("Task 1\n");
    #pragma omp task
        printf("Task 2\n");
}
```

**Koliko taskova se generiše: 6.** Nema direktive za podelu posla (`single`/`for`), pa
**svaka od 3 niti izvršava ceo blok** i kreira **po 2 taska** → 3 × 2 = 6.

**Da li je redosled definisan: NE.** Task se stavlja u red, a koja nit će ga preuzeti i kada
nije određeno. Jedina garancija je da će svi biti završeni do **implicitne barijere na kraju
`parallel` regiona**.

**Ako se pita kako dobiti 2 taska** — omotati u `single`:
```c
#pragma omp parallel
{
    #pragma omp single
    {
        #pragma omp task
            printf("Task 1\n");
        #pragma omp task
            printf("Task 2\n");
    }
}
```

---

# TIP 4 — Množenje matrica (1×)
```c
#pragma omp parallel for private(j, k) collapse(2)
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++) {
        double sum = 0.0;                  // lokalna -> nema trke
        for (int k = 0; k < N; k++)
            sum += A[i][k] * B[k][j];
        C[i][j] = sum;
    }
```
Nema zavisnosti — svaki `C[i][j]` piše tačno jedna iteracija. Akumulator `sum` je
**deklarisan unutar** petlje pa je automatski privatan.

---

# Kontrolna lista za pismeni

- Jesi li odgovorio na **a), b) i d)**, a ne samo c)? (bez toga je 0 poena)
- Za svaki akumulator: **koristi li se u telu?** Ako da → indukciona, treba formula
- Za zavisnost na nizu: **kojim smerom ide petlja?** RAW i WAR se razlikuju samo po tome
- WAR → kopiraj niz. RAW → zatvorena formula / distribucija / paralelizuj unutrašnju
- Kod ugnježdenih: linearni redni broj iteracije je **`i·M + j`**, ne `j`
- Ako paralelizuješ spoljnu, a akumuliraš u `c[j]` → **trka**; paralelizuj unutrašnju
- `critical` **nikad** unutar petlje po iteracijama — samo jednom po niti
- Napiši i vrednosti **skalarnih** promenljivih posle petlje (`d`, `prod`, `a`, `t`)
