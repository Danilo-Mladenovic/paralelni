# CUDA — PISMENI deo: tipovi zadataka i obrasci rešavanja

Analizirano 17 CUDA zadataka sa rokova 2020–2025.

| Tip | Pojavljivanja |
|---|---|
| **1. Stencil (susedni elementi) + deljiva memorija** | **6×** |
| **2. Elementwise operacija + redukcija (po vrsti/koloni/globalno)** | **7×** |
| 3. Pretraga / geometrija (argmax, poređenje svih sa svima) | 3× |
| 4. Analiza datog kernela (blokovi, warpovi, divergencija) | 1× |

---

# 0. Šta se traži u SVAKOM zadatku

Ove rečenice se ponavljaju doslovno i svaka nosi poene:

| Formulacija u zadatku | Šta konkretno moraš uraditi |
|---|---|
| „Maksimalno redukovati broj pristupa globalnoj memoriji" | **deljiva memorija** (`__shared__`) — bez toga gubiš poene |
| „Omogućiti pozivanje kernela za matrice/nizove **proizvoljne veličine**" | **grid-stride petlja** |
| „Pretpostaviti veličinu bloka od 256 niti i broj blokova **ne veći od 256**" | max 65 536 niti → za veći niz **MORA** grid-stride |
| „Obratiti pažnju na efikasnost paralelizacije" | coalescing, izbegavanje divergencije, `__syncthreads()` na pravom mestu |
| „Napisati kod za testiranje validnosti rezultata" | sekvencijalna verzija + poređenje |
| „Veličinu matrice unosi korisnik" | `scanf` + dinamička alokacija |

## Kostur koji uvek pišeš

```c
#define BLOCK 256
#define MAX_BLOCKS 256

__global__ void kernel(float *in, float *out, int n) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;      // ukupan broj niti u gridu

    for (int i = tid; i < n; i += stride) {   // grid-stride petlja
        out[i] = ...;
    }
}

int main() {
    int n;  scanf("%d", &n);
    size_t size = n * sizeof(float);

    float *h_in = (float*)malloc(size), *h_out = (float*)malloc(size);
    /* inicijalizacija h_in */

    float *d_in, *d_out;
    cudaMalloc((void**)&d_in,  size);
    cudaMalloc((void**)&d_out, size);
    cudaMemcpy(d_in, h_in, size, cudaMemcpyHostToDevice);

    int blocks = (n + BLOCK - 1) / BLOCK;         // ceil
    if (blocks > MAX_BLOCKS) blocks = MAX_BLOCKS; // ogranicenje iz zadatka

    kernel<<<blocks, BLOCK>>>(d_in, d_out, n);

    cudaMemcpy(h_out, d_out, size, cudaMemcpyDeviceToHost);

    /* provera: sekvencijalno racunanje i poredjenje */

    cudaFree(d_in); cudaFree(d_out);
    free(h_in); free(h_out);
    return 0;
}
```

**Zašto je grid-stride obavezna:** ograničenje „ne više od 256 blokova × 256 niti" daje
najviše 65 536 niti. Ako je niz od milion elemenata, `if (i < n)` **nije dovoljno** — većina
elemenata nikad ne bi bila obrađena. Grid-stride petlja rešava i to i „proizvoljnu veličinu".

---

# TIP 1 — Stencil: svaki izlazni element zavisi od suseda (6×)

**Kako prepoznaš:** izlaz na poziciji `i` (ili `[i][j]`) računa se iz **više susednih** ulaznih
elemenata — `A[i], A[i+1], A[i+2]`, ili cela `3×3` okolina.

## Ključno zapažanje

Svaki ulazni element čita **više različitih niti** (kod 3×3 stencila — čak **9** niti). Zato
zadatak i traži smanjenje pristupa globalnoj memoriji: **učitaj blok podataka jednom u
`__shared__`, pa svaka nit čita iz nje.**

## Halo (obod)

Nit koja računa izlaz na ivici bloka treba susede **izvan** svog bloka. Zato se u deljivu
memoriju učitava blok **proširen za obod**:

```
blok od 256 niti, stencil širine 3  →  shared niz od 256 + 2 elementa
```

## Obrazac 1D (npr. `B[i] = (2A[i] + 3A[i+1] + 4A[i+2]) / 9`)

