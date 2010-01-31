#include "bintoperations.h"
#include "bint.h"
#include "bintdefines.h"

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif
#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a))
#endif

namespace bintRoutines
{
	// Outputs error information to stderr and halts application execution. Takes error code as a parameter.
	void op_err(int err)
	{
		switch (err)
		{
		case bE_OK : return;
			break;
		case bE_OVERFLOW : std::cerr << "(bint) Exception: big integer overflow." << std::endl;
			break;
		case bE_DIVZERO : std::cerr << "(bint) Exception: big integer division by zero." << std::endl;
			break;
		case bE_INTERNAL : std::cerr << "(bint) Exception: internal error, contact developer." << std::endl;
			break;
		case bE_NOINVERSE : std::cerr << "(bint) Exception: no inverse element exists." << std::endl;
			break;
		default:
			std::cerr << "(bint) Exception: unknown error." << std::endl;
		}
		abort();
	}

	// Swaps two integers.
	void swap(int &a, int &b)
	{
		int t = a; a = b; b = t;
	}

	// Returns sign of integer a.
	int signum(int a)
	{
		if (a < 0) return -1;
		if (a > 0) return +1;
		return 0;
	}
}

/* Help routines */

// Compares two big integers a, b with no regard of thier signs and lengths and returns the result.
int bintOperations::cmp(const bint &a, const bint &b)
{
	int i;
	for (i = a.l; i <= bLen; i++)
		if (a.a[i] != b.a[i]) return a.a[i] - b.a[i];
	return 0;
}

// Compares two big integers a, b with no regard of thier signs and returns the result.
int bintOperations::cmpl(const bint &a, const bint &b)
{
	if (a.l == b.l) return cmp(a, b);
	if (a.l < b.l) return 1; else return -1;
}

// Compares two big integers a, b and returns the result.
int bintOperations::cmps(const bint &a, const bint &b)
{
	if (a.sgn * b.sgn == 1)	return (a.sgn == 1 ? cmpl(a, b) : -cmpl(a, b));
	if (a.sgn == 1) return 1; else return -1;
}

/* Operation routines */

// Calucaltes sum of two big integers a and b and saves result in big integer res.
int bintOperations::add(const bint &a, const bint &b, bint &res)
{
	if (b.is_zero())
	{
		res = a;
		return bE_OK;
	}
	if (a.is_zero())
	{
		res = b;
		return bE_OK;
	}
	int l, m, r, t;
	m = max(a.l, b.l);
	r = 0;
	for (l = bLen; l >= m; l--)
	{
		t = r + a.a[l] + b.a[l];
		r = t / bMod;
		res.a[l] = t % bMod;
	}
	if (a.l < b.l)
	{
		for (l = m - 1; l >= a.l; l--)
		{
			t = r + a.a[l];
			r = t / bMod;
			res.a[l] = t % bMod;
		}
		res.l = a.l;
	}
	else
	{
		for (l = m - 1; l >= b.l; l--)
		{
			t = r + b.a[l];
			r = t / bMod;
			res.a[l] = t % bMod;
		}
		res.l = b.l;
	}
	if (r > 0)
	{
		if (res.l == 0)
		{
			res.a[0] += r;
			return bE_OVERFLOW;
		}
		else
		{
			res.l--;
			res.a[res.l] = r;
		}
	}
	return bE_OK;
}

// Calucaltes sum of two big integers a and b and saves result in big integer a.
int bintOperations::inc(bint &a, const bint &b)
{
	if (b.is_zero()) return bE_OK;
	if (a.is_zero())
	{
		a = b;
		return bE_OK;
	}
	int l, m, r, t;
	m = max(a.l, b.l);
	r = 0;
	for (l = bLen; l >= m; l--)
	{
		t = r + a.a[l] + b.a[l];
		r = t / bMod;
		a.a[l] = t % bMod;
	}
	if (a.l < b.l)
	{
		for (l = m - 1; l >= a.l && r; l--)
		{
			t = r + a.a[l];
			r = t / bMod;
			a.a[l] = t % bMod;
		}
	}
	else
	{
		for (l = m - 1; l >= b.l; l--)
		{
			t = r + b.a[l];
			r = t / bMod;
			a.a[l] = t % bMod;
		}
		a.l = b.l;
	}
	if (r > 0)
	{
		if (a.l == 0)
		{
			a.a[0] += r;
			return bE_OVERFLOW;
		}
		else
		{
			a.l--;
			a.a[a.l] = r;
		}
	}
	return bE_OK;
}

