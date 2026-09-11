# OpenMP — tipovi zadataka i kako se razmišlja

# 0. Tri stvari od kojih sve zavisi

Skoro svaki OpenMP zadatak se rešava iz ove tri činjenice. Nauči ih pre svega ostalog.

### (1) `parallel` REPLICIRA, direktiva za podelu posla DELI

```c
#pragma omp parallel        // 4 niti, SVAKA izvršava ceo blok
{
    for (i=0;i<10;i++) f(); //  → f() se poziva 4 × 10 = 40 puta
}

#pragma omp parallel
{
    #pragma omp for         // iteracije se DELE među nitima
    for (i=0;i<10;i++) f(); //  → f() se poziva tačno 10 puta
}
```
**`parallel` sam po sebi ne deli ništa** — on samo kreira tim niti od kojih svaka izvršava
identičan kod. Tek `for` / `sections` / `single` dele posao.

### (2) Sve je DELJIVO po default-u — osim indeksa `omp for` petlje

Svaka promenljiva deklarisana **pre** paralelnog regiona je **shared**. Jedini automatski
izuzetak je **indeks petlje pridružene `omp for` direktivi** — on je uvek privatan.

Posledica: svaka privremena promenljiva u telu petlje (`y = sqrt(...)`, `t = ...`) je
**deljena** ako je ne proglasiš privatnom → **trka podataka**.

### (3) Trka podataka = ≥2 niti pristupaju istoj lokaciji, bar jedna piše, bez sinhronizacije

`x = x + 1` **nije atomično** — to su tri koraka (čitaj, dodaj, upiši). Dve niti mogu
pročitati istu vrednost pre nego što ijedna upiše → **jedno ažuriranje se gubi**.

---

# 1. TIP: „šta će biti ispisano / koliko puta"

**Tok razmišljanja — idi red po red i za svaku petlju pitaj se dva pitanja:**

1. **Ima li ta petlja `omp for` iznad sebe?**
   - DA → iteracije se dele → ukupno **N** izvršavanja tela
   - NE, ali je u paralelnom regionu → replicira se → **broj_niti × N**
2. **Ima li unutar tela direktivu koja ograničava KO izvršava?**
   - `master` → samo nit 0 → **N** izvršavanja (ostale niti prolaze prazno)
   - `single` → bilo koja jedna nit → **N**
   - `critical` / `atomic` → **NE menja broj** izvršavanja, samo ih **serijalizuje**

**Najveća zamka:** `critical` studenti čitaju kao „samo jednom". Netačno — `critical`
obezbeđuje da nisu **istovremeno**, a ne da se dešava **jednom**. Ako je petlja replicirana,
`critical` uredno pusti svih 40000 ispisa, samo jedan po jedan.

**Druga zamka:** `#pragma omp parallel` pa obična `for` petlja sa **deljenim** indeksom `i`
— tu formalno postoji **trka nad `i`**, pa broj izvršavanja nije strogo determinisan.
Očekivani odgovor je `broj_niti × N`, ali vredi napomenuti trku (nosi poene).

**Kontrolna tabela:**

| Konstrukcija | Broj izvršavanja tela |
|---|---|
| `parallel` + obična petlja | broj_niti × N |
| `parallel` + `for` | N |
| `parallel for` | N |
| `parallel` + obična petlja + `master` unutra | N |
| `parallel` + obična petlja + `critical` unutra | broj_niti × N (serijalizovano) |

*Javlja se: H7, H8.*

---

# 2. TIP: „koja vrednost će biti odštampana"

Ovde se sve svodi na to **kako se promenljiva deli između niti**. Za svaku promenljivu
odredi njen status, pa primeni pravilo:

| Status | Šta se dešava na ULASKU | Šta na IZLASKU |
|---|---|---|
| **shared** | jedna instanca za sve | promene ostaju vidljive |
| **private** | svaka nit dobija **neinicijalizovanu** kopiju | **ništa se ne kopira nazad** — spoljna vrednost **nepromenjena** |
| **firstprivate** | kopija inicijalizovana spoljnom vrednošću | ništa se ne kopira nazad |
| **lastprivate** | neinicijalizovana kopija | vrednost iz **poslednje** iteracije se upisuje napolje |
| **reduction(op:x)** | kopija = **neutralni element** operatora | sve kopije se kombinuju sa **originalnom** vrednošću |