```c
#define BLOCK 256
#define RADIUS 2                     // koliko dodatnih elemenata treba udesno

__global__ void stencil1D(const float *A, float *B, int n) {
    __shared__ float s[BLOCK + RADIUS];

    int stride = blockDim.x * gridDim.x;

    for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < n - 2; i += stride) {
        int t = threadIdx.x;

        s[t] = A[i];                             // svaka nit ucitava "svoj" element
        if (t < RADIUS && i + BLOCK < n)         // prve 2 niti ucitavaju i halo
            s[t + BLOCK] = A[i + BLOCK];

        __syncthreads();                          // OBAVEZNO pre citanja tudjih elemenata

        B[i] = (2.0f*s[t] + 3.0f*s[t+1] + 4.0f*s[t+2]) / 9.0f;

        __syncthreads();                          // pre sledece iteracije grid-stride petlje
    }
}
```

**Dve najčešće greške:**
1. **Nedostaje `__syncthreads()`** posle učitavanja — nit čita `s[t+1]` koji susedna nit
   možda još nije upisala.
2. **Nedostaje druga `__syncthreads()`** na kraju grid-stride petlje — sledeća iteracija bi
   prepisala `s[]` dok neke niti još čitaju staru.

## Obrazac 2D (`3×3` okolina)

```c
#define TILE 16

__global__ void stencil2D(const float *A, float *B, int N) {
    __shared__ float s[TILE+2][TILE+2];           // TILE + halo sa obe strane

    int col = blockIdx.x * TILE + threadIdx.x;
    int row = blockIdx.y * TILE + threadIdx.y;
    int tx = threadIdx.x + 1, ty = threadIdx.y + 1;   // pomeraj zbog halo-a

    // centar
    s[ty][tx] = (row < N && col < N) ? A[row*N + col] : 0.0f;

    // halo: leva/desna kolona, gornja/donja vrsta
    if (threadIdx.x == 0)        s[ty][0]        = (col>0)     ? A[row*N + col-1]   : 0.0f;
    if (threadIdx.x == TILE-1)   s[ty][TILE+1]   = (col<N-1)   ? A[row*N + col+1]   : 0.0f;
    if (threadIdx.y == 0)        s[0][tx]        = (row>0)     ? A[(row-1)*N + col] : 0.0f;
    if (threadIdx.y == TILE-1)   s[TILE+1][tx]   = (row<N-1)   ? A[(row+1)*N + col] : 0.0f;
    /* + 4 ugla ako su potrebni */

    __syncthreads();

    if (row < N && col < N) {
        float sum = 0.0f;
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
                sum += s[ty+dy][tx+dx];
        B[row*N + col] = sum / 9.0f;
    }
}
```
Poziv: `dim3 blk(TILE,TILE); dim3 grd((N+TILE-1)/TILE, (N+TILE-1)/TILE);`

**„Za ivične piksele, susede koji ne postoje zameniti vrednošću 0"** — to je upravo `: 0.0f`
u ternarnim izrazima gore. Ako zadatak umesto toga kaže *„uzima se onoliko suseda koliko ih
ima"*, moraš **brojati** stvarne susede i deliti tim brojem, ne sa 9.

## Varijante koje su padale

| Zadatak | Razlika |
|---|---|
| `B` je dimenzije `(N−2)×(N−2)` | izlazni indeksi idu 0..N−3, ulazni pomereni za +1 |
| **k prolaza** (anti-aliasing) | pozovi kernel `k` puta u petlji na hostu i **zameni pokazivače** (`ping-pong` bafer) — ne možeš čitati i pisati isti niz |
| **dva prolaza** (μ pa σ²) | prvi kernel računa srednje vrednosti, drugi varijansu; ili oba u jednom kernelu jer je ista `3×3` okolina već u deljivoj memoriji |
| uslov na pikselu (`>128`, pa prosek suseda `>64`) | `B[i] = (uslov1 && uslov2) ? 1 : 0;` — pazi na **divergenciju**, ali je ovde neizbežna |

**Ping-pong za k prolaza:**
```c
for (int p = 0; p < k; p++) {
    stencil2D<<<grd, blk>>>(d_A, d_B, N);
    float *tmp = d_A; d_A = d_B; d_B = tmp;   // zameni ulaz i izlaz
}
// rezultat je u d_A
```

---

# TIP 2 — Elementwise + redukcija (7×)

**Kako prepoznaš:** dva koraka u tekstu — *„izračunati C = f(A,B)"* pa *„i kreirati vektor
čiji je i-ti element **srednja vrednost / min / max** i-te vrste/kolone"*.

