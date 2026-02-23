#include <mpi.h>
#include <iosteam>
#include <vector>

#define K 8
#define N 4

int main(int argc, char **argv)
{
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int A[K][N], b[N], i, j;
  int *local_A, *local_res, *res;
  local_A = (int *)malloc(sizeof(int) * P * N);
  local_res = (int *)malloc(sizeof(int) * P);
  res = (int *)malloc(sizeof(int) * K);

  MPI_Datatype matrix, resized;

  if (rank == 0)
  {
    for (int i = 0; i < K; i++)
    {
      for (int j = 0; j < N; j++)
      {
        A[i][j] = i * j;
        b[j] = j;
      }
    }
  }

  MPI_Type_vector(K / P, N, N * P, MPI_INT, &matrix);
  MPI_Type_commit(&matrix);
  MPI_Type_create_resized(matrix, 0, N * sizeof(int), &resized);
  MPI_Type_commit(&resized);

  MPI_Bcast(b, N, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Scatter(A, 1, resized, local_A, P * N, MPI_INT, 0, MPI_COMM_WORLD);

  for (int i = 0; i < P; i++)
  {
    local_res[i] = 0;
    for (int j = 0; j < N; j++)
    {
      local_res[i] += local_A[i][j] * b[j];
    }
  }
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int rank, MPI_Comm comm);
int MPI_Scatter(void *send_buffer, int send_count, MPI_datatype send_type, void *recv_buffer, int recv_count, MPI_Datatype recv_type, int rank, MPI_Comm comm);
int MPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_create_resized(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype *newtype);
