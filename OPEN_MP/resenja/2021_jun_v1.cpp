n=4

i=3
x=res[3]+add[3]
sum[3]=sum[2]+x

i=2
x=res[2]+add[2]
sum[2]=sum[1]+x

#pragma omp parallel for
for(i=0; i<n; i++)
	sum_copy[i]=sum[i];

#pragma omp parallel for lastprivate(x)
for(i=n-1; i>1; i--)
{
	x = res[i] + add[i];
	sum[i] = sum_copy[i - 1] + x;
}