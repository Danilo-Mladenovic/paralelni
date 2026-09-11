# GPU / CUDA — tipovi zadataka i kako se razmišlja

# 0. Osnova

Cela oblast stoji na **preslikavanju softvera na hardver**:

```
SOFTVER              HARDVER
─────────────────────────────────────────
Grid (rešetka)   →   ceo GPU čip
Blok niti        →   JEDAN SM (nikad se ne deli između SM-ova)
Warp (32 niti)   →   SIMD jedinica unutar SM-a
Nit              →   skalarni procesor (SP)
```

**Iz ovog preslikavanja slede skoro svi odgovori.** Kad ne znaš odgovor, vrati se na pitanje:
*„gde se to fizički nalazi i ko tome može da priđe?"*

## Dve posledice koje se najviše pitaju

**(1) Zašto se posao deli DVA puta (na blokove, pa na niti)?**
Zato što GPU ima **više SM-ova**. Podela na **blokove** raspoređuje posao **između** SM-ova —
nezavisno, bez sinhronizacije. Podela na **niti** daje fino-granularni paralelizam **unutar**
jednog SM-a, gde niti mogu da sarađuju.

**(2) Zašto niti iz različitih blokova ne mogu da sarađuju?**
Dva razloga, i oba treba navesti:
- **fizički:** deljiva memorija je **lokalna za SM**; različiti blokovi su najčešće na
  različitim SM-ovima
- **logički (važniji):** **redosled izvršavanja blokova nije garantovan** — blok može biti
  pokrenut, završen i zamenjen drugim. Saradnja bez garantovanog redosleda nema smisla.

**To isto omogućava transparentno skaliranje:** pošto blokovi ne zavise jedan od drugog, isti
kod se na GPU sa više SM-ova automatski izvršava brže, bez izmene.

---

# 1. TIP: warp-ovi i zauzetost SM-a

Ovo je glavni računski zadatak. **Dva ograničenja se sukobljavaju** i moraš proveriti oba:

```
(a) kapacitet SM-a u NITIMA       (npr. 1536)
(b) maksimalan broj BLOKOVA na SM (npr. 8)
```

**Tok razmišljanja:**
```
1. niti_po_bloku   = dimenzije bloka pomnožene
2. warpova_po_bloku = niti_po_bloku / 32
3. blokova_na_SM   = min( ⌊kapacitet_niti / niti_po_bloku⌋ , limit_blokova )
4. zauzetost       = blokova_na_SM × niti_po_bloku / kapacitet_niti
```

## Dva načina da izgubiš performanse — prepoznaj oba

**Gubitak 1 — blok premali.** Limit **blokova** se dostigne pre nego što se popuni kapacitet
niti. Npr. blok od 64 niti: 8 blokova × 64 = 512 od 1536 → **33%**. Niti ima mesta, ali
blokova nema.

**Gubitak 2 — blok prevelik / ne deli kapacitet.** Npr. 576 niti: 1536/576 = 2.67 → staju
samo **2 bloka** (blok je nedeljiv) → 1152 od 1536 → **75%**.

**Gubitak 3 — nije umnožak 32.** Npr. 240 niti = 7.5 warp-ova → hardver alocira **8** warp-ova,
poslednji je polupopunjen → 16 niti stoji neaktivno u svakom bloku.

## Pravilo za „koja je veličina bloka najbolja"

Dobra veličina bloka ispunjava **oba** uslova:
1. **umnožak 32** (puni warp-ovi)
2. **deli kapacitet SM-a** u broju blokova koji **ne prelazi limit blokova**

*Provera:* 256 niti → 8 punih warp-ova ✔, 1536/256 = 6 blokova ≤ 8 ✔ → **100% zauzetosti**.

---

# 2. TIP: indeksiranje niti

**Standardno 1D mapiranje:**
```c
int i = blockIdx.x * blockDim.x + threadIdx.x;
```

**Zašto baš to — dva kriterijuma:**
1. **Bijektivnost** — svaka nit dobija jedinstven indeks, bez preklapanja i bez rupa
2. **Coalescing** — `threadIdx.x` se menja **najbrže**, pa **susedne niti u istom warp-u
   adresiraju susedne memorijske lokacije** → pristupi se spajaju u malo transakcija

**Kako prepoznaješ pogrešne varijante:**

| Izraz | Greška |
|---|---|
| `threadIdx.x + threadIdx.y` | ne koristi `blockIdx` → **svi blokovi na isti opseg** |
| `blockIdx.x + threadIdx.x` | zbir dve male vrednosti → **preklapanje** |
| `blockIdx.x * threadIdx.x` | proizvod → **masovno preklapanje** (sve gde je jedan 0) |
| `threadIdx.x * blockDim.x + blockIdx.x` | bijektivno, ali **razbija susednost** → nespojeni pristupi |

**Test koji uvek radi:** uvrsti nekoliko konkretnih vrednosti i vidi da li se dva različita
para `(blockIdx, threadIdx)` preslikavaju na **isti** indeks. Ako da → pogrešno.

