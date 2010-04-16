#ifndef _PARALLELPOLLARD_H
#define _PARALLELPOLLARD_H

#include <fstream>
#include "ParallelIdentity.h"

// Need some classes
class gf2n;
class ecurve;
class bint;
class epoint;

class ParallelPollard
{
public:
	/* Constructors */
	ParallelPollard(const ParallelIdentity &identity);

	/* Destructors */
	virtual ~ParallelPollard();

	/* Accessor methods */
	const ParallelIdentity &get_identity(void) const;

	/* Worker methods */
	virtual void run(void);

protected:
	ParallelIdentity identity;
	gf2n *field;
	ecurve *curve;

	bint *functionA;
	bint *functionB;
	epoint *functionR;
};

#endif
