# MPI (Message Passing Interface) Cheatsheet

## Osnovni Setup

```cpp
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int rank, size;

    // Inicijalizacija MPI-a
    MPI_Init(&argc, &argv);

    // Dobivanje ranga (ID) procesa
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Dobivanje broja procesora
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Tvoj kod ovdje...

    // Završetak MPI-a
    MPI_Finalize();
    return 0;
}
```

---

## MPI File I/O

### **MPI_File operacije**

```cpp
MPI_File fp;

// Otvaranje fajla
MPI_File_open(MPI_COMM_WORLD, "file.dat",
              MPI_MODE_CREATE | MPI_MODE_WRONLY,
              MPI_INFO_NULL, &fp);

// Pisanje na određenom offsetu
MPI_File_write_at_all(fp, offset, data, count, MPI_INT, MPI_STATUS_IGNORE);

// Čitanje sa određenim offsetom
MPI_File_read_at(fp, offset, data, count, MPI_INT, MPI_STATUS_IGNORE);

// Zatvorivanve fajla
MPI_File_close(&fp);
```

**Primjer - Pisanje i čitanje:**

```cpp
int my_size = 100 / size;
int *data = (int *)malloc(my_size * sizeof(int));

// Inicijalizacija
for (int i = 0; i < my_size; i++) {
    data[i] = rank * my_size + i;
}

// Pisanje
MPI_File fp;
MPI_File_open(MPI_COMM_WORLD, "podaci.dat",
              MPI_MODE_CREATE | MPI_MODE_WRONLY,
              MPI_INFO_NULL, &fp);
MPI_File_write_at_all(fp, rank * my_size * sizeof(int), data, my_size, MPI_INT, MPI_STATUS_IGNORE);
MPI_File_close(&fp);

// Čitanje
MPI_File_open(MPI_COMM_WORLD, "podaci.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &fp);
MPI_File_read_shared(fp, data, my_size, MPI_INT, MPI_STATUS_IGNORE);
MPI_File_close(&fp);
```

### **MPI_File_set_view** - Pogled na datoteku sa custom tipom

```cpp
MPI_File_set_view(fp, disp, etype, filetype, datarep, info);

// disp - inicijalni displacement
// etype - osnovni tip elementa
// filetype - tip koji se čita/piše
// datarep - "native" za prirodnu reprezentaciju
// info - MPI_INFO_NULL
```

---

## Česte šeme za rešavanje zadataka

### **1. Matričnog množenja (A × B = C)**

```cpp
int M = 16, N = 12, K = 8;  // A[M][N], B[N][K], C[M][K]

// Proces 0 genériše podatke
if (rank == 0) {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = i + j;
    // i B...
}

// Svaki proces dobija M/p redова
MPI_Scatter(A, 1, row_type, A_local, (M/size)*N, MPI_INT, 0, MPI_COMM_WORLD);

// Svi dobijaju B
MPI_Bcast(B, N*K, MPI_INT, 0, MPI_COMM_WORLD);

// Lokalno računanje
for (int i = 0; i < M/size; i++)
    for (int j = 0; j < K; j++) {
        C_local[i][j] = 0;
        for (int k = 0; k < N; k++)
            C_local[i][j] += A_local[i][k] * B[k][j];
    }

// Sabiranje rezultata
MPI_Gather(C_local, (M/size)*K, MPI_INT, C, 1, col_type, 0, MPI_COMM_WORLD);
```

---

### **2. 2D Paralelna Matrična Množenja (Cannon algoritam)**

Za `p = q*q` procesora u 2D grid-u:

