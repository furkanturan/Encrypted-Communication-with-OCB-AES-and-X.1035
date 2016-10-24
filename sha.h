/* sha.h */

#ifndef _SHA_H_
#define _SHA_H_ 1

#include "Globals.h"

/* The structure for storing SHS info */

typedef struct 
{
        uint32_t digest[ 5 ];            /* Message digest */
        uint32_t countLo, countHi;       /* 64-bit bit count */
        uint32_t data[ 16 ];             /* SHS data buffer */
} SHA_CTX;

/* Message digest functions */

void SHAInit(SHA_CTX *);
void SHAUpdate(SHA_CTX *, uint8_t *buffer, int count);
void SHAFinal(uint8_t *output, SHA_CTX *);

void sha1(uint8_t *hashResult, uint8_t *data, int nofDataBytes);

#endif /* end _SHA_H_ */

