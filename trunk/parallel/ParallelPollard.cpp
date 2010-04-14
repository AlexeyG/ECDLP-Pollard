#include "mpi.h"
#include "ParallelPollard.h"

/* ParallelPollard class */

/* Constructors */

// Creates a new instance of ParallelPollard class
ParallelPollard::ParallelPollard(const ParallelIdentity &identity) : identity(identity)
{
}

/* Destructors */

// Main destructor (virtual)
ParallelPollard::~ParallelPollard()
{
}

/* Accessor methods */

// Returns this processes' identity object.
const ParallelIdentity &ParallelPollard::get_identity(void) const
{
	return identity;
}

/* Worker methods */

// Dummy method, that runs the job.
void ParallelPollard::run(void)
{
}