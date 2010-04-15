#include "mpi.h"
#include "ParallelMaster.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"

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
}

/* Worker methods */

// Runs the job.
void ParallelMaster::run(void)
{
	int controlMessage = PARALLEL_NO_CONTROL_MESSAGE;

	while (controlMessage != PARALLEL_ABORT_CONTROL_MESSAGE)
	{
		switch (controlMessage)
		{
		case PARALLEL_INIT_CONTROL_MESSAGE :
			ParallelHelpers::receive_iteration_function(MANAGER_RANK, *curve, functionA, functionB, functionR);
			// Clear the AVL tree
			break;
		case PARALLEL_DONE_CONTROL_MESSAGE :
			// Stop accepting messages?
			break;
		}
		ParallelHelpers::sleep(LOOP_SLEEP);
		controlMessage = ParallelHelpers::receive_control_message();
	};
}

/* Helper methods */

// Receives configuration from manager.
void ParallelMaster::receive_config(void)
{
	field = ParallelHelpers::receive_field(MANAGER_RANK);
	curve = ParallelHelpers::receive_curve(MANAGER_RANK, *field);
}