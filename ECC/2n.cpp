#include <iostream>
#include "2ndefines.h"

#include "2n.h"
#include "2nfactory.h"
#include "2noperations.h"
#include "mult_table.h"
#include "sqr_table.h"
#include "div_table.h"

/* Constructors */

// Creates a new instance of lnum (attached to no field).
lnum::lnum(void)
{
	field = 0;
	default_constructor();
}

// Creates a new instance of lnum, that belongs to field owningField.
lnum::lnum(const gf2n &owningField)
{
	field = &owningField;
	default_constructor();
}

// Copy constructor: creates a new instance of lnum, belonging to same field as polynom a.
lnum::lnum(const lnum &a)
{
	field = a.field;
	copy_constructor(a);
}

// Constructs a new polynom, belonging to null field from a an array of integers.
lnum::lnum(unsigned int m[], int n)
{
	field = 0;
	int_array_constructor(m, n);
}

// Constructs a new polynom, belonging to field owningField from a an array of integers.
lnum::lnum(unsigned int m[], int n, const gf2n &owningField)
{
	field = &owningField;
	int_array_constructor(m, n);
}

// Constructs a new polynom, belonging to null field from its string representation. The representation can be binary, hex or decimal.
lnum::lnum(char *str)
{
	field = 0;
	string_constructor(str);
}

// Constructs a new polynom, belonging to field owningField from its string representation. The representation can be binary, hex or decimal.
lnum::lnum(char *str, const gf2n &owningField)
{
	field = &owningField;
	string_constructor(str);
}

/* Internal methods */

// Internal default constructor implentation.
void lnum::default_constructor()
{
	fill_zero(0, lLen);
	l = 0;
#ifndef lHEADER_TABLES
	if (!initTm)
	{
		initTm = true;
		fill_mult_table();
	}
#endif
}

// Internal copy constructor implentation.
void lnum::copy_constructor(const lnum &a)
{
	int i;
	l = a.l;
	fill_zero(l + 1,lLen);
	for (i = 0; i <= l; i++) p.a[i] = a.p.a[i];
}

// Internal int array constructor implentation.
void lnum::int_array_constructor(unsigned int m[], int n)
{
	int i;
	if (n > lLen) lnumRoutines::op_err(lE_ACC);
	fill_zero(n, lLen);
	if (!n) return;
	for (i = 0; i < n; i++) p.a[i] = m[i];
	l = n - 1;
	fix_deg();
}

// Internal string constructor implentation.
void lnum::string_constructor(char *str)
{
	fill_zero(0, lLen);
	l = 0;
	int i, j, s_len = strlen(str);
	if (!s_len) return;
	if (str[0] == lBinChar)
	{
		while (s_len && str[s_len-1] != '1') s_len--;
		i = 1;
		while (i < s_len)
		{
			j = 0;
			while (j < lbLen && i < s_len)
			{
				switch (str[i])
				{
				case '0' :
					j++;
					break;
				case '1' :
					p.a[l] ^= 1 << j;
					j++;
					break;
				case ' ' :
					break;
				default:
					lnumRoutines::op_err(lE_IO);
				}
				i++;
			}
			if (i<s_len) l++;
		}
	}
	else if (str[0] == lHexChar)
	{
		while (s_len && !((str[s_len - 1] > '0' && str[s_len - 1] <= '9') || (str[s_len - 1] >= 'A' && str[s_len - 1] <= 'F') || (str[s_len-1] >= 'a' && str[s_len - 1] <= 'f'))) s_len--;
		i = 1;
		unsigned char k;
		while (i < s_len)
		{
			j = 0;
			while (j < lbLen && i < s_len)
			{
				if (str[i] >= '0' && str[i] <= '9')
				{
					k = str[i] - '0';
					p.a[l] ^= k << j;
					j += lbHex;
				}
				else if (str[i] >= 'A' && str[i] <= 'F')
				{
					k = str[i] - 'A' + 10;
					p.a[l] ^= k << j;
					j += lbHex;
				}
				else if (str[i] >= 'a' && str[i] <= 'f')
				{
					k = str[i] - 'a' + 10;
					p.a[l] ^= k << j;
					j += lbHex;
				}
				else if (str[i] != ' ') lnumRoutines::op_err(lE_IO);
				i++;
			}
			if (i < s_len) l++;
		}
	}
	else
	{
		while (s_len && !(str[s_len-1] >= '0' && str[s_len - 1] <= '9')) s_len--;
		i = 0;
		int t = -1;
		while (i < s_len)
		{
			if (str[i] >= '0' && str[i] <= '9')	t = (t < 0 ? str[i] - '0' : t * 10 + str[i] - '0');
			else
			{
				if (t >= 0)
				{
					l = max(l, (!(t % lbLen) ? t / lbLen : t / lbLen + 1));
					if (!(*this)[t]) set_bit(t);
					t = -1;
				}
			}
			i++;
		}
		if (t >= 0)
		{
			l = max(l, (!(t % lbLen) ? t / lbLen : t / lbLen + 1));
			if (!(*this)[t]) set_bit(t);
		}
	}
	fix_deg();
}

