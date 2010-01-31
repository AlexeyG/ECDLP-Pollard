#ifndef _CRACK_DEFINESH
#define _CRACK_DEFINESH

/****************************/
/* Error code configuration */
/****************************/

// Operation completed successfully.
#define ckE_OK          0

// Error: attempting to perform operation on points, belonging different curves.
#define ckE_DIFFCURVES -1

// Error: invalid operation on working solution.
#define ckE_RUNNING    -2

/***************************/
/* Constants configuration */
/***************************/

// Maximum attempts to try running Pollards rho-methods before signaling failure.
#define MAX_POLLARD_ATTEMPTS 3

// Maximum allowed line length (this amount will be allocated for formated output using sprintf).
#define LINE_LEN             256

// Format used for printing time in verbose mode.
#define PRINT_FORMAT         "%.5lf"

// Minimum group order required to run Pollards rho-method. It must be a big intger.
#define POLLARD_MIN_ORDER    bint(1000)

// Number of sets used in Pollards rho-method.
#define POLLARD_SET_COUNT    16

// This argument is passed to epoint objects' f method (which is a bitwise and).
// The result of f fuction must be no more than POLLARD_SET_COUNT constant.
#define POLLARD_SET_ARG      0xF

#endif