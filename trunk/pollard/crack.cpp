#include "crack.h"
#include "crackdefines.h"
#include <iostream>
#include <ctime>
#include "../ecc/ecurve.h"
#include "../ecc/eccoperations.h"
#include "../ecc/bint.h"
#include "../ecc/bintoperations.h"

namespace crackRoutines
{
	// Outputs error information to stderr and halts application execution. Takes error code as a parameter.
	void op_err(int err)
	{
		switch (err)
		{
		case ckE_OK : return;
			break;
		case ckE_DIFFCURVES : std::cerr << "(crack) Exception: invalid operation on point, assigned to different curve.";
			break;
		default:
			std::cerr << "(crack) Exception: unknown error.";
		}
		abort();
	}

	// Solves a system of modular equations (from Pollig-Hellman algorithms) and returns it.
	// Arguments: factors is a pointer to array of pofactor sturctures, n is the length of this array and N is point G order.
	bint chinese_remainder_theorem(const pofactor *factors, int n, const bint &N)
	{
		int i, r;
		bint res;
		if (n == 0) return res;
		bint *m = new bint[n];
		bint u, v;
		for (i = 0; i < n; i++)
			if ((r = bintOperations::pow(factors[i].p, factors[i].k, m[i])) != bE_OK) bintRoutines::op_err(r);
		for (i = 0; i < n; i++)
		{
			u = N / m[i];
			//std::cout << "Going to invert: " << u % m[i] << std::endl;
			//std::cout << "over: " << m[i] << std::endl;
			if ((r = bintOperations::inv(u % m[i], m[i], v)) != bE_OK) bintRoutines::op_err(r);
			if (v.is_less_zero()) v += m[i];
			res = (res + ((factors[i].sol * v) % m[i]) * u) % N;
		}
		delete [] m;
		return res;
	}

	// Calculates curves point order factorization and returns it.
	// Returns null pointer in case of an error.
	pofactor *calculate_point_order_factorization(const epoint &point, int &factor_count)
	{
		int i;
		const ecurve &curve = point.get_curve();
		if (!curve.has_factor()) return 0;
		pofactor *result;
		bint order;
		point.order(order);
		factor_count = 0;
		const bfactor *factors = curve.get_factor();
		int curve_factors_count = curve.get_factor_length();
		for (i = 0; i < curve_factors_count; i++)
			if ((order % factors[i].p).is_zero()) factor_count++;
		if (factor_count == 0) return 0;
		result = new pofactor[factor_count];
		factor_count = 0;
		for (i = 0; i < curve_factors_count; i++)
		{
			if ((order % factors[i].p).is_zero())
			{
				result[factor_count].p = factors[i].p;
				result[factor_count].k = 1;
				order = order / factors[i].p;
				while ((order % factors[i].p).is_zero())
				{
					order = order / factors[i].p;
					result[factor_count].k++;
				}
				factor_count++;
			}
		}
		return result;
	}
}

/* Constructors */

// Creates a new instance of crack class, that will crack a cipher over a given elliptic curve.
// Points G, aG and bG must be set separately.
crack::crack(const ecurve &curve)
{
	this->curve = &curve;
	running = false;
}

// Creates a new instance of crack class, that will crack a cipher over a given elliptic curve.
// Takes crack data as arguments.
crack::crack(const ecurve &curve, const epoint &G, const epoint &xG) : G(G), xG(xG)
{
	this->curve = &curve;
	running = false;
	if (!curve.belongs_to_curve(G) || !curve.belongs_to_curve(xG))
		crackRoutines::op_err(ckE_DIFFCURVES);
}

/* Setter methods */

// Sets elliptic curve point G from ECDLP problem.
void crack::set_G(const epoint &G)
{
	if (running)
		crackRoutines::op_err(ckE_RUNNING);
	if (!curve->belongs_to_curve(G)) crackRoutines::op_err(ckE_DIFFCURVES);
	this->G = G;
}

// Sets elliptic curve point xG from ECDLP problem.
void crack::set_xG(const epoint &xG)
{
	if (running)
		crackRoutines::op_err(ckE_RUNNING);
	if (!curve->belongs_to_curve(xG)) crackRoutines::op_err(ckE_DIFFCURVES);
	this->xG = xG;
}

