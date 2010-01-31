#include "mpi.h"
#include "parallel.h"
#include "../ecc/2nfactory.h"
#include "../ecc/epoint.h"
#include "../ecc/ecurve.h"
#include "../ecc/helpers.h"
#include "../ecc/crypto.h"
#include "../ecc/eccoperations.h"
#include "../pollard/crack.h"
#include "../pollard/crackdefines.h"
#include "parallelhelpers.h"
#include "paralleldefines.h"
#include <iostream>
#include <ctime>

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

/* ParallelManager class */

/* Constructors */

// Creates a new instance of parallel manager.
// It takes a ParallelIdentity class as a parameter and file names:
// - input_filename - filename, contaning field, curve and point data.
// - config_filename - filename, containing parallel master-slave configuration.
// - encrypted_filename - filename, containing encrypted data.
// - output_filename - filename, to which we should save decrypted data.
ParallelManager::ParallelManager(const ParallelIdentity &identity, char *input_filename, char *config_filename, char *encrypted_filename, char *output_filename) : ParallelPollard(identity)
{
	open_files(input_filename, config_filename, encrypted_filename, output_filename);
	if (!read_input() || !read_config())
	{
		abort_all(identity.get_process_id());
		return;
	}
	send_config();
}

/* Destructors */

// Closes all files and frees any used memory.
ParallelManager::~ParallelManager()
{
	close_files();
	delete field;
	delete curve;
}

/* Accessor methods */

// Returns number of master processors for this task.
int ParallelManager::get_master_count(void) const
{
	return master_count;
}

/* Worker methods */

// Runs the job.
void ParallelManager::run(void)
{
	crypto *transform;
	bint solution;
	double work_time;
	if (!solve(G, bG, solution, true, "    ", work_time))
	{
		std::cout << "[-] Unable to solve ECDLP." << std::endl;
	}
	else
	{
		char *time_string = new char[LINE_LEN];
		sprintf(time_string, "%.5lf", work_time);
		std::cout << "[+] Solved ECDLP (Bobs key is " << solution << ") in " << time_string << " seconds." << std::endl;
		delete[] time_string;
		transform = new crypto(*curve);
		if (transform->get_message_length() <= 0)
		{
			std::cout << "    [-] Unable to decrypt data - field is too small." << std::endl;
		}
		else
		{
			transform->set_G(G);
			transform->set_keyB(solution);
			transform->override_aG(aG, true);
			int encrypted_data_length;
			unsigned char *encrypted_data = helpers::read_till_end(fenc, encrypted_data_length);
			int decrypted_data_length;
			unsigned char *decrypted_data = transform->decrypt(encrypted_data, encrypted_data_length, decrypted_data_length);
			helpers::output_decrypted_data(fout, decrypted_data, decrypted_data_length - 1); // Cut out the zero string termination char.
			delete[] encrypted_data;
			delete[] decrypted_data;
		}
	}
	// !!+!! Add some command to tell everyone, that we're done.
}

// Solves ECDLP for points G and xG, saves result to big integer result. Returns true if successful, otherwise - false.
bool ParallelManager::solve(const epoint &G, const epoint &xG, bint &result, bool verbose, char *offset, double &work_time)
{
	work_time = clock();
	int i, j;
	int factor_count;
	pofactor *factors = crackRoutines::calculate_point_order_factorization(G, factor_count);
	char *time_string = new char[LINE_LEN];
	bint order;
	G.order(order);
	for (i = 0; i < factor_count; i++)
	{
		if (verbose) std::cout << offset << "[i] Solving for prime-power subgroup (" << factors[i].p << " ^ " << factors[i].k << ")" << std::endl;
		factors[i].sol.zero();
		epoint temp = xG;
		bint power(1);
		bint temp_solution;
		bint N = order /  factors[i].p;
		epoint P(*curve), Q(*curve);
		eccOperations::mul(G, N, P);
		for (j = 0; j < factors[i].k; j++)
		{
			eccOperations::mul(temp, N, Q);
			int attempt = 0;
			bool solved = false;
			if (verbose) std::cout << offset << "    [i] Solving for (" << factors[i].p << " ^ " << j + 1 << ")" << std::endl;
			while (!solved && attempt < MAX_POLLARD_ATTEMPTS)
			{
				double work_time = 10;
				solved = pollard(P, Q, factors[i].p, temp_solution, work_time);
				if (verbose) sprintf(time_string, PRINT_FORMAT, work_time);
				if (!solved)
				{
					if (verbose) std::cout << offset << "        [-] (" << attempt + 1 << ") Fail: " << time_string << " seconds." << std::endl;
				}
				else
					if (verbose) std::cout << offset << "        [+] (" << attempt + 1 << ") Success: " << time_string << " seconds." << std::endl;
				attempt++;
			}
			if (!solved)
			{
				delete[] time_string;
				return false;
			}
			bint temp_number = temp_solution * power;
			factors[i].sol += temp_number;
			eccOperations::mul(G, temp_number, Q);
			temp -= Q;
			power = power * factors[i].p;
			N = N / factors[i].p;
		}
	}

	result = crackRoutines::chinese_remainder_theorem(factors, factor_count, order);
	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;

	delete[] time_string;
	return true;
}

