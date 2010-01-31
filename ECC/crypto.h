#ifndef _CRYPTO_H
#define _CRYPTO_H

#include "eccdefines.h"
#include "ecurve.h"
#include "epoint.h"
#include "bint.h"

/* Need some classes */
class lnum;
class gf2n;
class ecurve;
class epoint;
class bint;

namespace cryptoRoutines
{
	void op_err(int err);
}

class crypto
{
public:
	/* Constructors */
	crypto(const ecurve &curve);

	/* Accessors */
	const ecurve &get_curve(void) const;
	const epoint &get_G(void) const;
	const epoint &get_aG(void) const;
	const epoint &get_bG(void) const;
	const epoint &get_abG(void) const;
	const bint &get_keyA(void) const;
	const bint &get_keyB(void) const;
	int get_message_length(void) const;

	/* Setter methods */
	void set_G(const epoint &G);
	void set_keyA(const bint &keyA);
	void set_keyB(const bint &keyB);
	void set_keys(const bint &keyA, const bint &keyB);
	void override_aG(const epoint &aG, bool calculate_abG);
	void override_bG(const epoint &bG, bool calculate_abG);
	void override_abG(const epoint &abG);

	/* Helper methods */
	bint get_group_order(void) const;

	/* Crypto methods */
	epoint *encrypt(const unsigned char *data, int data_length, int &point_count) const;
	unsigned char *decrypt(const epoint *points, int point_count, int &data_length) const;
	unsigned char *decrypt(const unsigned char *data, int data_length, int &result_length) const;
	epoint *convert(const unsigned char *data, int data_length, int &point_count) const;

private:
	const ecurve *curve;
	epoint G, aG, bG, abG;
	bint keyA, keyB;

	int message_length;
};
#endif
