#include "helpers.h"
#include "configsystem.h"
#include <iostream>
#include "2nfactory.h"
#include "2n.h"
#include "epoint.h"
#include "ecurve.h"
#include "crypto.h"
#include "eccoperations.h"

namespace helpers
{
	/* User interaction routines */

	// Asks user a question and waits for an answer.
	bool ask_question(char *offset, char *question)
	{
		char *line = new char[LINE_LEN];
		bool done = false;
		while (!done)
		{
			std::cout << offset << "[?] " << question << std::endl;
			std::cout << offset << "    - (Y)es, (N)o" << std::endl;
			std::cout << offset << "> ";
			std::cin.getline(line, LINE_LEN);
			if (strlen(line) > 0)
				switch (line[0])
				{
					case 'y' :
					case 'Y' :
						delete[] line;
//						std::cout << std::endl;
						return true;
					case 'N' :
					case 'n' :
						delete[] line;
//						std::cout << std::endl;
						return false;
					default :
//						std::cout << std::endl;
						break;
				}
		}
		delete[] line;
		return false;
	}

	// Helps to choose an initial point with good order.
	epoint choose_initial_point(char *offset, bool &abort, const ecurve &curve)
	{
		bool done = false;
		abort = false;
		char *line = new char[LINE_LEN];
		epoint G(curve);
		bint order;
		G = curve.random_point();
		G.order(order);
		while (!done && !abort)
		{
			std::cout << offset << "[?] Set point with order " << order << " as initial?" << std::endl;
			std::cout << offset << "    - (Y)es, (N)o, (A)bort" << std::endl;
			std::cout << offset << "> ";
			std::cin.getline(line, LINE_LEN);
			if (strlen(line) > 0)
				switch (line[0])
				{
					case 'y' :
					case 'Y' :
						done = true;
	//					std::cout << std::endl;
						break;
					case 'N' :
					case 'n' :
	//					std::cout << std::endl;
						G = curve.random_point();
						G.order(order);
						break;
					case 'A' :
					case 'a' :
						abort = true;
	//					std::cout << std::endl;
						break;
					default :
	//					std::cout << std::endl;
						break;
				}
		}
		delete[] line;
		return G;
	}

	/* Output routines */

	// Writes elliptic curve point to stream f.
	bool output_point(std::ostream &f, const epoint &point)
	{
		const gf2n &field = point.get_curve().get_field();
		lnum packed(field);
		if (point.pack(packed) != pE_OK) return false;
		f << packed << std::endl;
		return true;
	}

	// Writes data, requiered to crack the cipher to stream f.
	bool output_crack_data(std::ostream &f, const crypto &transform)
	{
		int i;
		const ecurve &curve = transform.get_curve();
		const gf2n &field = curve.get_field();
		int factorsN = curve.get_factor_length();
		const bfactor *factors = curve.get_factor();
		f << field.get_module() << std::endl;
		f << curve.get_a() << std::endl;
		f << curve.get_b() << std::endl;
		f << factorsN << std::endl;
		for (i = 0; i < factorsN; i++)
			f << factors[i].p << " " << factors[i].k << std::endl;
		if (!output_point(f, transform.get_G())) return false;
		if (!output_point(f, transform.get_aG())) return false;
		if (!output_point(f, transform.get_bG())) return false;
		return true;
	}

	// Writes encrypted data to stream f.
	bool output_encrypted_data(std::ostream &f, epoint *points, int point_count)
	{
		int i, l;
		const char *values;
		lnum x;
		for (i = 0; i < point_count; i++)
		{
			if (points[i].pack(x) < 0) return false;
			values = (const char *)x.to_char();
			l = x.get_field().get_deg() + 1;
			l = (l % lbLen != 0 ? l / lbLen + 1 : l / lbLen);
			l *= lbLen / lbByte;
			f.write(values, l);
		}
		return true;
	}

