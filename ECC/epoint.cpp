#include "epoint.h"
#include "eccoperations.h"
#include "ecurve.h"
#include "2n.h"
#include "2noperations.h"
#include "2nfactory.h"
#include "bintoperations.h"

/* Constructors */

// Creates a new instance of epoint class (attached to no curve)
epoint::epoint(void)
{
	curve = 0;
}

// Creates a new instance of elliptic curve point by copying point p.
epoint::epoint(const epoint &p) : x(p.x), y(p.y)
{
	curve = p.curve;
}

// Creates a new instance of elliptic curve point with cordinates and belonging to curve ec.
epoint::epoint(const lnum &x, const lnum &y, const ecurve &ec) : x(x), y(y)
{
	int res;
	if ((res = lnumOperations::belong_to_same_nonzero_field(x, y)) < 0) lnumRoutines::op_err(res);
	if ((res = lnumOperations::belong_to_same_nonzero_field(x, ec.a)) < 0) lnumRoutines::op_err(res);
	curve = &ec;
}

// Creates a new instance of elliptic curve zero point, belonging to curve ec.
epoint::epoint(const ecurve &ec) : x(*ec.field), y(*ec.field)
{
	curve = &ec;
}

/* Helper methods */

// Returns true if a given point is infinity point, otherwise - false.
bool epoint::is_inf(void) const
{
	return x.is_zero() && y.is_zero();
}

// Returns true if given point belongs to it's curve, otherwise - false.
bool epoint::check(void) const
{
	lnum ch = lnumOperations::sqr(x);
	return is_inf() || (lnumOperations::sqr(y) + x * y + ch * x + curve->a * ch + curve->b).is_zero();
}

// Play a role of an "easy-calculatable" function, that is used in Pollards rho-method.
// This function returns an integer that is calculated as a result of bitwise AND operation with arguments arg and lowest x cordinates bits.
int epoint::f(int arg) const
{
	unsigned int *a = x.to_int();
	return a[0] & arg;
}

/* Accessor methods */

// Returns x cordinate of a given point.
lnum epoint::get_x(void) const
{
	return x;
}

// Sets res polynom to x cordinate of a given point.
void epoint::get_x(lnum &res) const
{
	res = x;
}

// Returns y cordinate of a given point.
lnum epoint::get_y(void) const
{
	return y;
}

// Sets res polynom to y cordinate of a given point.
void epoint::get_y(lnum &res) const
{
	res = y;
}

// Returns a constant pointer to a curve, which this point belongs to.
const ecurve &epoint::get_curve(void) const
{
	return *curve;
}

/* Setter methods */

// Sets given point to be infinity point.
void epoint::inf(void)
{
	x.zero();
	y.zero();
}

// Assigns current point to belong to elliptic curve ec.
void epoint::assign(const ecurve *ec)
{
	curve = ec;
}

/* Helper methods */

/*void cycle_sum(const epoint &what, bint times, epoint &res)
{
	res.inf();
	bint one; one.one();
	std::cout << "Need to calculate: " << times << std::endl;
	while (!times.is_zero())
	{
		if (times == bint(53560))
		{
			std::cout << "we're here!" << std::endl;
		}
		res += what;
		if (!res.check())
		{
			std::cout << "alex sucks" << std::endl;
			std::cout << times << std::endl;
		}
		times -= one;
	}
}*/

// Calculates elliptic curve points order and saves it to big integer res.
void epoint::order(bint &res) const
{
	if (curve == 0) eccRoutines::op_err(pE_UNASSIGNED);
	curve->order(res);
	//std::cout << "ord: " << res << std::endl;
	//curve->order_dumb(res);
	int i;
	bint t;
	epoint p(*curve), p2(*curve);
	for (i = 0; i < curve->nfac; i++)
	{
		bintOperations::pow(curve->factor[i].p, curve->factor[i].k, t);
		res = res / t;
		//std::cout << "res: " << res << std::endl;
		eccOperations::mul(*this, res, p);
		//cycle_sum(*this, res, p);
		while (!p.is_inf())
		{
			eccOperations::mul(p, curve->factor[i].p, p2); p = p2;
			//cycle_sum(p, curve->factor[i].p, p2);
			p = p2;
			res = res * curve->factor[i].p;
		}
	}
}