/* Accessor methods */

// Returns elliptic curve point G from Pollig-Hellman key exchange algorithms.
const epoint &crack::get_G(void)
{
	return G;
}

// Returns elliptic curve point xG from ECDLP problem.
const epoint &crack::get_xG(void)
{
	return xG;
}

/* Helper methods */

// Returns true if ECDLP solver is already working, otherwise - false.
bool crack::is_running(void)
{
	return running;
}

/* Control methods */

bool crack::solve(bint &result, bool verbose, char *offset, double &work_time)
{
	work_time = clock();
	if (running) crackRoutines::op_err(ckE_RUNNING);
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
				int iterations = -1;
				double work_time = 10;
				solved = pollard(P, Q, factors[i].p, temp_solution, iterations, work_time);
				if (verbose) sprintf(time_string, PRINT_FORMAT, work_time);
				if (!solved)
				{
					if (verbose) std::cout << offset << "        [-] (" << attempt + 1 << ") Fail: " << iterations << " iterations (" << time_string << " seconds)" << std::endl;
				}
				else
					if (verbose) std::cout << offset << "        [+] (" << attempt + 1 << ") Success: " << iterations << " iterations (" << time_string << " seconds)" << std::endl;
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

// Brutforce ECDLP solver.
bool crack::bruteforce(const epoint &P, const epoint &Q, const bint &order, bint &result, int &iterations, double &work_time)
{
	const ecurve &curve = P.get_curve();
	if (!curve.belongs_to_curve(Q)) return false;
	work_time = clock();
	bint i;
	bint one(1);
	epoint R(P.get_curve());
	R.inf();
	iterations = 0;
	while (i < order)
	{
		if (R == Q)
		{
			result = i;
			work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;
			return true;
		}
		iterations++;
		i += one;
		R += P;
	}
	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;
	return false;
}

// Pollards rho-method ECDLP solver.
bool crack::pollard(const epoint &P, const epoint &Q, const bint &order, bint &result, int &iterations, double &work_time)
{
	int i, j;
	iterations = 0;
	const ecurve &curve = P.get_curve();
	if (!curve.belongs_to_curve(Q)) return false;
	if (order < POLLARD_MIN_ORDER) return bruteforce(P, Q, order, result, iterations, work_time);

	epoint tempPoint(curve), X1(curve), X2(curve);
	bint c1, d1, c2, d2;
	bint *a = new bint[POLLARD_SET_COUNT];
	bint *b = new bint[POLLARD_SET_COUNT];
	epoint *R = new epoint[POLLARD_SET_COUNT];

	for (i = 0; i < POLLARD_SET_COUNT; i++)
	{
		a[i].random(); a[i] = a[i] % order;
		b[i].random(); b[i] = b[i] % order;
		eccOperations::mul(P, a[i], R[i]);
		eccOperations::mul(Q, b[i], tempPoint);
		R[i] += tempPoint;
	}

	c1.random(); c1 = c1 % order;
	d1.random(); d1 = d1 % order;
	eccOperations::mul(P, c1, X1);
	eccOperations::mul(Q, d1, tempPoint);
	X1 += tempPoint;
	c2 = c1; d2 = d1; X2 = X1;
	work_time = clock();
	do
	{
		j = X1.f(POLLARD_SET_ARG);
		X1 += R[j];
		c1 += a[j]; if (c1 >= order) c1 -= order;
		d1 += b[j]; if (d1 >= order) d1 -= order;
		for (i = 0; i < 2; i++)
		{
			j = X2.f(POLLARD_SET_ARG);
			X2 += R[j];
			c2 += a[j]; if (c2 >= order) c2 -= order;
			d2 += b[j]; if (d2 >= order) d2 -= order;
		}
		iterations++;
	} while (X1 != X2);
	work_time = (clock() - work_time) / (double)CLOCKS_PER_SEC;

	delete[] a;
	delete[] b;
	delete[] R;

	c1 -= c2; if (c1.is_less_zero()) c1 += order;
	d2 -= d1; if (d2.is_less_zero()) d2 += order;
	if (d2.is_zero()) return false;
	bintOperations::inv(d2, order, d1);
	if (d1.is_less_zero()) d1 += order;
	result = (c1 * d1) % order;
	return true;
}
