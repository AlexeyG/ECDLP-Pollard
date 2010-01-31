#ifndef _PARALLEL_H
#define _PARALLEL_H

#include <fstream>
#include "../ecc/epoint.h"

// Need some classes
class gf2n;
class ecurve;
class epoint;

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
};

class ParallelManager : public ParallelPollard
{
public:
	/* Constructors */
	ParallelManager(const ParallelIdentity &identity, char *input_filename, char *config_filename, char *encrypted_filename, char *output_filename);
	
	/* Destructors */
	virtual ~ParallelManager();

	/* Accessor methods */
	int get_master_count(void) const;

	/* Worker methods */
	virtual void run(void);

protected:
	bool solve(const epoint &G, const epoint &xG, bint &result, bool verbose, char *offset, double &work_time);
	bool pollard(const epoint &P, const epoint &Q, const bint &order, bint &result, double &work_time);

	/* Helper methods */
	void abort_all(int info) const;
	void send_config(void) const;

private:
	/* Internal methods */
	void open_files(char *input_filename, char *config_filename, char *encrypted_filename, char *output_filename);
	void close_files();
	bool read_config();
	bool read_input();

	int master_count;
	int condition_prefix_length;

	// Some elliptic curve points
	epoint G;
	epoint aG;
	epoint bG;

	// Stream variables
	std::ifstream fin;
	std::ifstream fconfig;
	std::ifstream fenc;
	std::ofstream fout;
};

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
private:

	// !!-!! to be removed
	int master_count;
	int condition_prefix_length;
};

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
