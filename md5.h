/* 
 * md5.h - md5 library
 **/

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
 * rights reserved.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 **/

/* The following makes MD default to MD5 if it has not already been
 * defined with C compiler flags.
 **/

#ifndef __M_MD5_h__
#define __M_MD5_h__

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

typedef struct {
    UINT4 state[4];                  /* state (ABCD) */
    UINT4 count[2];                  /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];        /* input buffer */
} MD5_CTX;

/* 
 * MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (MD5_CTX *context);

/* 
 * MD5 block update operation. Continues an MD5 message-digest
 * operation, processing another message block, and updating the
 * context.
 */
void MD5Update (MD5_CTX *context, unsigned char *input,unsigned int inputLen);

/* 
 * MD5 finalization. Ends an MD5 message-digest operation, writing the
 * the message digest and zeroizing the context.
 */
void MD5Final (unsigned char digest[16], MD5_CTX *context);

/* 
 * Digests a string and prints the result.
 */
char* MDString (char *string);

/*
 * Digests a file and prints the result.
 */
char* MDFile (char *filename);

#endif /* __M_MD5_h__ */
