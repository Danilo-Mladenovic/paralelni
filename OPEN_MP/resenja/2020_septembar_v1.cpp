#include <stdio.h>
#include <omp.h>

#define N 100

void mainsep2020()
{
	int start = 5;
	int d, d_seq = start, B[N + 1], B_seq[N + 1], B_copy[N + 1];

	for (int i = 0; i < N + 1; i++)
		B[i] = i + 1;

#pragma omp parallel for
	for (int i = 0; i < N + 1; i++)
	{
		B_seq[i] = B[i];
		B_copy[i] = B[i];
	}

	for (int m = 0; m < N; m++)
	{
		B_seq[m] = B_seq[m + 1] * d_seq;
		d_seq += m;
	}
	printf("%d\n", d_seq);

#pragma omp parallel for
	for (int m = 0; m < N; m++)
	{
		d = start + m * (m - 1) / 2;
		B[m] = B_copy[m + 1] * d;
	}

	d = start + N * (N - 1) / 2;

	printf("%d\n", d);

	if (d != d_seq)
		printf("Greska za d\n");

#pragma omp parallel for
	for (int i = 0; i < N + 1; i++)
		if (B[i] != B_seq[i])
			printf("Greska za B na lokaciji %d\n", i);
}