#include "2noperations.h"
#include "2n.h"
#include "2nfactory.h"

namespace lnumRoutines
{
	// Outputs error information to stderr and halts application execution. Takes error code as a parameter.
	void op_err(int err)
	{
		switch (err)
		{
		case lE_OK : return;
			break;
		case lE_OVERFLOW : std::cerr << "(lnum) Exception: polynom degree overflow." << std::endl;
			break;
		case lE_DIVZERO : std::cerr << "(lnum) Exception: division by zero." << std::endl;
			break;
		case lE_NOINVERSE : std::cerr << "(lnum) Exception: no inverse exists." << std::endl;
			break;
		case lE_ACC : std::cerr << "(lnum) Exception: access violation." << std::endl;
			break;
		case lE_IO : std::cerr << "(lnum) Exception: IO error." << std::endl;
			break;
		case lE_NULLFIELD : std::cerr << "(lnum) Exception: invalid operation on polynom without a field." << std::endl;
			break;
		case lE_DIFFFIELD : std::cerr << "(lnum) Exception: invalid operation on polynoms belonging to different fields." << std::endl;
			break;
		case lE_NOSOLUTION : std::cerr << "(lnum) Exception: equation has no solution." << std::endl;
			break;
		default:
			std::cerr << "(lnum) Exception: unknown error." << std::endl;
		}
		abort();
	}

	// Outputs integer a to stream s as a sequence of bits
	void lpBin_int(std::ostream& s, unsigned int a)
	{
		int i;
		for (i = 0; i < lbLen; i++)
		{
			if (a & 1) s << 1; else s << 0;
			a >>= 1;
		}
	}

	// Outputs integer a to stream s as a sequence of hex charters.
	void lpHex_int(std::ostream& s, unsigned int a)
	{
		int i,t;
		char ch;
		for (i = 0; i < lhLen; i++ , a >>= lbHex)
		{
			t=a & 0xF;
			if (t <= 9) ch='0' + t; else ch = 'A' + t - 10;
			s << ch;
		}
	}

	// Returns a number of set bits (bits equal to 1) in integer a.
	unsigned char bitcnt_int(unsigned int a)
	{
		unsigned char r=0;
		while (a)
		{
			if (a & 1) r++;
			a>>=1;
		}
		return r;
	}

	// Returns a product of 2 polynoms a and b of degree no more than 7. Uses shifting multiplication method.
	unsigned short shift_mul(unsigned char a, unsigned char b)
	{
		union
		{
			unsigned short r;
			unsigned char  r1[2];
		} res;
		res.r=0;
		int i;
		for (i=0;i<8;i++)
		{
			if (b & 1) res.r1[1]^=a;
			res.r>>=1;
			b>>=1;
		}
		return res.r;
	}

	// Returns the number of the most significant non-zero bit in integer a.
	char lHp(unsigned int a)
	{
		char b=-1;
		while (a)
		{
			b++;
			a>>=1;
		}
		return b;
	}

