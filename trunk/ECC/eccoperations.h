#ifndef _EPOINTOPERATIONS_H
#define _EPOINTOPERATIONS_H

/* Need some classes */
class epoint;
class bint;

// Predefining global external functions
namespace eccRoutines
{
	void op_err(int err);
}

class eccOperations
{
	/* Constructors */
private:
	eccOperations();

private:
	/* Operation routines */
	static int inv(const epoint &p, epoint &res);
	static int sum(const epoint &p, const epoint &q, epoint &res);

public:
	static int mul(const epoint &p, int k, epoint &res);
	static int mul(const epoint &p, bint k, epoint &res);

	friend class epoint;
};

#endif