**Uvek reši u dva kernela** (ili dva dela istog kernela sa `__syncthreads()` ako staje u
jedan blok). Ne pokušavaj sve u jednom prolazu.

## 2a — Redukcija PO KOLONI (jedna nit = jedna kolona)

```c
__global__ void meanPerColumn(const float *C, float *v, int N) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    for (int j = col; j < N; j += blockDim.x * gridDim.x) {
        float sum = 0.0f;
        for (int r = 0; r < N; r++)
            sum += C[r*N + j];          // nit j cita kolonu j
        v[j] = sum / N;
    }
}
```
**Zašto je ovo dobro:** susedne niti (`j`, `j+1`) čitaju `C[r*N+j]` i `C[r*N+j+1]` — **susedne
adrese** → pristupi su **spojeni (coalesced)**. Ovo je efikasan raspored i vredi ga navesti u
obrazloženju.

## 2b — Redukcija PO VRSTI (jedan blok = jedna vrsta)

Naivno rešenje „jedna nit = jedna vrsta" **radi ali je sporo**: nit `i` čita `C[i*N+c]`, a
susedna nit `C[(i+1)*N+c]` — razmak od `N` elemenata → **nespojeni** pristupi.

Bolje — **blok po vrsti, redukcija u deljivoj memoriji**:
```c
#define BLOCK 256

__global__ void minPerRow(const float *C, float *v, int N) {
    __shared__ float s[BLOCK];

    int row = blockIdx.x;                     // jedan BLOK obradjuje jednu vrstu
    if (row >= N) return;

    float local = INFINITY;
    for (int c = threadIdx.x; c < N; c += blockDim.x)   // niti dele kolone
        local = fminf(local, C[row*N + c]);

    s[threadIdx.x] = local;
    __syncthreads();

    // stablasta redukcija u deljivoj memoriji
    for (int st = blockDim.x / 2; st > 0; st >>= 1) {
        if (threadIdx.x < st)
            s[threadIdx.x] = fminf(s[threadIdx.x], s[threadIdx.x + st]);
        __syncthreads();
    }

    if (threadIdx.x == 0) v[row] = s[0];
}
```
Poziv: `minPerRow<<<N, BLOCK>>>(d_C, d_v, N);`

**Stablasta redukcija je obrazac koji moraš znati napamet** — pojavljuje se u pola zadataka.
Broj koraka je `log₂(BLOCK)`, `__syncthreads()` ide **unutar** petlje, posle svakog nivoa.

## 2c — Redukcija GLOBALNA (max/min celog niza)

Dvostepeno: svaki blok redukuje svoj deo, pa se `gridDim.x` rezultata završi u drugom prolazu.

```c
__global__ void maxReduce(const float *in, float *blockMax, int n) {
    __shared__ float s[BLOCK];

    float local = -INFINITY;
    for (int i = blockIdx.x*blockDim.x + threadIdx.x; i < n; i += blockDim.x*gridDim.x)
        local = fmaxf(local, in[i]);

    s[threadIdx.x] = local;
    __syncthreads();

    for (int st = blockDim.x/2; st > 0; st >>= 1) {
        if (threadIdx.x < st) s[threadIdx.x] = fmaxf(s[threadIdx.x], s[threadIdx.x+st]);
        __syncthreads();
    }

    if (threadIdx.x == 0) blockMax[blockIdx.x] = s[0];    // jedan rezultat po bloku
}
```
Zatim: ili pozovi isti kernel još jednom nad `blockMax` (dužine `blocks`), ili prekopiraj tih
≤256 vrednosti na host i završi tamo. **Oba su prihvatljiva — obrazloži izbor.**

**Zašto ne `atomicMax`:** za `float` ne postoji direktno (samo za cele brojeve). Ako baš treba
atomik nad float-om, koristi se `atomicCAS` trik — na ispitu je jednostavnije i sigurnije
dvostepeno rešenje.

## 2d — „Svaki 256. element = srednja vrednost prethodnih 255"

