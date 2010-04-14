#ifndef _PARALLELIDENTIFY_H
#define _PARALLELIDENTIFY_H

class ParallelIdentity
{
public:
	/* Constructors */
	ParallelIdentity(int processID, int processCount);
	ParallelIdentity();

	/* Accessor methods */
	int get_process_id() const;
	int get_process_count() const;
private:
	int processID;
	int processCount;
};

#endif
