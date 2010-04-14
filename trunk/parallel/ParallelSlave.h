#ifndef _PARALLELSLAVE_H
#define _PARALLELSLAVE_H

#include "ParallelPollard.h"

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

private:
	int master_count;
	int condition_prefix_length;
};

#endif
