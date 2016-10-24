#include "Globals.h"

#include "x1035.h"
#include "MersenneTwister.h"
#include "MGF.h"
#include "BigInteger.h"
#include "sha3.h"
#include "sha.h"
#include "Montgomery.h"


uint32_t Ra_data[] = { 0x389F8B7F, 0xE7F9BB88, 0xF07B3130, 0xE5AD2D4C, 0xE6ABBB09, 0xF4998EA1, 0x9950913E, 0xF4E9F14F, 0x237E7FBC, 0xA793A03A, 0x2CC5B9EA, 0x889ED0FC };
uint32_t Rb_data[] = { 0xFB2A0137, 0xF878C4B3, 0xB9D09511, 0x285954A1, 0xF7BADAFA, 0xF702EF59, 0xFF1816E4, 0xF51F2AE7, 0xFE2D7810, 0x8C006D5F, 0x3039CD1A, 0x474BE8C4 };

uint32_t PW_data[] = { 0x01020304, 0x05060708, 0x09101112, 0x13141516 };

void InitParams(BIGINT *M_PARAMS, BIGINT *S_PARAMS)
{
	uint8_t i;
	BIGINT q, Ra, Rb;

	uint32_t q_data[] = { 0xBEE48FAB, 0x452E0536, 0xF09989B8, 0x9AC65A06, 0x33478906, 0xA3AE846A, 0x3D5A48E3, 0xA26F8267, 0x7C307C0C, 0xCBF04973, 0x094268CC, 0xCB38A8BB, 0x44F507C6, 0xD41C936C, 0xD9DECF3F, 0xD760FFFB, 0x4BDB2D0E, 0x334DF412, 0x959D0AD6, 0x3AEB6CA9, 0xC7280894, 0x78CFE19D, 0x9229F938, 0xBCE6461D, 0x05AFF27D, 0x86702F5F, 0x4F7F924B, 0xCA661E1A, 0x582ECB70, 0xCB410424, 0x2FC33D82, 0xCC13E22E, 0x9AB0E6DE, 0xD372FE71, 0x98169BAF, 0x70285D3A, 0x8911650B, 0x582E15BF, 0xA63B60DE };
	
	q.Length = 39;
	Ra.Length = 12;
	Rb.Length = 12;

	q.Data = q_data;
	Ra.Data = Ra_data;
	Rb.Data = Rb_data;

	InitRandomGenerator(4351);

	for (i = 0; i<12; i++)
	{
		Ra.Data[i] = GenerateRandomNumber();
		Rb.Data[i] = GenerateRandomNumber();
	}
	
	Montgomery_Exponentiation(M_PARAMS[1], q, Ra);
	Montgomery_Exponentiation(S_PARAMS[2], q, Rb);

	// user defined PW value. 4*39 bits = 128 bits length
	memcpy(M_PARAMS[0].Data, PW_data, 4 * 4);
	memcpy(S_PARAMS[0].Data, PW_data, 4 * 4);
}

void PrepareTYPE0(PACKET Message, BIGINT *M_PARAMS)
{
	BIGINT X, H1;
	uint32_t X_data[39], H1_data[43] = { 0, };
	
	// Calculate H1(PW) with the PW stored in master's parameters M_PARAM

	H1.Length = 43;
	H1.Data = H1_data;

	MGF1024(H1.Data, M_PARAMS[0].Data);

	// Multiply H1(PW) and g^Ra mod p
	
	X.Length = 39;
	X.Data = X_data;
	
	Montgomery_Multiplication(X, H1, M_PARAMS[1]);

	// Output is the result of this multiplication.

	Message.Type = 0;
	Message.Counter = 1;
	Message.Length = 39;

	memcpy(Message.Data, X.Data, 39*4);
}

void ProcessTYPE0(PACKET Message, BIGINT *S_PARAMS)
{
	BIGINT X, H1, Rb;
	uint32_t X_data[39], H1_data[43];

	// Calculate H1(PW) with the PW stored in slave's parameters S_PARAM

	H1.Length = 43;
	H1.Data = H1_data;

	MGF1024(H1.Data, S_PARAMS[0].Data);

	// Divide received X to H1(PW) to fetch g^ra mod p
	
	X.Length = 39;
	X.Data = X_data;

	memcpy(X.Data, Message.Data, 39 * 4);
	
	// Store fetched g^ra mod p into slave's parameters S_PARAM

	Montgomery_Divide(S_PARAMS[1], X, H1);

	// Calculate g^(Ra*Rb) mod p, and store it into slave's paramteers S_PARAM

	Rb.Length = 12;
	Rb.Data = Rb_data;

	Montgomery_Exponentiation(S_PARAMS[3], S_PARAMS[1], Rb);
}

void PrepareTYPE1(PACKET Message, BIGINT *S_PARAMS)
{
	BIGINT Y, H2;
	uint32_t Y_data[39], H2_data[43], S1[4];

	// Calculate H1(PW) with the PW stored in slave's parameters S_PARAM

	H2.Length = 43;
	H2.Data = H2_data;

	MGF1024(H2.Data, S_PARAMS[0].Data);

	// Multiply H2(PW) and g^Rb mod p

	Y.Length = 39;
	Y.Data = Y_data;
	
	Montgomery_Multiplication(Y, H2, S_PARAMS[2]);

	// Calculate S1 with the slave's parameters S_PARAM

	sha3((uint8_t*)S1, (uint8_t*)S_PARAMS[0].Data, 400);

	// Output is the concatenation of H2 and S1

	Message.Type = 1;
	Message.Counter = 2;
	Message.Length = 39 + 4;

	memcpy(Message.Data, Y.Data, 39 * 4);
	memcpy(Message.Data + 39, S1, 4 * 4);
}