/* Help methods */

// Returns true if polynom is zero, otherwise - false.
bool lnum::is_zero(void) const
{
	return !l && !p.a[0];
}

// Returns true if plynom is one, otherwise - false.
bool lnum::is_one(void) const
{
	return !l && p.a[0] == 1;
}

// Returns degree of the polynom.
int lnum::deg(void) const
{
	return l * lbLen + lnumRoutines::lHp(p.a[l]);
}

// Returns true if this polynom is assigned to a field, otherwise - false.
bool lnum::has_field(void) const
{
	return field != 0;
}

/* Setter methods */

// Sets the polynome to zero.
void lnum::zero(void)
{
	fill_zero(0, l + 1);
	l = 0;
}

// Sets the polynome to one.
void lnum::one(void)
{
	fill_zero(0, l + 1);
	l = 0;
	set_bit(0);
}

// Togles i-th bit on and off (changes i-th coeffitient in polynom).
void lnum::set_bit(int i)
{
	if (i / lbLen >= lLen) return;
	p.a[i / lbLen] ^= 1 << (i % lbLen);
	i /= lbLen;
	if (i > l) l = i;
	fix_deg();
}

// Generates a random polynom of a given degree.
void lnum::random(int pow)
{
	if (pow < 0) return;
	int i;
	i = pow / lbLen;
	pow %= lbLen;
	lnumOperations::lrandom((*this), i);
	if (!(p.a[i] & (1 << pow))) p.a[i] ^= (1 << pow);
	p.a[i] <<= lbLen - pow - 1;
	p.a[i] >>= lbLen - pow - 1;
	l = i;
}

// Generates a random polynom of a degree less or equal to given.
void lnum::random_le(int pow)
{
	if (pow < 0) return;
	int i;
	i=pow / lbLen;
	pow %= lbLen;
	lnumOperations::lrandom(*this, i);
	if (i <= l)
	{
		p.a[i] <<= lbLen - pow - 1;
		p.a[i] >>= lbLen - pow - 1;
		fix_deg();
	}
}

/* Accessor methods */

const gf2n &lnum::get_field() const
{
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	return (*field);
}

// Returns a pointer to unsigned char array of polynom coeffitients
unsigned char *lnum::to_char(void) const
{
	return (unsigned char *) p.a1;
}

// Returns a pointer to unsigned short array of polynom coeffitients
unsigned short *lnum::to_short(void) const
{
	return (unsigned short *) p.a2;
}

// Returns a pointer to unsigned int array of polynom coeffitients
unsigned int *lnum::to_int(void) const
{
	return (unsigned int *) p.a;
}

/* Additional methods */

// Computes number of non-zero bits of a polynom
int lnum::non_zero_bits(void) const
{
	int i;
	unsigned int r = 0;
	for (i = 0; i <= l; i++) r += lnumRoutines::bitcnt_int(p.a[i]);
	return r;
}

