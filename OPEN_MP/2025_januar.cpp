#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define M 32

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int t_s = 3, t_p = 3, k = 4;
  int *c_s, *c_p, *e;

  c_s = (int *)malloc(sizeof(int) * M);
  c_p = (int *)malloc(sizeof(int) * M);
  e = (int *)malloc(sizeof(int) * (t + M * k));

    
}