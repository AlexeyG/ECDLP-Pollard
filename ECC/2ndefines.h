#ifndef _2NDEFINES_H
#define _2NDEFNES_H

/******************************/
/* Input Output configuration */
/******************************/

// Leading charter for binary output\input
#define lBinChar 'b'

// Leading charter for hexademical output\input
#define lHexChar 'x'

/****************************/
/* lnum class configuration */
/****************************/

// Length of polynoms in unsigned integers
#define lLen     100

/*****************************************/
/* Karacuba multiplication configuration */
/*****************************************/

// Lower bound for using Karacuba multiplication. Used in kmul operation.
#define lMaxkmul 100

/*****************************/
/* Storage types information */
/*****************************/

// Number of binary digits in an unsigned integer
#define lbLen    32

// Number of bits in byte
#define lbByte   8

// Number of hexademical digits in an unsigned integer
#define lhLen    8

// Number of bits in a hexademical digit
#define lbHex    4

/*************************************/
/* Enum-like constants configuration */
/*************************************/
/* Do not edit this                  */
/*************************************/

// Use fast division (does not work with all field generators) - is fast only for small weight field generators
#define lMODf  0

// Use normal division method
#define lMODn  1

// Output data in binary format by default
#define loutBin 0

// Output data in hexademical format by default
#define loutHex 1

// Default data output mode
#define loutDefault loutBin

/****************************/
/* Error code configuration */
/****************************/

// Operation completed successfully
#define lE_OK          0

// Error: overflow occured
#define lE_OVERFLOW   -1

// Error: division by zero
#define lE_DIVZERO    -2

// Error: no inverse exists for elements
#define lE_NOINVERSE  -3

// Error: access violation
#define lE_ACC        -4

// Error: input\output error
#define lE_IO         -5

// Error: no field error
#define lE_NULLFIELD  -6

// Error: operand field mismatch
#define lE_DIFFFIELD - 7

// Error: no solution exists (for equation)
#define lE_NOSOLUTION -8

/****************************/
/* Other configuration      */
/****************************/

// Should we build multiplication and squaring tables? Undefine this to build them.
#define lHEADER_TABLES

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef lHEADER_TABLES
extern bool initTm = false;
#endif

extern unsigned short Tm[1 << 8][1 << 8]; // multiplication table
extern unsigned int   Ts[1 << 16];        // squaring table
extern unsigned short Td[1 << 8][1 << 8]; // division table
#endif
