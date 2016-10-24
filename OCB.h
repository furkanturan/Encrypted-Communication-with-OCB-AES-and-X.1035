#ifndef _OCB_H_
#define _OCB_H_

#include "Globals.h"

#define OCB_ENCRYPT 1
#define OCB_DECRYPT 0

#define KEY_LENGTH		16	// 128 bits for AES
#define NONCE_LENGTH	12	// 96 bits as it is recommended by OCB
#define TAG_LENGTH		16	// 128 bits is chosen

int OCB(uint8_t *ciphertext, uint32_t *key, uint32_t *nonce, uint32_t *associateddata, uint8_t ad_block_count, uint8_t *plaintext, uint8_t plaintext_length, uint8_t mode);

#endif