	// Multiplys two polynoms of degree less or equal to 31 using Karacuba decomposition scheme and returns result as polynoms r1 and r2.
	void int_mul(unsigned int a, unsigned int b, unsigned int &r1, unsigned int &r2)
	{
		union
		{
			unsigned short r;
			unsigned char  r1[2];
		} O[4],T[2][2],F;
		union
		{
			unsigned int   a;
			unsigned char  a1[4];
		} A,B;
		union
		{
			unsigned int    a[2];
			unsigned char  a1[8];
		} P;
		unsigned char D[6],S[6];
		A.a=a;
		B.a=b;
		O[0].r=Tm[A.a1[0]][B.a1[0]];
		O[1].r=Tm[A.a1[1]][B.a1[1]];
		O[2].r=Tm[A.a1[2]][B.a1[2]];
		O[3].r=Tm[A.a1[3]][B.a1[3]];
		T[0][0].r=Tm[A.a1[0] ^ A.a1[1]][B.a1[0] ^ B.a1[1]];
		T[1][0].r=Tm[A.a1[2] ^ A.a1[3]][B.a1[2] ^ B.a1[3]];
		T[0][1].r=Tm[A.a1[0] ^ A.a1[2]][B.a1[0] ^ B.a1[2]];
		T[1][1].r=Tm[A.a1[1] ^ A.a1[3]][B.a1[1] ^ B.a1[3]];
		F.r=Tm[A.a1[0] ^ A.a1[1] ^ A.a1[2] ^ A.a1[3]][B.a1[0] ^ B.a1[1] ^ B.a1[2] ^ B.a1[3]];
		D[0]=O[1].r1[0] ^ O[0].r1[1];
		D[1]=O[2].r1[0] ^ O[1].r1[1];
		D[2]=O[3].r1[0] ^ O[2].r1[1];
		D[3]=O[0].r1[0] ^ O[3].r1[1];
		D[4]=O[1].r1[0] ^ O[0].r1[1];
		D[5]=O[2].r1[0] ^ O[1].r1[1];
		S[0]=O[0].r1[0] ^ D[0];
		S[1]=S[0] ^ D[1];
		S[2]=S[1] ^ D[2];
		S[3]=S[2] ^ D[3];
		S[4]=S[3] ^ D[4];
		S[5]=S[4] ^ D[5];
		P.a1[0]=O[0].r1[0];
		P.a1[1]=S[0] ^ T[0][0].r1[0];
		P.a1[2]=S[1] ^ T[0][0].r1[1] ^ T[0][1].r1[0];
		P.a1[3]=S[2] ^ T[0][0].r1[0] ^ T[1][0].r1[0] ^ T[0][1].r1[0] ^ T[0][1].r1[1] ^ T[1][1].r1[0] ^ F.r1[0];
		P.a1[4]=S[3] ^ T[0][0].r1[1] ^ T[1][0].r1[1] ^ T[0][1].r1[1] ^ T[1][1].r1[0] ^ T[1][1].r1[1] ^ F.r1[1];
		P.a1[5]=S[4] ^ T[1][0].r1[0] ^ T[1][1].r1[1];
		P.a1[6]=S[5] ^ T[1][0].r1[1];
		P.a1[7]=O[3].r1[1];
		r1=P.a[0];
		r2=P.a[1];
	}

	// Calculates multiplication and squaring tables. Used only when creating these tables.
	void fill_mult_table(void)
	{
#ifndef lHEADER_TABLES
		int i,j;
#endif
#ifdef lGEN_MULT_TABLE
		for (i=0;i<(1 << 8);i++)
		{
			Tm[0][i]=Tm[i][0]=0;
			Tm[i][i]=shift_mul((unsigned char)i,(unsigned char)i);
		}
		for (i=1;i<(1 << 8)-1;i++)
			for (j=i+1;j<(1 << 8);j++) Tm[i][j]=Tm[j][i]=shift_mul((unsigned char)i,(unsigned char)j);
#endif
#ifdef lGEN_SQR_TABLE
		unsigned int t;
		for (i=0;i<(1 << 16);i++) int_mul((unsigned int)i,(unsigned int)i,Ts[i],t);
#endif
	}

	// Swaps two integers.
	void swap(int &a, int &b)
	{
		int t = a; a = b; b = t;
	}
}

/* Constructors */

lnumOperations::lnumOperations()
{
}

/* Help routines */

// Returns an integer < 0 if a < b, an integer > 0 if a > b and 0 if a = b.
int lnumOperations::cmp(const lnum &a, const lnum &b)
{
	int da,db,i;
	da = a.deg(); db = b.deg();
	if (da > db) return 1;
	if (da < db) return -1;
	for (i = 0; i <= a.l; i++)
		if (da = a.p.a[i] - b.p.a[i]) return da;
	return 0;
}

