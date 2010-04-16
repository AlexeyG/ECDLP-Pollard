#include "mpi.h"
#include "ParallelSlave.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"

/* ParallelSlave class */

/* Constructors */

// Main identity constructor
ParallelSlave::ParallelSlave(const ParallelIdentity &identity)
	: ParallelPollard(identity)
{
	receive_config();
}

/* Destructors */

// Frees any used memory.
ParallelSlave::~ParallelSlave()
{
}

/* Worker methods */

// Runs the job.
void ParallelSlave::run(void)
{
	int controlMessage = PARALLEL_NO_CONTROL_MESSAGE;
	bool generatePoints = false;

	while (controlMessage != PARALLEL_ABORT_CONTROL_MESSAGE)
	{
		switch (controlMessage)
		{
		case PARALLEL_INIT_CONTROL_MESSAGE :
			receive_pollard_parameters();
			generatePoints = true;
			break;
		case PARALLEL_DONE_CONTROL_MESSAGE :
			generatePoints = false;
			break;
		}

		if (generatePoints)
		{
			// check if we should send point
			// find out where to send it
			// send it
			generate_next_point();
		}
		controlMessage = ParallelHelpers::receive_control_message();
	};
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

// Receives iteration function configuration from manager.
void ParallelSlave::receive_iteration_function(void)
{
	for (int i = 0; i < PARALLEL_SET_COUNT; i++)
	{
		functionA[i] = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_ITERATION_COEF_TAG);
		functionB[i] = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_ITERATION_COEF_TAG);
		functionR[i] = ParallelHelpers::receive_point(MANAGER_RANK, *curve);
	}
}

// Receives initial point from manager.
void ParallelSlave::receive_initial_point(void)
{
	coefC = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_INITIAL_POINT_BINT_TAG);
	coefD = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_INITIAL_POINT_BINT_TAG);
	pointX = ParallelHelpers::receive_point(MANAGER_RANK, *curve);
}

// Receives all parameters required to start Pollard's algorithm
void ParallelSlave::receive_pollard_parameters(void)
{
	receive_iteration_function();
	receive_initial_point();
	groupOrder = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_GROUP_ORDER_TAG);
}

// Generates next point in sequence
void ParallelSlave::generate_next_point(void)
{
	int setInd = pointX.f(PARALLEL_SET_ARG);
	pointX += functionR[setInd];
	coefC += functionA[setInd]; if (coefC >= groupOrder) coefC -= groupOrder;
	coefD += functionB[setInd]; if (coefD >= groupOrder) coefD -= groupOrder;
}