// Calucaltes difference of two big integers a and b and saves result in big integer res.
int bintOperations::sub(const bint &a, const bint &b, bint &res)
{
	if (b.is_zero())
	{
		res = a;
		return bE_OK;
	}
	int i, r, t;
	r = 0;
	if (cmpl(a, b) > 0)
	{
		for (i = bLen; i >= b.l; i--)
		{
			t = a.a[i] - b.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			res.a[i] = t;
		}
		for (i = b.l - 1; i >= a.l; i--)
		{
			t = a.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			res.a[i] = t;
		}
		res.l = a.l;
	}
	else
	{
		for (i = bLen; i >= a.l; i--)
		{
			t = b.a[i] - a.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			res.a[i] = t;
		}
		for (i = a.l - 1; i >= b.l; i--)
		{
			t = b.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			res.a[i] = t;
		}
		res.l = b.l;
	}
	res.fix_len();
	res.fix_zero();
	return bE_OK;
}

// Calucaltes difference of two big integers a and b and saves result in big integer a.
int bintOperations::dec(bint &a, const bint &b)
{
	if (b.is_zero()) return bE_OK;
	int i, r, t;
	r = 0;
	if (cmpl(a, b) > 0)
	{
		for (i = bLen; i >= b.l; i--)
		{
			t = a.a[i] - b.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			a.a[i] = t;
		}
		for (i = b.l - 1; i >= a.l; i--)
		{
			t = a.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			a.a[i] = t;
		}
	}
	else
	{
		for (i = bLen; i >= a.l; i--)
		{
			t = b.a[i] - a.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			a.a[i] = t;
		}
		for (i = a.l - 1; i >= b.l; i--)
		{
			t = b.a[i] - r;
			r = 0;
			while (t < 0)
			{
				r++;
				t += bMod;
			}
			a.a[i] = t;
		}
		a.l = b.l;
	}
	a.fix_len();
	return bE_OK;
}

// Calucaltes product of two big integers a and b and saves result in big integer res.
int bintOperations::mul(const bint &a, const bint &b, bint &res)
{
	if (a.is_zero() || b.is_zero())
	{
		res.zero();
		return bE_OK;
	}
	if (a.is_one())
	{
		res = b;
		return bE_OK;
	}
	if (b.is_one())
	{
		res = a;
		return bE_OK;
	}
	res.fill_zero();
	int l, j, r, t, z;
	r = 0;
	for (l = bLen; l >= a.l; l--)
	{
		for (j = bLen; j >= b.l; j--)
		{
			z = -bLen + l + j;
			t = a.a[l] * b.a[j] + res.a[z] + r;
			res.a[z] = t % bMod;
			r = t / bMod;
		}
		j = -bLen + l + b.l;
		while (r) // похоже этот цикл выполняется максимум 1 раз при r>0
		{
			if (j == 0)
			{
				res.a[0] += r;
				return bE_OVERFLOW;
			}
			else
			{
				j--;
				t = res.a[j] + r;
				res.a[j] = t % bMod;
				r = t / bMod;
			}
		}
	}
	res.l = max(a.l - bLen + b.l - 1 - 3, 0);
	res.fix_len();
	return bE_OK;
}

