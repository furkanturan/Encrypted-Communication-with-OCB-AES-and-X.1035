#include "MGF.h"
#include "sha.h"

void MGF1024(uint32_t *Destination, uint32_t *Source)
{
	uint8_t i, counter = 0;

	uint32_t ModifiedSource[] = { Source[0], Source[1], Source[2], Source[3] };
	uint32_t tmp[4];

	for (i = 0; i < 10; i++)
	{
		ModifiedSource[0] += counter++;

		sha1((uint8_t*)(Destination) + i * 16, (uint8_t*)ModifiedSource, 16);
	}

	ModifiedSource[0] += counter++;

	sha1((uint8_t*)(tmp), (uint8_t*)ModifiedSource, 1);

	memcpy((uint8_t*)(Destination)+i * 16, tmp, 12);	
}

