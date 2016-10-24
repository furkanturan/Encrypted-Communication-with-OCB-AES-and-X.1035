#ifndef _MERSENNE_TWISTER_H_
#define _MERSENNE_TWISTER_H_

#include "Globals.h"

void InitRandomGenerator(uint32_t seed);
uint32_t GenerateRandomNumber(void);

#endif