### Neutralni elementi (ključno za reduction zadatke)
```
+  →  0        *  →  1        -  →  0
&& →  1        || →  0        &  →  svi bitovi 1        |  → 0        ^ → 0
```

**Kako se razmišlja o `reduction` zadatku — tri koraka:**
1. Svaka nit kreće od **neutralnog elementa**, NE od početne vrednosti promenljive
2. Izvrši telo nad tom privatnom kopijom
3. Kombinuj: `rezultat = originalna_vrednost  op  kopija₁ op kopija₂ op …`

*Primer razmišljanja:* `q=2`, `reduction(*:q)`, 4 niti, telo `q += 2`.
Kopije kreću od **1** (neutral za `*`) → svaka postane 1+2 = 3 → kombinovanje:
`2 × 3 × 3 × 3 × 3 = 162`. **Ne** 4×4×4×4, i **ne** kreće se od 2.

**Zamka sa `private`:** ako je promenljiva `private`, a čitaš je **posle** regiona — dobijaš
**staru, spoljnu vrednost**, kao da se paralelni region nikad nije desio. Sav rad je izgubljen.

**Zamka sa `shared` bez zaštite:** rezultat je **nedeterminisan**. Ne piši konkretan broj kao
odgovor — piši opseg i objasni zašto (izgubljena ažuriranja). Ako sabiraš pozitivne brojeve,
rezultat je **≤** tačnog zbira, nikad veći.

*Javlja se: H9, H10, H4.*

---

# 3. TIP: „postoji li trka podataka / je li program korektan"

**Tok razmišljanja:**
1. Popiši sve promenljive u paralelnom regionu i odredi im status (shared po default-u!)
2. Za svaku **deljivu** promenljivu pitaj: **piše li joj bar jedna nit?**
3. Ako da → ima li sinhronizacije (`critical`, `atomic`, `reduction`, `barrier`)?
4. Ako nema → **trka postoji**

**Drugo pitanje koje se često traži uz to: koje su vrednosti moguće?** To nije isto što i
„postoji li trka". Razmisli **šta se uopšte upisuje**:
- ako sve niti upisuju svoj `thread_id` → moguće vrednosti su samo **0..broj_niti−1**, nikad
  bilo šta drugo
- ako se radi `x = x + 1` → moguć je bilo koji broj **do** tačnog zbira, nikad preko

Zato su odgovori tipa „elementi mogu imati **bilo koju** vrednost" po pravilu **netačni** —
trka ne znači haos, znači nedeterminizam **u okviru onoga što se upisuje**.

*Javlja se: H1, H3, H4.*

### Podtip: dve niti čekaju na istu deljivu promenljivu
```
Thread 1          Thread 2
a++;              a++;
while(a < 2);     while(a < 2);
```
**Kako se razmišlja:** ne gledaj niti odvojeno — gledaj **koje su krajnje vrednosti `a`
moguće**:
- oba inkrementa prođu → `a = 2` → uslov `a<2` pada za **obe** niti → **obe izlaze**
- jedan inkrement se izgubi → `a = 1` **zauvek** → uslov važi za **obe** → **nijedna ne izlazi**

Pošto obe niti čekaju **istu promenljivu sa istim uslovom**, ne postoji ishod u kome jedna
izađe a druga ne. Zaključak: **ili obe ili nijedna**.

---

# 4. TIP: „je li ovaj program korektan" — deadlock

**Gde se traži deadlock:** kombinacija konstrukcije koja **propušta jednu nit** sa
konstrukcijom koja **čeka sve niti**.

```c
#pragma omp critical      // propušta JEDNU nit
{
    #pragma omp barrier   // čeka SVE niti  →  DEADLOCK
}
```
Nit koja je ušla u kritičnu sekciju čeka ostale na barijeri; ostale ne mogu ući jer je
sekcija zauzeta. Međusobno beskonačno čekanje.

**Opšte pravilo za pamćenje:** barijeru mora sresti **svaka nit tima, isti broj puta, istim
redosledom**. Sve što je stavlja u kod koji ne izvršavaju sve niti — `critical`, `single`,
`master`, grana `if` koja ne važi za sve — je **greška**.

*Javlja se: H2.*

---

# 5. TIP: „zašto nema ubrzanja / zašto su performanse loše"

Dva scenarija, i moraš prepoznati koji je u pitanju.

