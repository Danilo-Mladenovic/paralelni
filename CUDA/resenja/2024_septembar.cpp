#include <stdio.h>
#include <stdlib.h>


#define N 1040
#define M 2
#define BLOCKSIZE 256


__global__ void findPattern(char *dA, int *dB,char *pattern) {

    __shared__ char mem[BLOCKSIZE + M + 1];
    int tid = blockIdx.x * blockDim.x + threadIdx.x; 

    //racunajuci da je m manje od 256;
    if(tid < N) {
        mem[threadIdx.x] = dA[tid];

        if(tid + M < N && threadIdx.x < M) {
            mem[threadIdx.x + BLOCKSIZE] = dA[tid + BLOCKSIZE];
        }
    }

    __syncthreads();

    if(tid < N - M) {
    int valid = 1;
    for(int i=0; i<M; i++) {
        if(mem[i + threadIdx.x] != pattern[i]) {
            valid = 0;
            break;
        }
    }

    if(valid == 1){
        dB[tid] = 1;
    }
    }
}

int main() {
    char *A, *dA, *pattern, *dPattern;
    int *B, *dB; 

    char pom[4] = {'A', 'U', 'C', 'G'};

    A = (char *)malloc(N * sizeof(char));
    B = (int *)malloc(N * sizeof(int));
    pattern = (char *)malloc(M * sizeof(char));

    for(int i = 0; i < N; i++) {
        A[i] = pom[rand() % 4];
        printf("%c ", A[i]);
        B[i] = 0;

        if(i < M) {
            pattern[i] = pom[i];
        }
    }

    printf("\n");

    cudaMalloc(&dA,N*sizeof(char));
    cudaMalloc(&dB,N*sizeof(int));
    cudaMalloc(&dPattern,M*sizeof(char));

    cudaMemcpy(dA,A,N*sizeof(char),cudaMemcpyHostToDevice);
    cudaMemcpy(dB,B,N*sizeof(int),cudaMemcpyHostToDevice);
    cudaMemcpy(dPattern,pattern,M*sizeof(char),cudaMemcpyHostToDevice);

    findPattern<<<(N+BLOCKSIZE -1)/BLOCKSIZE,BLOCKSIZE>>>(dA,dB,dPattern);


    cudaMemcpy(B,dB,N * sizeof(int), cudaMemcpyDeviceToHost);



    for(int i = 0; i < N; i++) {
        if(B[i] == 1) {
            printf("(%c, %c) at %d\n", A[i], A[i + 1], i);
        }
    }

    printf("\n");

    free(A);
    free(B);
    cudaFree(dA);
    cudaFree(dB);

    return 0;
}