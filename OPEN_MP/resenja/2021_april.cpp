#include <stdio.h>
#include <omp.h>

#define N 100

int f(int val)
{
	return val + 5;
}

void mainapr2021()
{
	int start = 0, step = 2;
	int v, sum = 0, v_seq = start, sum_seq = 0;

	for (int i = 0; i < N; i++)
	{
		sum_seq += f(v_seq);
		v_seq += step;
	}

#pragma omp parallel for reduction(+ : sum)
	for (int i = 0; i < N; i++)
	{
		int vp = start + i * step;
		sum += f(vp);
	}

	if (sum != sum_seq)
		printf("Greska sum\n");

	v = start + N * step;
	if (v != v_seq)
		printf("Greska v\n");
}