// Computes parity of a polynom
bool lnum::parity(void) const
{
	return (non_zero_bits() & 1 ? true : false);
}

// Computes trace of a polynom. Returns true if trace is one, otherwise - false
bool lnum::trace(void) const
{
	int i, t, r = 0;
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum traceVec = field->get_trace_vector();
	t = min(l, traceVec.l);
	for (i = 0; i <= l; i++) r += lnumRoutines::bitcnt_int(p.a[i] & traceVec.p.a[i]);
	return (r & 1) > 0;
}

// Computes half-trace of a polynom
lnum lnum::half_trace(void) const
{
	lnum res;
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum traceVec = field->get_trace_vector();
	const lnum *halfTraceArr = field->get_half_trace_array();
	int i, j = 0, k, u;
	unsigned int t;
	for (i = 0; i <= l; i++)
	{
		for (k = 0, t = p.a[i]; t; k++, t >>= 1)
			if (t & 1)
			{
				u = k + j;
				while (!(u & 1) && u)
				{
					if (traceVec[u]) res.set_bit(0);
					u >>= 1;
					res.set_bit(u);
				}
				if (u) res += halfTraceArr[u >> 1];
			}
		j += lbLen;
	}
	return res;
}

// Computes half-trace of a polynom and save result to polynom res
void lnum::half_trace(lnum &res) const
{
	int i, j = 0, k, u;
	unsigned int t;
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum traceVec = field->get_module();
	const lnum *halfTraceArr = field->get_half_trace_array();
	res.zero();
	for (i = 0; i <= l; i++)
	{
		for (k = 0, t = p.a[i]; t; k++ ,t >>= 1)
			if (t & 1)
			{
				u = k + j;
				while (!(u & 1) && u)
				{
					if (traceVec[u]) res.set_bit(0);
					u >>= 1;
					res.set_bit(u);
				}
				if (u) res += halfTraceArr[u >> 1];
			}
		j += lbLen;
	}
}

/* Factory methods */

// Creates a zero polynom, belonging to no field.
lnum lnum::create_nullmodule()
{
	lnum res;
	return res;
}

// Creates a polynom belonging to no field from its string represetation.
lnum lnum::create_nullmodule(char *str)
{
	lnum res(str);
	return res;
}

// Creates a polynom belonging to no field from an integer array m of length n.
lnum lnum::create_nullmodule(unsigned int m[], int n)
{
	lnum res(m, n);
	return res;
}

/* Private methods */

// Fills part of polynom from s to f with zeros.
void lnum::fill_zero(int s, int f) // s = 0, f= lLen
{
	int i;
	s = max(s, 0);
	f = min(f, lLen);
	for (i = s; i < f; i++) p.a[i] = 0;
}

// Fixes degree of a polynom.
void lnum::fix_deg(void)
{
	while (l > 0 && !p.a[l]) l--;
}

/* Operators */

// Sets an instance of lnum class.
lnum& lnum::operator=  (const lnum &b)
{
	int i;
	if (&b != this)
	{
		//if (!b.field) lnumRoutines::op_err(lE_NULLFIELD);
		field = b.field;
		fill_zero(b.l + 1, l + 1);
		l = b.l;
		for (i = 0; i <= l; i++) p.a[i] = b.p.a[i];
	}
	return (*this);
}

// Accessor to i-th bit of a polynom.
bool lnum::operator[] (int i) const
{
	int k = i / lbLen;
	if (k >= lLen || k < 0) lnumRoutines::op_err(lE_ACC);
	return (p.a[k] & ((unsigned int)1 << (i % lbLen))) > 0;
}

// Return true if 2 polynoms are equal, otherwise - false.
bool lnum::operator== (const lnum &b) const
{
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	return !lnumOperations::cmp(*this, b);
}

// Return true if 2 polynoms are not equal, otherwise - false.
bool lnum::operator!= (const lnum &b) const
{
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	return lnumOperations::cmp(*this, b) != 0;
}

