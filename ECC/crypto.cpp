#include "cryptodefines.h"
#include "crypto.h"
#include "eccoperations.h"
#include "ecurve.h"
#include "2nfactory.h"
#include "2noperations.h"
#include "bint.h"

namespace cryptoRoutines
{
	// Outputs error information to stderr and halts application execution. Takes error code as a parameter.
	void op_err(int err)
	{
		switch (err)
		{
		case cE_OK : return;
			break;
		case cE_DIFFCURVE : std::cerr << "(crypto) Exception: invalid operation on point, assigned to different curve.";
			break;
		default:
			std::cerr << "(crypto) Exception: unknown error.";
		}
		abort();
	}
}

/* Constructors */

// Creates a new instance of crypto transfomer.
crypto::crypto(const ecurve &curve) : G(curve), aG(curve), bG(curve), abG(curve)
{
	this->curve = &curve;
	int degree = curve.get_field().get_deg();
	message_length = (degree - pMarkBits) / lbByte;
}

/* Accessors */

// Returns curve, used in this crypto transformer.
const ecurve &crypto::get_curve(void) const
{
	return *curve;
}

// Returns initial point G, used in this crypto transformer.
const epoint &crypto::get_G(void) const
{
	return G;
}

// Returns public key point aG, used in this crypto transformer.
const epoint &crypto::get_aG(void) const
{
	return aG;
}

// Returns public key point bG, used in this crypto transformer.
const epoint &crypto::get_bG(void) const
{
	return bG;
}

// Returns secrete key point abG, used in this crypto transformer.
const epoint &crypto::get_abG(void) const
{
	return abG;
}

// Returns secret number keyA, used in this crypto transformer.
const bint &crypto::get_keyA(void) const
{
	return keyA;
}

// Returns secret number keyB, used in this crypto transformer.
const bint &crypto::get_keyB(void) const
{
	return keyB;
}

// Returns maximum possible single message length (in bytes).
int crypto::get_message_length(void) const
{
	return message_length;
}

/* Setter methods */

// Sets initial point G for our crypto transformer.
void crypto::set_G(const epoint &G)
{
	if (!curve->belongs_to_curve(G)) cryptoRoutines::op_err(cE_DIFFCURVE);
	this->G = G;

	// recalculate all
	eccOperations::mul(G, keyA, aG);
	eccOperations::mul(G, keyB, bG);
	eccOperations::mul(aG, keyB, abG);
}

// Sets private key for user Alice in our crypto transformer.
void crypto::set_keyA(const bint &keyA)
{
	this->keyA = keyA;

	// recalculate all
	eccOperations::mul(G, keyA, aG);
	eccOperations::mul(aG, keyB, abG);
}

// Sets private key for user Bob in our crypto transformer.
void crypto::set_keyB(const bint &keyB)
{
	this->keyB = keyB;

	// recalculate all
	eccOperations::mul(G, keyB, bG);
	eccOperations::mul(bG, keyA, abG);
}

// Sets private keys for both users in our crypto transformer.
void crypto::set_keys(const bint &keyA, const bint &keyB)
{
	this->keyA = keyA;
	this->keyB = keyB;

	// recalculate all
	eccOperations::mul(G, keyA, aG);
	eccOperations::mul(G, keyB, bG);
	eccOperations::mul(aG, keyB, abG);
}

// Sets point aG directly without Alices key. Boolean argument indicates wether we should recalculated abG using aG and Bobs key.
void crypto::override_aG(const epoint &aG, bool calculate_abG)
{
	this->aG = aG;
	if (calculate_abG) eccOperations::mul(aG, keyB, abG);
}

// Sets point bG directly without Bobs key. Boolean argument indicates wether we should recalculated abG using bG and Alices key.
void crypto::override_bG(const epoint &bG, bool calculate_abG)
{
	this->bG = bG;
	if (calculate_abG) eccOperations::mul(bG, keyA, abG);
}

// Sets point abG directly without Alices and Bobs keys or points aG and bG.
void crypto::override_abG(const epoint &abG)
{
	this->abG = abG;
}

/* Helper methods */

bint crypto::get_group_order(void) const
{
	bint res;
	G.order(res);
	return res;
}

/* Crypto methods */

