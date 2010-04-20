#include "bint.h"
#include "bintoperations.h"

/* Constructors */

// Creates a new instance of bint class with zero big integer.
bint::bint(void)
{
	zero();
}

// Creates a new instance of big integer class from integer b.
bint::bint(int b)
{
	//bint t;
	if (b == 0)
	{
		zero();
	}
	else
	{
		if (b < 0)
		{
			b = -b;
			sgn = -1;
		} else sgn = 1;
		l = bLen + 1;
		while (b > 0)
		{
			l--;
			a[l] = b % bMod;
			b /= bMod;
		}
	}
}

// Creates a new instance of big integer class from a string representation of this integer.
bint::bint(char *str)
{
	int len, i, t, h, s;
	len = strlen(str);
	if (!len)
	{
		zero();
		return;
	}
	t = 0;
	h = 1;
	l = bLen  +1;
	s = 0;
	if (str[0] == '-')
	{
		sgn = -1;
		s++;
	} else sgn = 1;
	if (str[0] == '+') s++;
	while (s < len && str[s] == '0') s++;
	while (len && !(str[len - 1] >= '0' && str[len - 1] <= '9')) len--;
	for (i = len - 1;i >= s; i--)
	{
		if (!isdigit(str[i])) break;
		t += h * (str[i] - '0');
		h *= 10;
		if (h >= bMod)
		{
			if (!l) bintRoutines::op_err(bE_OVERFLOW);
			l--;
			a[l] = t % bMod;
			t /= bMod;
			h /= bMod;
		}
	}
	if (t > 0)
	{
		if (!l) bintRoutines::op_err(bE_OVERFLOW);
		l--;
		a[l] = t;
	}
	if (l > bLen) zero();
	fix_len();
	fix_zero();
}

// Creates a new instance of big integer class from an array of integers.
bint::bint(const int *integers, int length, int sign)
{
	int i;
	if (length > bLen + 1) bintRoutines::op_err(bE_OVERFLOW);
	if (sign != -1 && sign != +1) bintRoutines::op_err(bE_OVERFLOW);
	for (i = 0; i < length; i++)
	{
		if (integers[i] < 0 || integers[i] >= bMod) bintRoutines::op_err(bE_OVERFLOW);
		a[bLen + 1 - length + i] = integers[i];
	}
	l = bLen + 1 - length;
	sgn = (short)sign;
	fix_len();
	fix_zero();
}

/* Private methods */

// Fixes length of a big intger (decrements it as needed).
void bint::fix_len(void)
{
	while (l < bLen && !a[l]) l++;
}

// Fixes zero representation in big integer class. We represent zero as +0.
void bint::fix_zero(void)
{
	if (is_zero() && sgn < 0) sgn = 1;
}

// Fills the big integer with zeros.
void bint::fill_zero(void)
{
	int i;
	for (i = 0; i <= bLen; i++) a[i] = 0;
	l = 0;
}

// Reverses the big integer. Where the hell do I use this?!
void bint::rev(void)
{
	int i, j;
	i = l; j = bLen;
	while (i < j)
	{
		bintRoutines::swap(a[i], a[j]);
		i++;
		j--;
	}
}

/* Help methods */

// Returns true if big integer is zero, otherwise - false.
bool bint::is_zero(void) const
{
	return l == bLen && a[bLen] == 0;
}

// Returns true if big integer is one, otherwise - false.
bool bint::is_one(void) const
{
	return l == bLen && a[bLen] == 1 && sgn == 1;
}

// Returns true if big integer is odd, otherwise - false.
bool bint::is_odd(void) const
{
	return a[bLen] & 1;
}

// Returns true if big integer is less than zero, otherwise - false.
bool bint::is_less_zero(void) const
{
	return sgn < 0;
}

// Returns true if big integer is greater than zero, otherwise - false.
bool bint::is_greater_zero(void) const
{
	return sgn > 0;
}

/* Setter methods */

// Sets a big integer to zero.
void bint::zero(void)
{
	l = bLen;
	a[bLen] = 0;
	sgn = 1;
}

// Sets a big integer to one.
void bint::one(void)
{
	l = bLen;
	a[bLen] = 1;
	sgn = 1;
}

// Generates a random big integer.
void bint::random(void)
{
    int i;
    l = 0;
    for (i = 0; i <= bLen; i++) a[i] = rand() % bMod;
}

/* Accessor methods */

// Returns the least significat portion of digits digits.
unsigned int bint::low_int(void) const
{
	return a[bLen];
}

// Returns sign of the big integer.
short bint::get_sign() const
{
	return sgn;
}

// Returns the number of used digit groups in big integer.
int bint::get_length() const
{
	return l;
}

// Returns a pointer to an array of bit integers digit groups.
const int *bint::get_ints(void) const
{
	return a;
}

// Returns i-th digit group of the big integer.
int bint::operator[] (int index) const
{
	if (index > bLen) bintRoutines::op_err(bE_ACCVIOLATION);
	return a[index];
}

/* Operators */

