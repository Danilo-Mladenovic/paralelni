#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BR_NITI 256

__global__ void pronadi_podsekvencu(char *A, char *pattern, int n, int m, int *B);

int main()
{
    int n, m;
    printf("Unesite duzinu glavnog niza (n): ");
    scanf("%d", &n);
    printf("Unesite duzinu podsekvence (m): ");
    scanf("%d", &m);

    char *A = (char *)malloc(n * sizeof(char));
    char *pattern = (char *)malloc(m * sizeof(char));
    int *B = (int *)malloc(n * sizeof(int));

    printf("Unesite RNK sekvencu (A,U,C,G): ");
    scanf("%s", A);
    printf("Unesite trazenu podsekvencu: ");
    scanf("%s", pattern);

    // Device memorija
    char *d_A, *d_pattern;
    int *d_B;

    cudaMalloc((void **)&d_A, n * sizeof(char));
    cudaMalloc((void **)&d_pattern, m * sizeof(char));
    cudaMalloc((void **)&d_B, n * sizeof(int));

    // Kopiranje na device
    cudaMemcpy(d_A, A, n * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(d_pattern, pattern, m * sizeof(char), cudaMemcpyHostToDevice);

    // Kernel launch - omoguciti rad sa proizvoljnom velicinom
    int br_blokova = (n + BR_NITI - 1) / BR_NITI;
    if (br_blokova > 1024)
        br_blokova = 1024; // Ogranicenje broja blokova

    pronadi_podsekvencu<<<br_blokova, BR_NITI>>>(d_A, d_pattern, n, m, d_B);

    // Kopiranje rezultata nazad
    cudaMemcpy(B, d_B, n * sizeof(int), cudaMemcpyDeviceToHost);

    // Ispis rezultata
    printf("\nRezultat:\n");
    printf("Niz B: ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", B[i]);
    }
    printf("\n");

    // Pronalazenja pozicija
    printf("Pozicije gde je pronadjena podskvenca: ");
    for (int i = 0; i < n; i++)
    {
        if (B[i] == 1)
            printf("%d ", i);
    }
    printf("\n");

    // Oslobadjanje memorije
    free(A);
    free(pattern);
    free(B);
    cudaFree(d_A);
    cudaFree(d_pattern);
    cudaFree(d_B);

    return 0;
}

__global__ void pronadi_podsekvencu(char *A, char *pattern, int n, int m, int *B)
{
    // Grid-stride loop - omogucava rad sa proizvoljnom velicinom n
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = gridDim.x * blockDim.x; // Ukupan broj niti u gridu

    while (idx < n)
    {
        if (idx <= n - m)
        {
            // Proveravamo da li se podskvenca nalazi na poziciji idx
            int match = 1;
            for (int i = 0; i < m; i++)
            {
                if (A[idx + i] != pattern[i])
                {
                    match = 0;
                    break;
                }
            }
            B[idx] = match;
        }
        else
        {
            // Pozicije blize kraja kada nema dovoljno elemenata
            B[idx] = 0;
        }

        idx += stride; // Pređi na sledeći blok elemenata
    }
}
