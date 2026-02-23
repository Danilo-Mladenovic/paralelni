# CUDA Ispitni Zadaci - Brza Priprema i Ključni Obrasci

## 📋 Tipovi Zadataka i Pristup

### 1. **REDUCTION operacije** (NAJČEŠĆI tip)
**Prepoznavanje**: Tražiš sumu, min, max, proizvod, broj elemenata koji zadovoljavaju uslov...

**Standardni pristup:**
```cuda
__global__ void reductionKernel(float* input, float* output, int n) {
    __shared__ float sdata[BLOCK_SIZE];
    
    // Faza 1: Grid-stride loop za učitavanje i inicijalnu redukciju
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float sum = 0;
    
    while (i < n) {
        sum += input[i];  // ili druga operacija
        i += gridDim.x * blockDim.x;
    }
    sdata[tid] = sum;
    __syncthreads();
    
    // Faza 2: Redukcija unutar bloka (BITNO: stride /= 2)
    for (int stride = blockDim.x / 2; stride > 0; stride /= 2) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }
    
    // Faza 3: Thread 0 upisuje rezultat
    if (tid == 0) {
        atomicAdd(output, sdata[0]);  // ili output[blockIdx.x] = sdata[0];
    }
}
```

**Ključne greške:**
- Zaboravljaš `__syncthreads()` posle svakog koraka
- Koristiš `stride *= 2` umesto `stride /= 2`
- Ne koristiš grid-stride loop kada `n > gridDim.x * blockDim.x`

---

### 2. **STENCIL operacije** (susedni elementi)
**Prepoznavanje**: Računaš nešto na osnovu susednih elemenata (konvolucija, blurring, lokalni prosjeci...)

**Pristup sa shared memory:**
```cuda
__global__ void stencilKernel(float* input, float* output, int n, int radius) {
    __shared__ float temp[BLOCK_SIZE + 2*RADIUS];  // +halo zone
    
    int gid = blockIdx.x * blockDim.x + threadIdx.x;  // globalni indeks
    int lid = threadIdx.x + RADIUS;                   // lokalni indeks (sa offsetom)
    
    // Učitaj glavni deo
    if (gid < n) {
        temp[lid] = input[gid];
    }
    
    // Učitaj levi halo (samo prvi threadovi)
    if (threadIdx.x < RADIUS && gid >= RADIUS) {
        temp[lid - RADIUS] = input[gid - RADIUS];
    }
    
    // Učitaj desni halo (samo poslednji threadovi)
    if (threadIdx.x < RADIUS && gid + blockDim.x < n) {
        temp[lid + blockDim.x] = input[gid + blockDim.x];
    }
    
    __syncthreads();
    
    // Računaj rezultat koristeći shared memory
    if (gid < n) {
        float result = 0;
        for (int i = -radius; i <= radius; i++) {
            result += temp[lid + i];
        }
        output[gid] = result / (2*radius + 1);
    }
}
```

**Ključne stvari:**
- Uvek `__shared__` mora biti `blockDim + 2*radius`
- Halo zone učitavaju SAMO rubni threadovi
- Lokalni indeks = `threadIdx.x + radius`

---

### 3. **TRANSFORM operacije** (elementwise)
**Prepoznavanje**: Jednostavna operacija na svakom elementu nezavisno

**Brz template:**
```cuda
__global__ void transformKernel(float* input, float* output, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Grid-stride loop (za bilo koje n)
    while (i < n) {
        output[i] = someFunction(input[i]);
        i += gridDim.x * blockDim.x;
    }
}
```

**Kada treba shared memory:**
- Kada čitaš isti element više puta
- Kada trebaš komunikaciju između threadova u bloku

---

### 4. **HISTOGRAM / COUNTING**
**Prepoznavanje**: Brojanje učestalosti, grupisanje...

**Dva pristupa:**

