d = 1, n = 4 m = 0 b[0] = b[1] *d d = 1 + 0 m = 1 b[1] = b[2] *d d = 1 + 0 + 1 m = 2 b[2] = b[3] *d d = 1 + 0 + 1 + 2 m = 3 b[3] = b[4] *d d = 1 + 0 + 1 + 2 + 3

			 = > d + (m * (m - 1) / 2)

									 int b[n],
			 b_copy[n], m, d, d_copy : d = d_copy = 1;
#pragma omp parallel for shared(b, b_copy)
for (m = 0; m < n; m++)
	b_copy[i] = b[i];
#pragma omp parallel for reduction(+ : d) firstprivate(d_copy)
for (m = 0; m < n; m++)
{
	b[m] = b_copy[m] * (m * (m - 1) / 2);
	d += m;
}