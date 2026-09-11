# MPI (Message Passing Interface) Cheatsheet

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
