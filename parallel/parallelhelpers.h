#ifndef _PARALLELHELPERS_H
#define _PARALLELHELPERS_H

#include "../ecc/ecurve.h"

// Need some classes
class ecurve;
class gf2n;
class lnum;
class bint;

namespace ParallelHelpers
{
	// Helper methods
	bool is_power_of_two(int a);
	void sleep(int miliseconds);
	int make_tag(int pattern);

	// Parallel commands
	void send_control_message(const int message, int process);
	int receive_control_message();

	// Communication commands
	void send_curve(const ecurve &curve, int process);
	void send_point(const epoint &point, int process);
	void send_field(const gf2n &field, int process);
	void send_lnum(const lnum &x, int process, int pattern);
	void send_bint(const bint &num, int process, int pattern);
	void send_bfactor(const bfactor &factor, int process, int pattern);
	ecurve *receive_curve(int process, const gf2n &field);
	gf2n *receive_field(int process);
	lnum receive_lnum(int process, int pattern);
	lnum receive_lnum(int process, int pattern, const gf2n &field);
	bint receive_bint(int process, int pattern);
	bfactor receive_bfactor(int process, int pattern);
	epoint receive_point(int process, const ecurve &curve);
}

#endif
