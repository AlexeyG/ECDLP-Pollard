#ifndef _2N_H
#define _2N_H

#include <iostream>
#include "2ndefines.h"

/* Need some classes */
class gf2n;
class lnumOperations;

union lnum_t
{
	unsigned int   a[lLen];
	unsigned short a2[2 * lLen]; 
	unsigned char  a1[4 * lLen];
};

class lnum 
{
	/* Constructors */
private:
	lnum(unsigned int m[], int n);
	lnum(char *str);
public:
	lnum(void);
	lnum(const gf2n &owningField);
	lnum(const lnum &a);
	lnum(unsigned int m[], int n, const gf2n &owningField);
	lnum(char *str, const gf2n &owningField);

private:
	/* Internal methods */
	void default_constructor();
	void copy_constructor(const lnum &a);
	void int_array_constructor(unsigned int m[], int n);
	void string_constructor(char *str);

public:

	/* Help methods */
	bool is_zero(void) const;
	bool is_one(void) const;
	int deg(void) const;
	bool has_field(void) const;

	/* Setter methods */
	void zero(void);
	void one(void);
	void set_bit(int i);
	void random(int pow);
	void random_le(int pow);

	/* Accessor methods */
	const gf2n &get_field() const;
	unsigned char *to_char(void) const;
	unsigned short *to_short(void) const;
	unsigned int *to_int(void) const;

	/* Additional methods */
	int non_zero_bits(void) const;
	bool parity(void) const;
	bool trace(void) const;
	lnum half_trace(void) const;
	void half_trace(lnum &res) const;

	/* Factory methods */
	static lnum create_nullmodule();
	static lnum create_nullmodule(char *str);
	static lnum create_nullmodule(unsigned int m[], int n);

	/* Operators */
	lnum& operator=  (const lnum &b);
	bool operator[] (int i) const;

	bool operator== (const lnum &b) const;
	bool operator!= (const lnum &b) const;
	bool operator>= (const lnum &b) const;
	bool operator<= (const lnum &b) const;
	bool operator>  (const lnum &b) const;
	bool operator<  (const lnum &b) const;

	lnum operator+  (const lnum &b) const;
	void operator+= (const lnum &b);
	lnum operator-  (const lnum &b) const;
	void operator-= (const lnum &b);
	lnum operator*  (const lnum &b) const;
	void operator*= (const lnum &b);
	lnum operator/  (const lnum &b) const;
	void operator/= (const lnum &b);
	lnum operator-  () const;

	friend std::ostream& operator<< (std::ostream& s, const lnum& a);
private:
	void fill_zero(int s = 0, int f = lLen);
	void fix_deg(void);

	lnum_t p;
	int l;

	const gf2n *field;

	friend class lnumOperations;
	friend class gf2n;
};

/* To be removed */
std::ostream& operator<< (std::ostream& s, const lnum& a);

#endif // _2N_H
