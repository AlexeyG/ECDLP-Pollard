#include "mpi.h"
#include "ParallelHelpers.h"
#include "ParallelDefines.h"
#include "ParallelMaster.h"
#include "../ecc/ecurve.h"
#include "../ecc/epoint.h"
#include "../ecc/2nfactory.h"
#include "../ecc/2n.h"
#include "../ecc/bint.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <iostream>

namespace ParallelHelpers
{
	const ParallelIdentity *identity;

	/* Helper methods */

	// Initializes identity variable used internally in communication.
	void initialize(const ParallelIdentity *identity)
	{
		ParallelHelpers::identity = identity;
	}

	// Frees the identity variable memory.
	void finalize(void)
	{
		delete ParallelHelpers::identity;
	}

	// Returns true if integer a is power of 2, otherwise - false.
	bool is_power_of_two(int a)
	{
		return (a & (a - 1)) == 0;
	}

	// Sleeps for some miliseconds.
	void sleep(int miliseconds)
	{
#ifdef WIN32
		Sleep(miliseconds);
#else
		usleep(miliseconds);
#endif
	}

	// Makes a message tag modifier (used to group messages).
	inline int make_tag(void)
	{
		int source = identity->get_process_id() % (PARALLEL_TAG_SOURCE_PATTERN + 1);
		int index = (rand() % PARALLEL_TAG_INDEX_PATTERN) + 1;
		return index | (source << PARALLEL_TAG_SOURCE_SHIFT);
	}

	// Returns tag modifier or zero if the modifier has no tag.
	inline int extract_tag(int tag)
	{
		return tag & PARALLEL_TAG_PATTERN;
	}

	/* Parallel commands */

	// Returns true if we have a message with given pattern from ANY source.
	bool have_incoming_message(int pattern)
	{
		int result;
        MPI_Iprobe(MPI_ANY_SOURCE, pattern, MPI_COMM_WORLD, &result, MPI_STATUS_IGNORE);
		return result != 0;
	}

	// Sends a control message
	MPI_Request send_control_message(int message, int process)
	{
		MPI_Request req;
		MPI_Isend(static_cast<void *>(&message), 1, MPI_INT, process, PARALLEL_CONTROL_TAG, MPI_COMM_WORLD, &req);
		return req;
	}

	// Returns a control message if there is one
	int receive_control_message(void)
	{
		int result;
        if (!have_incoming_message(PARALLEL_CONTROL_TAG))
			return PARALLEL_NO_CONTROL_MESSAGE;
		MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, PARALLEL_CONTROL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		return result;
	}

	// Extracts and sends (if a new tag is generated) tag to destination process.
	int extract_and_send_tag(int process, int pattern)
	{
		int mod = extract_tag(pattern);
		if (mod == 0)
		{
			mod = make_tag();
			MPI_Send(&mod, 1, MPI_INT, process, pattern, MPI_COMM_WORLD);
		}

		return mod;
	}