// Generates a random polynom parts upto index l.
void lnumOperations::lrandom(lnum &a, int l)
{
	int i, t;
	if (l < 0) return;
	a.fill_zero(0, a.l + 1);
	a.l = l;
	for (i = 0; i <= l; i++)
	{
		a.p.a2[2 * i] = rand();
		t = rand() % 3;
		if (t == 1) a.p.a2[2 * i] *= 2;
		else if (t == 2) a.p.a2[2 * i] = a.p.a2[2 * i] * 2 + 1;
		/************************************************/
		a.p.a2[2 * i + 1] = rand();
		t = rand() % 3;
		if (t == 1) a.p.a2[2 * i + 1] *= 2;
		else if (t == 2) a.p.a2[2 * i + 1] = a.p.a2[2 * i + 1] * 2 + 1;
	}
	a.fix_deg();
}

/* Operation routines */

// Saves sum of polynom a and b to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::xor(const lnum &a, const lnum &b, lnum &res)
{
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	int i;
	res.l = max(a.l, b.l);
	res.fill_zero(res.l + 1, lLen);
	for (i = 0; i <= res.l; i++) res.p.a[i] = a.p.a[i] ^ b.p.a[i];
	res.fix_deg();
	return lE_OK;
}

// Saves sum of polynom a and b to polynom a.
// Returns one of the error codes stating the result of operation.
int lnumOperations::xor(lnum &a, const lnum &b)
{
	if (a.field != b.field) return lE_DIFFFIELD;
	int i;
	a.l = max(a.l, b.l);
	for (i = 0; i <= a.l; i++) a.p.a[i] ^= b.p.a[i];
	a.fix_deg();
	return lE_OK;
}

// Saves inverse of polynom a modulo polynom b to polynom d2.
// Returns one of the error codes stating the result of operation.
int lnumOperations::inv(const lnum &a, const lnum &b, lnum &d2)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != d2.field) return lE_DIFFFIELD;
	if (b.is_zero()) return lE_DIVZERO;
	lnum d(*a.field), d1(*a.field);
	lnum u = b, v = a, r(*a.field);
	d1.set_bit(0); d2.zero();
	while (!v.is_zero())
	{
		int du = u.deg(), dv = v.deg();
		if (du < dv)
		{
			lswap(u, v);
			lswap(d1, d2);
			lnumRoutines::swap(du, dv);
		}
		shl(v, du - dv, r);  xor(r,u);  // no exception can occur here, because (du-dv) < lLen*lbLen
		shl(d1, du - dv, d); xor(d,d2);
		u = v; v = r;
		d2 = d1; d1 = d;
	}
	if (!u.is_one()) return lE_NOINVERSE;
	return lE_OK;
}

// Saves product of inverse of polynom a modulo polynom b and polynom oper to polynom d2.
// Returns one of the error codes stating the result of operation.
int lnumOperations::inv_ex(const lnum &oper, const lnum &a, const lnum &b, lnum &d2)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != oper.field) return lE_DIFFFIELD;
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != d2.field) return lE_DIFFFIELD;
	if (b.is_zero()) return lE_DIVZERO;
	lnum d(*a.field), d1(*a.field);
	lnum u = b, v = a, r(*a.field);
	d1 = oper; d2.zero();
	while (!v.is_zero())
	{
		int du = u.deg(), dv = v.deg();
		if (du < dv)
		{
			lswap(u, v);
			lswap(d1, d2);
			lnumRoutines::swap(du, dv);
		}
		shl(v, du - dv, r);  xor(r, u);  // no exception can occur here, because (du - dv) < lLen * lbLen
		shl(d1, du - dv, d); xor(d, d2);
		u = v; v = r;
		d2 = d1; d1 = d;
	}
	if (!u.is_one()) return lE_NOINVERSE;
	return lE_OK;
}

// Saves product of inverse of polynom a modulo polynom b and polynom d2 to polynom d2.
// Returns one of the error codes stating the result of operation.
int lnumOperations::inv_ex(lnum &d2, const lnum &a, const lnum &b)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != d2.field) return lE_DIFFFIELD;
	if (b.is_zero()) return lE_DIVZERO;
	lnum d(*a.field), d1(*a.field);
	lnum u = b, v = a, r(*a.field);
	d1 = d2; d2.zero();
	while (!v.is_zero())
	{
		int du = u.deg(), dv = v.deg();
		if (du < dv)
		{
			lswap(u, v);
			lswap(d1, d2);
			lnumRoutines::swap(du, dv);
		}
		shl(v, du - dv, r);  xor(r, u);  // no exception can occur here, because (du-dv) < lLen*lbLen
		shl(d1, du - dv, d); xor(d, d2);
		u = v; v = r;
		d2 = d1; d1 = d;
	}
	if (!u.is_one()) return lE_NOINVERSE;
	return lE_OK;
}

