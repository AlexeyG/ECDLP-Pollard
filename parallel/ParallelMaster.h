#ifndef _PARALLELMASTER_H
#define _PARALLELMASTER_H

#include "ParallelPollard.h"

// need some calsses
class ParallelIdentiry;

class ParallelMaster : public ParallelPollard
{
public:
	/* Constructors */
	ParallelMaster(const ParallelIdentity &identity);

	/* Destructors */
	virtual ~ParallelMaster();

	/* Worker methods */
	virtual void run(void);

protected:
	/* Helper methods */
	void receive_config(void);
};

#endif