Poklapa se sa veličinom bloka → **svaki blok radi jednu grupu**:
```c
__global__ void vecAddAndMean(const float *A, const float *B, float *C, int n) {
    __shared__ float s[256];
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    float val = (i < n) ? A[i] + B[i] : 0.0f;
    C[i] = val;

    s[threadIdx.x] = (threadIdx.x < 255) ? val : 0.0f;   // poslednji se ne racuna u sumu
    __syncthreads();

    for (int st = 128; st > 0; st >>= 1) {
        if (threadIdx.x < st) s[threadIdx.x] += s[threadIdx.x + st];
        __syncthreads();
    }

    if (threadIdx.x == 255 && i < n) C[i] = s[0] / 255.0f;   // pregazi 256. element
}
```

## Sažetak: koji raspored za koju redukciju

| Traži se | Raspored | Zašto |
|---|---|---|
| po **koloni** | 1 nit = 1 kolona | pristupi su spojeni |
| po **vrsti** | 1 **blok** = 1 vrsta + stablasta redukcija | 1 nit = 1 vrsta bi bila nespojena |
| **globalno** (ceo niz/matrica) | dvostepeno: blok → `blockMax[]` → drugi prolaz | nema globalne sinhronizacije unutar kernela |
| po **dijagonali** | 1 nit = 1 dijagonalni element `A[i*N+i]`, pa globalna redukcija | dijagonala je samo N elemenata |

---

# TIP 3 — Pretraga i geometrija (3×)

**Kako prepoznaš:** *„naći 10 najbližih tačaka"*, *„kružnicu sa najvećom površinom"*,
*„da li se pravougaonik preklapa sa bilo kojim drugim"*.

## 3a — argmax (traži se i **koji** element, ne samo vrednost)

Razlika u odnosu na običnu redukciju: moraš pratiti **indeks** zajedno sa vrednošću.

```c
__global__ void maxArea(const float *r, float *bestVal, int *bestIdx, int n) {
    __shared__ float sv[BLOCK];
    __shared__ int   si[BLOCK];

    float best = -1.0f;  int bi = -1;
    for (int i = blockIdx.x*blockDim.x + threadIdx.x; i < n; i += blockDim.x*gridDim.x) {
        float area = 3.14159265f * r[i] * r[i];
        if (area > best) { best = area; bi = i; }     // pamti I indeks
    }
    sv[threadIdx.x] = best;  si[threadIdx.x] = bi;
    __syncthreads();

    for (int st = blockDim.x/2; st > 0; st >>= 1) {
        if (threadIdx.x < st && sv[threadIdx.x + st] > sv[threadIdx.x]) {
            sv[threadIdx.x] = sv[threadIdx.x + st];
            si[threadIdx.x] = si[threadIdx.x + st];   // indeks putuje ZAJEDNO sa vrednoscu
        }
        __syncthreads();
    }
    if (threadIdx.x == 0) { bestVal[blockIdx.x] = sv[0]; bestIdx[blockIdx.x] = si[0]; }
}
```
**„Šta izmeniti da se nađu DVE sa najvećom površinom":** čuvaj **par** (najveći, drugi
najveći) u svakoj niti i u redukciji spajaj parove — pri poređenju dva para uzmi dve najveće
od četiri vrednosti. Isto radi i za „10 najbližih" (v. dole).

## 3b — k najbližih (k = 10)

```c
__global__ void distances(const float *x, const float *y, float *d, int n, int p) {
    for (int i = blockIdx.x*blockDim.x + threadIdx.x; i < n; i += blockDim.x*gridDim.x) {
        float dx = x[i] - x[p], dy = y[i] - y[p];
        d[i] = (i == p) ? INFINITY : dx*dx + dy*dy;   // izbaci samu tacku; bez sqrt!
    }
}
```
**Dva praktična poteza koja nose poene:**
- **ne računaj `sqrt`** — poređenje kvadrata rastojanja daje isti poredak, a štediš operaciju
- zadatu tačku isključi postavljanjem na `INFINITY`, ne granom koja pravi divergenciju

Za samih 10 najbližih: ili prekopiraj `d[]` na host pa uradi delimično sortiranje (potpuno
prihvatljivo — 10 elemenata je trivijalan posao), ili u kernelu održavaj listu od 10 u
deljivoj memoriji. **Na ispitu je jasnije prvo rešenje uz obrazloženje** da paralelizacija
ima smisla za `O(n)` deo (rastojanja), a ne za izbor 10 elemenata.

## 3c — svaki sa svakim (preklapanje pravougaonika)

