#ifndef _2NFACTORY_H
#define _2NFACTORY_H

#include "2ndefines.h"
#include "2n.h"

/* Need some classes */
class lnum;

class gf2n
{
public:
	gf2n(const lnum &mod);
	gf2n(char *str);
	gf2n(unsigned int m[], int n);
	gf2n(const gf2n &field);
	~gf2n();

	/* Accessors */
	char get_output_mode() const;
	char get_division_mode() const;

	int get_non_zero_bit_count() const;
	const int *get_non_zero_bits() const;

	int get_deg() const;

	int get_trace_one_element() const;

	const lnum &get_module() const;
	const lnum &get_trace_vector() const;
	const lnum &get_sqrt_x() const;
	const lnum *get_half_trace_array() const;

	/* Setter methods */
	void set_output_mode(int output_mode);

	/* Helper methods */
	bool belongs_to_field(const lnum &a) const;

	/* Factory methods */
	lnum create_lnum();
	lnum create_lnum(unsigned int m[], int n);
	lnum create_lnum(char *str);

private:
	/* Internal routines */
	void calc_trace_vector(const lnum &m, int n, lnum &res);
	void calc_half_trace_matrix(int n, lnum Th[]);
	void calc_sqrt_x(lnum &sqrt_x);
	int set_module();
	void initialize();

	int *non_zero_bits;
	int non_zero_bit_count;

	//int MSG_LEN, MSG_LEN_EX;

	char loutMode;                                 // Output mode used for this field
	char ldivMode;                                 // Division mode used for this field

	int tr1e;                                      // Trace(x^tr1e) = 1

	lnum module;                                   // The actual field generator
	lnum trace_vector;                             // Trace vector
	lnum *half_trace_array;                        // Half-trace polynom array
	lnum sqrt_x;                                   // Square root of polynom x
};

#endif
