#ifndef _ECURVE_H
#define _ECURVE_H

#include "eccdefines.h"
#include "2n.h"
#include "bint.h"

/* Need some classes */
class lnum;
class gf2n;
class bint;
class epoint;
class eccOperations;

// Struct used to store factorization.
typedef struct
{
	bint p; // the factor itself
	int k;  // factors power
} bfactor;

class ecurve
{
	/* Constructors */
public:
	ecurve(const lnum &a, const lnum &b, int nf = 0, const bfactor *bf = 0);

	/* Destructors */
	~ecurve();

	/* Helper methods */
	void assign(epoint &p) const;
	bool mark(unsigned int a[], int n, epoint &res) const;
	epoint random_point(void) const;
	bool belongs_to_curve(const epoint &G) const;
	bool has_factor(void) const;
	bool is_over_field(const gf2n &check_field) const;

	/* Accessor methods */
	const lnum &get_a(void) const;
	const lnum &get_b(void) const;
	const lnum discr(void) const;
	const gf2n &get_field(void) const;
	int get_factor_length(void) const;
	const bfactor *get_factor(void) const;

	/* Packing methods */
	int unpack(const lnum &poly, epoint &res) const;

	/* Order calculation methods */
	void order(bint &res) const;
#ifdef _ECURVE_ORDER_DUMB
	void order_dumb(bint &res) const;
#endif

	/* Setter methods */
	void set_factor(int nf, const bfactor *bf);

private:
	lnum a, b;
	const gf2n *field;

	bfactor *factor;
	int nfac;

	friend class epoint;
	friend class eccOperations;
	// WTF is this anyway??
	//friend int mark(epoint &p, epoint &q, epoint &res);
};
#endif

// F(X,Y) = Y^2 + X * Y + X^3 + a * X^2 + b = 0
