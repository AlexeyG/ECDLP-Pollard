#include "mpi.h"
#include "ParallelIdentity.h"
#include "ParallelPollard.h"
#include "ParallelManager.h"
#include "ParallelMaster.h"
#include "ParallelSlave.h"
#include "ParallelHelpers.h"
#include "ParallelDefines.h"
#include "config.h"
#include <ctime>

ParallelPollard *pollard;

int main(int argc, char *argv[])
{
	int master_count;
	int procID;

	MPI_Init(&argc, &argv);
	ParallelHelpers::initialize(new ParallelIdentity());
	procID = ParallelHelpers::identity->get_process_id();
	srand((unsigned int)time(0) * (procID + 1));

	if (procID == MANAGER_RANK)
	{
		pollard = new ParallelManager(*ParallelHelpers::identity, INPUT_FILE_NAME, CONFIG_FILE_NAME, CIPHER_FILE_NAME, OUTPUT_FILE_NAME);
		master_count = ((ParallelManager *)pollard)->get_master_count();
	}

	// Send configuration info to everyone.
	MPI_Bcast(&master_count, 1, MPI_INT, MANAGER_RANK, MPI_COMM_WORLD);

	if (procID != MANAGER_RANK)
	{
		if (procID <= master_count)
			pollard = new ParallelMaster(*ParallelHelpers::identity);
		else
			pollard = new ParallelSlave(*ParallelHelpers::identity);
	}
	
	pollard->run();

	delete pollard;
	ParallelHelpers::finalize();
	MPI_Finalize();
	return 0;
}
