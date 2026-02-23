# OpenMP Cheat‑Sheet (tipovi + mini primeri)

## 1) Zavisnost unazad `A[i]=A[i+1]+…` + redukcija proizvoda

Prepoznavanje: koristi `A[i+1]` u računu `A[i]`; često `prod *= A[i]`.
Trik: kopija/shift (`A_copy[i]=A[i+1]`) i paralela + `reduction`.

**Mini primer**

```c
// sekvencijalno
for (int i=0;i<N-1;i++) {
  A[i] = A[i+1] + B[i]*C[i];
  prod *= A[i];
}

// paralelno
#pragma omp parallel
{
  #pragma omp for
  for (int i=0;i<N-1;i++) A_copy[i] = A[i+1];

  #pragma omp for reduction(*:prod)
  for (int i=0;i<N-1;i++) {
    A[i] = A_copy[i] + B[i]*C[i];
    prod *= A[i];
  }
}
```

## 2) Skalarna rekurencija `d` (zatvorena forma)

Prepoznavanje: `d` se menja po iteraciji i koristi u računu elemenata.
Trik: zatvorena forma, npr. `d(m)=d0 + m*(m-1)/2`.

**Mini primer**

```c
// sekvencijalno
for (int m=0;m<N;m++) {
  b[m] = b[m+1] * d;
  d = d + m;
}

// paralelno (d0 = početna vrednost)
#pragma omp parallel for
for (int m=0;m<N;m++) {
  int dm = d0 + m*(m-1)/2;
  b[m] = b_copy[m+1] * dm;
}
```

## 3) Skalarni `x` u petlji `Z[k]=k+x; x=k`

Prepoznavanje: `x` dobija poslednju iteraciju, `Z[k]` zavisi samo od `k`.
Trik: direktna formula `Z[k] = 2*k - 1` (za `k>0`), `x = N-1`.

**Mini primer**

```c
// sekvencijalno
for (int k=0;k<N;k++) {
  Z[k] = k + x;
  x = k;
}

// paralelno (zatvorena forma)
Z[0] = x0;
#pragma omp parallel for
for (int k=1;k<N;k++) {
  Z[k] = 2*k - 1;
}
// x = N-1
```

## 4) Ugnježdene petlje + globalni brojač `z+=const`

Prepoznavanje: `z` raste konstantno kroz ugnježdene petlje.
Trik: linearizuj indeks: `z = base + step*(i*N + j)`.

**Mini primer**

```c
// sekvencijalno
z = m;
for (int i=0;i<N;i++)
  for (int j=0;j<N;j++) {
    X[j] += Y[z];
    z += 2;
  }

// paralelno
int z = m;
#pragma omp parallel for reduction(+:z)
for (int i = 0; i < N; i++) {
  int z_local = m + 2*(i*N);   // početak za red i
  for (int j = 0; j < N; j++) {
    X[j] += Y[z_local];
    z_local += 2;
    z += 2;                    // redukcija broji ukupno korake
  }
}

```

## 5) Redukcija maksimuma

Prepoznavanje: traži se maksimum niza/matrice.
Trik: `reduction(max:localMax)` ili lokalno + `critical`.

**Mini primer**

```c
int maxv = INT_MIN;
#pragma omp parallel for reduction(max:maxv)
for (int i=0;i<N;i++)
  if (A[i] > maxv) maxv = A[i];
```

## 6) Direktive/semantika (task/critical/master/parallel for)

Prepoznavanje: pitanja o broju štampanja, data race, izvršavanju taskova.
Trik: znati razliku `master` vs `single`, `critical` serijalizuje, `task` stvara poslove.

**Mini primer**

```c
#pragma omp parallel
{
  #pragma omp single
  {
    #pragma omp task
    doA();
    #pragma omp task
    doB();
  }
}
```

## 7) Paralelno množenje matrica

Prepoznavanje: kvadratne matrice, C = A \* B.
Trik: `collapse(2)` po `i,j`, `k` ostaje sekvencijalan.

**Mini primer**

```c
#pragma omp parallel for
for (int i = 0; i < N; i++) {
  for (int j = 0; j < N; j++) {
    int sum = 0;
    for (int k = 0; k < N; k++)
      sum += A[i][k] * B[k][j];
    C[i][j] = sum;
  }
}
```

## 8) Ugnježdene petlje sa zavisnošću po indeksu (stride)

Prepoznavanje: `b[i][j] = f(b[i-2][j])` ili slično.
Trik: paralelizuj po nezavisnoj dimenziji (npr. `j`) ili po paritetu `i`.

**Mini primer**

```c
// sekvencijalno
for (int i=2;i<N;i++)
  for (int j=0;j<M;j++)
    b[i][j] = 4 * b[i-2][j];

// paralelno po j
#pragma omp parallel for
for (int j=0;j<M;j++)
  for (int i=2;i<N;i++)
    b[i][j] = 4 * b[i-2][j];
```

## 9) Lanac po `a[i-1]` ili `a[i+1]` + skalarni akumulator `d`

Prepoznavanje: `a[i]` koristi `a[i-1]` ili `a[i+1]`; `d` se multiplikativno menja.
Trik: kopija/shift za `a`, zatvorena forma za `d` (proizvod/kvocijent).

**Mini primer**

```c
// sekvencijalno
for (int i=1;i<N;i++) {
  d = d * z[i];
  a[i] = b[i]*c[pom + N - i - 1] + a[i-1];
}

// paralelno (ideja)
#pragma omp parallel for
for (int i=1;i<N;i++)
  a[i] = b[i]*c[pom + N - i - 1] + a_copy[i-1];
```

## 10) Floyd–Warshall (3 petlje)

Prepoznavanje: `path[i][j] > path[i][k] + path[k][j]`.
Trik: `k` mora ostati spolja; paralelizovati `i,j` za fiksni `k`.

**Mini primer**

```c
for (int k = 0; k < N; k++) {
  #pragma omp parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      int via = path[i][k] + path[k][j];
      if (path[i][j] > via)
        path[i][j] = via;
    }
  }
}
```