```cpp
int q = sqrt(p);
MPI_Comm cart_comm;
int dims[2] = {q, q}, periods[2] = {1, 1};
MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);

int coords[2];
MPI_Cart_coords(cart_comm, rank, 2, coords);
int my_row = coords[0], my_col = coords[1];

int s = n / q;  // veličina lokalne matrice

// Inicijalna rotacija
// A: rotacija levo duž redа
// B: rotacija gore duž kolone

// Glavni loop - q koraka
for (int step = 0; step < q; step++) {
    // Lokalno množenje
    for (int i = 0; i < s; i++) {
        for (int j = 0; j < s; j++) {
            C_local[i][j] = 0;
            for (int k = 0; k < s; k++) {
                C_local[i][j] += A_local[i][k] * B_local[k][j];
            }
        }
    }

    // Rotacija A ulevo duž reda
    int left, right;
    MPI_Cart_shift(cart_comm, 1, 1, &right, &left);
    MPI_Sendrecv_replace(A_local, s*s, MPI_INT, left, 0, right, 0, cart_comm, MPI_STATUS_IGNORE);

    // Rotacija B gore duž kolone
    int up, down;
    MPI_Cart_shift(cart_comm, 0, 1, &down, &up);
    MPI_Sendrecv_replace(B_local, s*s, MPI_INT, up, 0, down, 0, cart_comm, MPI_STATUS_IGNORE);
}
```

---

### **3. Pronalaženja min/max sa lokacijom**

```cpp
struct {
    int value;
    int rank;
} local_min, global_min;

local_min.value = my_value;
local_min.rank = rank;

MPI_Reduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);

if (rank == 0) {
    printf("Min: %d, from rank %d\n", global_min.value, global_min.rank);
}
```

---

### **4. Paralelna redukcija (tree pattern)**

Umesto `MPI_Reduce`, kod može da koristi tree pattern:

```cpp
int local_sum = my_value;

for (int step = 1; step < size; step *= 2) {
    if ((rank / step) % 2 == 0) {
        int source = rank + step;
        if (source < size) {
            MPI_Recv(&value, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_sum += value;
        }
    } else {
        int dest = rank - step;
        MPI_Send(&local_sum, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        return;
    }
}
```

---

### **5. Paralelno sortiranje - Odd-Even Sort**

```cpp
int n = 100;  // elementi za sortiranje
int my_n = n / size;
int *my_data = (int *)malloc(my_n * sizeof(int));

// Inicijalizacija
for (int i = 0; i < my_n; i++)
    my_data[i] = rand();

// Sortiranje lokalno
qsort(my_data, my_n, sizeof(int), compare);

// Odd-Even transposiciona sortiranja
for (int phase = 0; phase < size; phase++) {
    int partner = (phase % 2 == 0)
                  ? (rank % 2 == 0 ? rank + 1 : rank - 1)
                  : (rank % 2 == 0 ? rank - 1 : rank + 1);

    if (partner >= 0 && partner < size) {
        int *other = (int *)malloc(my_n * sizeof(int));
        MPI_Sendrecv(my_data, my_n, MPI_INT, partner, 0,
                     other, my_n, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Merge - čuva manje tačne, drugu tačnu
        if ((rank > partner && phase % 2 == 0) ||
            (rank < partner && phase % 2 == 1)) {
            // Čuvam manje
            int *merged = (int *)malloc(2 * my_n * sizeof(int));
            merge(my_data, other, merged, my_n);
            memcpy(my_data, merged, my_n * sizeof(int));
            free(merged);
        } else {
            // Čuvam veće
            // slično
        }
        free(other);
    }
}
```

---

### **6. File I/O sa MPI Datatype-om**

```cpp
// Često se koristi za kompleksne formate fajlova

MPI_Datatype filetype, etype = MPI_INT;
int blocklengths[2] = {10, 20};
int displacements[2] = {0, 100};

MPI_Type_indexed(2, blocklengths, displacements, MPI_INT, &filetype);
MPI_Type_commit(&filetype);

MPI_File fp;
MPI_File_open(MPI_COMM_WORLD, "complex.dat",
              MPI_MODE_CREATE | MPI_MODE_WRONLY,
              MPI_INFO_NULL, &fp);

MPI_File_set_view(fp, rank * 120 * sizeof(int), etype, filetype, "native", MPI_INFO_NULL);
MPI_File_write_all(fp, data, 100, MPI_INT, MPI_STATUS_IGNORE);
MPI_File_close(&fp);

MPI_Type_free(&filetype);
```

