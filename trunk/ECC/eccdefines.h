#ifndef _ECCDEFINES_H
#define _ECCDEGINES_H

/******************************/
/* ecurve class configuration */
/******************************/

// Number of bits required to definatly mark data on our curve.
#define pMarkBits 4

/*************************************/
/* Enum-like constants configuration */
/*************************************/
/* Do not edit this                  */
/*************************************/

/****************************/
/* Error code configuration */
/****************************/

// Operation completed successfully.
#define pE_OK           0

// Error: attempting to perform operation on points, belonging different curves.
#define pE_DIFFCURVES  -1

// Error: attempting to perform operation on different points, belonging to no curve.
#define pE_UNASSIGNED  -2

// Error: invalid operation on infinity point.
#define pE_INF         -3

// Error: error occured while packing a point.
#define pE_PACKERROR   -4

/* Ecurve error codes */

// Error: unable to solve internal equation, when unpacking point.
#define pE_UNPACKERROR -5

// Error: invalid operation on polyom, owned by another field (other than our curves field).
#define pE_DIFFFIELD   -6

/***************************/
/* Pack code configuration */
/***************************/

// Error: internal error. Should never happen.
#define ppE_INTERNAL   -2

// Error: equation, used in packing has no solution. Should never happen.
#define ppE_NOSOL      -1

// Error: can not pack infinity point.
#define ppE_INF         0

// Operation complete successfully - result is first solution.
#define ppE_1ST         1

// Operation complete successfully - result is second solution.
#define ppE_2ND         2

#endif
