#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 6

bool proveriRezultate(int *nizSekv, int *nizParalelno, int xSekv, int xParalelno);

int main()
{
    omp_set_num_threads(omp_get_num_procs());

    int x;
    int z[N];
    x = 1;

    for (int k = 0; k < N; ++k)
    {
        z[k] = k + x;
        x = k;
    }

    int zParalelno[N];
    int xParalelno = 1;

    zParalelno[0] = 1;
#pragma omp parallel for lastprivate(xParalelno)
    for (int k = 1; k < N; ++k)
    {
        zParalelno[k] = k + k - 1;
        xParalelno = k;
        printf("Thread: %d xParalelno: %d\n", omp_get_thread_num(), xParalelno);
    }

    printf("x: %d\n", x);

    printf("xParalelno: %d\n", xParalelno);

    if (proveriRezultate(z, zParalelno, x, xParalelno))
    {
        printf("RESENJA SU ISTA!");
    }
    else
    {
        printf("RESENJA NISU ISTA!");
    }

    printf("Sekvencno\n");
    for (int i = 0; i < N; i++)
        printf("%d ", z[i]);
    printf("\n\n");

    printf("Paralelno\n");
    for (int i = 0; i < N; i++)
        printf("%d ", zParalelno[i]);
    printf("\n\n");

    // printf("sekvM: %d.\nsekvA:\n", sekvM);
    // for (int i = 0; i < N; i++)
    //     printf("%d ", sekvA[i]);
}

bool proveriRezultate(int *nizSekv, int *nizParalelno, int xSekv, int xParalelno)
{

    bool flag = true;
    for (int i = 0; i < N; i++)
    {
        if (nizSekv[i] != nizParalelno[i])
        {
            flag = false;
            return flag;
        }
    }

    if (xSekv != xParalelno)
    {
        flag = false;
    }
    return flag;
}