/* Packing methods */

// Returns info required for packing elliptic curve points to polynoms.
// This operation can return an error code or numbers 1 or 2, indicating if this points y cordinate is first or second solution of the appropriate equation respectively.
int epoint::pack_info(void) const
{
	if (is_inf()) return ppE_INF;
	lnum c = lnumOperations::sqr(x);
	lnum u(x.get_field()), v(x.get_field());
	c = c * x + curve->a * c + curve->b;
	if (lnumOperations::solve_quadratic(x, c, u, v) != lE_OK) return ppE_NOSOL;
	if (u == y) return ppE_1ST;
	if (v == y) return ppE_2ND;
	return ppE_INTERNAL;
}

// Packs point into a polynom.
// Returns error code, indicating result of the operation.
int epoint::pack(lnum &res) const
{
	res = x;
	int r = pack_info();
	if (r < 0 && r != ppE_INF) return pE_PACKERROR;
	int fieldDegree = x.get_field().get_deg();
	if (r == ppE_INF || r == ppE_2ND) res.set_bit(fieldDegree);
	return pE_OK;
}

/* Data marking methods */

// Unmarks data from current elliptic point and saves in to polynom res.
void epoint::unmark(lnum &res) const
{
	if (is_inf()) eccRoutines::op_err(pE_INF);
	res = x;
	lnumOperations::modify(res, x.get_field().get_deg() - pMarkBits, 0, pMarkBits);
}

// Unmarks data from current elliptic point and returns result.
lnum epoint::unmark(void) const
{
	if (is_inf()) eccRoutines::op_err(pE_INF);
	lnum res = lnum::create_nullmodule();
	unmark(res);
	return res;
}

/* Operators */

// Sets an instance of point class.
epoint& epoint::operator= (const epoint &p)
{
	x = p.x;
	y = p.y;
	curve = p.curve;
	return *this;
}

// Returns negated elliptic curve point.
epoint epoint::operator- (void) const
{
	epoint res(*curve);
	eccOperations::inv(*this, res);
	return res;
}

// Returns true if two elliptic curve points are equal, otherwise - false.
bool epoint::operator== (const epoint &q) const
{
	return this->x == q.x && this->y == q.y;
}

// Returns true if two elliptic curve points are NOT equal, otherwise - false.
bool epoint::operator!= (const epoint &q) const
{
	return this->x != q.x || this->y != q.y;
}

// Returns sum of two elliptic curve points.
epoint epoint::operator+ (const epoint &q) const
{
	int r;
	epoint res(*curve);
	if (r = eccOperations::sum(*this, q, res)) eccRoutines::op_err(r);
	return res;
}

// Returns difference of two elliptic curve points.
epoint epoint::operator- (const epoint &q) const
{
	int r;
	epoint res(*curve), qinv(*curve);
	eccOperations::inv(q, qinv);
	if (r = eccOperations::sum(*this, qinv, res)) eccRoutines::op_err(r);
	return res;
}

// Increments current elliptic curve point by a given point.
void epoint::operator+= (const epoint &q)
{
	int r;
	epoint res(*curve);
	if (r = eccOperations::sum(*this, q, res)) eccRoutines::op_err(r);
	*this = res;
}

// Decrements current elliptic curve point by a given point.
void epoint::operator-= (const epoint &q)
{
	int r;
	epoint res(*curve), qinv(*curve);
	eccOperations::inv(q, qinv);
	if (r = eccOperations::sum(*this, qinv, res)) eccRoutines::op_err(r);
	*this = res;
}