```c
__global__ void overlap(const float *A, int *B, int n) {
    // format: A = [a1..an, b1..bn, c1..cn, d1..dn]
    for (int i = blockIdx.x*blockDim.x + threadIdx.x; i < n; i += blockDim.x*gridDim.x) {
        float ax1 = A[i], ay1 = A[n+i], ax2 = A[2*n+i], ay2 = A[3*n+i];
        int found = 0;

        for (int j = 0; j < n && !found; j++) {
            if (j == i) continue;
            float bx1 = A[j], by1 = A[n+j], bx2 = A[2*n+j], by2 = A[3*n+j];
            if (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1) found = 1;
        }
        B[i] = found;
    }
}
```
**Nit `i` poredi svoj pravougaonik sa svim ostalima** — `O(n)` po niti, `O(n²)` ukupno ali
paralelno. Format `[a1..an, b1..bn, …]` je **struct-of-arrays** i to je namerno: susedne niti
čitaju `A[i]` i `A[i+1]` → **spojeni pristupi**. Vredi to napomenuti.

---

# TIP 4 — Analiza datog kernela (1×)

Dat je gotov kernel i host funkcija, pa se postavljaju pitanja. Poziv je oblika
`vecAddKernel<<<ceil(n/256), 256>>>(...)`.

| Pitanje | Kako se računa |
|---|---|
| Koliko blokova za n=1000? | `ceil(1000/256) = ⌈3.9⌉ = **4**` |
| Koliko warp-ova u svakom bloku? | `256/32 = **8**` |
| Koliko niti u gridu? | `4 × 256 = **1024**` |
| Ima li divergencije za n=1000? | **DA**, u liniji `if (i < n)`. Pokrenuto je 1024 niti, a validno je 1000 → niti 1000–1023 idu drugom granom. One su u **poslednjem warp-u** (niti 992–1023), gde se 8 niti ponaša drukčije od 24 → divergencija **unutar jednog warp-a**. |
| Ima li divergencije za n=768? | **NE**. `ceil(768/256) = 3` blokova × 256 = **768 niti tačno** → uslov `i < n` je tačan za **sve** niti, nijedan warp se ne deli. |
| Šta kernel radi? | sabira dva vektora element po element |

**Ključ za divergenciju:** ne pitaj se „ima li niti koje ne rade", nego **„da li se unutar
jednog warp-a niti razilaze"**. Ako `n` deli veličinu bloka bez ostatka → nema divergencije.

---

# Testiranje (traži se skoro uvek)

```c
void sekvencijalno(float *A, float *B, int n) {
    for (int i = 0; i < n-2; i++)
        B[i] = (2*A[i] + 3*A[i+1] + 4*A[i+2]) / 9.0f;
}

/* ... posle cudaMemcpy nazad ... */
float *ref = (float*)malloc(size);
sekvencijalno(h_A, ref, n);

int ok = 1;
for (int i = 0; i < n-2; i++)
    if (fabs(ref[i] - h_B[i]) > 1e-5) { printf("GRESKA na %d\n", i); ok = 0; break; }
printf(ok ? "OK\n" : "NEISPRAVNO\n");
```
Za `float` **uvek poredi sa tolerancijom** (`1e-5`), nikad sa `==` — redosled sabiranja se
razlikuje pa se javljaju zaokruživanja.

---

# Kontrolna lista

- Piše li „redukovati pristupe globalnoj memoriji"? → **`__shared__` obavezno**
- Piše li „proizvoljna veličina" ili „blokova ne više od 256"? → **grid-stride petlja**
- Jesi li stavio `__syncthreads()` **posle** punjenja deljive memorije?
- Jesi li stavio `__syncthreads()` **na kraju** grid-stride petlje (pre prepisivanja shared)?
- Kod stablaste redukcije — `__syncthreads()` je **unutar** petlje, posle svakog nivoa
- Redukcija po **koloni** → nit po koloni; po **vrsti** → **blok** po vrsti
- Globalni max/min → **dvostepeno**, nema globalne sinhronizacije u kernelu
- Kod argmax — **indeks putuje zajedno sa vrednošću** kroz celu redukciju
- Ivice: „nepostojeći susedi = 0" ili „deli brojem stvarnih suseda"? Pročitaj tačno
- Kod k prolaza — **zameni pokazivače** (ping-pong), ne piši u isti niz iz kog čitaš
- Poređenje `float` rezultata **sa tolerancijom**, ne `==`
- Rastojanja — **bez `sqrt`**, poredi kvadrate
