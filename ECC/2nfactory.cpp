#include "2nfactory.h"
#include "2n.h"
#include "2noperations.h"

/* Constructors */

// Creates a new instance of field class from a given module.
gf2n::gf2n(const lnum &mod) : trace_vector(*this), sqrt_x(*this), module(mod)
{
	initialize();
}

// Creates a new instance of field class from a given string.
gf2n::gf2n(char *str) : trace_vector(*this), sqrt_x(*this), module(str, *this)
{
	//lnum mod(str, *this);
	initialize();
}

// Creates a new instance of field class from a given array of integers m of length n.
gf2n::gf2n(unsigned int m[], int n) : trace_vector(*this), sqrt_x(*this), module(m, n, *this)
{
	//lnum mod(m, n, *this);
	initialize();
}

// Field copy constructor
gf2n::gf2n(const gf2n &field) : module(field.module), sqrt_x(field.sqrt_x), trace_vector(field.trace_vector)
{
	int i;
	non_zero_bit_count = field.non_zero_bit_count;

	half_trace_array = new lnum[field.get_deg() / 2];
	non_zero_bits = new int[non_zero_bit_count + 1];

	//module = field.module;
	//trace_vector = field.trace_vector;
	tr1e = field.tr1e;
	//sqrt_x = field.sqrt_x;
	for (i = 0; i < (non_zero_bit_count + 1) / 2; i++)
		half_trace_array[i] = field.half_trace_array[i];
	for (i = 0; i <= non_zero_bit_count; i++)
		non_zero_bits[i] = field.non_zero_bits[i];

	loutMode = field.loutMode;
	ldivMode = field.ldivMode;
}

gf2n::~gf2n()
{
	delete [] half_trace_array;
	delete [] non_zero_bits;
}

/* Internal routines */

// Calculates trace vector for given field generator polynom m of degree n.
// Saves result to polynom res.
void gf2n::calc_trace_vector(const lnum &m, int n, lnum &res)
{
	tr1e = -1;
	int i;
	lnum a(m);
	a.set_bit(n);
	res.one();
	lnum cv(*this);
	for (i = 1; i < n; i++)
	{
		if (i & 1)
		{
			lnumOperations::shr(a, n - i, cv);
			lnumOperations::scalar(cv, res);
			if (cv.parity())
			{
				if (tr1e < 0) tr1e = i;
				res.set_bit(i);
			}
		}
		else if (res[i / 2]) res.set_bit(i);
	}
	if (!(n & 1)) res.set_bit(0);
}

// Calculates half trace matrix for given degree n.
// Saves result to array of polynoms Th.
void gf2n::calc_half_trace_matrix(int n, lnum Th[])
{
	int i, j, k;
	lnum c(*this);
	n--;
	for (i = 1; i < n; i += 2)
	{
		c.zero();
		c.set_bit(i);
		k = i >> 1;
		Th[k] = c;
		for (j = 1; j <= (n >> 1); j++)
		{
			Th[k] = lnumOperations::sqr(lnumOperations::sqr(Th[k]));
			Th[k] += c;
		}
	}
}

// Calculates square root of x.
// Saves result to polynom sqrt_x.
void gf2n::calc_sqrt_x(lnum &sqrt_x)
{
	int i;
	sqrt_x.zero();
	sqrt_x.set_bit(1);
	for (i = 1; i < non_zero_bits[non_zero_bit_count]; i++) sqrt_x = lnumOperations::sqr(sqrt_x);
}

// Sets the module and does all the necessary initialization calculations.
// Polynom module must generate a field.
// Returns one of the error codes stating the result of operation.
int gf2n::set_module()
{
	int i , j, q, p = 0;
	module.field = this;
	non_zero_bit_count = 0;
	if (module.is_zero()) return lE_DIVZERO;
	for (i = 0; i <= module.l; i++)
	{
		q = module.p.a[i];
		for (j = 0; j < lbLen && q; j++)
		{
			if (q & 1) non_zero_bits[non_zero_bit_count++] = p + j;
			q >>= 1;
		}
		p += lbLen;
	}
	non_zero_bit_count--;
	if ((non_zero_bits[non_zero_bit_count] - non_zero_bits[non_zero_bit_count - 1]) >= lbLen) ldivMode = lMODf; else ldivMode = lMODn;
	calc_trace_vector(module, non_zero_bits[non_zero_bit_count], trace_vector);
	if (non_zero_bits[non_zero_bit_count] & 1) calc_half_trace_matrix(non_zero_bits[non_zero_bit_count], half_trace_array);
	calc_sqrt_x(sqrt_x);

	/* Clean up this mess, wtf is this anyway?? */
	/*MSG_LEN = non_zero_bits[non_zero_bit_count];
	MSG_LEN = (MSG_LEN % lbByte ? MSG_LEN / lbByte + 1 : MSG_LEN / lbByte);
	MSG_LEN_EX = non_zero_bits[non_zero_bit_count] + 1;
	MSG_LEN_EX = (MSG_LEN_EX % lbByte ? MSG_LEN_EX / lbByte + 1 : MSG_LEN_EX / lbByte);*/
	return lE_OK;
}

// Initializes an instance of a class. Used in constructors.
void gf2n::initialize()
{
	non_zero_bit_count = module.non_zero_bits();

	// must be of length (lLen * lbLen + 1) / 2
	half_trace_array = new lnum[module.deg() / 2];
	non_zero_bits = new int[non_zero_bit_count + 1];

	// Set default settings
	loutMode = loutBin;
	ldivMode = lMODn;

	set_module();
}

/* Accessors */

// Returns output mode set for this field.
char gf2n::get_output_mode() const
{
	return loutMode;
}

// Returns division mode set for this field.
char gf2n::get_division_mode() const
{
	return ldivMode;
}

// Returns number of non-zero bits in module polynom minus 1.
int gf2n::get_non_zero_bit_count() const
{
	return non_zero_bit_count;
}

// Returns pointer to array of non-zero bits in module polynom.
const int *gf2n::get_non_zero_bits() const
{
	return non_zero_bits;
}

// Returns degree of module polynom.
int gf2n::get_deg() const
{
	return non_zero_bits[non_zero_bit_count];
}

// Returns k, such as x^k has trace equal to 1.
int gf2n::get_trace_one_element() const
{
	return tr1e;
}

// Returns the field generator itself.
const lnum &gf2n::get_module() const
{
	return module;
}

// Returns calculated trace vector.
const lnum &gf2n::get_trace_vector() const
{
	return trace_vector;
}

// Returns calculated square root from x.
const lnum &gf2n::get_sqrt_x() const
{
	return sqrt_x;
}

// Returns pointer to calculated half-trace polynom array.
const lnum *gf2n::get_half_trace_array() const
{
	return half_trace_array;
}

/* Setter methods */

void gf2n::set_output_mode(int output_mode)
{
	switch (output_mode)
	{
	case loutBin :
	case loutHex :
		loutMode = output_mode;
		break;
	default :
		break;
	}
}

/* Helper methods */

bool gf2n::belongs_to_field(const lnum &a) const
{
	return a.field == this;
}

/* Factory methods */

// Creates a zero polynom, belonging to this field and returns it.
lnum gf2n::create_lnum()
{
	lnum res(*this);
	return res;
}

// Creates a polynom from array m (of length n) of unsigned integers, belonging to this field and returns it.
lnum gf2n::create_lnum(unsigned int m[], int n)
{
	lnum res(m, n, *this);
	return res;
}

// Creates a polynom, belonging to this fiels from its string representation and returns it.
lnum gf2n::create_lnum(char *str)
{
	lnum res(str, *this);
	return res;
}
