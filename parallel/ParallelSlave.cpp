#include "mpi.h"
#include "ParallelSlave.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"
#include "../ecc/2nfactory.h"
#include "../ecc/ecurve.h"

/* ParallelSlave class */

/* Constructors */

// Main identity constructor
ParallelSlave::ParallelSlave(const ParallelIdentity &identity) : ParallelPollard(identity)
{
	receive_config();
}

/* Destructors */

// Frees any used memory.
ParallelSlave::~ParallelSlave()
{
	delete field;
	delete curve;
}

/* Worker methods */

// Runs the job.
void ParallelSlave::run(void)
{
	while (!ParallelHelpers::is_aborted())
	{
		//
		ParallelHelpers::sleep(LOOP_SLEEP);
	}
}

/* Helper methods */

// Receives configuration from manager.
void ParallelSlave::receive_config(void)
{
	MPI_Recv(&master_count, 1, MPI_INT, MANAGER_RANK, PARALLEL_MASTER_COUNT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&condition_prefix_length, 1, MPI_INT, MANAGER_RANK, PARALLEL_CONDITION_PREFIX_LENGTH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	field = ParallelHelpers::receive_field(MANAGER_RANK);
	curve = ParallelHelpers::receive_curve(MANAGER_RANK, *field);
}