uint8_t ProcessTYPE1(PACKET Message, BIGINT *M_PARAMS)
{
	BIGINT Y, H2, Ra;
	uint32_t Y_data[39], H2_data[43], S1[4];

	// Calculate H2(PW) with the PW stored in master's parameters M_PARAM

	H2.Length = 43;
	H2.Data = H2_data;

	MGF1024(H2.Data, M_PARAMS[0].Data);

	// Divide received Y to H2(PW) to fetch g^rb mod p

	Y.Length = 39;
	Y.Data = Y_data;

	memcpy(Y.Data, Message.Data, 39 * 4);

	// Store fetched g^rb mod p into master's parameters M_PARAM

	Montgomery_Divide(M_PARAMS[2], Y, H2);

	// Calculate g^(Ra*Rb) mod p, and store it into master's parameters M_PARAM

	Ra.Length = 12;
	Ra.Data = Ra_data;

	Montgomery_Exponentiation(M_PARAMS[3], M_PARAMS[2], Ra);

	// Calculate S1 with the master's parameters M_PARAM

	sha3((uint8_t*)S1, (uint8_t*)M_PARAMS[0].Data, 400);

	// Compare received S1 and calculated S1, and return the result

	return memcmp(S1, Message.Data + 39, 4 * 4) == 0;
}

void PrepareTYPE2(PACKET Message, BIGINT *M_PARAMS)
{
	uint32_t S2[4];
	
	sha3((uint8_t*)S2, (uint8_t*)M_PARAMS[0].Data, 400);

	// Output is S2

	Message.Type = 2;
	Message.Counter = 2;
	Message.Length = 4;

	memcpy(Message.Data, S2, 4 * 4);
}

uint8_t ProcessTYPE2(PACKET Message, BIGINT *S_PARAMS)
{
	uint32_t S2[4];

	sha3((uint8_t*)S2, (uint8_t*)S_PARAMS[0].Data, 400);

	// Compare received S2 and calculated S2, and return the result

	return memcmp(S2, Message.Data, 4 * 4) == 0;
}

uint8_t EstablishKey(uint32_t *KeyMaster, uint32_t *KeySlave)
{
	// M/S_PARAMS[0] = Password				// 128 bits		- 4 int32
	// M/S_PARAMS[1] = q^Ra mod p			// 1248 bits	- 39 int32
	// M/S_PARAMS[2] = q^Rb mod p			// 1248 bits	- 39 int32
	// M/S_PARAMS[3] = q^(Ra*Rb) mod p		// 1248 bits	- 39 int32
	
	
	BIGINT M_PARAMS[4], S_PARAMS[4];

	uint32_t m_ParamData[121];
	uint32_t s_ParamData[121];
	
	PACKET Channel;
	uint32_t Channel_data[68+4];

	Channel.Type = 0;
	Channel.Counter = 0;
	Channel.Length = 0;
	Channel.Data = Channel_data;

	M_PARAMS[0].Length = 4;		M_PARAMS[0].Data = m_ParamData;			S_PARAMS[0].Length = 4;		S_PARAMS[0].Data = s_ParamData;
	M_PARAMS[1].Length = 39;	M_PARAMS[1].Data = m_ParamData + 4;		S_PARAMS[1].Length = 39;	S_PARAMS[1].Data = s_ParamData + 4;
	M_PARAMS[2].Length = 39;	M_PARAMS[2].Data = m_ParamData + 43;	S_PARAMS[2].Length = 39;	S_PARAMS[2].Data = s_ParamData + 43;
	M_PARAMS[3].Length = 39;	M_PARAMS[3].Data = m_ParamData + 82;	S_PARAMS[3].Length = 39;	S_PARAMS[3].Data = s_ParamData + 82;

	InitParams(M_PARAMS, S_PARAMS);

	// Master Calculates Type0 Message, and transmits it to Slave over the channel

	PrepareTYPE0(Channel, M_PARAMS);

	// Slave Receives Type0 Message, and fetches g^ra mod b value
	
	ProcessTYPE0(Channel, S_PARAMS);
	
	// Slave Calculates Type1 Message, and transmits it to Master over the channel
	
	PrepareTYPE1(Channel, S_PARAMS);

	// Master Receives Type1 Message, and fetches g^rb mod b value, and checks validity of the received message
	
	if (ProcessTYPE1(Channel, M_PARAMS))
	{
		/*
		printf("Master's Parameters \n\n");
		Display_BIGD(M_PARAMS[0]);	printf("\n\n");
		Display_BIGD(M_PARAMS[1]);	printf("\n\n");
		Display_BIGD(M_PARAMS[2]);	printf("\n\n");
		Display_BIGD(M_PARAMS[3]);	printf("\n\n");

		printf("Slave's Parameters \n\n");
		Display_BIGD(S_PARAMS[0]);	printf("\n\n");
		Display_BIGD(S_PARAMS[1]);	printf("\n\n");
		Display_BIGD(S_PARAMS[2]);	printf("\n\n");
		Display_BIGD(S_PARAMS[3]);	printf("\n\n");
	
		printf("S1 is acknowledged \n\n");
		*/
		// Master Calculates Type2 Message, and transmits it to Slave over the channel
		
		PrepareTYPE2(Channel, M_PARAMS);

		// Slave and checks validity of the received message

		if (ProcessTYPE2(Channel, S_PARAMS))
		{
			/*printf("S2 is acknowledged \n\n");*/

			// Both sides calculate secret key

			sha1((uint8_t*)(KeyMaster), (uint8_t*)M_PARAMS[0].Data, 400);
			sha1((uint8_t*)(KeySlave), (uint8_t*)S_PARAMS[0].Data, 400);

			return 1;
		}
		else return 0;
	}
	else return 0;
}