// 
bool ParallelManager::pollard(const epoint &P, const epoint &Q, const bint &order, bint &result, double &work_time)
{
	work_time = clock();
	// we will send init command, after that we will send the iteration function
	// generate random points for iteration function
	// send random points to slaves
	// send initial points to slaves
	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;
}

/* Helper methods */

// Sends abort with error info to all running processes.
void ParallelManager::abort_all(int info) const
{
	int process;
	int count = identity.get_process_count();
	int currentID = identity.get_process_id();
	for (process = 0; process < count; process++)
		MPI_Send(&info, 1, MPI_INT, process, PARALLEL_ABORT_TAG, MPI_COMM_WORLD);
}

// Sends configuration to all processors.
void ParallelManager::send_config(void) const
{
	// curve, points G, aG and bG
	int process;
	int count = identity.get_process_count();
	int currentID = identity.get_process_id();
	MPI_Request req;

	// !!-!! to be removed
	// Master count and condition_prefix_length for masters.
	for (process = 1; process <= master_count; process++)
	{
		MPI_Isend((void *)&master_count, 1, MPI_INT, process, PARALLEL_MASTER_COUNT_TAG, MPI_COMM_WORLD, &req);
		MPI_Isend((void *)&condition_prefix_length, 1, MPI_INT, process, PARALLEL_CONDITION_PREFIX_LENGTH_TAG, MPI_COMM_WORLD, &req);
	}

	// Master count and condition_prefix_length for slaves.
	for (process = master_count + 1; process < count; process++)
	{
		MPI_Isend((void *)&master_count, 1, MPI_INT, process, PARALLEL_MASTER_COUNT_TAG, MPI_COMM_WORLD, &req);
		MPI_Isend((void *)&condition_prefix_length, 1, MPI_INT, process, PARALLEL_CONDITION_PREFIX_LENGTH_TAG, MPI_COMM_WORLD, &req);
	}

	for (process = 1; process < count; process++)
	{
		ParallelHelpers::send_field(*field, process);
		ParallelHelpers::send_curve(*curve, process);
	}
}

/* Internal methods */

// Opens all needed file streams:
// - input_filename - filename, contaning field, curve and point data.
// - config_filename - filename, containing parallel master-slave configuration.
// - encrypted_filename - filename, containing encrypted data.
// - output_filename - filename, to which we should save decrypted data.
void ParallelManager::open_files(char *input_filename, char *config_filename, char *encrypted_filename, char *output_filename)
{
	fin.open(input_filename);
	fconfig.open(config_filename);
	fenc.open(encrypted_filename, std::ios::in | std::ios::binary);
	fout.open(output_filename, std::ios::in | std::ios::binary);
}

// Closes all open file streams.
void ParallelManager::close_files()
{
	fin.close();
	fconfig.close();
	fenc.close();
	fout.close();
}

// Reads config data.
bool ParallelManager::read_config()
{
	fconfig >> master_count;
	fconfig >> condition_prefix_length;
	if (master_count < 0)
	{
		std::cout << "[-] Invalid number of master processors specified (must be positive)." << std::endl;
		return false;
	}
	if (!ParallelHelpers::is_power_of_two(master_count))
	{
		std::cout << "[-] Invalid number of master processors specified (must be power of two)." << std::endl;
		return false;
	}
	if (master_count >= identity.get_process_count())
	{
		std::cout << "[-] Invalid number of master processors specified (must be less than process count)." << std::endl;
		return false;
	}
	if (condition_prefix_length < 0)
	{
		std::cout << "[-] Invalid condition prefix length specified (must be positive)." << std::endl;
		return false;
	}
	// !!+!! check that our field is big enough for our configuration
	return true;
}

// Reads input data.
bool ParallelManager::read_input()
{
	field = helpers::read_field(fin);
	std::cout << "[+] Created field (degree " << field->get_deg() << ")." << std::endl;
	curve = helpers::read_curve(fin, *field);
	if (!curve->has_factor())
	{
		std::cout << "[-] No factorization of curves order avaliable => exiting." << std::endl;
		return false;
	}
	bint curveOrder;
	curve->order(curveOrder);
	std::cout << "[+] Created elliptic curve (order " << curveOrder << ")." << std::endl;
	G = helpers::read_next_point(fin, *curve);
	aG = helpers::read_next_point(fin, *curve);
	bG = helpers::read_next_point(fin, *curve);
	if (!G.check() || !aG.check() || !bG.check())
	{
		std::cout << "[-] Incorrect input data - points do not belong to curve => exiting." << std::endl;
		return false;
	}
	return true;
}

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