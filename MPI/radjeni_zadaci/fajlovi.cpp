primer1 :

#define N 10

    int local_baf[N];
MPI_Status st;

MPI_File file;
MPI_File_open(MCW, "primer1.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

MPI_File_seek(file, rank * N * sizeof(int), MPI_SEEK_SET);

MPI_File_write(file, local_baf, N, MPI_INT, &st);

MPI_File_close(&file);

II nacin ukoliko nemamo MPI_File_seek

    MPI_File_write_at(file, rank * N * sizeof(int), local_baf, N, MPI_INT, &st);
///////////////////////////////////////////////////////////////////////////////////////////////
primer2 :

    int bafer[2 * NW];

MPI_File file;

MPI_File_open(MCW, "primer2.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

MPI_Datatype filetype;

MPI_Type_vector(2, NW, NW *size, MPI_INT, &filetype);
MPI_Type_commit(&filetype);
MPI_File_set_view(file, rank * NW * sizeof(int), MPI_INT, filetype, "native", MPI_STATUS_IGNORE);

MPI_File_write_all(file, bafer, 2 * NW, MPI_INT, MPI_STATUS_IGNORE);
MPI_File_close(&file);
//////////////////////////////////////////////////////////////////////////////////////////////'

Zadatak1:
           
a)

#define N 10 

    int local_baf[N];

for (i = 0; i < N; i++)
{
   local_baf[i] = rank * N + i;
}

MPI_File file;

MPI_File_open(MCW, "zadatak.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

MPI_File_seek(file, rank * N * sizeof(int), MPI_SEEK_SET);

MPI_File_write(file, local_baf, N, MPI_INT, MPI_STATUS_IGNORE);

MPI_File_close(&file);
//////////////////////////////////////////////////////////////////////////////////////////////
b)  MPI_File_open(MCW,"zadatak.txt", MPI_MODE_RDONLY, MPI_INFO_NULL,&file);

MPI_File_read_at(file, rank * N * sizeof(int), local_baf, N, MPI_INT, MPI_STATUS_IGNORE);

MPI_File_close(&file);
/////////////////////////////////////////////////////////////////////////////////////////////

 c)  MPI_File file1;
 MPI_File_open(MCW, "zadatak1.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file1);

 MPI_Datatype filetype;
 MPI_Type_vector(N / 2, 2, 2 * size, MPI_INT, &filetype);
 MPI_Type_commit(&filetype);
 MPI_File_set_view(file1, rank * 2 * sizeof(int), MPI_INT, filetype, "native", MPI_STATUS_IGNORE);

 MPI_File_write_all(file1, local_baf, N, MPI_INT, MPI_STATUS_IGNORE);

 MPI_File_close(&file1);
 ////////////////////////////////////////////////////////////////////////////////////////////////

 BLANKETI :

     okt 2 2023 int local_baf[s];

 MPI_Datatype new_type, new_type_r;
 MPI_Type_vector(s, 1, q, MPI_INT, &new_type);
 MPI_Type_commit(&new_type);
 MPI_Type_create_resized(new_type, 0, 1 * sizeof(int), &new_type_r);
 MPI_Type_commit(&new_type_r);

 if (col_id == 0)
 {
    MPI_Scatter(c, 1, new_type_r, local_baf, s, MPI_INT, 0, rowcomm);

    MPI_File file;
    MPI_File_open(rowcomm, "okt2-c.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

    MPI_File_set_view(file, rank * sizeof(int), MPI_INT, new_type, "native", MPI_STATUS_IGNORE);
    MPI_File_write_all(file, local_baf, s, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);
 }
 ////////////////////////////////////////////////////////////////////////////////////////////////

 okt 2023

     int local_a[s * m];

 MPI_Datatype filetype;
 MPI_Type_vector(m, s, size *s, MPI_INT, &filetype);
 MPI_Type_commit(&filetype);

 MPI_File file;
 MPI_File_open(MCW, "matA.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

 MPI_File_set_view(file, rank * s * sizeof(int), MPI_INT, filetype, "native", MPI_STATUS_IGNORE);

 MPI_File_read_all(file, local_a, s *m, MPI_INT, MPI_STATUS_IGNORE);

 MPI_File_close(&file);

 bez fajlova :

     if (rank == 0)
 {
    for (i = 0; i < k; i++)
    {
       for (j = 0; j < m; j++)
       {
          A[i][j] = rand() % 20;
       }
    }
 }

 MPI_Scatter(A, s *m, MPI_INT, local_a, s *m, MPI_INT, 0, MCW);
 ///////////////////////////////////////////////////////////////////////
 free(Sandra);

 MPI_Finalize();