// Calucaltes product of big integer a and integer b and saves result in big integer res.
int bintOperations::mul_short(const bint &a, int c, bint &res)
{
	int b;
	if (c < 0) b = -c; else b = c;
	if (b >= bMod) return mul(a, bint(c), res);
	if (a.is_zero() || b==0)
	{
		res.zero();
		return bE_OK;
	}
	if (a.is_one())
	{
		res.l = bLen + 1;
		while (b > 0)
		{
			res.l--;
			res.a[res.l] = b % bMod;
			b /= bMod;
		}
		return bE_OK;
	}
	if (b == 1)
	{
		res = a;
		return bE_OK;
	}
	res.fill_zero();
	int l, r, t;
	r = 0;
	for (l = bLen; l >= a.l; l--)
	{
		t = a.a[l] * b + r;
		r = t / bMod;
		res.a[l] = t % bMod;
	}
	res.l = a.l;
	if (r)
	{
		if (res.l == 0)
		{
			res.a[0] += r;
			return bE_OVERFLOW;
		}
		else
		{
			res.l--;
			res.a[res.l] = r;
		}
	}
	return bE_OK;
}

// Calucaltes remainder of division of big integer d by big integer b and saves result in big integer a.
int bintOperations::mod(const bint &d, const bint &b, bint &a)
{
	if (b.is_zero()) return bE_DIVZERO;
	if (d.l > b.l)    // a<b
	{
		a = d;
		return bE_OK;
	}
	int c;
	if (d.l == b.l)
	{
		c = cmp(d, b);
		if (c < 0)     // a<b
		{
			a = d;
			return bE_OK;
		}
		else if (!c) // a=b
		{
			a.zero();
			return bE_OK;
		}
	}
	// actual division
	a = d;
	bint t;
	int l, r, m, s, f;
	s = a.l + bLen - b.l;
	while (sub_cmp(a, s, b, bLen) < 0) s++;
	while (cmpl(a, b) >= 0)
	{
		//cout << "a: " << a << endl;
		//cout << "b: " << b << endl;
		l = 0;
		r = bMod;
		while (r - l > 1)
		{
			m = (l + r) / 2;
			mul(b, m, t);
			f = sub_cmp(a, s, t, bLen);
			if (!f)
			{
				l = r = m;
				break;
			}
			else if (f > 0) l = m;
			else r = m;
		}
		mul(b, l, t);
		/*cout << "l= " << l << endl;
		cout << a << endl;
		cout << t << endl;
		cout << "---" << endl;
		cout << endl;*/
		if (l = sub_sub(a, s, t, bLen)) return l;
		s++;
	}
	return bE_OK;
}

// Calucaltes quotient of division of big integer d by big integer b and saves result in big integer res.
int bintOperations::div(const bint &d, const bint &b, bint &res)
{
	if (b.is_zero()) return bE_DIVZERO;
	if (d.l > b.l)    // a<b
	{
		res.zero();
		return bE_OK;
	}
	int c;
	if (d.l == b.l)
	{
		c = cmp(d, b);
		if (c < 0)     // a<b
		{
			res.zero();
			return bE_OK;
		}
		else if (!c) // a=b
		{
			res.one();
			return bE_OK;
		}
	}
	// actual division
	bint t, a;
	a = d;
	res.zero();
	int l, r, m, s, f;
	s = a.l + bLen - b.l;
	while (sub_cmp(a, s, b, bLen) < 0) s++;
	while (cmpl(a, b) >= 0)
	{
		l = 0;
		r = bMod;
		while (r - l > 1)
		{
			m = (l + r) / 2;
			mul(b, m, t);
			f = sub_cmp(a, s, t, bLen);
			if (!f)
			{
				l = r = m;
				break;
			}
			else if (f > 0) l = m;
			else r = m;
		}
		res.a[res.l] = l;
		if (!res.l) return bE_OVERFLOW;
		res.a[--res.l] = 0;
		mul(b, l, t);
		if (l = sub_sub(a, s, t, bLen)) return l;
		s++;
	}
	res.fix_len();
	res.rev();
	return bE_OK;
}