**A) Globalna atomics (sporije, jednostavnije):**
```cuda
__global__ void histogramKernel(int* input, int* histogram, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    
    while (i < n) {
        atomicAdd(&histogram[input[i]], 1);
        i += gridDim.x * blockDim.x;
    }
}
```

**B) Privatne kopije po bloku (brže):**
```cuda
__global__ void histogramKernel(int* input, int* histogram, int n, int bins) {
    __shared__ int localHist[NUM_BINS];
    
    // Inicijalizuj lokalnu kopiju
    if (threadIdx.x < bins) {
        localHist[threadIdx.x] = 0;
    }
    __syncthreads();
    
    // Popuni lokalnu kopiju
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    while (i < n) {
        atomicAdd(&localHist[input[i]], 1);
        i += gridDim.x * blockDim.x;
    }
    __syncthreads();
    
    // Spoji u globalnu
    if (threadIdx.x < bins) {
        atomicAdd(&histogram[threadIdx.x], localHist[threadIdx.x]);
    }
}
```

---

## 🎯 Opšta Strategija Rešavanja

### Korak 1: Prepoznaj tip zadatka
- **Tražiš jedno broj?** → Reduction
- **Susedni elementi?** → Stencil
- **Nezavisne operacije?** → Transform
- **Brojanje/grupisanje?** → Histogram

### Korak 2: Odluči o shared memory
**Koristi shared memory ako:**
- Više threadova čita iste podatke
- Potrebna redukcija unutar bloka
- Stencil sa radius > 1

**Ne koristi shared memory ako:**
- Svaki thread čita svoj element samo jednom
- Jednostavno mapiranje input → output

### Korak 3: Grid/Block konfiguracija
```cpp
// Standardna formula
int blockSize = 256;  // ili 128, 512
int numBlocks = (n + blockSize - 1) / blockSize;

// Ograniči broj blokova ako je n jako veliki
numBlocks = min(numBlocks, 1024);

kernel<<<numBlocks, blockSize>>>(...);
```

---

## ⚠️ TOP Greške na Ispitu

1. **Zaboravljanje `__syncthreads()`** posle shared memory pristupa
2. **Loše indeksiranje:**
   - Globalni: `blockIdx.x * blockDim.x + threadIdx.x`
   - Lokalni (shared): `threadIdx.x` (+ offset za halo)
3. **Zaboravljanje grid-stride loop** kada `n > broj_threadova`
4. **Ne proveriš `if (i < n)`** pre pristupa memoriji
5. **Reduction stride:** ide UNAZAD (`stride /= 2`), ne napred
6. **Atomics:** koristi `atomicAdd` za paralelno pisanje na istu lokaciju

---

## 📝 Checklist Pre Pokretanja

```cpp
// 1. Alokacija na device
float *d_input, *d_output;
cudaMalloc(&d_input, n * sizeof(float));

// 2. Kopiranje na device
cudaMemcpy(d_input, h_input, n * sizeof(float), cudaMemcpyHostToDevice);

// 3. Kernel launch
kernel<<<numBlocks, blockSize>>>(...);

// 4. Provera grešaka
cudaError_t err = cudaGetLastError();
if (err != cudaSuccess) printf("Error: %s\n", cudaGetErrorString(err));

// 5. Kopiranje nazad
cudaMemcpy(h_output, d_output, size, cudaMemcpyDeviceToHost);

// 6. Oslobađanje
cudaFree(d_input);
```

---

## 💡 Brzi Saveti

- **Počni od CPU verzije:** razumi algoritam prvo
- **Nacrtaj šta koji thread radi:** vizualizuj mapiranje
- **Shared memory veličina:** uvek zaokruži na `blockDim.x` (+ halo)
- **Dva prolaza?** Prvi računa, drugi agrega (uobičajeno za složenije operacije)
- **Proveri edge case-ove:** šta ako je `n < blockSize`?

---

Kreni da rešavaš zadatke, a ako se zapneš na konkretnom zadatku, pitaj me! Fokusiraj se na prepoznavanje obrasca i mapiranje na jedan od gornjih template-a.