// Return true if first polynom is greater or equal, otherwise - false.
bool lnum::operator>= (const lnum &b) const
{
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	return lnumOperations::cmp(*this, b) >= 0;
}

// Returns true if first polynom is less or equal, otherwise - false.
bool lnum::operator<= (const lnum &b) const
{
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	return lnumOperations::cmp(*this, b) <= 0;
}

// Returns true if first polynom is greater, otherwise - false.
bool lnum::operator> (const lnum &b) const
{
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	return lnumOperations::cmp(*this, b) > 0;
}

// Returns true if first polynom is less or equal, otherwise - false.
bool lnum::operator<  (const lnum &b) const
{
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	return lnumOperations::cmp(*this, b) < 0;
}

// Returns sum of 2 polynoms.
lnum lnum::operator+ (const lnum &b) const
{
	if (!b.field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum res(*field);
	int r;
	if ((r = lnumOperations::xor((*this), b, res)) != lE_OK) lnumRoutines::op_err(r);
	return res;
}

// Increments polynom by a given polynom.
void lnum::operator+= (const lnum &b)
{
	if (!b.field) lnumRoutines::op_err(lE_NULLFIELD);
	int r;
	if ((r = lnumOperations::xor((*this), b)) != lE_OK) lnumRoutines::op_err(r);
}

// Returns difference of 2 polynoms.
lnum lnum::operator- (const lnum &b) const
{
	if (!b.field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum res(*field);
	int r;
	if ((r = lnumOperations::xor((*this), b, res)) != lE_OK) lnumRoutines::op_err(r);
	return res;
}

// Decrements polynom by a given polynom.
void lnum::operator-= (const lnum &b)
{
	if (!b.field) lnumRoutines::op_err(lE_NULLFIELD);
	int r;
	if ((r = lnumOperations::xor((*this), b)) != lE_OK) lnumRoutines::op_err(r);
}

// Returns product of 2 polynoms.
lnum lnum::operator*  (const lnum &b) const
{
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	lnum res(*field);
	int i;
	if (this == &b)
	{
		if (i = lnumOperations::sqr((*this), res)) lnumRoutines::op_err(i);
	}
	else if (i = lnumOperations::mul((*this), b, res)) lnumRoutines::op_err(i);
	lnumOperations::mod(res, res);
	return res;
}

// Multiplys polynom by a given polynom.
void lnum::operator*= (const lnum &b)
{
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum res(*field);
	int i;
	if (this == &b)
	{
		if (i = lnumOperations::sqr(*this, res)) lnumRoutines::op_err(i);
	}
	else if (i = lnumOperations::mul(*this, b , res)) lnumRoutines::op_err(i);
	lnumOperations::mod(res,*this);
}

// Returns quotient of 2 polynoms.
lnum lnum::operator/  (const lnum &b) const
{
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum res(*field);
	int r;
	if (r = lnumOperations::inv_ex(*this, b, b.field->get_module(), res)) lnumRoutines::op_err(r);
	lnumOperations::mod(res, res);
	return res;
}

// Devides polynom by a given polynom.
void lnum::operator/= (const lnum &b)
{
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	if (field != b.field) lnumRoutines::op_err(lE_DIFFFIELD);
	int r;
	if (r = lnumOperations::inv_ex(*this, b, b.field->get_module())) lnumRoutines::op_err(r);
	lnumOperations::mod((*this), (*this));
}

// Returns arithmeticly negated polynom.
lnum lnum::operator-  () const
{
	if (!field) lnumRoutines::op_err(lE_NULLFIELD);
	lnum res(*this);
	return res;
}


/* Should we (RE)move this? */

std::ostream& operator<< (std::ostream& s, const lnum& a)
{
	int outputMode = (a.has_field() ? a.get_field().get_output_mode() : loutDefault);
	switch (outputMode)
	{
	case loutBin :
		lnumOperations::lpBin(s, a);
		break;
	case loutHex :
		lnumOperations::lpHex(s,a);
		break;
	default :
		break;
	}
	return s;
}
