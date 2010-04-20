#ifndef _PARALLELMASTER_H
#define _PARALLELMASTER_H

#include "ParallelPollard.h"
#include "../ecc/bint.h"
#include "../ecc/2n.h"
#include "../AVL/AVLTree.h"

// need some calsses
class ParallelIdentiry;

// Structure we use in the master to store infomation in the tree
typedef struct
{
public :
	int instance;
	lnum key;
	bint c;
	bint d;
} ParallelData;

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
	void receive_pollard_parameters(void);
	void send_solution(bool haveSolution, const bint &solution) const;
	bool solve_congruence(bint c1, bint d1, bint c2, bint d2, bint &result) const;

protected:
	AVLTree<ParallelData *> tree;
	bint groupOrder;
};

#endif