// Calucaltes quotient of division of big integer a integer b and saves result in big integer res.
int bintOperations::div_short(const bint &a, int b, bint &res)
{
	if (!b) return bE_DIVZERO;
	if (b < 0) b = -b;
	if (b == 1)
	{
		res = a;
		return bE_OK;
	}
	res.l = a.l;
	int i, r;
	r = 0;
	for (i = a.l; i <= bLen; i++)
	{
		r = r * bMod + a.a[i];
		res.a[i] = r / b;
		r %= b;
	}
	res.fix_len();
	return bE_OK;
}

// Calucaltes invers of big integer a modulo big integer m and saves result in big integer d2.
int bintOperations::inv(const bint &a, const bint &m, bint &d2)
{
	bint d1, u, v, r, q, d;
	d2.zero(); d1.one();
	u = m; v = a;
	int ret;
	while (!v.is_zero())
	{
		if (ret = mod(u, v, r)) return ret;
		if (ret = div(u - r, v, q)) return ret;
		d = d2 - q * d1;
		u = v; v = r;
		d2 = d1; d1 = d;
	}
	if (!u.is_one()) return bE_NOINVERSE;
	return bE_OK;
}

// Internal routine. Used to compare parts of big integers.
int bintOperations::sub_cmp(const bint &a, int af, const bint &b, int bf)
{
	int la, lb;
	la = af - a.l;
	lb = bf - b.l;
	if (la > lb) return 1;
	else if (la < lb) return -1;
	else
	{
		int i;
		for (i = 0; i <= la; i++)
			if (a.a[a.l + i] != b.a[b.l + i]) return a.a[a.l + i] - b.a[b.l + i];
		return 0;
	}
}

// Internal routine. Used to decrement parts of big integers.
int bintOperations::sub_sub(bint &a, int af, const bint &b, int bf)
{
	int i, j;
	if (b.is_zero()) return bE_OK;
	if (af - a.l < bf - b.l) return bE_INTERNAL;
	int r, t;
	r = 0;
	for (i = af, j = bf; j >= b.l; i--, j--)
	{
		t = a.a[i] - b.a[j] - r;
		r = 0;
		while (t < 0)
		{
			r++;
			t += bMod;
		}
		a.a[i] = t;
	}
	for (/*i=b.l-1*/;i >= a.l; i--)
	{
		t = a.a[i] - r;
		r = 0;
		while (t < 0)
		{
			r++;
			t += bMod;
		}
		a.a[i] = t;
	}
	if (r) return bE_INTERNAL;
	a.fix_len();
	return bE_OK;
}

// Calucaltes a^n and saves result in big integer res.
int bintOperations::pow(const bint &a, int n, bint &res)
{
	int d[32], i;
	if (n < 0) n = -n;
	if (a.is_zero())
	{
		res.zero();
		return bE_OK;
	}
	res.one();
	if (n == 0) return bE_OK;
	for (i = 0; i < 32; i++) d[i] = 0;
	i = 0;
	while (n > 0)
	{
		d[i] = n & 1;
		n >>= 1;
		i++;
	}
	bint t;
	for (i = 31; i >= 0; i--)
	{
		if (n = mul(res, res, t)) return n;
		if (d[i])
		{
			if (n = mul(t, a, res)) return n;
		} else res = t;
	}
	if (a.sgn > 0) res.sgn = 1;
	else
	{
		if (n & 1) res.sgn = -1;
		else res.sgn = 1;
	}
	return bE_OK;
}

/*int bintOperations::inv_ring(const bint &a, const bint &m, bint &d2)
{
	bint d1, u, v, r, q, d, Z, tmp;
	d2.zero(); d1.one();
	Z.zero();
	u = m; v = a;
	int ret;
	while (!v.is_zero())
	{
		if (ret = mod(u, v, r)) return ret;
		tmp = u - r;
		if (tmp < Z) tmp += m;
		if (ret = div(tmp, v, q)) return ret;
		d = d2 - q * d1;
		if (d < Z) d += m;
		if (d >= m) d = d % m;
		u = v; v = r;
		d2 = d1; d1 = d;
	}
	if (!u.is_one()) return bE_NOINVERSE;
	return bE_OK;
}*/
