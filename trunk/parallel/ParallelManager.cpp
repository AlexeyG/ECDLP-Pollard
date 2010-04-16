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
	bool result = !read_input() || !read_config();
	send_config();

	if (!result)
		send_control_message_to_all(PARALLEL_ABORT_CONTROL_MESSAGE);
}

/* Destructors */

// Closes all files and frees any used memory.
ParallelManager::~ParallelManager()
{
	close_files();
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
	// !!-!!
	ParallelHelpers::sleep(100000);
	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;

	delete[] time_string;
	return true;
}

// Starts parallel Pollard's rho method. Handle all data distribution.
bool ParallelManager::pollard(const epoint &P, const epoint &Q, const bint &order, bint &result, double &work_time)
{
	work_time = clock();

	const ecurve &curve = P.get_curve();
	if (!curve.belongs_to_curve(Q))
	{
		work_time = 0;
		return false;
	}

	// Tell everyone to start thier engines!
	send_control_message_to_all(PARALLEL_INIT_CONTROL_MESSAGE);
	// Generate iteration function to slaves
	generate_interation_function(order, P, Q);
	// Send everything to slaves
	send_pollard_parameters(order, P, Q);

	int controlMessage = PARALLEL_NO_CONTROL_MESSAGE;
	while (controlMessage != PARALLEL_ABORT_CONTROL_MESSAGE && controlMessage != PARALLEL_DONE_CONTROL_MESSAGE)
	{
		//
		controlMessage = ParallelHelpers::receive_control_message();
	}

	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;
	return true;
}

/* Helper methods */

// Sends iteration function to other processes
void ParallelManager::send_iteration_function() const
{
	int process;
	int count = identity.get_process_count();

	for (int i = 0; i < PARALLEL_SET_COUNT; i++)
		for (process = master_count + 1; process < count; process++)
		{
			ParallelHelpers::send_bint(functionA[i], process, PARALLEL_ITERATION_COEF_TAG);
			ParallelHelpers::send_bint(functionB[i], process, PARALLEL_ITERATION_COEF_TAG);
			ParallelHelpers::send_point(functionR[i], process);
		}
}

// Sends control message to all running processes.
void ParallelManager::send_control_message_to_all(int message) const
{
	int process;
	int count = identity.get_process_count();
	for (process = 0; process < count; process++)
		ParallelHelpers::send_control_message(message, process);
}

// Sends configuration to all processors.
void ParallelManager::send_config(void) const
{
	// curve, points G, aG and bG
	int process;
	int count = identity.get_process_count();
	int currentID = identity.get_process_id();
	MPI_Request req;

	// Master count and condition_prefix_length for slaves.
	for (process = master_count + 1; process < count; process++)
	{
		MPI_Isend((void *)&master_count, 1, MPI_INT, process, PARALLEL_MASTER_COUNT_TAG, MPI_COMM_WORLD, &req);
		MPI_Isend((void *)&condition_prefix_length, 1, MPI_INT, process, PARALLEL_CONDITION_PREFIX_LENGTH_TAG, MPI_COMM_WORLD, &req);
	}

	for (process = 0; process < count; process++)
		if (process != MANAGER_RANK)
		{
			ParallelHelpers::send_field(*field, process);
			ParallelHelpers::send_curve(*curve, process);
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
		c.random(); c = c % order;
		d.random(); d = d % order;
		eccOperations::mul(P, c, X);
		eccOperations::mul(Q, d, tempPoint);
		X += tempPoint;

		ParallelHelpers::send_bint(c, process, PARALLEL_INITIAL_POINT_BINT_TAG);
		ParallelHelpers::send_bint(d, process, PARALLEL_INITIAL_POINT_BINT_TAG);
		ParallelHelpers::send_point(X, process);
	}
}

// Send all required configuration information to slaves.
void ParallelManager::send_pollard_parameters(const bint &order, const epoint &P, const epoint &Q) const
{
	int process;
	int count = identity.get_process_count();

	// Send iteration function to slaves
	send_iteration_function();

	// Generate & send initial points for slaves
	generate_and_send_initial_points(order, P, Q);

	// Send group order to slaves
	for (process = master_count + 1; process < count; process++)
		ParallelHelpers::send_bint(order, process, PARALLEL_GROUP_ORDER_TAG);
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