---

##常用podaci tipovi

| Tip                  | MPI Konstanta            |
| -------------------- | ------------------------ |
| `int`                | `MPI_INT`                |
| `float`              | `MPI_FLOAT`              |
| `double`             | `MPI_DOUBLE`             |
| `long long`          | `MPI_LONG_LONG`          |
| `unsigned long long` | `MPI_UNSIGNED_LONG_LONG` |
| `char`               | `MPI_CHAR`               |

---

## Kompajliranje i pokretanje

```bash
# Kompajliranje
mpicc -o program program.c
mpicxx -o program program.cpp

# Pokretanje sa 4 procesa
mpirun -np 4 ./program
```

---

## Česti Errors i Rešenja

| Problem                                          | Rešenje                                                                          |
| ------------------------------------------------ | -------------------------------------------------------------------------------- |
| `Cannot assign to return value of MPI_Comm_rank` | `MPI_Comm_rank` vraća `int`, ne smete koristiti `&`                              |
| Greške sa memory-om                              | Pazite na veličine buffer-а u `Scatter/Gather`                                   |
| Deadlock                                         | Pazite na redosled `Send/Recv`, koristite `MPI_Sendrecv`                         |
| `MPI_STATUS_IGNORE` vraća error                  | Koristite `MPI_STATUS_IGNORE` samo za ne-blocking operacije                      |
| Greške sa MPI_Datatype-om                        | Morate pozvati `MPI_Type_commit()` nakon kreiranja                               |
| `MPI_Scatter` ne podeljuje podatke               | Proverite da li ste kreirali custom tip pravilno sa `MPI_Type_create_resized`    |
| Program se "zapeta"                              | Koristite `MPI_Barrier()` za sinhronizaciju, ili proverite redosled komunikacije |

---

## Best Practices 🎯

### **1. Validacija ulaza**

```cpp
if (rank == 0) {
    // Validacija da je "n" deljivo sa "p"
    if (n % size != 0) {
        printf("Greska: n mora biti deljivo sa brojem procesora\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}
MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Sigurni su svi znaju
```

### **2. Memory allocation sa greške**

```cpp
int *data = (int *)malloc(size_in_bytes);
if (data == NULL) {
    fprintf(stderr, "Allocation failed!\n");
    MPI_Finalize();
    exit(1);
}
// ... korišćenje ...
free(data);
```

### **3. MPI_Sendrecv umesto Send/Recv parade**

```cpp
// Izbegava deadlock u slučaju kao:
// Proces 0: Send -> Recv
// Proces 1: Send -> Recv (deadlock jer oba čekaju Send!)

// Koriste MPI_Sendrecv umesto toga:
MPI_Sendrecv(send_buf, send_count, MPI_INT, dest, tag,
             recv_buf, recv_count, MPI_INT, source, tag,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
```

### **4. Proverite dimenzije pri Scatter/Gather**

```cpp
int local_size = n / size;  // Proverite da je n % size == 0!

// Scatter slanja "local_size" elemenata SVAKOM procesu
MPI_Scatter(global_data, local_size, MPI_INT,
            local_data, local_size, MPI_INT,
            0, MPI_COMM_WORLD);
// Svaki proces očekuje local_size podataka!
```

### **5. Korišćenja MPI_Barrier za debugging**

```cpp
printf("Proces %d stupio do barijere\n", rank);
MPI_Barrier(MPI_COMM_WORLD);  // Svi čekaji
printf("Proces %d prešao barijeru\n", rank);
```

---

## Kompleksne Šeme - Napredne Tehnike

### **Striding - Čitanje podataka sa određenim razmakom**

Koristi se često pri učitavanju iz fajlova:

