#ifndef _CRACK_H
#define _CRACK_H

#include "../ecc/epoint.h"
#include "../ecc/bint.h"

/* Need some classes */
class ecurve;
class epoint;
class bint;

typedef struct
{
	bint p;   // factor itself
	bint sol; // part of Pollig-Hellman algorithm - solution of ECDLP for this prime-power subgroup
	int k;    // factors power
} pofactor;

namespace crackRoutines
{
	void op_err(int err);
	bint chinese_remainder_theorem(const pofactor *factors, int n, const bint &N);
	pofactor *calculate_point_order_factorization(const epoint &point, int &factor_count);
}

class crack
{
public:
	/* Constructors */
	crack(const ecurve &curve);
	crack(const ecurve &curve, const epoint &G, const epoint &xG);

	/* Setter methods */
	void set_G(const epoint &G);
	void set_xG(const epoint &xG);

	/* Accessor methods */
	const epoint &get_G(void);
	const epoint &get_xG(void);

	/* Helper methods */
	bool is_running(void);

	/* Control methods */
	bool solve(bint &result, bool verbose, char *offset, double &work_time);

	/* Solution methods */
	static bool pollard(const epoint &P, const epoint &Q, const bint &order, bint &result, int &iterations, double &work_time);
	static bool bruteforce(const epoint &P, const epoint &Q, const bint &order, bint &result, int &iterations, double &work_time);

private:

	bool running;

	const ecurve *curve;
	epoint G, xG;
};
#endif