	// Outputs array of data to a file.
	void output_decrypted_data(std::ostream &f, const unsigned char *data, int data_length)
	{
		f.write((const char *)data, data_length);
	}

	/* Input routines */

	// Reads next polynom from a stream and returns it.
	lnum read_next_polynom(std::istream &f)
	{
		char *str = new char[LINE_LEN];
		f.getline(str, LINE_LEN);
		lnum res = lnum::create_nullmodule(str);
		delete[] str;

		return  res;
	}

	// Reads next polynom of field field and returns it.
	lnum read_next_polynom(std::istream &f, const gf2n &field)
	{
		char *str = new char[LINE_LEN];
		f.getline(str, LINE_LEN);
		lnum res(str, field);
		delete[] str;

		return res;
	}

	// Reads next packed point from stram and returns it.
	epoint read_next_point(std::istream &f, const ecurve &curve)
	{
		lnum x = read_next_polynom(f, curve.get_field());
		epoint point(curve);
		int res;
		if (res = curve.unpack(x, point)) eccRoutines::op_err(res);
		return point;
	}

	// Reads data requiered for field creation from file and returns pointer to gf2n object.
	gf2n *read_field(std::istream &f)
	{
		return new gf2n(read_next_polynom(f));
	}

	// Reads data requiered for curve creation from file and returns pointer to ecurve object.
	ecurve *read_curve(std::istream &f, const gf2n &field)
	{
		int i;
		char *str, *num;
		lnum coefA = read_next_polynom(f, field);
		lnum coefB = read_next_polynom(f, field);
		int factorN;
		f >> factorN;
		if (factorN == 0)
			return new ecurve(coefA, coefB);
		bfactor *factors = new bfactor[factorN];
		str = new char[LINE_LEN];
		num = new char[LINE_LEN];
		f.getline(str, LINE_LEN); // just to read the rest of the line
		for (i = 0; i < factorN; i++)
		{
			f.getline(str, LINE_LEN);
			factors[i].k = -1;
			sscanf(str, "%s%i", num, &factors[i].k);
			if (factors[i].k < 0) factors[i].k = 1;
			factors[i].p = bint(num);
		}
		delete[] str;
		delete[] num;
		return new ecurve(coefA, coefB, factorN, factors);
	}

	// Reads all bytes from given stream till the end of file.
	// It also adds a zero byte after the read text
	unsigned char *read_till_end(std::istream &f, int &text_len)
	{
		text_len = 0;
		f.seekg (0, std::ios::end);
		text_len = (int)f.tellg();
		f.seekg (0, std::ios::beg);
		unsigned char *str = new unsigned char[text_len + 1];
		f.read((char *)str, text_len);
		str[text_len] = 0;

		return str;
	}

	/* Algorithms */

	// Calculates and sets curves order factorization.
	void calc_factorization(ecurve &curve)
	{
		int n = 0;
		bfactor *bf = new bfactor[MAX_FACTORS];
		bint bi(2), t, one;
		one.one();
		curve.order(t);
		bf[0].k = 0;
		while (bi <= t)
		{
			if ((t % bi).is_zero())
			{
				bf[n].p = bi;
				bf[n].k = 1;
				t = t / bi;
			}
			while ((t % bi).is_zero())
			{
				bf[n].k++;
				t = t / bi;
			}
			if (bf[n].k > 0)
			{
				n++;
				bf[n].k = 0;
			}
			bi += one;
		}
		curve.set_factor(n, bf);
		delete[] bf;
	}

	// Helps to choose private key for Alice and Bob.
	// Returns true if a key can be chosen, otherwise - false. Key is returned through key parameter.
	bool choose_key(bint &key, const bint &ord)
	{
		bint three(3);
		if (ord.is_zero() || ord <= three || ord.is_less_zero()) return false;
		key.zero();
		bint half = ord / 2;
		while (key.is_zero() || key < half)
		{
			key.random();
			key = key % ord;
		}
		return true;
	}
}