```cpp
MPI_Datatype stride_type;

// Čita svaki q-ti element
MPI_Type_vector(count, 1, stride, MPI_INT, &stride_type);
MPI_Type_commit(&stride_type);

MPI_File fp;
MPI_File_open(MPI_COMM_WORLD, "data.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &fp);
MPI_File_set_view(fp, rank * sizeof(int), MPI_INT, stride_type, "native", MPI_INFO_NULL);
MPI_File_read_all(fp, data, count, MPI_INT, MPI_STATUS_IGNORE);
MPI_File_close(&fp);
```

### **Ring komunikacija - Prosledi dalje pattern**

Svaki proces šalje svom susedu:

```cpp
int next = (rank + 1) % size;
int prev = (rank - 1 + size) % size;

for (int round = 0; round < size; round++) {
    // Prosledi dalje moj podatak
    MPI_Sendrecv_replace(&my_data, 1, MPI_INT, next, 0,
                         prev, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Proces svoga dela...
}
```

### **Svi prema svim (All-to-All) komunikacija**

```cpp
// Svaki proces ima svoj niz od "n" elemenata
// Razmenjuje sa svim ostalima

int *sendbuf = (int *)malloc(n * size * sizeof(int));
int *recvbuf = (int *)malloc(n * size * sizeof(int));

// Popunavanje sendbuf
for (int i = 0; i < n * size; i++) {
    sendbuf[i] = rank * n + (i % n);
}

// All-to-All - svaki proces i šalje procesu j se[i*n : (i+1)*n]
MPI_Alltoall(sendbuf, n, MPI_INT,
             recvbuf, n, MPI_INT,
             MPI_COMM_WORLD);

// recvbuf[i*n : (i+1)*n] sadrži podatke od procesa i
```

### **Prefix Sum (Scan) - Paralelna linearna rezcija**

```cpp
// Svaki proces i dobija sum[0..i]
int my_value = rank * 10;
int result = 0;

MPI_Scan(&my_value, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

// result na procesu 0 = 0*10
// result na procesu 1 = 0*10 + 1*10
// result na procesu 2 = 0*10 + 1*10 + 2*10
// itd.
```

---

## Tehnike za Optimizaciju Performansi

### **1. Non-blocking komunikacija**

Za slučajeve gde trebate istovremenu prikupljanje i komunikaciju:

```cpp
MPI_Request request;

// Non-blocking send
MPI_Isend(sendbuf, count, MPI_INT, dest, tag, MPI_COMM_WORLD, &request);

// Radite nešto drugo
do_computation();

// Čekajte na završetak
MPI_Wait(&request, MPI_STATUS_IGNORE);
```

### **2. Korišćenja MPI_Request niza za više operacija**

```cpp
MPI_Request requests[10];
MPI_Status statuses[10];

for (int i = 0; i < 10; i++) {
    MPI_Isend(buffers[i], size, MPI_INT, dests[i], i,
              MPI_COMM_WORLD, &requests[i]);
}

// Čekajte sve istovremeno
MPI_Waitall(10, requests, statuses);
```

### **3. Overlap comm sa computation**

```cpp
// Send prve polovine, računaj drugu, čekaj, završi ostatak
MPI_Isend(data_first_half, n/2, MPI_INT, dest, 0, MPI_COMM_WORLD, &request);
compute_on_second_half(data_second_half);
MPI_Wait(&request, MPI_STATUS_IGNORE);
MPI_Isend(data_second_half, n/2, MPI_INT, dest, 1, MPI_COMM_WORLD, &request);
MPI_Wait(&request, MPI_STATUS_IGNORE);
```

---

## Debugging i Testing

### **Ispis debugging informacija**

```cpp
if (rank == 0) {
    printf("=== DEBUG INFO ===\n");
    printf("Total processes: %d\n", size);
    printf("Data size: %d\n", data_size);
}

MPI_Barrier(MPI_COMM_WORLD);

printf("[Rank %d] Local data size: %d\n", rank, local_size);
MPI_Barrier(MPI_COMM_WORLD);
```

### **Valgrind za MPI**

