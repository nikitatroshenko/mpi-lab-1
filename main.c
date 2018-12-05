#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void receive_data(int world_size);

void compose_and_send_data(int world_rank, int world_size);

int main(int argc, char **argv)
{
	int world_size;
	int world_rank;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	compose_and_send_data(world_rank, world_size);

	MPI_Finalize();
}

void compose_and_send_data(int world_rank, int world_size)
{
	srand(world_rank);
	int data_size = 10 + world_size + world_rank + rand() % 10;
	char *data = calloc(data_size, sizeof *data);
	MPI_Request request;

	snprintf(data, data_size, "%d out of %d says hello, my master", world_rank, world_size);
	data[data_size - 2] = '!';

	MPI_Isend(data, data_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);

	if (world_rank == 0) {
		receive_data(world_size);
	}

	MPI_Wait(&request, MPI_STATUS_IGNORE);
	free(data);
}

void receive_data(int world_size)
{
	char **world_data = calloc(world_size, sizeof *world_data);
	int i;

	for (i = 0; i < world_size; i++) {
		MPI_Status status;
		char *data;
		int data_size;
		int source;

		MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_CHAR, &data_size);

		source = status.MPI_SOURCE;
		data = calloc(data_size, sizeof *data);

		MPI_Recv(data,
			 data_size,
			 MPI_CHAR,
			 source,
			 0,
			 MPI_COMM_WORLD,
			 &status);
		world_data[source] = data;
	}

	for (i = 0; i < world_size; i++) {
		printf("%d -> %s\n", i, world_data[i]);
	}

	for (i = 0; i < world_size; i++) {
		free(world_data[i]);
	}
	free(world_data);
}
