#ifndef _PARALLELDEFINES_H
#define _PARALLELDEFINES_H

/*************************/
/* General configuration */
/*************************/

// Rank of manager process (it will only when set to zero).
#define MANAGER_RANK       0

// Define sleep time in a loop.
#define LOOP_SLEEP         200

// Number of random points in interation function
#define PARALLEL_SET_COUNT 16

// Mask for Pollard's partitioning function
#define PARALLEL_SET_ARG   0xF

// Pattern for message ID tags
#define PARALLEL_TAG_PATTERN                 0x00100000

/**********************************/
/* MPI message tags configuration */
/**********************************/

// Big integer array length MODIFIER TAG
#define PARALLEL_BINT_LENGTH_TAG             0x00000001

// Pollard's order parameter TAG
#define PARALLEL_GROUP_ORDER_TAG             0x09000000

// Initial point big integer TAG
#define PARALLEL_INITIAL_POINT_BINT_TAG      0x08000000

// Interation function coefficient
#define PARALLEL_ITERATION_COEF_TAG          0x07000000

// Polynom message TAG
#define PARALLEL_POLYNOM_TAG                 0x06000000

// Elliptic point Y coordinate message TAG
#define PARALLEL_POLYNOM_POINT_X_TAG         0x06100000

// Elliptic point X coordinate message TAG
#define PARALLEL_POLYNOM_POINT_Y_TAG         0x06200000

// Curve message TAG
#define PARALLEL_CURVE_TAG                   0x05000000

// Curve coefitiend a message TAG
#define PARALLEL_CURVE_A_TAG                 0x05100000

// Curve coefitiend b message TAG
#define PARALLEL_CURVE_B_TAG                 0x05200000

// Curve facorization length TAG
#define PARALLEL_CURVE_FACTOR_LENGTH_TAG     0x05300000

// Curve facorization unit TAG
#define PARALLEL_CURVE_FACTOR_TAG            0x05400000

// Field message TAG
#define PARALLEL_FIELD_TAG                   0x04000000

// Field output mode message TAG
#define PARALLEL_FIELD_OUTPUT_MODE_TAG       0x04100000

// Master count message TAG
#define PARALLEL_MASTER_COUNT_TAG            0x03000000

// Condition prefix length message TAG
#define PARALLEL_CONDITION_PREFIX_LENGTH_TAG 0x02000000

// Control message TAG
#define PARALLEL_CONTROL_TAG                 0x01000000

/**************************************/
/* MPI control messages configuration */
/**************************************/

// No message (returned when we have no message on input)
#define PARALLEL_NO_CONTROL_MESSAGE         0x00000000

// Init message (before we send initialization data)
#define PARALLEL_INIT_CONTROL_MESSAGE       0x00001000

// Done message (when Pollard for a singe subgroup finishes)
#define PARALLEL_DONE_CONTROL_MESSAGE       0x00002000

// Abort message (when we just finish doing everything)
#define PARALLEL_ABORT_CONTROL_MESSAGE      0x00003000

/***************************/
/* Constants configuration */
/***************************/

// Maximum allowed line length (this amount will be allocated for reading lines from files).
#define LINE_LEN    256

#endif