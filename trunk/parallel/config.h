#ifndef _CONFIG_H
#define _CONFIG_H

/****************************/
/* File names configuration */
/****************************/

// Input file name - field and curve data aswell as points G, aG and bG are read from this file.
// This file is usually prepared by ecc executable.
#define INPUT_FILE_NAME  "output.txt"

// Config gile name - parallel configuration is stored in this file.
#define CONFIG_FILE_NAME "config.txt"

// Output file name - decrypted data is stored in this file.
#define OUTPUT_FILE_NAME "dec.txt"

// Cipher file name - file, from which encrypted data is written (it will be decrypted when cipher is cracked).
#define CIPHER_FILE_NAME "enc.txt"

/***************************/
/* Constants configuration */
/***************************/

// Maximum allowed line length (this amount will be allocated for reading lines from files).
#define LINE_LEN    256

// Buffer size in bytes.
#define BUFFER_SIZE (1024 * 1024 * 8)

#endif
