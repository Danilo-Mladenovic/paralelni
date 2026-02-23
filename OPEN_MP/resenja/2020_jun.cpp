i = 0, j = 0 i = 1, j = 0 a[0] = a[0] + b[s] a[0] = a[0] + b[s + k + k] z = s + k z = s + k + k + k

																																																			i = 0,
			 j = 1 i = 1, j = 1 a[1] = a[1] + b[s + k];
a[1] = a[1] + b[s + k + k + k];
z = s + k + k;
z = s + k + k + k + k;

#pragma omp parallel private(i) for reduction(+ : z)
for (j = 0; j < n; j++)
	for (i = 0; i < n; i++)
	{
		a[j] += b[s + (i * n + j) * k];
		z += k;
	}