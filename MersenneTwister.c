#include "Globals.h"
#include "MersenneTwister.h"


#define N 624
#define M 397

uint32_t MT[N];				// State vector
uint32_t index2 = N + 1;	// Set index2 to N+1, which means uninitialised

void InitRandomGenerator(uint32_t seed)
{
	MT[0] = seed & 0xffffffff;
	for (index2 = 1; index2<N; index2++)
	{
		MT[index2] =
			(1812433253 * (MT[index2 - 1] ^ (MT[index2 - 1] >> 30)) + index2);
		MT[index2] &= 0xffffffff;
	}
}

uint32_t GenerateRandomNumber(void)
{
	unsigned long y;

	// generate N words

	if (index2 >= N)
	{
		int j;

		// if unititialised, initialise it with preset seed
		if (index2 == N + 1)
			InitRandomGenerator(5489);

		for (j = 0; j<N - M; j++)
		{
			y = (MT[j] & 0x80000000) | (MT[j + 1] & 0x7fffffff);
			MT[j] = MT[j + M] ^ (y >> 1);
		}

		if (y % 2 != 0)
		{
			MT[j] = MT[j] ^ 0x9908b0df;
		}

		index2 = 0;
	}

	y = MT[index2++];

	// Break Linearity
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= (y >> 18);

	return y;
}