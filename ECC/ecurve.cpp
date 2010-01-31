#include "ecurve.h"
#include "epoint.h"
#include "2n.h"
#include "2noperations.h"
#include "2nfactory.h"
#include "bint.h"
#include "bintoperations.h"

/* Constructors */

// Creates a new instance of ecurve with parameters a and b set to aa and bb and curves order factorization set by nf and bf.
ecurve::ecurve(const lnum &a, const lnum &b, int nf, const bfactor *bf) : a(a), b(b)
{
	int res, i;
	if ((res = lnumOperations::belong_to_same_nonzero_field(a, b)) < 0) lnumRoutines::op_err(res);
	field = &a.get_field();

	if (nf < 0) nf = 0;
	nfac = nf;
	if (nfac != 0) factor = new bfactor[nfac]; else factor = 0;
	for (i = 0; i < nfac; i++)
		factor[i] = bf[i];
}

/* Destructors */

// Frees used memory.
ecurve::~ecurve()
{
	delete [] factor;
}

/* Helper methods */

// Assignes point p to give curve
void ecurve::assign(epoint &p) const
{
	p.assign(this);
}

// Attempts to mark data array a of length n and saves result to elliptic curve point res.
// Returns true if successfull, otherwise - false.
bool ecurve::mark(unsigned int a[], int n, epoint &res) const
{
	if (n <= 0) return false;
	int dega, att = 0;
	int fieldPower = field->get_deg();
	res.x = lnum(a, n, *field); dega = res.x.deg();
	if (dega + pMarkBits >= fieldPower) return false;
	lnum c = lnum::create_nullmodule();
	lnum t = lnum::create_nullmodule();
	res.curve = this;
	//loutMode=loutBin;
	//std::cout << "mod:     " << lMod << std::endl;
	do
	{
		lnumOperations::modify(res.x, fieldPower - pMarkBits, att, pMarkBits);
		//std::cout << "marking: " << res.x << std::endl;
		c = res.curve->b;
		t = lnumOperations::sqr(res.x);
		c += res.curve->a * t;
		c += t * res.x;
		if (lnumOperations::solve_quadratic(res.x, c, res.y, t) == lE_OK) return true;
		att++;
	} while (att < (1 << pMarkBits));
	return false;
}

// Returns a random point, assigned to current curve.
epoint ecurve::random_point(void) const
{
	lnum x(*field), y(*field), x2(*field), v(*field);
	epoint G(*this);
	int fieldDegree = field->get_deg();
	bool gotSolution = false;
	while (!gotSolution)
	{
		x.random_le(fieldDegree - 1);
		x2 = lnumOperations::sqr(x);
		gotSolution = lnumOperations::solve_quadratic(x, x2 * x + a * x2 + b, y, v) == lE_OK;
		if (gotSolution) G = epoint(x, y, *this);
	}
	return G;
}

// Returns true if given point belongs to current curve, otherwise - false.
bool ecurve::belongs_to_curve(const epoint &G) const
{
	return G.curve == this;
}

// Returns true if curve has factorization of its order set, otherwise - false.
bool ecurve::has_factor(void) const
{
	return factor != 0;
}

// Returns true if curve is over field check_field, otherwise - false.
bool ecurve::is_over_field(const gf2n &check_field) const
{
	return field == &check_field;
}

/* Accessor methods */

// Returns reference to coeffitient a of this curve.
const lnum &ecurve::get_a(void) const
{
	return a;
}

// Returns reference to coeffitient b of this curve.
const lnum &ecurve::get_b(void) const
{
	return b;
}

// Returns discriminant of this curve.
const lnum ecurve::discr(void) const
{
	return b;
}

// Returns field for current elliptic curve.
const gf2n &ecurve::get_field(void) const
{
	return *field;
}

// Returns number of factors in curves order factorization
int ecurve::get_factor_length(void) const
{
	return nfac;
}

// Returns pointer to factorization array.
const bfactor *ecurve::get_factor(void) const
{
	return factor;
}

/* Packing methods */

// Unpacks a point from a given polynom.
// Returns error code, indicating result of the operation.
int ecurve::unpack(const lnum &poly, epoint &res) const
{
	bool secondSolution;
	lnum x(poly);
	const gf2n &field = x.get_field();
	if (!is_over_field(field)) return pE_DIFFFIELD;

	int fieldDegree = field.get_deg();
	secondSolution = x[fieldDegree];
	if (secondSolution) x.set_bit(fieldDegree);

	if (secondSolution && x.is_zero())
	{
		res = epoint(*this);
		return pE_OK;
	}

	lnum y(field);
	lnum c = b;
	lnum t = lnumOperations::sqr(x);
	c += a * t;
	c += t * x;
	int operationsResult;
	if (secondSolution) operationsResult = lnumOperations::solve_quadratic(x, c, t, y);
	else operationsResult = lnumOperations::solve_quadratic(x, c, y, t);
	if (operationsResult < 0) return pE_UNPACKERROR;
	res = epoint(x, y, *this);
	return pE_OK;
}

/* Order calculation methods */

// Calculates elliptic curves order and saves result to big integer res.
void ecurve::order(bint &res) const
{
	bint c0(2), c1, cr, aa;
	int i;
	int ct = 0;
	int fieldPower = field->get_deg();
	if (b.is_zero()) ct++;     // x=0; y=0;
	if (b.is_one()) ct++;      // x=0; y=1;
	if ((a + b).is_one()) ct += 2; // x=1; y=0; или x=1; y=1;
	aa = bint(2 - ct);
	c1 = aa;
	//__int64 _aa=2-ct,_c1=_aa,_c0=2,_cr;
	//std::cout << aa << std::endl << c1 << std::endl << c0 << std::endl << std::endl;
	for (i = 2;i <= fieldPower; i++)
	{
		cr = aa * c1 - c0 - c0;
		c0 = c1;
		c1 = cr;
		/*_cr=_aa*_c1-_c0-_c0;
		_c0=_c1;
		_c1=_cr;
		std::cout << "inter: " << cr << std::endl;
		std::cout << "inter: " << _cr << std::endl;*/
	}
	if (ct = bintOperations::pow(bint(2), fieldPower, res)) bintRoutines::op_err(ct);
	res += bint(1) - cr;
}

#ifdef _ECURVE_ORDER_DUMB
// Calculates elliptic curves order and saves result to big integer res.
// This is a slow routine, wich works only for very small extension powers. This routine is used for DEBUG only.
void ecurve::order_dumb(bint &res) const
{
	res.zero();
	bint one; one.one();

	int i, j;
	int fieldPower = field->get_deg();
	unsigned int t[1];

	lnum x(*field);
	lnum y(*field);
	for (i = 0; i < (1 << fieldPower); i++)
	{
		t[0] = i;
		x = lnum(t, 1, *field);
		for (j = 0; j < (1 << fieldPower); j++)
		{
			t[0] = j;
			y = lnum(t, 1, *field);
			epoint pp(x, y, this);
			if (pp.check()) res += one;
		}
	}
	res += one;
}
#endif

/* Setter methods */

void ecurve::set_factor(int nf, const bfactor *bf)
{
	int i;

	if (nf < 0) nf = 0;
	nfac = nf;
	if (nfac != 0) factor = new bfactor[nfac]; else factor = 0;
	for (i = 0; i < nfac; i++)
		factor[i] = bf[i];
}
