#ifndef _PARALLELMANAGER_H
#define _PARALLELMANAGER_H

#include "ParallelPollard.h"
#include "ParallelIdentity.h"
#include "../ecc/epoint.h"
#include <iostream>

// Need some classes
class bint;
class epoint;

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

#endif
