N=3

i=0						i=1
a[0]=a[1]+b[0]*c[0]		a[1]=a[2]+b[1]*c[1]
t=t*a[0]				t=t*a[1]

t=1;
#pragma omp parallel for shared(a, a_copy)
for(i=0; i<N-1; i++)
	a_copy[i]=a[i];
#pragma omp parallel for shared(a, a_copy) reduction(*:t)
for(i=0; i<N-1; i++)
{
	a[i]=a_copy[i+1]+b[i]*c[i];
	t*=a[i];
}

N=3				
b=2, c=3, t=1
a[0]=1, a[1]=2, a[2]=3
//sekvencijalno
i=0
a[0]=2+2*3;
t=1*8;

i=1
a[1]=3+2*3;
t=8*9;
-->t=72
//paralelno
i=0
a[0]=2+2*3;
t=1*8;

i=1
a[1]=3+2*3;
t=1*9
-->t=9*8=72