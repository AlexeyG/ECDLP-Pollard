#include "mpi.h"
#include "ParallelMaster.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"
#include "../ecc/2nfactory.h"
#include "../ecc/ecurve.h"

/* ParallelMaster class */

/* Constructors */

// Main identity constructor
ParallelMaster::ParallelMaster(const ParallelIdentity &identity) : ParallelPollard(identity)
{
	receive_config();
}

/* Destructors */

// Frees any used memory.
ParallelMaster::~ParallelMaster()
{
	delete field;
	delete curve;
}

/* Worker methods */

// Runs the job.
void ParallelMaster::run(void)
{
	while (!ParallelHelpers::is_aborted())
	{
		//
		ParallelHelpers::sleep(LOOP_SLEEP);
	}
}

/* Helper methods */

// Receives configuration from manager.
void ParallelMaster::receive_config(void)
{
	// !!-!! to be removed
	MPI_Recv(&master_count, 1, MPI_INT, MANAGER_RANK, PARALLEL_MASTER_COUNT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&condition_prefix_length, 1, MPI_INT, MANAGER_RANK, PARALLEL_CONDITION_PREFIX_LENGTH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	field = ParallelHelpers::receive_field(MANAGER_RANK);
	curve = ParallelHelpers::receive_curve(MANAGER_RANK, *field);
}