i=0, j=0			i=1, j=0
x[0]=x[0]+y[m]		x[0]=x[0]+y[m+2+2]	
z=m+2				z=m+2+2+2

i=0, j=1			i=1, j=1
x[1]=x[1]+y[m+2]	x[1]=x[1]+y[m+2+2+2]
z=m+2+2				z=m+2+2+2+2

z=m;
#pragma omp parallel for private(i) reduction(+:z)
for(i=0; i<n; i++)
	for(j=0; j<n; j++)
	{
		x[j]+=y[m+(i*n+j)*2];
		z+=2;
	}