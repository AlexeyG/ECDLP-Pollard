#ifndef _BINT_H
#define _BINT_H

#include <iostream>
#include <ostream>
#include "bintdefines.h"

/* Need some classes */
class bint;

class bint
{
	/* Constructors */
public:
	bint(void);
	bint(int b);
	bint(char *str);
	bint(const int *integers, int length, int sign);

	/* Help methods */
	bool is_zero(void) const;
	bool is_one(void) const;
	bool is_odd(void) const;
	bool is_less_zero(void) const;
	bool is_greater_zero(void) const;

	/* Setter methods */
	void zero(void);
	void one(void);
	void random(void);

	/* Accessor methods */
	unsigned int low_int(void) const;
	short get_sign() const;
	int get_length() const;
	const int *get_ints(void) const;
	int operator[] (int index) const;

	/* Operators */
	void operator=  (const bint &b);
	bool operator>  (const bint &b) const;
	bool operator<  (const bint &b) const;
	bool operator>= (const bint &b) const;
	bool operator<= (const bint &b) const;
	bool operator== (const bint &b) const;
	bool operator!= (const bint &b) const;
	bint operator+ (const bint &b) const;
	bint operator- (const bint &b) const;
	bint operator- () const;
	bint operator* (const bint &b) const;
	bint operator/ (const bint &b) const;
	bint operator% (const bint &b) const;
	void operator+= (const bint &b);
	void operator-= (const bint &b);
	bint operator/ (const int b) const;

private:
	/* Private methods */
	void fix_len(void);
	void fix_zero(void);
	void fill_zero(void);
	void rev(void);

	int a[bLen + 1];
	short sgn;
	int l;

	friend class bintOperations;
};

std::ostream& operator<< (std::ostream& s, const bint& a);
#endif
