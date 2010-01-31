#ifndef _BINTDEFINES_H
#define _BINTDEFINES_H

/****************************/
/* bint class configuration */
/****************************/

// Length of big integers.
#define bLen     250

// Calculation module (for faster arithmetics).
#define bMod     10000

// Number of digits to store in one array element.
#define bModLen  4

/*************************************/
/* Enum-like constants configuration */
/*************************************/
/* Do not edit this                  */
/*************************************/

/****************************/
/* Error code configuration */
/****************************/

// Operation completed successfully
#define bE_OK            0

// Error: overflow occured
#define bE_OVERFLOW     -1

// Error: division by zero
#define bE_DIVZERO      -2

// Error: internal error
#define bE_INTERNAL     -3

// Error: no inverse element exists
#define bE_NOINVERSE    -4

// Error: access violation
#define bE_ACCVIOLATION -5

#endif
