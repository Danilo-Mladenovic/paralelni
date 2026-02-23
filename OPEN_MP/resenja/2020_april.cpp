start=2, sum=0, step=3
v=start

i=0
sum=f(v)
v=start+step

i=1
sum=f(v)+f(v)
v=start+step+step

i=2
sum=f(v)+f(v)+f(v)
v=start+step+step+step

sum=0;
start=2;
step=3;
v=start;
#pragma omp parallel for reduction(+:v)
for(int i=0; i<n, i++)
{
	sum=(i+1)*f(start+(i+1)*step);
	v+=step;
}