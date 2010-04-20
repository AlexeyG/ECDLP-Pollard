#include "mpi.h"
#include "ParallelMaster.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"
#include "../ecc/bintoperations.h"

/* ParallelMaster class */

/* Constructors */

// Main identity constructor
ParallelMaster::ParallelMaster(const ParallelIdentity &identity)
	: ParallelPollard(identity)
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
	bool acceptPoints = false;

	ParallelData *newPoint;
	bint result;

	while (controlMessage != PARALLEL_ABORT_CONTROL_MESSAGE)
	{
		switch (controlMessage)
		{
		case PARALLEL_INIT_CONTROL_MESSAGE :
			tree.Clear();
			receive_pollard_parameters();
			acceptPoints = true;
			break;
		case PARALLEL_DONE_CONTROL_MESSAGE :
			acceptPoints = false;
			break;
		}

		if (acceptPoints && ParallelHelpers::have_incoming_message(PARALLEL_PARALLELDATA_TAG))
		{
			newPoint = ParallelHelpers::receive_ParallelData(MPI_ANY_SOURCE, *field, PARALLEL_PARALLELDATA_TAG);
			if (newPoint->instance != instance)
				delete newPoint;
			else
			{
				Node<ParallelData *> *oldNode = tree.Add(newPoint);
				if (oldNode != NULL)
				{
					bool haveSolution = solve_congruence(newPoint->c, newPoint->d, oldNode->dat->c, oldNode->dat->d, result);
					send_solution(haveSolution, result);
				}
			}
		}
		controlMessage = ParallelHelpers::receive_control_message();
	};
	tree.Clear();
}

/* Helper methods */

// Receives configuration from manager.
void ParallelMaster::receive_config(void)
{
	int mod = ParallelHelpers::extract_and_receive_tag(MANAGER_RANK, PARALLEL_CONFIG_GROUP);

	field = ParallelHelpers::receive_gf2n(MANAGER_RANK, PARALLEL_GF2N_TAG ^ mod);
	curve = ParallelHelpers::receive_ecurve(MANAGER_RANK, *field, PARALLEL_ECURVE_TAG ^ mod);
}

// Receives all parameters required to start Pollard's algorithm
void ParallelMaster::receive_pollard_parameters(void)
{
	MPI_Recv(&instance, 1, MPI_INT, MANAGER_RANK, PARALLEL_LENGTH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	groupOrder = ParallelHelpers::receive_bint(MANAGER_RANK, PARALLEL_BINT_TAG);
}

// Notifies manager about completion of the algorithm (collision found) and sends found solution (if we have one) to the manager.
void ParallelMaster::send_solution(bool haveSolution, const bint &solution) const
{
	ParallelHelpers::send_control_message(PARALLEL_DONE_CONTROL_MESSAGE, MANAGER_RANK);
	int haveSolutionInt = (haveSolution ? 1 : 0);

	int mod = ParallelHelpers::extract_and_send_tag(MANAGER_RANK, PARALLEL_SOLUTION_GROUP);

	MPI_Send((void *)&instance, 1, MPI_INT, MANAGER_RANK, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
	MPI_Send((void *)&haveSolutionInt, 1, MPI_INT, MANAGER_RANK, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
	if (haveSolution)
		ParallelHelpers::send_bint(solution, MANAGER_RANK, PARALLEL_BINT_TAG ^ mod);
}

// Solves an equation for key determination from collision data.
bool ParallelMaster::solve_congruence(bint c1, bint d1, bint c2, bint d2, bint &result) const
{
	c1 -= c2;
	if (c1.is_less_zero())
		c1 += groupOrder;

	d2 -= d1;
	if (d2.is_less_zero())
		d2 += groupOrder;

	if (d2.is_zero())
		return false;

	bintOperations::inv(d2, groupOrder, d1);
	if (d1.is_less_zero())
		d1 += groupOrder;
	result = (c1 * d1) % groupOrder;

	return true;
}