// Sets a big integer.
void bint::operator= (const bint &b)
{
	int i;
	sgn = b.sgn;
	l = b.l;
	for (i = l; i <= bLen; i++) a[i] = b.a[i];
}

// Returns true if big integer is greater than b, otherwise - false.
bool bint::operator>  (const bint &b) const
{
	return bintOperations::cmps(*this, b) > 0;
}

// Returns true if big integer is less than b, otherwise - false.
bool bint::operator<  (const bint &b) const
{
	return bintOperations::cmps(*this, b) < 0;
}

// Returns true if big integer is greater or equal to b, otherwise - false.
bool bint::operator>= (const bint &b) const
{
	return bintOperations::cmps(*this, b) >= 0;
}

// Returns true if big integer is less or euqal to b, otherwise - false.
bool bint::operator<= (const bint &b) const
{
	return bintOperations::cmps(*this, b) <= 0;
}

// Returns true if big integer is equal to b, otherwise - false.
bool bint::operator== (const bint &b) const
{
	return !bintOperations::cmps(*this, b);
}

// Returns true if big integer is not equal b, otherwise - false.
bool bint::operator!= (const bint &b) const
{
	return bintOperations::cmps(*this, b) != 0;
}

// Returns sum of this big integer and big integer b.
bint bint::operator+ (const bint &b) const
{
	bint res;
	int r;
	if ((sgn * b.sgn) == 1)
	{
		res.sgn = sgn;
		if (r = bintOperations::add(*this, b, res)) bintRoutines::op_err(r);
	}
	else
	{
		if (bintOperations::cmpl(*this, b) > 0)
		{
			res.sgn = sgn;
			if (r = bintOperations::sub(*this, b, res)) bintRoutines::op_err(r);
		}
		else
		{
			res.sgn = b.sgn;
			if (r = bintOperations::sub(b, *this, res)) bintRoutines::op_err(r);
		}
	}
	return res;
}

// Returns difference of this big integer and big integer b.
bint bint::operator- (const bint &b) const
{
	bint res;
	int r;
	if (sgn * b.sgn == 1)
	{
		if (r = bintOperations::sub(*this, b, res)) bintRoutines::op_err(r);
		if ((r = bintOperations::cmpl(*this, b)) > 0) res.sgn = sgn;
		else if (r) res.sgn = -b.sgn;
	}
	else
	{
		if (r = bintOperations::add(*this, b, res)) bintRoutines::op_err(r);
		res.sgn = sgn;
	}
	return res;
}

// Returns negated long integer.
bint bint::operator- () const
{
	bint res;
	res = *this;
	if (!is_zero()) res.sgn = -sgn;
	return res;
}

// Returns big integer multiplied by big integer b.
bint bint::operator* (const bint &b) const
{
	bint res;
	int r;
	if (r = bintOperations::mul(*this, b, res)) bintRoutines::op_err(r);
	res.sgn = sgn * b.sgn;
	return res;
}

// Returns big integer devided by big integer b.
bint bint::operator/ (const bint &b) const
{
	bint res;
	int r;
	if (r = bintOperations::div(*this, b, res)) bintRoutines::op_err(r);
	res.sgn = sgn * b.sgn;
	return res;
}

// Returns big integer modulo big integer b.
bint bint::operator% (const bint &b) const
{
	bint res;
	int r;
	if (r = bintOperations::mod(*this, b, res)) bintRoutines::op_err(r);
	res.sgn = sgn;
	return res;
}

// Increment big integer by big integer b.
void bint::operator+= (const bint &b)
{
	int r;
	if (sgn * b.sgn==1)
	{
		if (r = bintOperations::inc(*this, b)) bintRoutines::op_err(r);
	}
	else
	{
		r = bintOperations::cmpl(*this, b);
		if (r == 0) zero();
		else
		{
			if (r < 0) sgn = b.sgn;
			if (r = bintOperations::dec(*this, b)) bintRoutines::op_err(r);
		}
	}
}

// Decrement big integer by big integer b.
void bint::operator-= (const bint &b)
{
	int r;
	if (sgn * b.sgn == 1)
	{
		r = bintOperations::cmpl(*this, b);
		if (r == 0) zero();
		else
		{
			if (r < 0) sgn = -b.sgn;
			if (r = bintOperations::dec(*this, b)) bintRoutines::op_err(r);
		}
	}
	else
	{
		if (r = bintOperations::inc(*this, b)) bintRoutines::op_err(r);
	}
}

// Returns big integer devided by big integer b.
bint bint::operator/ (int b) const
{
	bint res;
	int r;
	if (r = bintOperations::div_short(*this, b, res)) bintRoutines::op_err(r);
	res.sgn = sgn * bintRoutines::signum(b);
	return res;
}

std::ostream& operator<< (std::ostream& s, const bint& a)
{
	int i, j, l = a.get_length();
	char tmp[(bModLen + 1) * (bModLen + 1)];
	if (l == -1) s << "-";
	for (i = a.get_length(); i <= bLen; i++)
	{
		sprintf(tmp, "%i", a[i]);
		j = strlen(tmp);
		for (;j < bModLen && i > l; j++) s << 0;
		s << tmp;
	}
	return s;
}