// Saves product of polynoms a and b to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::mul(const lnum &a, const lnum &b, lnum &res)
{
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;

	res.fill_zero();
	res.l = a.l + b.l + 1;
	if (res.l >= lLen) return lE_OVERFLOW;
	unsigned int r1, r2;
	int i, j;
	for (i = 0; i <= a.l; i++)
		for (j = 0; j <= b.l; j++)
		{
			lnumRoutines::int_mul(a.p.a[i], b.p.a[j], r1, r2);
			res.p.a[i + j] ^= r1;
			res.p.a[i + j + 1] ^= r2;
		}
	res.fix_deg();
	return lE_OK;
}

// Karacuba multiplication: stores product of polynoms a and b in polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::kmul(const lnum &a, const lnum &b, lnum &res)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	lnum a1(*a.field), b1(*a.field), a2(*a.field), b2(*a.field), r1(*a.field), r2(*a.field), r3(*a.field);
	int l , i;
	l = (max(a.l, b.l) + 1) / 2;
	if (l <= lMaxkmul) return mul(a, b, res);
	else
	{
		for (i = l; i < a.l; i++) a1.p.a[i - l] = a.p.a[i];
		a1.l = a.l - l;
		for (i = l; i < b.l; i++) b1.p.a[i - l] = b.p.a[i];
		b1.l = b.l - l;
		for (i = 0; i < l; i++)
		{
			a2.p.a[i] = a.p.a[i];
			b2.p.a[i] = b.p.a[i];
		}
		a2.l = l;
		b2.l = l;
		if (i = kmul(a1, b1, r1)) return i;
		if (i = kmul(a1 - a2, b1 - b2, r2)) return i;
		if (i = kmul(a2, b2, r3)) return i;
		if (r3.l + l >= lLen || r2.l + l >= lLen || r1.l + 2 * l >= lLen) return lE_OVERFLOW;
		res.fill_zero();
		for (i = 0; i <= r3.l; i++)
		{
			res.p.a[i] = r3.p.a[i];
			res.p.a[i + l] = r3.p.a[i];
		}
		for (i = 0; i <= r2.l; i++) res.p.a[i + l] ^= r2.p.a[i];
		for (i = 0; i <= r1.l; i++) res.p.a[i + l] ^= r1.p.a[i];
		for (i = 0; i <= r1.l; i++) res.p.a[i + 2 * l] ^= r1.p.a[i];
	}
	return lE_OK;
}

// Saves square of polynom a to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::sqr(const lnum &a, lnum &res)
{
	if (a.field != res.field) return lE_DIFFFIELD;
	int i;
	if (2 * a.l >= lLen) return lE_OVERFLOW;
	res.fill_zero(2 * a.l, res.l + 1);
	for (i = 0; i <= 2 * a.l; i += 2)
	{
		res.p.a[i] = Ts[a.p.a2[i]];
		res.p.a[i + 1] = Ts[a.p.a2[i + 1]];
	}
	res.l = 2 * (a.l + 1) - 1;
	if (res.l >= lLen) return lE_OVERFLOW;
	res.fix_deg();
	return lE_OK;
}

