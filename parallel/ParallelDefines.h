#ifndef _PARALLELDEFINES_H
#define _PARALLELDEFINES_H

/*************************/
/* General configuration */
/*************************/

// Rank of manager process (it will only when set to zero).
#define MANAGER_RANK                  0

// Define sleep time in a loop.
#define LOOP_SLEEP                    200

// Number of random points in interation function
#define PARALLEL_SET_COUNT            16

// Mask for Pollard's partitioning function
#define PARALLEL_SET_ARG              0xF

// Minimum order to call sequential Pollard's algorithm instead of parallel one.
#define PARALLEL_SEQUENTIAL_MIN_ORDER bint(240000000)

/**********************************/
/* MPI message tags configuration */
/**********************************/

// Source pattern for message ID tags
#define PARALLEL_TAG_SOURCE_PATTERN          0x000000FF

// Bit shift for source pattern for message ID tags
#define PARALLEL_TAG_SOURCE_SHIFT            4

// Index pattern for message ID tags
#define PARALLEL_TAG_INDEX_PATTERN           0x0000FFFF

// Pattern for message ID tags
#define PARALLEL_TAG_PATTERN                 0x00FFFFFF

// Iteration function message GROUP TAG
#define PARALLEL_ITERATION_FUNCTION_GROUP    0x7F000000

// Class configuration message GROUP TAG
#define PARALLEL_CONFIG_GROUP                0x7E000000

// Initial point message GROUP TAG
#define PARALLEL_INITIAL_POINT_GROUP         0x7D000000

// Collision solution message GROUP TAG
#define PARALLEL_SOLUTION_GROUP              0x7C000000

// Integer length message TAG
#define PARALLEL_LENGTH_TAG                  0x09000000

// ParallelData message TAG
#define PARALLEL_PARALLELDATA_TAG            0x08000000

// bfactor message TAG
#define PARALLEL_BFACTOR_TAG                 0x07000000

// gf2n message TAG
#define PARALLEL_GF2N_TAG                    0x06000000

// ecurve message TAG
#define PARALLEL_ECURVE_TAG                  0x05000000

// epoint message TAG
#define PARALLEL_EPOINT_TAG                  0x04000000

// bint message TAG
#define PARALLEL_BINT_TAG                    0x03000000

// lnum message TAG
#define PARALLEL_LNUM_TAG                    0x02000000

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
#define LINE_LEN         256

// Maximum allowed integer
#define PARALLEL_MAX_INT 0x0FFFFFFF

#endif