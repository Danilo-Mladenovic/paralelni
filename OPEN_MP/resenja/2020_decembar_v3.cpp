#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 6
#define mPocetno 1

bool proveriRezultate(int *nizSekv, int *nizParalelno, int xSekv, int xParalelno);

int main()
{
    omp_set_num_threads(omp_get_num_procs());
    int X[N];
    int Y[N * N * N];
    int xParalelno[N];

    for (size_t i = 0; i < N; i++)
    {
        X[i] = i;
        xParalelno[i] = X[i];
    }

    for (size_t i = 0; i < N * N * N; i++)
    {
        Y[i] = rand() % 102;
    }

    int m;
    m = mPocetno;
    int z = m;
    for (size_t i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            X[j] += Y[z];
            z += 2;
        }
    }

    int mParalelno = mPocetno;
    int zParalelno = mParalelno;
    int zPomocno = zParalelno;
#pragma omp parallel for reduction(+ : zParalelno)
    for (int i = 0; i < N * N; i++)
    {
        xParalelno[i % N] += Y[zPomocno + i * 2];
        zParalelno = zParalelno + 2;
    }

    printf("z: %d\n", z);

    printf("zParalelno: %d\n", zParalelno);

    if (proveriRezultate(X, xParalelno, z, zParalelno))
    {
        printf("RESENJA SU ISTA!\n");
    }
    else
    {
        printf("RESENJA NISU ISTA!\n");
    }

    printf("Sekvencno\n");
    for (int i = 0; i < N; i++)
        printf("%d ", X[i]);
    printf("\n\n");

    printf("Paralelno\n");
    for (int i = 0; i < N; i++)
        printf("%d ", xParalelno[i]);
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
