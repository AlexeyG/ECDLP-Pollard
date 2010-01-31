#ifndef _2NOPERATIONS_H
#define _2NOPERATIONS_H

#include <ostream>
#include <iostream>
#include "2ndefines.h"

/* Need some classes */
class lnum;
class gf2n;

// Predefining global external functions
namespace lnumRoutines
{
	void op_err(int err);
	void lpBin_int(std::ostream& s, unsigned int a);
	void lpHex_int(std::ostream& s, unsigned int a);
	unsigned char bitcnt_int(unsigned int a);
	unsigned short shift_mul(unsigned char a, unsigned char b);
	char lHp(unsigned int a);
	void int_mul(unsigned int a, unsigned int b, unsigned int &r1, unsigned int &r2);
	void fill_mult_table(void);
}

class lnumOperations
{
	/* Constructors */
private:
	lnumOperations();

private:
	/* Help routines */
	static int cmp(const lnum &a, const lnum &b);
	static void lrandom(lnum &a, int l);
	static void lswap(lnum &a, lnum &b);

	/* Operation routines */
	static int xor(const lnum &a, const lnum &b, lnum &res);
	static int xor(lnum &a, const lnum &b);
	static int inv(const lnum &a, const lnum &b, lnum &d2);
	static int inv_ex(const lnum &oper, const lnum &a, const lnum &b, lnum &d2);
	static int inv_ex(lnum &d2, const lnum &a, const lnum &b);
	static int mul(const lnum &a, const lnum &b, lnum &res);
	static int kmul(const lnum &a, const lnum &b, lnum &res);
	static int shl(const lnum &a, int count, lnum &res);
	static int shr(const lnum &a, int count, lnum &res);
	static int sqr(const lnum &a, lnum &res);
	static int scalar(const lnum &a, const lnum &b, lnum &res);
	static int scalar(lnum &a, const lnum &b);
	static int bit_neg(const lnum &a, lnum &res);
	static int sqrt(const lnum &a, lnum &res);
	static int mods(const lnum &a, const lnum &b, const int L[], int lN, lnum &res);
	static int modn(const lnum &a, const lnum &b, lnum &res);
	static int mod(const lnum &a, lnum &res);

public:
	/* Alrorithms */
	static int solve_quadratic_red(const lnum &a, lnum &res);
	static int solve_quadratic(const lnum &b, const lnum &c, lnum &res, lnum &res2);

	/* Conditions */
	static int belong_to_same_nonzero_field(const lnum &a, const lnum &b);

	/* Printing routines */
	static void lpBin(std::ostream &s, const lnum &a);
	static void lpHex(std::ostream &s, const lnum &a);

	/* Modification routines */
	static void modify(lnum &a, int pos, unsigned int what, int count);

	/* Operation routines */
	static lnum sqr(const lnum &a);

	friend class lnum;
	friend class gf2n;
};
#endif