// Encrypts text of length len, returns a pointer to an array of epoint objects, representing single messages.
// Variable point_count holds the length of the array.
// This function returns 0 in case of an error (unable to mark message).
epoint *crypto::encrypt(const unsigned char *data, int data_length, int &point_count) const
{
	int i, j, k, buf_len;
	char *tmp;
	point_count = data_length / message_length;
	if (data_length % message_length != 0) point_count++;
	if (point_count == 0 || data == 0) return 0;
	epoint *points = new epoint[point_count];
	for (i = 0; i < point_count; i++)
		points[i] = epoint(*curve);
	buf_len = message_length;
	while ((buf_len * lbByte) % lbLen != 0) buf_len++;
	int buf_len_in_ints = (buf_len * lbByte) / lbLen;
	tmp = new char[buf_len];

	j = 0;
	for (i = 0; i < point_count; i++)
	{
		for (k = 0; k < message_length && j < data_length; j++, k++)
			tmp[k] = data[j];
		for (; k < buf_len; k++)
			tmp[k] = 0;
		if (!curve->mark((unsigned int *)tmp, buf_len_in_ints, points[i]))
		{
			delete[] points;
			points = 0;
			break;
		}
		else points[i] += abG;
	}
	delete[] tmp;
	return points;
}

// Decrypts data from an array of points of length point_count, returns a pointer to an array of char, representing the data.
// Variable data_length holds the length of returned array.
// This function returns 0 in case of an error.
unsigned char *crypto::decrypt(const epoint *points, int point_count, int &data_length) const
{
	int i, j;
	unsigned char *data;
	unsigned char *point_data;
	data_length = message_length * point_count;
	if (data_length <= 0 || points == 0) return 0;
	for (i = 0; i < point_count; i++)
		if (!curve->belongs_to_curve(points[i])) return 0;
	data = new unsigned char[data_length];
	epoint tmp(*curve);
	for (i = 0; i < point_count; i++)
	{
		tmp = points[i] - abG;
		lnum res = tmp.unmark();
		point_data = res.to_char();
		for (j = 0; j < message_length; j++)
			data[i * message_length + j] = point_data[j];
	}
	return data;
}

// Decrypts data from an array of bytes of length data_length, returns a pointer to an array of unsigned chars, representing the data.
// Variable data_length holds the length of returned array.
// This function returns 0 in case of an error.
unsigned char *crypto::decrypt(const unsigned char *data, int data_length, int &result_length) const
{
	int i, j;
	const gf2n &field = curve->get_field();
	int fieldDegree = field.get_deg();

	int bytes_per_point = fieldDegree + 1;
	while (bytes_per_point % lbLen != 0) bytes_per_point++;
	int ints_per_point = bytes_per_point / lbLen;
	bytes_per_point /= lbByte;

	if (data_length % bytes_per_point != 0) return 0;

	int point_count = data_length / bytes_per_point;
	result_length = point_count * message_length;
	epoint point(*curve);
	unsigned char *result = new unsigned char[result_length];
	for (i = 0; i < point_count; i++)
	{
		lnum x((unsigned int *)(data + i * bytes_per_point), ints_per_point, field);
		if (curve->unpack(x, point) != cE_OK)
		{
			delete[] result;
			return 0;
		}
		point -= abG;
		point.unmark(x);
		unsigned char *value = x.to_char();
		for (j = 0; j < message_length; j++)
			result[i * message_length + j] = value[j];
	}
	return result;
}

// Converts unsigned char encrypted data to ellictic curve points, representing this encrypted data.
// Variable point_count holds the length of returned array.
// This function returns 0 in case of an error (unable to unpack point).
epoint *crypto::convert(const unsigned char *data, int data_length, int &point_count) const
{
	int i;
	const gf2n &field = curve->get_field();
	int fieldDegree = field.get_deg();

	int bytes_per_point = fieldDegree + 1;
	while (bytes_per_point % lbLen != 0) bytes_per_point++;
	int ints_per_point = bytes_per_point / lbLen;
	bytes_per_point /= lbByte;

	if (data_length % bytes_per_point != 0) return 0;

	point_count = data_length / bytes_per_point;
	epoint *points = new epoint[point_count];
	for (i = 0; i < point_count; i++)
	{
		lnum x((unsigned int *)(data + i * bytes_per_point), ints_per_point, field);
		if (curve->unpack(x, points[i]) != cE_OK)
		{
			delete[] points;
			return 0;
		}
	}
	return points;
}
