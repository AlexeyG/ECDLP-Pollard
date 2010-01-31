#ifndef _BINTOPERATIONS_H
#define _BINTOPERATIONS_H

#include "bintdefines.h"

/* Need some classes */
class bint;

// Predefining global external functions
namespace bintRoutines
{
	void op_err(int err);
	void swap(int &a, int &b);
	int signum(int a);
}

class bintOperations
{
	/* Constructors */
private:
	bintOperations();

private:
	/* Help routines */
	static int cmp(const bint &a, const bint &b);
	static int cmps(const bint &a, const bint &b);
	static int cmpl(const bint &a, const bint &b);

	/* Operation routines */
	static int add(const bint &a, const bint &b, bint &res);
	static int inc(bint &a, const bint &b);
	static int sub(const bint &a, const bint &b, bint &res);
	static int dec(bint &a, const bint &b);
	static int mul(const bint &a, const bint &b, bint &res);
	static int mul_short(const bint &a, int b, bint &res);
	static int mod(const bint &a, const bint &b, bint &res);
	static int div(const bint &a, const bint &b, bint &res);
	static int div_short(const bint &a, int b, bint &res);
	static int sub_cmp(const bint &a, int af, const bint &b, int bf);
	static int sub_sub(bint &a, int af, const bint &b, int bf);
	//static int inv_ring(const bint &a, const bint &m, bint &d2);

public:
	static int pow(const bint &a, int n, bint &res);
	static int inv(const bint &a, const bint &b, bint &res);

	friend class bint;
};

#endif
