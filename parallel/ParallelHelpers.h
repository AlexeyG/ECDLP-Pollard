#ifndef _PARALLELHELPERS_H
#define _PARALLELHELPERS_H

#include "ParallelMaster.h"
#include "ParallelIdentity.h"
#include "../ecc/ecurve.h"

// Need some classes
class ecurve;
class gf2n;
class lnum;
class bint;

namespace ParallelHelpers
{
	extern const ParallelIdentity *identity;

	// Helper methods
	void initialize(const ParallelIdentity *identity);
	void finalize(void);
	bool is_power_of_two(int a);
	void sleep(int miliseconds);
	int make_tag(void);
	int extract_tag(int tag);

	// Parallel commands
	bool have_incoming_message(int pattern);
	MPI_Request send_control_message(int message, int process);
	int receive_control_message();
	int extract_and_send_tag(int process, int pattern);
	int extract_and_receive_tag(int process, int pattern);

	// Communication commands
	void send_ecurve(const ecurve &curve, int process, int pattern);
	void send_epoint(const epoint &point, int process, int pattern);
	void send_gf2n(const gf2n &field, int process, int pattern);
	void send_lnum(const lnum &x, int process, int pattern);
	void send_bint(const bint &num, int process, int pattern);
	void send_bfactor(const bfactor &factor, int process, int pattern);
	void send_ParallelData(int instance, const lnum &packedPoint, const bint &coefC, const bint &coefD, int process, int pattern);
	ecurve *receive_ecurve(int process, const gf2n &field, int pattern);
	gf2n *receive_gf2n(int process, int pattern);
	lnum receive_lnum(int process, int pattern);
	lnum receive_lnum(int process, const gf2n &field, int pattern);
	bint receive_bint(int process, int pattern);
	bfactor receive_bfactor(int process, int pattern);
	epoint receive_epoint(int process, const ecurve &curve, int pattern);
	ParallelData *receive_ParallelData(int process, const gf2n &field, int pattern);
}

#endif
