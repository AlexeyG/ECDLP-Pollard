#ifndef _PARALLELDEFINES_H
#define _PARALLELDEFINES_H

/*************************/
/* General configuration */
/*************************/

// Rank of manager process (it will only when set to zero).
#define MANAGER_RANK 0

// Define sleep time in a loop.
#define LOOP_SLEEP  200

// Pattern for message ID tags
#define PARALLEL_TAG_PATTERN                 0x00100000

/**********************************/
/* MPI message tags configuration */
/**********************************/

// Polynom message TAG
#define PARALLEL_POLYNOM_TAG                 0x06000000

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

// Abort message TAG
#define PARALLEL_ABORT_TAG                   0x01000000

/***************************/
/* Constants configuration */
/***************************/

// Maximum allowed line length (this amount will be allocated for reading lines from files).
#define LINE_LEN    256

#endif