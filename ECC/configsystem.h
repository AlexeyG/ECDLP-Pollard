#ifndef _CONFIGSYSTEM_H
#define _CONFIGSYSTEM_H

/****************************/
/* File names configuration */
/****************************/

// Input file name - field and curve data must be stored in this file.
#define INPUT_FILE_NAME "input.txt"

// Output file name - field and curve data is output to this file. It is then used for cracking the cipher.
#define OUTPUT_FILE_NAME "output.txt"

// Message file name - contains data we want to encrypt.
#define TEXT_FILE_NAME "msg.txt"

// Cipher file name - file, to which encrypted data is written (it will be decrypted when cipher is cracked).
#define CIPHER_FILE_NAME "enc.txt"

/***************************/
/* Constants configuration */
/***************************/

// Maximum allowed line length (this amount will be allocated for reading lines from files).
#define LINE_LEN    256

// Maximum possible number of factors (used when calculating factorization on our own).
#define MAX_FACTORS 100


/************************/
/* Output configuration */
/************************/

// Output mode for applications field.
#define FIELD_OUTPUT_MODE loutHex

#endif
