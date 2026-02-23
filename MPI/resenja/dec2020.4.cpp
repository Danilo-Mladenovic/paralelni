#include <stdio.h>
#include <mpi.h>

void maindec20204(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	struct zaposleni
	{
		int maticni_broj;
		float prosecna_plata;
		char ime[30];
		char prezime[30];
	};

	zaposleni zaposleni;
	int count = 4;
	int blklen[4] = {1, 1, 30, 30};
	MPI_Datatype tipovi[4] = {MPI_INT, MPI_FLOAT, MPI_CHAR, MPI_CHAR};
	MPI_Aint addr0, addr1, addr2, addr3;
	MPI_Get_address(&(zaposleni.maticni_broj), &addr0);
	MPI_Get_address(&(zaposleni.prosecna_plata), &addr1);
	MPI_Get_address(&(zaposleni.ime), &addr2);
	MPI_Get_address(&(zaposleni.prezime), &addr3);
	int displ[4] = {0, addr1 - addr0, addr2 - addr0, addr3 - addr0};
	MPI_Datatype radnik;
	MPI_Type_create_struct(count, blklen, displ, tipovi, &radnik);
	MPI_Type_commit(&radnik);

	struct zaposleni radnici[3];
	if (rank == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			printf("Unesite podatke o zaposlenima:\n");
			printf("Radnik %d:\n", i + 1);
			printf("	Maticni broj: ");
			scanf("%d", &(radnici[i].maticni_broj));
			printf("	Ime: ");
			scanf("%s", &(radnici[i].ime));
			printf("	Prezime: ");
			scanf("%s", &(radnici[i].prezime));
			printf("	Prosecna plata: ");
			scanf("%f", &(radnici[i].prosecna_plata));
		}
	}
	MPI_Bcast(radnici, 3, radnik, 0, MPI_COMM_WORLD);

	printf("Proces %d:", rank);
	for (int i = 0; i < 3; i++)
	{
		printf("\n	Radnik %d:\n", i + 1);
		printf("		Maticni broj: %d\n", radnici[i].maticni_broj);
		printf("		Ime: %s\n", radnici[i].ime);
		printf("		Prezime: %s\n", radnici[i].prezime);
		printf("		Prosecna plata: %f", radnici[i].prosecna_plata);
	}

	MPI_Finalize();
}