```bash
# Pronalaženja memory leak-а u MPI kodu
mpirun -np 4 valgrind --leak-check=full ./program
```

### **Korišćenja MPI_Wtime za merenje vremena**

```cpp
double start_time = MPI_Wtime();

// Kod koji merimo...

double end_time = MPI_Wtime();
if (rank == 0) {
    printf("Time: %f seconds\n", end_time - start_time);
}
```

---

## Quick Reference - Najčešće Korišćene Funkcije

| Funkcija          | Zašta?                                    |
| ----------------- | ----------------------------------------- |
| `MPI_Scatter`     | Podeljenje podataka                       |
| `MPI_Gather`      | Prikupljanje rezultata                    |
| `MPI_Bcast`       | Slanje istih podataka svima               |
| `MPI_Reduce`      | Kombinovanje rezultata (sum, min, max...) |
| `MPI_Allreduce`   | Reduce ali rezultat dostupan svima        |
| `MPI_Type_vector` | Custom tipovi za ne-uzastopne podatke     |
| `MPI_Comm_split`  | Pravljenje podgrupa procesora             |
| `MPI_Cart_create` | 2D/3D topologija procesora                |
| `MPI_File_*`      | Paralelno čitanje/pisanje fajlova         |

---

## Template - Starter kod

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD
#define n 16  // Veličina problema

int main(int argc, char **argv) {
    int rank, size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    start_time = MPI_Wtime();

    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int local_size = n / size;

    //Initijalizacija globalnih podataka
    int *global_data = NULL;
    if (rank == 0) {
        global_data = (int *)malloc(n * sizeof(int));
        for (int i = 0; i < n; i++)
            global_data[i] = i;
    }

    int *local_data = (int *)malloc(local_size * sizeof(int));
    int *result = (int *)malloc(local_size * sizeof(int));

    // Distribucija
    MPI_Scatter(global_data, local_size, MPI_INT,
                local_data, local_size, MPI_INT, 0, MCW);

    // Lokalno računanje
    for (int i = 0; i < local_size; i++)
        result[i] = local_data[i] * 2;

    // Prikupljanje
    MPI_Gather(result, local_size, MPI_INT,
               global_data, local_size, MPI_INT, 0, MCW);

    if (rank == 0) {
        for (int i = 0; i < n; i++)
            printf("%d ", global_data[i]);
        free(global_data);
    }

    free(local_data);
    free(result);

    end_time = MPI_Wtime();
    if (rank == 0)
        printf("Vreme: %f\n", end_time - start_time);

    MPI_Finalize();
    return 0;
}
```

---

## Česti Scenariji

### Scenario 1: Min sa lokacijom

```cpp
struct { int value; int rank; } local_min, global_min;
local_min.value = my_min; local_min.rank = rank;
MPI_Reduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, 0, MCW);
MPI_Bcast(&global_min, 1, MPI_2INT, 0, MCW);
```

### Scenario 2: Sekvencijalne zavisnosti

```cpp
// Za result[i] = f(result[i-1])
for (int step = 1; step < size; step++) {
    if (rank >= step) {
        int from = rank - step;
        MPI_Recv(&prev, 1, MPI_INT, from, 0, MCW, MPI_STATUS_IGNORE);
    }
    if (rank + step < size) {
        MPI_Send(&local, 1, MPI_INT, rank + step, 0, MCW);
    }
    MPI_Barrier(MCW);
}
```

---

## Dodatne Napomene

- **Svi procesi moraju biti sinhronizovani** sa `Scatter`, `Gather`, `Bcast`, `Reduce`
- **Koristite `MPI_Barrier()`** za eksplicitnu sinhronizaciju
- **Za deadlock prevenciju** - pazite na redosled, koristite `MPI_Sendrecv`
- **Memory alignment** - sa custom datatype-ima koristite `MPI_Type_create_resized`
- **Varijabilna veličina N** - dodajte validaciju ako `n % size != 0`
- **Debugging** - testiranje sa manje procesora je brže
