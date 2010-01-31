#include "mpi.h"
#include "parallel.h"
#include "paralleldefines.h"
#include "config.h"
#include <ctime>

// Stream variables
std::ifstream fin;
std::ifstream fenc;
std::ofstream fout;

ParallelIdentity *identity;
ParallelPollard *pollard;

int main(int argc, char *argv[])
{
	int master_count;
	//int size;
	//void *buffer = new char[BUFFER_SIZE];

	MPI_Init(&argc, &argv);
	//MPI_Buffer_attach(buffer, BUFFER_SIZE);
	identity = new ParallelIdentity();
	if (identity->get_process_id() == MANAGER_RANK)
	{
		srand((unsigned int)time(0));
		pollard = new ParallelManager(*identity, INPUT_FILE_NAME, CONFIG_FILE_NAME, CIPHER_FILE_NAME, OUTPUT_FILE_NAME);
		master_count = ((ParallelManager *)pollard)->get_master_count();
		// read curve data, crack data
		// read configuration options
		// start Pollig-Hellman
	}

	// Send configuration info to everyone.
	MPI_Bcast(&master_count, 1, MPI_INT, MANAGER_RANK, MPI_COMM_WORLD);

	if (identity->get_process_id() != MANAGER_RANK)
	{
		if (identity->get_process_id() <= master_count)
			pollard = new ParallelMaster(*identity);
		else
			pollard = new ParallelSlave(*identity);
	}
	pollard->run();
	//MPI_Buffer_detach(&buffer, &size);
	MPI_Finalize();
	//delete [] buffer;
	return 0;
}
