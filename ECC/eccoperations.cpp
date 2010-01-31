#include "eccoperations.h"
#include "epoint.h"
#include "ecurve.h"
#include "2n.h"
#include "2noperations.h"
#include "bint.h"
#include "bintoperations.h"

namespace eccRoutines
{
	// Outputs error information to stderr and halts application execution. Takes error code as a parameter.
	void op_err(int err)
	{
		switch (err)
		{
		case pE_OK : return;
			break;
		case pE_DIFFCURVES : std::cerr << "(ecc) Exception: performing operations on points, belonging to different curves." << std::endl;
			break;
		case pE_UNASSIGNED : std::cerr << "(ecc) Exception: performing operations on unassigned points." << std::endl;
			break;
		case pE_INF : std::cerr << "(ecc) Exception: invalid operation on infinity point." << std::endl;
			break;
		case pE_PACKERROR : std::cerr << "(ecc) Exception: error occured while packing a point." << std::endl;
			break;
		case pE_UNPACKERROR : std::cerr << "(ecc) Exception: unable to solve internal equation, when unpacking point." << std::endl;
			break;
		case pE_DIFFFIELD : std::cerr << "(ecc) Exception: invalid operation on polyom, owned by another field (other than our curves field)." << std::endl;
			break;
		default:
			std::cerr << "(ecc) Exception: unknown error." << std::endl;
		}
		abort();
	}
}

// Inverses elliptic point p and saves result to elliptic point res.
// Returns error code, indication completion result.
int eccOperations::inv(const epoint &p, epoint &res)
{
	res = p;
	if (!p.is_inf()) res.y += p.x;
	return pE_OK;
}

// Calculates sum of two elliptic points and saves result to point res.
// Returns error code, indicating completion result.
int eccOperations::sum(const epoint &p, const epoint &q, epoint &res)
{
	if (p.curve == 0 || q.curve == 0) return pE_UNASSIGNED;
	if (p.curve != q.curve) return pE_DIFFCURVES;
	const gf2n &ourField = p.curve->get_field();
	lnum lambda(ourField);
	lnum x3(ourField);
	res.curve = p.curve;
	if (p.is_inf())
	{
		res = q;
		return pE_OK;
	}
	if (q.is_inf())
	{
		res = p;
		return pE_OK;
	}
	if (q == -p)
	{
		res.inf();
		return pE_OK;
	}
	if (p.x != q.x)
	{
		lambda = (q.y - p.y) / (q.x - p.x);
		x3 = lnumOperations::sqr(lambda) + lambda + p.curve->a + p.x + q.x;
		res.x = x3;
		res.y = x3 + p.y + lambda * (x3 + p.x);
		return pE_OK;
	}
	else
	{
		lambda = p.x + p.y / p.x;
		x3 = lnumOperations::sqr(lambda) + lambda + p.curve->a;
		lambda.set_bit(0); // lambda = lambda + 1
		res.x = x3;
		res.y = lnumOperations::sqr(p.x) + lambda * x3;
		return pE_OK;
	}
	return pE_OK;
}

// Calculates k * p ans saves result to res.
// Returs error code, indicating result of the operation.
int eccOperations::mul(const epoint &p, int k, epoint &res)
{
	int i;
	res.inf();
	res.curve = p.curve;
	if (k < 0) k = -k;
	if (!k) return pE_OK;
	for (i = 31; i >= 0; i--)
	{
		res += res;
		if (k & (1 << i)) res += p;
	}
	return pE_OK;
}

// Calculates k * p ans saves result to res.
// Returs error code, indicating result of the operation.
int eccOperations::mul(const epoint &p, bint k, epoint &res)
{
	//int i,j;
	res.inf();
	res.curve = p.curve;
	if (k.is_less_zero()) k = -k;
	if (k.is_zero()) return pE_OK;
	bint one; one.one();
	epoint p2 = p;
	while (!k.is_zero())
		if (!k.is_odd())
		{
			p2 += p2;
			k = k / 2;
		}
		else
		{
			res += p2;
			k -= one;
		}
	return pE_OK;
}
