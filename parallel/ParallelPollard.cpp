#include "mpi.h"
#include "ParallelPollard.h"
#include "ParallelDefines.h"
#include "../ecc/bint.h"
#include "../ecc/epoint.h"
#include "../ecc/ecurve.h"
#include "../ecc/2nfactory.h"

/* ParallelPollard class */

/* Constructors */

// Creates a new instance of ParallelPollard class
ParallelPollard::ParallelPollard(const ParallelIdentity &identity)
	: identity(identity)
{
	// Now allocate memory for iteration function
	functionA = new bint[PARALLEL_SET_COUNT];
	functionB = new bint[PARALLEL_SET_COUNT];
	functionR = new epoint[PARALLEL_SET_COUNT];

	instance = 0;
}

/* Destructors */

// Main destructor (virtual)
ParallelPollard::~ParallelPollard()
{
	delete field;
	delete curve;

	delete [] functionA;
	delete [] functionB;
	delete [] functionR;
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
