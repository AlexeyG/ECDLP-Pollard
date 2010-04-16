#include "mpi.h"
#include "ParallelHelpers.h"
#include "ParallelDefines.h"
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
	/* Helper methods */

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

	// Makes a new message ID tag (used in communicating big stuff).
	int make_tag(int pattern)
	{
		int rnd = rand() % PARALLEL_TAG_PATTERN;
		return pattern ^ rnd;
	}

	/* Parallel commands */

	// Sends a control message
	void send_control_message(const int message, int process)
	{
		MPI_Request req;
		MPI_Isend(const_cast<void *>(static_cast<const void *>(&message)), 1, MPI_INT, process, PARALLEL_CONTROL_TAG, MPI_COMM_WORLD, &req);
	}

	// Returns a control message if there is one
	int receive_control_message(void)
	{
		int result;
        MPI_Iprobe(MPI_ANY_SOURCE, PARALLEL_CONTROL_TAG, MPI_COMM_WORLD, &result, MPI_STATUS_IGNORE);
        if (result == 0)
			return PARALLEL_NO_CONTROL_MESSAGE;

		MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, PARALLEL_CONTROL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		return result;
	}

	/* Communication commands */

	// Sends elliptic curve to given process.
	void send_curve(const ecurve &curve, int process)
	{
		int i;
		int length = (curve.has_factor() ? curve.get_factor_length() : 0);
		const bfactor *factors = curve.get_factor();

		MPI_Send(&length, 1, MPI_INT, process, PARALLEL_CURVE_FACTOR_LENGTH_TAG, MPI_COMM_WORLD);
		for (i = 0; i < length; i++)
			send_bfactor(factors[i], process, PARALLEL_CURVE_FACTOR_TAG);

		send_lnum(curve.get_a(), process, PARALLEL_CURVE_A_TAG);
		send_lnum(curve.get_b(), process, PARALLEL_CURVE_B_TAG);
	}

	// Sends elliptic curve to given process.
	void send_point(const epoint &point, int process)
	{
		send_lnum(point.get_x(), process, PARALLEL_POLYNOM_POINT_X_TAG);
		send_lnum(point.get_y(), process, PARALLEL_POLYNOM_POINT_Y_TAG);
	}

	// Sends field with header to given process.
	void send_field(const gf2n &field, int process)
	{
		int outputMode = field.get_output_mode();
		send_lnum(field.get_module(), process, PARALLEL_FIELD_TAG);
		MPI_Send(&outputMode, 1, MPI_INT, process, PARALLEL_FIELD_OUTPUT_MODE_TAG, MPI_COMM_WORLD);
	}

	// Sends a polynom, using given pattern.
	void send_lnum(const lnum &x, int process, int pattern = PARALLEL_POLYNOM_TAG)
	{
		/*int messageID = make_tag(pattern);
		//int length = (x.deg() + 1) / lbLen;
		MPI_Send(&messageID, 1, MPI_INT, process, pattern, MPI_COMM_WORLD);
		MPI_Send(x.to_int(), lLen, MPI_INT, process, messageID, MPI_COMM_WORLD);*/
		MPI_Send(x.to_int(), lLen, MPI_INT, process, pattern, MPI_COMM_WORLD);
	}

	// Sends a big integer, using a given pattern.
	void send_bint(const bint &num, int process, int pattern)
	{
		int len = num.get_length();
		const int *buf = num.get_ints() + len;
		len = bLen + 1 - len;
		MPI_Send(&len, 1, MPI_INT, process, pattern | PARALLEL_BINT_LENGTH_TAG, MPI_COMM_WORLD);
		MPI_Send((void *)buf, len, MPI_INT, process, pattern, MPI_COMM_WORLD);
	}

	// Sends a factorization unit, using a given pattern.
	void send_bfactor(const bfactor &factor, int process, int pattern)
	{
		MPI_Send((void *)&factor.k, 1, MPI_INT, process, pattern, MPI_COMM_WORLD);
		send_bint(factor.p, process, pattern);
	}

	// Receives an elliptic curve and returns it.
	ecurve *receive_curve(int process, const gf2n &field)
	{
		int i;
		int length;
		MPI_Recv(&length, 1, MPI_INT, process, PARALLEL_CURVE_FACTOR_LENGTH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		bfactor *factors = new bfactor[length];
		for (i = 0; i < length; i++)
			factors[i] = receive_bfactor(process, PARALLEL_CURVE_FACTOR_TAG);
		ecurve *result = new ecurve(receive_lnum(process, PARALLEL_CURVE_A_TAG, field), receive_lnum(process, PARALLEL_CURVE_B_TAG, field), length, factors);	
		delete [] factors;
		return result;
	}

	// Receives field and returns it.
	gf2n *receive_field(int process)
	{
		int outputMode;
		gf2n *result = new gf2n(receive_lnum(process, PARALLEL_FIELD_TAG));

		MPI_Recv(&outputMode, 1, MPI_INT, process, PARALLEL_FIELD_OUTPUT_MODE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		result->set_output_mode(outputMode);
		return result;
	}

	// Receives and returns a polynom, attached to no field.
	lnum receive_lnum(int process, int pattern)
	{
		/*int messageID;*/
		unsigned int *buffer = new unsigned int[lLen];

		/*MPI_Recv(&messageID, 1, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(buffer, lLen, MPI_INT, process, messageID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);*/

		MPI_Recv(buffer, lLen, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		lnum result = lnum::create_nullmodule(buffer, lLen);
		delete [] buffer;

		return result;
	}

	// Receives and returns a polynom, attached to given field.
	lnum receive_lnum(int process, int pattern, const gf2n &field)
	{
		//int messageID;
		unsigned int *buffer = new unsigned int[lLen];

		/*MPI_Recv(&messageID, 1, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(buffer, lLen, MPI_INT, process, messageID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);*/
		MPI_Recv(buffer, lLen, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		lnum result = lnum(buffer, lLen, field);
		delete [] buffer;

		return result;
	}

	// Receives and returns a bit integer.
	bint receive_bint(int process, int pattern)
	{
		int len;
		MPI_Recv(&len, 1, MPI_INT, process, pattern | PARALLEL_BINT_LENGTH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		int *buffer = new int[len];
		MPI_Recv(buffer, len, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		bint result = bint(buffer, len);
		delete [] buffer;

		return result;
	}

	// Receives and returns a factorization unit.
	bfactor receive_bfactor(int process, int pattern)
	{
		bfactor factor;
		MPI_Recv(&factor.k, 1, MPI_INT, process, pattern, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		factor.p = receive_bint(process, pattern);
		return factor;
	}

	// Sends elliptic curve to given process.
	epoint receive_point(int process, const ecurve &curve)
	{
		const gf2n &field = curve.get_field();
		lnum x = receive_lnum(process, PARALLEL_POLYNOM_POINT_X_TAG, field);
		lnum y = receive_lnum(process, PARALLEL_POLYNOM_POINT_Y_TAG, field);
		return epoint(x, y, curve);
	}
}
