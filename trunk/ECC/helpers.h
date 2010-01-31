#ifndef _HELPERS_H
#define _HELPERS_H

#include <fstream>

/* Need some classes */
class epoint;
class lnum;
class gf2n;
class ecurve;
class bint;
class crypto;

namespace helpers
{
	/* User interaction routines */
	bool ask_question(char *offset, char *question);
	epoint choose_initial_point(char *offset, bool &abort, const ecurve &curve);

	/* Output routines */
	bool output_point(std::ostream &f, const epoint &point);
	bool output_crack_data(std::ostream &f, const crypto &transform);
	bool output_encrypted_data(std::ostream &f, epoint *points, int point_count);
	void output_decrypted_data(std::ostream &f, const unsigned char *data, int data_length);

	/* Input routines */
	lnum read_next_polynom(std::istream &f);
	lnum read_next_polynom(std::istream &f, const gf2n &field);
	epoint read_next_point(std::istream &f, const ecurve &curve);
	gf2n *read_field(std::istream &f);
	ecurve *read_curve(std::istream &f, const gf2n &field);
	unsigned char *read_till_end(std::istream &f, int &text_len);

	/* Algorithms */
	void calc_factorization(ecurve &curve);
	bool choose_key(bint &key, const bint &ord);
}
#endif
