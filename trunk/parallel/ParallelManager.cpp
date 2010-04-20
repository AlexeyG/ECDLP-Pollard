#include "mpi.h"
#include "ParallelManager.h"
#include "ParallelDefines.h"
#include "ParallelHelpers.h"
#include "../ecc/2nfactory.h"
#include "../ecc/ecurve.h"
#include "../ecc/crypto.h"
#include "../ecc/helpers.h"
#include "../ecc/eccoperations.h"
#include "../ecc/eccoperations.h"
#include "../pollard/crack.h"
#include "../pollard/crackdefines.h"
#include <ctime>

/* ParallelManager class */

/* Constructors */

// Creates a new instance of parallel manager.
// It takes a ParallelIdentity class as a parameter and file names:
// - input_filename - filename, contaning field, curve and point data.
// - config_filename - filename, containing parallel master-slave configuration.
// - encrypted_filename - filename, containing encrypted data.
// - output_filename - filename, to which we should save decrypted data.
ParallelManager::ParallelManager(const ParallelIdentity &identity, char *input_filename, char *config_filename, char *encrypted_filename, char *output_filename)
	: ParallelPollard(identity)
{
	open_files(input_filename, config_filename, encrypted_filename, output_filename);
	bool result = read_input() && read_config();
	
	send_config();

	if (!result)
		send_control_message_to_all(PARALLEL_ABORT_CONTROL_MESSAGE);
}

/* Destructors */

