#include "mpi.h"
#include "ParallelSlave.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"
#include "../ecc/2nfactory.h"

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
	if (conditionPrefixLength != 0)
		delete [] conditionPrefixLength;
	if (conditionPrefix != 0)
		delete [] conditionPrefix;
}

/* Worker methods */

// Runs the job.
void ParallelSlave::run(void)
{
	int controlMessage = PARALLEL_NO_CONTROL_MESSAGE;
	bool generatePoints = false;

	lnum packedPoint;

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
			generate_next_point();
			int masterInd = should_send();
			if (masterInd > 0)
			{
				if (pointX.pack(packedPoint) == pE_OK)
					ParallelHelpers::send_ParallelData(instance, packedPoint, coefC, coefD, masterInd, PARALLEL_PARALLELDATA_TAG);
			}
		}
		controlMessage = ParallelHelpers::receive_control_message();
	};
}

/* Helper methods */

// Receives configuration from manager.
void ParallelSlave::receive_config(void)
{
	conditionPrefix = 0;
	conditionPrefixLength = 0;

	int mod = ParallelHelpers::extract_and_receive_tag(MANAGER_RANK, PARALLEL_CONFIG_GROUP);

	field = ParallelHelpers::receive_gf2n(MANAGER_RANK, PARALLEL_GF2N_TAG ^ mod);
	curve = ParallelHelpers::receive_ecurve(MANAGER_RANK, *field, PARALLEL_ECURVE_TAG ^ mod);

	MPI_Recv(&master_count, 1, MPI_INT, MANAGER_RANK, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	conditionPrefix = new lnum[master_count];
	conditionPrefixLength = new int[master_count];
	MPI_Recv((void *)conditionPrefixLength, master_count, MPI_INT, MANAGER_RANK, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	for (int i = 0; i < master_count; i++)
		conditionPrefix[i] = ParallelHelpers::receive_lnum(MANAGER_RANK, *field, PARALLEL_LNUM_TAG ^ mod);
}

// Receives iteration function configuration from manager.
void ParallelSlave::receive_iteration_function(void)
{
	int mod = ParallelHelpers::extract_and_receive_tag(MANAGER_RANK, PARALLEL_ITERATION_FUNCTION_GROUP);

	for (int i = 0; i < PARALLEL_SET_COUNT; i++)
	{
		functionA[i] = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_BINT_TAG ^ mod);
		functionB[i] = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_BINT_TAG ^ mod);
		functionR[i] = ParallelHelpers::receive_epoint(MANAGER_RANK, *curve, PARALLEL_EPOINT_TAG ^ mod);
	}
}

// Receives initial point from manager.
void ParallelSlave::receive_initial_point(void)
{
	int mod = ParallelHelpers::extract_and_receive_tag(MANAGER_RANK, PARALLEL_INITIAL_POINT_GROUP);

	coefC = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_BINT_TAG ^ mod);
	coefD = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_BINT_TAG ^ mod);
	pointX = ParallelHelpers::receive_epoint(MANAGER_RANK, *curve, PARALLEL_EPOINT_TAG ^ mod);
}

// Receives all parameters required to start Pollard's algorithm
void ParallelSlave::receive_pollard_parameters(void)
{
	MPI_Recv(&instance, 1, MPI_INT, MANAGER_RANK, PARALLEL_LENGTH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	receive_iteration_function();
	receive_initial_point();
	groupOrder = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_BINT_TAG);
}

// Generates next point in sequence
void ParallelSlave::generate_next_point(void)
{
	int setInd = pointX.f(PARALLEL_SET_ARG);
	pointX += functionR[setInd];
	coefC += functionA[setInd]; if (coefC >= groupOrder) coefC -= groupOrder;
	coefD += functionB[setInd]; if (coefD >= groupOrder) coefD -= groupOrder;
}

// Returns ID of master to send current point to or zero if the point should not be sent.
int ParallelSlave::should_send(void) const
{
	lnum x;
	pointX.get_x(x);
	int xDeg = x.deg();
	unsigned int *xInts = x.to_int();
	unsigned int *yInts;
	unsigned int sum;

	for (int i = 0; i < master_count; i++)
	{
		yInts = conditionPrefix[i].to_int();
		int len = min(xDeg, conditionPrefixLength[i]);
		int shift = len % lbLen;
		len /= lbLen;
		sum = 0;
		for (int j = 0; j < len; j++)
			sum |= xInts[j] ^ yInts[j];
		sum |= (!shift ? 0 : (xInts[len] ^ yInts[len]) << (lbLen - shift));
		if (sum == 0)
			return i + 1;
	}

	return 0;
}
