#ifndef _PARALLELSLAVE_H
#define _PARALLELSLAVE_H

#include "ParallelPollard.h"
#include "../ecc/epoint.h"
#include "../ecc/bint.h"

// Need some classes
class ParallelIdentity;

class ParallelSlave : public ParallelPollard
{
public:
	/* Constructors */
	ParallelSlave(const ParallelIdentity &identity);

	/* Destructors */
	virtual ~ParallelSlave();

	/* Worker methods */
	virtual void run(void);

protected:
	/* Helper methods */
	void receive_config(void);
	void receive_iteration_function();
	void receive_initial_point(void);
	void receive_pollard_parameters(void);
	void generate_next_point(void);

private:
	int master_count;
	int condition_prefix_length;

	/* Generator variables */
	epoint pointX;
	bint coefC;
	bint coefD;
	bint groupOrder;
};

#endif