### 5a — `critical` obuhvata ceo posao
```c
#pragma omp parallel for
for (i=0;i<n;i++) {
    #pragma omp critical
    sum = sum + data[i];      // ← ovo JE ceo posao petlje
}
```
**Kako razmišljaš:** ako je unutar `critical` sve što petlja radi, onda je **celo izvršenje
serijalizovano** — u svakom trenutku radi tačno jedna nit. Uz to se plaća zaključavanje i
otključavanje brave u **svakoj** iteraciji, pa je paralelna verzija često **sporija** od
sekvencijalne.

**Rešenje — smanji broj ulazaka u kritičnu sekciju sa n na broj_niti:** privatna delimična
suma po niti (bez ikakve sinhronizacije), pa `critical` **jednom po niti** na kraju.

### 5b — lažno deljenje (false sharing)
Prepoznaje se po formulaciji: *„svaka nit pristupa samo svojoj promenljivoj, ali su
promenljive u susednim memorijskim lokacijama"*.

**Kako razmišljaš:** keš koherentni protokol radi na nivou **celog keš bloka**, ne pojedinačne
promenljive. Ako x, y i z završe u istom bloku, upis niti 1 u x **invalidira ceo blok** kod
niti 2 i 3 — pa one imaju promašaj pri pristupu svojim y i z, iako **logički ne dele ništa**.
Rezultat: blok se neprekidno prebacuje između procesora („ping-pong"), performanse propadaju
bez ijedne greške u programu.

**Rešenje:** padding — razmak između promenljivih da završe u različitim keš blokovima.

**Ovo je i razlog zašto se kod ručnih redukcija preko niza `part[thread_id]` mora paziti** —
susedni elementi niza su tipičan uzrok lažnog deljenja.

*Javlja se: H5, H6.*

---

# 6. TIP: `schedule` — ko izvršava koju iteraciju

**Kako radi `static, chunk`:** iteracije se seku na blokove veličine chunk i dodeljuju
nitima **round-robin, unapred**:
```
nit koja dobija blok c  =  c mod broj_niti
blok kome pripada iteracija i  =  ⌊i / chunk⌋
```
Odatle:
```
nit(i) = ⌊i / chunk⌋ mod broj_niti
```

**Da bi našao sve iteracije jedne niti t:** uzmi blokove `c = t, t+P, t+2P, …` (P = broj
niti), svaki pokriva iteracije `[c·chunk, c·chunk + chunk − 1]`.

**Zamka:** „76. iteracija" — proveri da li se misli na **indeks** i=76 ili na **redni broj**
(i=75). Napiši koju konvenciju koristiš.

*Javlja se: H11.*

---

# 7. TIP: „koji `schedule` izabrati" / „poređaj po brzini"

**Tok razmišljanja — dva faktora koja se sukobljavaju:**

**Faktor 1 — BALANS.** Pogledaj unutrašnju petlju i pitaj se: **da li posao po iteraciji
zavisi od i?**
```c
for (j = i;  j < n; j++)   →  posao OPADA sa i   →  NEURAVNOTEŽENO (trougaono)
for (j = 0;  j < n; j++)   →  posao je ISTI      →  URAVNOTEŽENO (pravougaono)
```

**Faktor 2 — CENA PLANIRANJA (overhead).**
- `static` (bilo koji chunk) → raspodela se računa **unapred** → **nula** overhead-a
- `dynamic, k` → nit traži novi blok kad završi → **broj dodela = broj_iteracija / k**
  sinhronizacija

**Odluka:**

| Situacija | Izbor | Zašto |
|---|---|---|
| Uravnoteženo opterećenje | **`static`** (bez chunk-a) | balans je već dobar, a overhead nula |
| Neuravnoteženo, overhead se **zanemaruje** | **`static,k` ILI `dynamic,k`** sa malim k | oba daju odličan balans |
| Neuravnoteženo, overhead se **računa** | **`static,k`** sa malim k | mali chunk meša teške i lake iteracije **bez** ijedne sinhronizacije |

**Ključno zapažanje za trougaonu petlju:** `static` **bez chunk-a** je **najgori** izbor — daje
svakoj niti veliki **susedni** blok, pa prva nit dobija sve najteže iteracije, a poslednja sve
najlakše (disbalans preko 1.7×). Isti taj `static` sa **chunk=1** je skoro savršen (disbalans
~1.03), jer round-robin svakoj niti podeli i teške i lake.

**Poredak po brzini za trougaonu petlju** (od najbržeg): sitniji chunk pobeđuje krupniji, a
`static` bez chunk-a je poslednji.

*Javlja se: H12, H13.*

---

# 8. TIP: uklanjanje barijera / `nowait`

**Tok razmišljanja — za svaku barijeru posebno pitaj:**

> „Da li naredni kod čita podatke koje su u prethodnoj petlji pisale **druge** niti?"

- **NE** → barijera je nepotrebna → `nowait`
- **DA** → barijera je **neophodna**, ne diraj je

**Zašto baš „druge niti":** nit uvek vidi svoje sopstvene upise. Problem nastaje samo ako
iteracija `i` u drugoj petlji čita element koji je u prvoj petlji računala **neka druga** nit
(npr. `v[i-1]`, ili se opsezi ne poklapaju).

**Dodatna dva pravila:**

1. **Barijera odmah posle implicitne barijere je redundantna.** Eksplicitni `#pragma omp
   barrier` odmah nakon `omp for` (koji već ima implicitnu barijeru) — obriši jedan od njih.

2. **Barijera na kraju `parallel` regiona se NE MOŽE ukloniti.** Zato poslednja `omp for`
   direktiva u regionu skoro uvek sme da dobije `nowait` — kraj regiona ionako sinhronizuje.

**Neintuitivan, ali koristan zaključak:** ako prva petlja ima `nowait`, a druga nema, barijera
na kraju **druge** petlje garantuje da su sve niti završile **i prvu i drugu** — jer nit stiže
do kraja druge tek pošto je prošla kroz prvu. Zato jedna barijera često pokriva dve petlje.

*Javlja se: H14, H15.*

---

# 9. TIP: paralelizuj dati kod

**Tok razmišljanja — uvek istim redom:**

### Korak 1 — ima li loop-carry zavisnost?
Pogledaj da li iteracija koristi rezultat prethodne (`x = x + 2`, `c = foo(c)`,
`C[i] = C[i-1] * …`). Ako nema → idi na korak 3.

### Korak 2 — ako ima, koja je vrsta?

| Vrsta | Prepoznavanje | Rešenje |
|---|---|---|
| **Indukciona promenljiva** | `x` se uvećava za konstantu u svakoj iteraciji | **zatvorena formula**: izrazi x direktno preko i (`x = 2*i`) — zavisnost nestaje |
| **Nezavisna od ostatka** | zavisni deo (`c = foo(c)`) ne utiče na nezavisni deo (`a[i] += …`) | **podeli petlju** (loop distribution): nezavisni deo paralelno, zavisni sekvencijalno |
| **Prava, nerešiva** | rezultat iteracije stvarno ulazi u sledeću | paralelizuj **unutrašnju** petlju; spoljna ostaje sekvencijalna |

### Korak 3 — proglasi privatnim sve privremene promenljive
Svaka promenljiva koja se **upisuje u telu petlje** a nije element niza indeksiran po `i`
mora biti `private` (`y = sqrt(A[i])` → `private(y)`). Inače trka.

Ako ti ta vrednost treba i **posle** petlje → `lastprivate` umesto `private`.

### Korak 4 — akumulacija?
Ako se nešto sabira/množi kroz sve iteracije → `reduction`. Ako zadatak **zabranjuje**
`reduction`, vidi TIP 10.

### Korak 5 — broj niti, ako se traži
```c
omp_set_num_threads(omp_get_num_procs());   // ili num_threads(omp_get_num_procs())
```

*Javlja se: H25, H26, H27.*

---

# 10. TIP: „paralelizuj BEZ `reduction`"

Ovo je najčešći „napiši kod" zadatak. Postoje **dva** obrasca — nauči oba jer zadatak ume da
zabrani i `critical`.

### Obrazac A — privatni akumulator + `critical` jednom po niti
```c
#pragma omp parallel
{
    int local = 0;                     // ili 1 za množenje

    #pragma omp for
    for (i = 0; i < N; i++)
        local += ...;                  // BEZ sinhronizacije

    #pragma omp critical
    global += local;                   // samo JEDNOM po niti
}
```

### Obrazac B — vektor delimičnih rezultata (kad se traži „pomoću vektora")
```c
int nt = omp_get_max_threads();
double part[nt];

#pragma omp parallel
{
    int id = omp_get_thread_num();
    part[id] = 0;                      // neutralni element

    #pragma omp for
    for (i = 0; i < N; i++)
        part[id] += ...;
}
for (int t = 0; t < nt; t++) global += part[t];   // spajanje posle regiona
```

**Ista logika za bilo koji operator** — samo promeni početnu vrednost na **neutralni element**
(0 za `+`, 1 za `*`, `-INFINITY` za `max`).

### Podtip: maksimum matrice
Pitanje glasi *„da li je moguće paralelizovati traženje maksimuma"*.

**Odgovor i obrazloženje:** DA — `max` je **asocijativna i komutativna** operacija, pa redosled
poređenja ne utiče na rezultat i posao se sme podeliti. Ograničenje je praktično, ne logičko:
stariji OpenMP standard za C/C++ **nema ugrađen `reduction(max:...)`** (dodat od OpenMP 3.1),
pa se radi ručno po obrascu A sa `local = -INFINITY`.

*Javlja se: H23, H24, H28.*

---

# 11. TIP: pitanja sa ponuđenim odgovorima

Ne uči odgovore napamet — uči **činjenicu iza njih**:

| Činjenica | Šta se iz nje pita |
|---|---|
| OpenMP je za **deljivu** memoriju (MPI je za distribuiranu) | tvrdnja „za distribuiranu memoriju" je **netačna** |
| OpenMP podržava **i UMA i NUMA** | obe su deljiva memorija — NUMA samo ima neuniformno vreme pristupa |
| `critical` daje **uzajamno isključenje** | konflikti po podacima unutar `critical` **nisu mogući** |
| Deadlock **jeste** moguć u `parallel` | v. TIP 4 |
| Zavisnosti po podacima u `omp for` **jesu** moguće | zato i daju pogrešan rezultat |
| `atomic` **ne prima argument** | `#pragma omp atomic(x)` je pogrešna sintaksa |
| `omp_get_*` su **get** funkcije | `omp_get_num_threads(4)` je besmislica; postavlja se sa `omp_set_num_threads(4)` ili `OMP_NUM_THREADS` |
| `single` = **bilo koja jedna** nit, **ima** barijeru | odgovor na „samo jedna nit izvrši blok" je `single`, **ne** `master` |
| `master` = **specifično nit 0**, **nema** barijeru | zato zahteva ručni `barrier` ako je sinhronizacija potrebna |
| OpenMP = **Open Multi-Processing** | — |

### Koje odredbe idu uz koju direktivu
```
parallel:  if, num_threads, default, private, firstprivate, shared, copyin, reduction
for:       private, firstprivate, lastprivate, reduction, schedule, ordered, nowait
sections:  private, firstprivate, lastprivate, reduction, nowait
```
**Logika iza toga, da ne moraš napamet:**
- `schedule` / `ordered` / `nowait` se tiču **podele iteracija i barijere** → samo `for`
  (i `nowait` uz `sections`) — `parallel` nema šta da rasporeduje, a njegova barijera je
  neuklonjiva
- `shared` / `if` / `num_threads` / `copyin` se tiču **kreiranja tima** → samo `parallel`
- `lastprivate` traži pojam „poslednje iteracije" → samo `for` i `sections`

### Prioritet određivanja broja niti (opadajući)
```
if(false) → 1 nit  >  num_threads(n)  >  omp_set_num_threads(n)  >  OMP_NUM_THREADS  >  default
```
**Caka:** `omp_get_num_threads()` pozvan **van** paralelnog regiona uvek vraća **1** — jer je
aktivna samo master nit. Za broj dostupnih niti pre regiona koristi `omp_get_max_threads()`.

*Javlja se: H16–H22.*

---

# 12. Kontrolna lista pre nego što predaš odgovor

- Je li petlja **replicirana** ili **podeljena**? (najčešća greška u zadacima sa brojanjem)
- Jesi li proverio status **svake** promenljive? Sve što nije `for`-indeks je **shared** dok
  ne kažeš drugačije.
- Kod `reduction` — jesi li krenuo od **neutralnog elementa**, a ne od početne vrednosti?
- Kod `private` — jesi li rekao da se vrednost **ne vraća** napolje?
- Kod trke — jesi li naveo **koje su vrednosti moguće**, a ne samo „nedeterminisano"?
- Kod paralelizacije — jesi li **sve privremene promenljive** proglasio privatnim?
- Kod `nowait` — čita li naredni kod ono što su pisale **druge** niti?
- Kod `schedule` — jesi li pogledao da li unutrašnja petlja zavisi od `i`?