	// Extracts tag and receives it when required.
	int extract_and_receive_tag(int process, int pattern)
	{
		int mod = extract_tag(pattern);
		if (mod == 0)
			MPI_Recv(&mod, 1, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		return mod;
	}

	/* Communication commands */

	// Sends elliptic curve to given process.
	void send_ecurve(const ecurve &curve, int process, int pattern = PARALLEL_ECURVE_TAG)
	{
		int i;
		int length = (curve.has_factor() ? curve.get_factor_length() : 0);
		const bfactor *factors = curve.get_factor();

		int mod = extract_and_send_tag(process, pattern);

		MPI_Send(&length, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
		for (i = 0; i < length; i++)
			send_bfactor(factors[i], process, PARALLEL_BFACTOR_TAG ^ mod);

		send_lnum(curve.get_a(), process, PARALLEL_LNUM_TAG ^ mod);
		send_lnum(curve.get_b(), process, PARALLEL_LNUM_TAG ^ mod);
	}

	// Sends elliptic curve to given process.
	void send_epoint(const epoint &point, int process, int pattern = PARALLEL_EPOINT_TAG)
	{
		int mod = extract_and_send_tag(process, pattern);

		send_lnum(point.get_x(), process, PARALLEL_LNUM_TAG ^ mod);
		send_lnum(point.get_y(), process, PARALLEL_LNUM_TAG ^ mod);
	}

	// Sends field with header to given process.
	void send_gf2n(const gf2n &field, int process, int pattern = PARALLEL_GF2N_TAG)
	{
		int mod = extract_and_send_tag(process, pattern);

		int outputMode = field.get_output_mode();
		send_lnum(field.get_module(), process, PARALLEL_LNUM_TAG ^ mod);
		MPI_Send(&outputMode, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
	}

	// Sends a polynom, using given pattern.
	void send_lnum(const lnum &x, int process, int pattern = PARALLEL_LNUM_TAG)
	{
		MPI_Send(x.to_int(), lLen, MPI_INT, process, pattern, MPI_COMM_WORLD);
	}

	// Sends a big integer, using a given pattern.
	void send_bint(const bint &num, int process, int pattern = PARALLEL_BINT_TAG)
	{
		int mod = extract_and_send_tag(process, pattern);

		int sign = num.get_sign();
		int len = num.get_length();
		const int *buf = num.get_ints() + len;
		len = bLen + 1 - len;
		MPI_Send(&sign, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
		MPI_Send(&len, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
		MPI_Send((void *)buf, len, MPI_INT, process, PARALLEL_BINT_TAG ^ mod, MPI_COMM_WORLD);
	}

	// Sends a factorization unit, using a given pattern.
	void send_bfactor(const bfactor &factor, int process, int pattern = PARALLEL_BFACTOR_TAG)
	{
		int mod = extract_and_send_tag(process, pattern);

		MPI_Send((void *)&factor.k, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
		send_bint(factor.p, process, PARALLEL_BINT_TAG ^ mod);
	}

	// Sends ParallelData structure information to process with ID process.
	void send_ParallelData(int instance, const lnum &packedPoint, const bint &coefC, const bint &coefD, int process, int pattern = PARALLEL_PARALLELDATA_TAG)
	{
		int mod = extract_and_send_tag(process, pattern);

		MPI_Send(&instance, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD);
		ParallelHelpers::send_lnum(packedPoint, process, PARALLEL_LNUM_TAG ^ mod);
		ParallelHelpers::send_bint(coefC, process, PARALLEL_BINT_TAG ^ mod);
		ParallelHelpers::send_bint(coefD, process, PARALLEL_BINT_TAG ^ mod);
	}

	// Receives an elliptic curve and returns it.
	ecurve *receive_ecurve(int process, const gf2n &field, int pattern = PARALLEL_ECURVE_TAG)
	{
		int i;
		int length;

		int mod = extract_and_receive_tag(process, pattern);

		MPI_Recv(&length, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		bfactor *factors = new bfactor[length];
		for (i = 0; i < length; i++)
			factors[i] = receive_bfactor(process, PARALLEL_BFACTOR_TAG ^ mod);
		ecurve *result = new ecurve(receive_lnum(process, field, PARALLEL_LNUM_TAG ^ mod), receive_lnum(process, field, PARALLEL_LNUM_TAG ^ mod), length, factors);	
		delete [] factors;
		return result;
	}

	// Receives field and returns it.
	gf2n *receive_gf2n(int process, int pattern = PARALLEL_GF2N_TAG)
	{
		int outputMode;

		int mod = extract_and_receive_tag(process, pattern);

		gf2n *result = new gf2n(receive_lnum(process, PARALLEL_LNUM_TAG ^ mod));
		MPI_Recv(&outputMode, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		result->set_output_mode(outputMode);
		return result;
	}

	// Receives and returns a polynom, attached to no field.
	lnum receive_lnum(int process, int pattern = PARALLEL_LNUM_TAG)
	{
		unsigned int *buffer = new unsigned int[lLen];

		MPI_Recv(buffer, lLen, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		lnum result = lnum::create_nullmodule(buffer, lLen);
		delete [] buffer;

		return result;
	}

	// Receives and returns a polynom, attached to given field.
	lnum receive_lnum(int process, const gf2n &field, int pattern = PARALLEL_LNUM_TAG)
	{
		unsigned int *buffer = new unsigned int[lLen];

		MPI_Recv(buffer, lLen, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		lnum result = lnum(buffer, lLen, field);
		delete [] buffer;

		return result;
	}

	// Receives and returns a bit integer.
	bint receive_bint(int process, int pattern = PARALLEL_BINT_TAG)
	{
		int len, sign;

		int mod = extract_and_receive_tag(process, pattern);

		MPI_Recv(&sign, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&len, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		int *buffer = new int[len];
		MPI_Recv(buffer, len, MPI_INT, process, PARALLEL_BINT_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		bint result = bint(buffer, len, sign);
		delete [] buffer;
		return result;
	}

	// Receives and returns a factorization unit.
	bfactor receive_bfactor(int process, int pattern = PARALLEL_BFACTOR_TAG)
	{
		bfactor factor;

		int mod = extract_and_receive_tag(process, pattern);

		MPI_Recv(&factor.k, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		factor.p = receive_bint(process, PARALLEL_BINT_TAG ^ mod);
		return factor;
	}

	// Sends elliptic curve to given process.
	epoint receive_epoint(int process, const ecurve &curve, int pattern = PARALLEL_EPOINT_TAG)
	{
		const gf2n &field = curve.get_field();

		int mod = extract_and_receive_tag(process, pattern);

		lnum x = receive_lnum(process, field, PARALLEL_LNUM_TAG ^ mod);
		lnum y = receive_lnum(process, field, PARALLEL_LNUM_TAG ^ mod);
		return epoint(x, y, curve);
	}

	// Receives ParallelData structure from ANY source.
	ParallelData *receive_ParallelData(int process, const gf2n &field, int pattern = PARALLEL_PARALLELDATA_TAG)
	{
		ParallelData *result = new ParallelData;

		int mod = extract_and_receive_tag(process, pattern);

		MPI_Recv(&result->instance, 1, MPI_INT, process, PARALLEL_LENGTH_TAG ^ mod, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		result->key = ParallelHelpers::receive_lnum(process, field, PARALLEL_LNUM_TAG ^ mod);
		result->c = ParallelHelpers::receive_bint(process, PARALLEL_BINT_TAG ^ mod);
		result->d = ParallelHelpers::receive_bint(process, PARALLEL_BINT_TAG ^ mod);
		return result;
	}
}
