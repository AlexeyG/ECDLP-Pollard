#include "ParallelIdentity.h"
#include "mpi.h"

/* ParallelIdentity class */

/* Constructors */

// Main identity constructor
ParallelIdentity::ParallelIdentity(int processID, int processCount) : processID(processID), processCount(processCount)
{
}

// Empty constructor - gets the required info automaticly.
ParallelIdentity::ParallelIdentity()
{
	MPI_Comm_rank(MPI_COMM_WORLD, &processID);
	MPI_Comm_size(MPI_COMM_WORLD, &processCount);
}

/* Accessor methods */

// Returns process id.
int ParallelIdentity::get_process_id() const
{
	return processID;
}

// Returns process count.
int ParallelIdentity::get_process_count() const
{
	return processCount;
}