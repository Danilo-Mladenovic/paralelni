#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 6
#define dPocetno 1

bool proveriRezultate(int *nizSekv, int *nizParalelno, int xSekv, int xParalelno);

int main()
{
    omp_set_num_threads(omp_get_num_procs());
    int b[N];
    int bParalelno[N];

    for (size_t i = 0; i < N; i++)
    {
        b[i] = rand() % 102;
        bParalelno[i] = b[i];
    }

    int d;
    d = dPocetno;

    for (int m = 0; m < N; m++)
    {
        b[m] = b[m + 1] * d;
        d = d + m;
    }

    int dParalelno = dPocetno;

    int bPomocno[N];
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        bPomocno[i] = bParalelno[i];
    }

    int dPomocno = dParalelno;
#pragma omp parallel for reduction(+ : d)
    for (int m = 0; m < N; m++)
    {
        bParalelno[m] = bPomocno[m + 1] * ((m * (m - 1)) / 2 + dPomocno);
        dParalelno += m;
    }

    printf("d: %d\n", d);

    printf("dParalelno: %d\n", dParalelno);

    if (proveriRezultate(b, bParalelno, d, dParalelno))
    {
        printf("RESENJA SU ISTA!\n");
    }
    else
    {
        printf("RESENJA NISU ISTA!\n");
    }

    printf("Sekvencno\n");
    for (int i = 0; i < N; i++)
        printf("%d ", b[i]);
    printf("\n\n");

    printf("Paralelno\n");
    for (int i = 0; i < N; i++)
        printf("%d ", bParalelno[i]);
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