// Closes all files and frees any used memory.
ParallelManager::~ParallelManager()
{
	close_files();
	if (conditionPrefixLength != 0)
		delete [] conditionPrefixLength;
	if (conditionPrefix != 0)
		delete [] conditionPrefix;
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
	
	send_control_message_to_all(PARALLEL_ABORT_CONTROL_MESSAGE);
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
				delete [] time_string;
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

// Starts parallel Pollard's rho method. Handle all data distribution.
bool ParallelManager::pollard(const epoint &P, const epoint &Q, const bint &order, bint &result, double &work_time)
{
	work_time = clock();
	int controlMessage = PARALLEL_NO_CONTROL_MESSAGE;

	const ecurve &curve = P.get_curve();
	if (!curve.belongs_to_curve(Q))
	{
		work_time = 0;
		return false;
	}

	// Should we cann sequential Pollard?
	if (order < PARALLEL_SEQUENTIAL_MIN_ORDER)
	{
		crack *seq = new crack(curve, P, Q);
		bool solved =  seq->solve(result, false, "", work_time);
		delete seq;

		return solved;
	}

	// Tell everyone to start thier engines!
	send_control_message_to_all(PARALLEL_INIT_CONTROL_MESSAGE);
	// Generate iteration function to slaves
	generate_interation_function(order, P, Q);
	// Send everything to slaves
	send_pollard_parameters(order, P, Q);
	
	bool success = false;
	int solutionInstance = 0;

	while (controlMessage != PARALLEL_ABORT_CONTROL_MESSAGE)
	{
		if (controlMessage == PARALLEL_DONE_CONTROL_MESSAGE)
		{
			success = receive_solution(solutionInstance, result);
			if (solutionInstance == instance)
			{
				send_control_message_to_all(PARALLEL_DONE_CONTROL_MESSAGE);
				break;
			}
		}
		controlMessage = ParallelHelpers::receive_control_message();
	}

	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;
	return success;
}

/* Helper methods */

// Sends iteration function to other processes
void ParallelManager::send_iteration_function() const
{
	int process;
	int count = identity.get_process_count();

	for (process = master_count + 1; process < count; process++)
	{
		int mod = ParallelHelpers::extract_and_send_tag(process, PARALLEL_ITERATION_FUNCTION_GROUP);
		for (int i = 0; i < PARALLEL_SET_COUNT; i++)	
		{
			ParallelHelpers::send_bint(functionA[i], process, PARALLEL_BINT_TAG ^ mod);
			ParallelHelpers::send_bint(functionB[i], process, PARALLEL_BINT_TAG ^ mod);
			ParallelHelpers::send_epoint(functionR[i], process, PARALLEL_EPOINT_TAG ^ mod);
		}
	}
}

// Sends control message to all running processes.
void ParallelManager::send_control_message_to_all(int message) const
{
	int process;
	int count = identity.get_process_count();
	MPI_Request *req = new MPI_Request[count - 1];
	for (process = 1; process < count; process++)
		req[process - 1] = ParallelHelpers::send_control_message(message, process);

	MPI_Waitall(count - 1, req, MPI_STATUSES_IGNORE);
	delete [] req;
}

// Sends configuration to all processors.
void ParallelManager::send_config(void) const
{
	// curve, points G, aG and bG
	int process;
	int count = identity.get_process_count();
	int currentID = identity.get_process_id();
	MPI_Request req;

	for (process = 0; process < count; process++)
		if (process != MANAGER_RANK)
		{
			int mod = ParallelHelpers::extract_and_send_tag(process, PARALLEL_CONFIG_GROUP);

			ParallelHelpers::send_gf2n(*field, process, PARALLEL_GF2N_TAG ^ mod);
			ParallelHelpers::send_ecurve(*curve, process, PARALLEL_ECURVE_TAG ^ mod);

			// Master count and condition_prefix_length for slaves.
			if (process > master_count)
			{
				MPI_Send((void *)&master_count, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
				MPI_Send((void *)conditionPrefixLength, master_count, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
				for (int i = 0; i < master_count; i++)
					ParallelHelpers::send_lnum(conditionPrefix[i], process, PARALLEL_LNUM_TAG ^ mod);
			}
		}
}

// Generates and sends initial points for slaves. It is done here, because we don't want to rely on slaves' random point generator.
void ParallelManager::generate_and_send_initial_points(const bint &order, const epoint &P, const epoint &Q) const
{
	int process;
	int count = identity.get_process_count();
	const ecurve &curve = P.get_curve();

	epoint X(curve);
	epoint tempPoint(curve);
	bint c, d;


	for (process = master_count + 1; process < count; process++)
	{
		int mod = ParallelHelpers::extract_and_send_tag(process, PARALLEL_INITIAL_POINT_GROUP);

		c.random(); c = c % order;
		d.random(); d = d % order;
		eccOperations::mul(P, c, X);
		eccOperations::mul(Q, d, tempPoint);
		X += tempPoint;

		ParallelHelpers::send_bint(c, process, PARALLEL_BINT_TAG ^ mod);
		ParallelHelpers::send_bint(d, process, PARALLEL_BINT_TAG ^ mod);
		ParallelHelpers::send_epoint(X, process, PARALLEL_EPOINT_TAG ^ mod);
	}
}

// Send all required configuration information to slaves.
void ParallelManager::send_pollard_parameters(const bint &order, const epoint &P, const epoint &Q)
{
	int process;
	int count = identity.get_process_count();

	// Define instance - used in synchronization.
	instance = (rand() % PARALLEL_MAX_INT) + 1;

	// Send instance to everyone
	for (process = 1; process < count; process++)
		MPI_Send((void *)&instance, 1, MPI_INT, process, PARALLEL_LENGTH_TAG, MPI_COMM_WORLD);

	// Send iteration function to slaves
	send_iteration_function();

	// Generate & send initial points for slaves
	generate_and_send_initial_points(order, P, Q);

	// Send group order to everyone
	for (process = 1; process < count; process++)
		ParallelHelpers::send_bint(order, process, PARALLEL_BINT_TAG);
}

// Receives a solution from a master. Returns true if it is a valid solution and false otherwise.
bool ParallelManager::receive_solution(int &solutionInstance, bint &solution) const
{
	int resultInt;
	bool result;

	int mod = ParallelHelpers::extract_and_receive_tag(MPI_ANY_SOURCE, PARALLEL_SOLUTION_GROUP);

	MPI_Recv((void *)&solutionInstance, 1, MPI_INT, MPI_ANY_SOURCE, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv((void *)&resultInt, 1, MPI_INT, MPI_ANY_SOURCE, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	result = resultInt != 0;
	if (result)
		solution = ParallelHelpers::receive_bint(MPI_ANY_SOURCE, PARALLEL_BINT_TAG ^ mod);

	return result;
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
	fenc.open(encrypted_filename, std::ios::binary | std::ios::in);
	fout.open(output_filename, std::ios::binary | std::ios::out);
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
	conditionPrefix = 0;
	conditionPrefixLength = 0;

	fconfig >> master_count;
	if (master_count < 0)
	{
		std::cout << "[-] Invalid number of master processors specified (must be positive)." << std::endl;
		return false;
	}
	if (master_count >= identity.get_process_count())
	{
		std::cout << "[-] Invalid number of master processors specified (must be less than process count)." << std::endl;
		return false;
	}

	conditionPrefix = new lnum[master_count];
	conditionPrefixLength = new int[master_count];

	int fieldDeg = field->get_deg();

	for (int i = 0; i < master_count; i++)
	{
		fconfig >> conditionPrefixLength[i];
		if (conditionPrefixLength[i] < 0)
		{
			std::cout << "[-] Invalid condition prefix length for processor " << i + 1 << " (must be non-negative)" << std::endl;
			return false;
		}
		if (conditionPrefixLength[i] >= fieldDeg)
		{
			std::cout << "[-] Invalid condition prefix length for processor " << i + 1 << " (must be less than field degree)" << std::endl;
			return false;
		}
		conditionPrefix[i] = helpers::read_next_polynom(fconfig, *field);
	}
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

// Generate random numbers and random points from group, which are used to define interation function.
void ParallelManager::generate_interation_function(const bint &order, const epoint &P, const epoint &Q)
{
	const ecurve &curve = P.get_curve();
	epoint tempPoint(curve);

	for (int i = 0; i < PARALLEL_SET_COUNT; i++)
	{
		functionA[i].random();
		functionA[i] = functionA[i] % order;
		functionB[i].random(); functionB[i] = functionB[i] % order;
		eccOperations::mul(P, functionA[i], functionR[i]);
		eccOperations::mul(Q, functionB[i], tempPoint);
		functionR[i] += tempPoint;
	}
}