## Broj blokova
```
broj_blokova = ceil(N / blockDim)
ukupno_niti  = broj_blokova × blockDim        (uvek ≥ N)
```
Pošto se skoro nikad ne poklapa tačno, **obavezna je provera `if (i < N)`** u kernelu — koja
istovremeno izaziva **divergenciju u poslednjem bloku**.

---

# 3. TIP: divergencija niti

**Definicija koja se traži:** niti **unutar ISTOG warp-a** krenu različitim granama if/else.
Pošto warp izvršava SIMD (svi istu instrukciju), grane se **serijalizuju** — prvo se izvrši
jedna dok ostale niti miruju, pa druga.

**Ključna reč je „unutar istog warp-a".** Ako svih 32 niti jednog warp-a idu **istim** putem,
nema divergencije — bez obzira što drugi warp ide drugim putem.

**Gde se javlja u tipičnom kodu:** kad `N` **nije deljivo** veličinom bloka, poslednji blok
ima niti sa `i ≥ N` koje preskaču `if (i < N)` dok ostale rade → divergencija.
Ako `N` **jeste** deljivo — nema je.

---

# 4. TIP: memorija — ko čemu može da priđe

| Memorija | Vidljivost | Brzina |
|---|---|---|
| Registri | **jedna nit** | najbrža |
| Lokalna | **jedna nit** (fizički deo globalne) | spora |
| **Deljiva (shared)** | **sve niti JEDNOG bloka** | brza |
| Globalna | **sve niti svih blokova** | najsporija |

**Pitanje koje se javlja: koliko instanci lokalne promenljive postoji?**
```
broj_blokova × niti_po_bloku
```
Jer su registri/lokalna memorija **privatni po niti** — svaka nit ima svoju kopiju, bez obzira
kom bloku pripada.

**Tačno/netačno pitanja:**
- niti **u istom bloku** komuniciraju preko deljive memorije → **TAČNO**
- niti **u različitim blokovima** komuniciraju preko deljive memorije → **NETAČNO** (v. 0.2)

**Zašto se isplati deljiva memorija:** učitaš podatak iz globalne **jednom** u shared, pa ga
niti bloka koriste više puta bez ponovnog odlaska u globalnu.

---

# 5. TIP: overhead na GPU

**Pitaj se: da li se to dešava NA GPU i da li troši vreme koje nije korisno izračunavanje?**

| Stavka | Overhead? | Zašto |
|---|---|---|
| Prenos host → GPU | **DA** | `cudaMemcpy` je **sinhron**, ide preko spore veze |
| Prenos GPU → host | **DA** | isto |
| Divergencija niti | **DA** | grane se serijalizuju, gubi se paralelizam |
| Keš promašaji na **host** procesoru | **NE** | to je overhead **CPU** dela, ne GPU izvršavanja |

**Vezano za to — sinhronost, koja se posebno pita:**
- **`cudaMemcpy` je SINHRON** — CPU čeka da se kopiranje završi
- **poziv kernela je ASINHRON** — CPU odmah nastavlja

Zato transferi **stvarno blokiraju** program, a pokretanje kernela ne.

---

# 6. Ostale činjenice koje se pitaju

## Kvalifikatori
| | Poziva se sa | Izvršava se na | Povratna vrednost |
|---|---|---|---|
| `__global__` | host | device | **mora biti void** |
| `__device__` | device | device | bilo koja |
| `__host__` | host | host | bilo koja |

`__global__` **ne može vraćati vrednost** — rezultat ide u memoriju preko prosleđenog
pokazivača. `__global__` sme da pozove `__device__` funkciju.

## Ograničenja i grid-stride petlja
Broj niti po bloku (tipično **1024**) i broj blokova po dimenziji su hardverski ograničeni.
Za veće nizove svaka nit obrađuje **više** elemenata:
```c
int tid = threadIdx.x + blockIdx.x * blockDim.x;
while (tid < N) {
    c[tid] = a[tid] + b[tid];
    tid += blockDim.x * gridDim.x;      // ukupan broj niti u gridu
}
```

## Sinhronizacija
`__syncthreads()` sinhronizuje **sve niti jednog bloka**. **Ne postoji** ekvivalent za ceo
grid unutar jednog poziva kernela — jedina sinhronizacija između blokova je **kraj kernela**.

---

# 7. Kontrolna lista

- Kod zauzetosti — jesi li proverio **oba** ograničenja (niti **i** blokovi)?
- Je li veličina bloka **umnožak 32**? Ako nije → polupopunjen warp
- Blokovi su **nedeljivi** — `⌊ ⌋`, ne decimalni broj blokova
- Kod indeksiranja — jesi li proverio **preklapanje** uvrštavanjem konkretnih vrednosti?
- Divergencija se odnosi na **isti warp**, ne na ceo blok
- Deljiva memorija = **jedan blok**; obrazloži i fizički i logički razlog
- `cudaMemcpy` sinhron, poziv kernela asinhron
- Keš promašaji na hostu **nisu** GPU overhead