// Returns square of polynom a.
lnum lnumOperations::sqr(const lnum &a)
{
	if (!a.field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum res(*a.field);
	sqr(a, res);
	mod(res, res);
	return res;
}

// Shifts polynom a count bits left and saves result to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::shl(const lnum &a, int count, lnum &res)
{
	if (a.field != res.field) return lE_DIFFFIELD;
	int delta, shift, i;
	unsigned int t;
	if (!count)
	{
		res = a;
		return lE_OK;
	}
	if (a.deg() + count >= lLen * lbLen) return lE_OVERFLOW;
	res.zero();
	delta = count >> 5;
	shift = count % lbLen;
	for (i = 0; i <= a.l; i++)
	{
		res.p.a[i + delta] ^= a.p.a[i] << shift;
		if (t = (!shift ? 0 : (a.p.a[i] >> (lbLen - shift)))) res.p.a[i + delta + 1] ^= t;
	}
	res.l = min(a.l + delta + 1, lLen);
	res.fix_deg();
	return lE_OK;
}

// Shifts polynom a count bits right and saves result to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::shr(const lnum &a, int count, lnum &res)
{
	if (a.field != res.field) return lE_DIFFFIELD;
	int delta, shift, i;
	unsigned int t;
	if (!count || count >= lLen * lbLen)
	{
		res = a;
		return lE_OK;
	}
	res.zero();
	if (count >= a.deg() + 1)
	{
		return lE_OK;
	}
	delta = count >> 5;
	shift = count % lbLen;
	for (i = 0; i <= a.l - delta; i++)
	{
		res.p.a[i] ^= a.p.a[i + delta] >> shift;
		t = ((!shift) || (i + delta) > a.l ? 0 : (a.p.a[i + delta + 1] << (lbLen - shift)));
		res.p.a[i] ^= t;
	}
	res.l = max(a.l - delta, 0);
	res.fix_deg();
	return lE_OK;
}

// Saves scalar product of polynoms a and b in polynom res
// Returns one of the error codes stating the result of operation.
int lnumOperations::scalar(const lnum &a, const lnum &b, lnum &res)
{
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	int i;
	res.l = min(a.l, b.l);
	for (i = 0; i <= res.l; i++) res.p.a[i] = a.p.a[i] & b.p.a[i];
	res.fix_deg();
	return lE_OK;
}

// Saves scalar product of polynoms a and b in polynom a
// Returns one of the error codes stating the result of operation.
int lnumOperations::scalar(lnum &a, const lnum &b)
{
	if (a.field != b.field) return lE_DIFFFIELD;
	int i;
	if (a.l > b.l)
	{
		a.fill_zero(b.l + 1, a.l + 1);
		a.l = b.l;
	}
	for (i = 0; i <= a.l; i++) a.p.a[i] &= b.p.a[i];
	a.fix_deg();
	return lE_OK;
}

// Calculates square root of polynom a and saves result to polynom res.
int lnumOperations::sqrt(const lnum &a, lnum &res)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	int i;
	res.zero();
	lnum t(*a.field);
	for (i = 0; i <= a.deg(); i++)
		if (a[i])
		{
			if (i & 1)
			{
				shl(a.field->get_sqrt_x(), i >> 1, t);
				res += t;
			}
			else
			{
				res.set_bit(i >> 1);
			}
		}
	return lE_OK;
}

/* Conditions */

int lnumOperations::belong_to_same_nonzero_field(const lnum &a, const lnum &b)
{
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field == 0) return lE_NULLFIELD;
	return lE_OK;
}

/* Printing routines */

// Outputs polynom a to stream s in binary form.
void lnumOperations::lpBin(std::ostream &s, const lnum &a)
{
	int i;
	s << lBinChar;
	for (i = 0; i <= a.l; i++)
	{
		lnumRoutines::lpBin_int(s, a.p.a[i]);
		if (i < a.l) s << " ";
	}
}

// Outputs polynom a to stream s in hexademical form.
void lnumOperations::lpHex(std::ostream &s, const lnum &a)
{
    int i;
	s << lHexChar;
	for (i = 0;i <= a.l; i++)
	{
		lnumRoutines::lpHex_int(s, a.p.a[i]);
		if (i < a.l) s << " ";
	}
}

// Swaps polynoms a and b.
void lnumOperations::lswap(lnum &a, lnum &b)
{
	int i,l;
	const gf2n *tempField;
	unsigned int t;
	l = max(a.l, b.l);
	for (i = 0; i <= l; i++)
	{
		t = a.p.a[i];
		a.p.a[i] = b.p.a[i];
		b.p.a[i] = t;
	}
	t = a.l;
	a.l = b.l;
	b.l = t;
	tempField = a.field;
	a.field = b.field;
	b.field = tempField;
}

