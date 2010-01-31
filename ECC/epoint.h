#ifndef _EPOINT_H
#define _EPOINT_H

#include "2n.h"
#include "eccdefines.h"

/* Need some classes */
class bint;
class ecurve;
class eccOperations;

class epoint
{
	/* Constructors */
public:
	epoint(void);
	epoint(const epoint &p);
	epoint(const lnum &x, const lnum &y, const ecurve &ec);
	epoint(const ecurve &ec);

	/* Helper methods */
	bool is_inf(void) const;
	bool check(void) const;
	int f(int arg) const;

	/* Accessor methods */
	lnum get_x(void) const;
	void get_x(lnum &res) const;
	lnum get_y(void) const;
	void get_y(lnum &res) const;
	const ecurve &get_curve(void) const;

	/* Setter methods */
	void inf(void);
	void assign(const ecurve *ec);

	/* Helper methods */
	void order(bint &res) const;

	/* Packing methods */
	int pack_info(void) const;
	int pack(lnum &res) const;

	/* Data marking methods */
	void unmark(lnum &res) const;
	lnum unmark(void) const;

	/* Operators */
	epoint& operator=  (const epoint &p);
	bool operator== (const epoint &q) const;
	bool operator!= (const epoint &q) const;
	epoint operator+ (const epoint &q) const;
	epoint operator- (const epoint &q) const;
	void operator+= (const epoint &q);
	void operator-= (const epoint &q);
	epoint operator- (void) const;

private:
	lnum x, y;
	const ecurve *curve;

	friend class eccOperations;
	friend class ecurve;
};
#endif