// Modulus operation for small weight module.
// Takes array L of positions of non-zero coeffitients of module and length of this array lN.
// Saves remainder of polynom a to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::mods(const lnum &a, const lnum &b, const int L[], int lN, lnum &res)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	if (b.is_zero()) return lE_DIVZERO;
	int da = a.l * lbLen + lnumRoutines::lHp(a.p.a[a.l]), dvq = lnumRoutines::lHp(b.p.a[b.l]), db = b.l * lbLen + dvq;
	res = a;
	if (da < db) return lE_OK;
	int i;
	unsigned int w;

	// memory allocation is done here to adapt the procedure for parallel execution
	i = a.field->get_non_zero_bit_count();
	int *modsl = new int[i];
	int *modst = new int[i];

	for (i = 0; i < lN; i++)
	{
		modsl[i] = lbLen * a.l - db + L[i];
		//modst[i]=((modsl[i] >> 5) << 5) ^ modsl[i]; // mod 2^5
		modst[i] = modsl[i] % lbLen;
		modsl[i] >>= 5;
	}
	while (res.l > b.l)
	{
		if (res.p.a[res.l])
		{
			w = res.p.a[res.l];
			for (i = 0; i < lN; i++)
			{
				res.p.a[modsl[i]] ^= (w << modst[i]);
				res.p.a[modsl[i] + 1] ^= (!modst[i] ? 0 : (w >> (lbLen - modst[i])));
			}
			res.p.a[res.l] = 0;
		}
		res.l--;
		for (i = 0; i < lN; i++) modsl[i]--;
	}
	if (lnumRoutines::lHp(res.p.a[res.l]) >= dvq)
	{
		w = (res.p.a[res.l] >> dvq) << dvq;
		for (i = 0; i < lN; i++)
		{
			if (modsl[i] >= 0) res.p.a[modsl[i]] ^= (w << modst[i]);
			res.p.a[modsl[i] + 1] ^= (!modst[i] ? 0 : (w >> (lbLen - modst[i])));
		}
		res.p.a[res.l] = (!dvq ? 0 : (res.p.a[res.l] << (lbLen - dvq)) >> (lbLen - dvq));
	}
	res.fix_deg();

	delete [] modsl;
	delete [] modst;

	return lE_OK;
}

// Saves remainder of polynom a modulo polynom b to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::modn(const lnum &a, const lnum &b, lnum &res)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != b.field) return lE_DIFFFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	int i, delta, dmod;
	int ia, ib, da, db;
	unsigned int r1, r2;
	unsigned short x, z, w[lbByte];
	if (b.is_zero()) return lE_DIVZERO;
	res = a;
	ia = lnumRoutines::lHp(res.p.a[res.l]); ib = lnumRoutines::lHp(b.p.a[b.l]);
	da = res.l * lbLen + ia; db = b.l * lbLen + ib;
	for (i = 0; i < lbByte; i++)
		if (i <= ib) w[i] = b.p.a[b.l] >> (ib - i);
		else w[i] = (b.l ? (b.p.a[b.l] << (i - ib)) ^ (!(ib - i) ? 0 : (b.p.a[b.l - 1] >> (lbLen - i + ib))) : b.p.a[b.l]);
	while (da > db)
	{
		delta = da - db;
		i = min(delta, lbByte); i = min(i, ia + 1);
		x = res.p.a[res.l] >> (ia - i + 1);
		z = Td[w[i - 1]][x] >> (lbByte - i); delta -= lnumRoutines::lHp(z);
		dmod = delta % lbLen; delta >>= 5;
		for (i = 0; i <= b.l; i++)
		{
			lnumRoutines::int_mul(z, b.p.a[i], r1, r2);
			res.p.a[i + delta] ^= r1 << dmod;
			r1 = (!dmod ? 0 : r1 >> (lbLen - dmod)) ^ r2 << dmod; if (r1) res.p.a[i + delta + 1] ^= r1;
			r2 = (!dmod ? 0 : r2 >> (lbLen - dmod));              if (r2) res.p.a[i + delta + 2] ^= r2;
		}
		// recalculating ia and da - SLOW?
		da -= ia;
		while (res.l > 0 && !res.p.a[res.l])
		{
			res.l--;
			da -= lbLen;
		}
		ia = lnumRoutines::lHp(res.p.a[res.l]);
		da += ia;
	}
	if (da == db)
	{
		for (i = 0; i <= res.l; i++) res.p.a[i] ^= b.p.a[i];
		res.fix_deg();
	}
	return lE_OK;
}

// Normalizes polynom a over its field and saves result to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::mod(const lnum &a, lnum &res)
{
	int r = -1;
	switch (a.field->get_division_mode())
	{
	case lMODf : r = mods(a, a.field->get_module(), a.field->get_non_zero_bits(), a.field->get_non_zero_bit_count(), res);
		break;
	case lMODn : r = modn(a, a.field->get_module(), res);
		break;
	}
	return r;
}

// Negates polynom a bitwise and saves result to polynom res.
// Returns one of the error codes stating the result of operation.
int lnumOperations::bit_neg(const lnum &a, lnum &res)
{
	if (a.field != res.field) return lE_DIFFFIELD;

	int i;
	res.l = a.l;
	for (i = 0; i <= a.l; i++) res.p.a[i] = ~a.p.a[i];
	i = lbLen - lnumRoutines::lHp(a.p.a[a.l]) - 1;
	res.p.a[a.l] <<= i;
	res.p.a[a.l] >>= i;
	res.fix_deg();
	return lE_OK;
}

/* Algorithms */

// Solves equation Z * Z + Z = a, returns true if successful, otherwise - false.
// Solution is returned through polynom res, second solution is polynom res + 1.
int lnumOperations::solve_quadratic_red(const lnum &a, lnum &res)
{
	if (!a.field) return lE_NULLFIELD;
	if (a.field != res.field) return lE_DIFFFIELD;
	if (a.trace()) return lE_NOSOLUTION;

	int deg = a.field->get_deg();
	if (deg & 1) a.half_trace(res);
	else
	{
		lnum u(*a.field);
		res.zero();
		u.zero(); u.set_bit(a.field->get_trace_one_element());
		int i;
		for (i = 1; i < deg; i++)
		{
			u = sqr(u);
			res = sqr(res);
			res += u * a;
			u.set_bit(a.field->get_trace_one_element());
		}
	}
	return lE_OK;
}

// Solves equation Z * Z + b * Z = c, returns true if successful, otherwise - false.
// Solutions are returned though polynoms res and res2.
int lnumOperations::solve_quadratic(const lnum &b, const lnum &c, lnum &res, lnum &res2)
{
	if (!b.field) return lE_NULLFIELD;
	if (b.field != c.field) return lE_DIFFFIELD;
	if (b.field != res.field) return lE_DIFFFIELD;
	if (b.field != res2.field) return lE_DIFFFIELD;

	if (b.is_zero())
	{
		sqrt(c, res);
		res2.zero();
		return lE_OK;
	}
	lnum cc(c / sqr(b));
	int r;
	if (r = solve_quadratic_red(cc, res)) return r;
	res.set_bit(0);
	res2 = res * b;
	res.set_bit(0);
	res *= b;
	return lE_OK;
}

/* Modification routines */

// Changes count bits of polynom a to count bits from integer what.
void lnumOperations::modify(lnum &a, int pos, unsigned int what, int count)
{
	// CHECKMARKER : Should we add field checks here? Where am I using this routine anyway?
	int j = 0, p = -1;
	while (j < count)
	{
		if (what & 1)
		{
			if (!a[pos + j])
			{
				a.set_bit(pos + j);
				p = pos + j;
			}
		}
		else if (a[pos + j]) a.set_bit(pos + j);
		what >>= 1;
		j++;
	}
	j = max(a.l * lbLen + lnumRoutines::lHp(a.p.a[a.l]), p);
	a.l = j >> 5;
	if (j % lbLen) a.l++;
	a.fix_deg();
}
