#include "Globals.h"
#include "Montgomery.h"

#define MONT_BUFF_SIZE (39+4)

uint32_t R2_1376_mod_p[]	= { 0xB96EB342, 0x0219BCAA, 0xC9B86ED6, 0x5083C1AC, 0x3CD242FD, 0x879E77A7, 0x35770033, 0xB494AB4B, 0x982DF3DC, 0x2DA4A0D7, 0xBC322911, 0x74051AF4, 0x6096EC5E, 0xE8FE9507, 0x93DE34F5, 0x376CBA0F, 0xA285ED68, 0xAB8D7AAB, 0x5F29A839, 0x94FAFB61, 0xF562DF28, 0x9CAF8648, 0xA9560628, 0x574BC529, 0x60C0AE1A, 0x3F77B875, 0x21890320, 0xFFF83941, 0x1AEB10D6, 0x4B80F7AB, 0x8682471F, 0x14CF91DF, 0xF45BA72D, 0x50101A54, 0xC2644484, 0x4DC5F8CE, 0xD319F29B, 0xC386F3B4, 0x49D5ED06, 0, 0, 0, 0 };
uint32_t R2_1248_mod_p[]	= { 0x9F4F8855, 0xA2450D08, 0x98930098, 0xBD4EE698, 0x49E450DE, 0x91D1AEE6, 0x7156ACBB, 0xA6F811A1, 0x2AC40DDE, 0x07D5C700, 0xFC25AE4A, 0xBCB16331, 0x8820C76C, 0xBA7E196E, 0xC4D9BEF5, 0x41496B32, 0x5CE64597, 0xD99C5A20, 0x2D45DB7C, 0xC249B6CE, 0xA519024C, 0x657074EF, 0xC81B5D97, 0x275294A2, 0xCF596975, 0x5BCEBBB9, 0x9934ABEC, 0x0AE72F1C, 0x8191627E, 0xD23C210F, 0x4C1B7C2D, 0x24292EB7, 0xF22172A3, 0x1268C852, 0x4CCB2393, 0xDA40CE5F, 0xA4F47C07, 0x9F635012, 0x150D7886, 0, 0, 0, 0 };
uint32_t R_1248_mod_p[]		= { 0x7E16B459, 0x61D5440E, 0xB691F90F, 0x5AA3E64F, 0xD741BFF3, 0xFF6C7BE2, 0xA15044BC, 0x8CD3716D, 0x9061D760, 0x1993C4C9, 0xDCBEA4E5, 0xB9C1CFF2, 0xDBCFCDB9, 0x95C669D8, 0x92630A96, 0x4D48E4D8, 0xD5C94308, 0x08FD2907, 0x8DCECF4B, 0x71AE64D5, 0xC244EDA2, 0xF68A9DB7, 0x90654502, 0x0B244BF7, 0x578FC054, 0x948758A0, 0xB3E2723E, 0xDD86BA00, 0xEDA9F505, 0x197F3B86, 0x43A2B760, 0x6F344150, 0xF7718A27, 0x3F9E13A9, 0xAA6E9DE0, 0x3E21093C, 0x543B713C, 0x48873A2D, 0x567672A4, 0, 0, 0, 0 };
uint32_t Modulus[]			= { 0x81E94BA7, 0x9E2ABBF1, 0x496E06F0, 0xA55C19B0, 0x28BE400C, 0x0093841D, 0x5EAFBB43, 0x732C8E92, 0x6F9E289F, 0xE66C3B36, 0x23415B1A, 0x463E300D, 0x24303246, 0x6A399627, 0x6D9CF569, 0xB2B71B27, 0x2A36BCF7, 0xF702D6F8, 0x723130B4, 0x8E519B2A, 0x3DBB125D, 0x09756248, 0x6F9ABAFD, 0xF4DBB408, 0xA8703FAB, 0x6B78A75F, 0x4C1D8DC1, 0x227945FF, 0x12560AFA, 0xE680C479, 0xBC5D489F, 0x90CBBEAF, 0x088E75D8, 0xC061EC56, 0x5591621F, 0xC1DEF6C3, 0xABC48EC3, 0xB778C5D2, 0xA9898D5B, 0, 0, 0, 0 };
uint32_t Sprime0 = 0x502553E9;

void Montgomery_Multiplication(BIGINT Output, BIGINT Input1, BIGINT Input2)
{
	uint8_t MONT_SIZE;
	uint16_t k, l;

	uint32_t M;
	uint64_t tmp = 0;

	uint8_t rgm = 0, Pk;

	// Initialize all result addresses to zero
	uint32_t res[MONT_BUFF_SIZE * 2] = { 0, };

	MONT_SIZE = Input1.Length;
	if (Input2.Length > MONT_SIZE) MONT_SIZE = Input2.Length;

	// Calculate multi-precision multiplication and montgomery result
	// together in single outer loop

	for (k = 0; k < Input1.Length; k++)
	{
		// Calculate multi-precision multiplication
		tmp = 0;

		for (l = 0; l < Input2.Length; l++)
		{
			tmp = ((uint64_t)Input1.Data[k] * (uint64_t)Input2.Data[l]) + (uint64_t)res[k + l] + (uint64_t)tmp;

			res[k + l] = tmp;
			tmp >>= 32;
		}

		res[k + l] += tmp;
	}

	for (k = 0; k < MONT_SIZE; k++)
	{
		// Calculate montgomery multiplication,
		// using  previous result
		tmp = 0;

		M = res[k] * Sprime0;

		for (l = 0; l < MONT_SIZE; l++)
		{
			tmp += (uint64_t)res[k + l] + ((uint64_t)M * (uint64_t)Modulus[l]);
			res[k + l] = tmp;
			tmp >>= 32;
		}

		Pk = (uint8_t)k;
		while (tmp != 0)
		{
			tmp += res[Pk + MONT_SIZE];
			res[Pk + MONT_SIZE] = tmp;
			tmp >>= 32;
			Pk++;

			if (Pk == MONT_SIZE)
				break;
		}
	}

	// In final step, decide if result is greater than the modulus
	// and put result into RGM (Result is Greater than Modulus) variable.

	rgm = tmp;

	k = MONT_SIZE - 1;
	while (rgm == 0 && k > 0)
	{
		if (res[k + MONT_SIZE] > Modulus[k])
			rgm = 1;
		else if (Modulus[k] > res[k + MONT_SIZE])
			break;
		else
			k--;
	}

	// If Result is Greater than Modulus,
	// Substract modulus from it

	if (rgm == 1)
	{
		tmp = 0;

		for (k = 0; k < MONT_SIZE; k++)
		{
			tmp += (uint64_t)res[k + MONT_SIZE] - (uint64_t)Modulus[k];

			res[k + MONT_SIZE] = tmp;

			if ((tmp & 0x8000000000000000) == 0)
				tmp >>= 32;
			else
				tmp = 0xFFFFFFFFFFFFFFFF;
		}
	}

	if (Input1.Length < Input2.Length)	l = Input1.Length;
	else l = Input2.Length;

	for (k = 0; k < l; k++)
	{
		Output.Data[k] = res[k + MONT_SIZE];
	}
}

void Montgomery_Exponentiation(BIGINT Output, BIGINT X, BIGINT Exponent)
{
	char i, j;
	uint8_t k, firstone = 0, bit;

	BIGINT A, Xtilde, R2modP;
		
	uint32_t A_data[MONT_BUFF_SIZE];
	uint32_t Xtilde_data[MONT_BUFF_SIZE];

	R2modP.Length = X.Length;
	A.Length = X.Length;
	Xtilde.Length = A.Length;

	R2modP.Data = R2_1248_mod_p;
	A.Data = A_data;
	Xtilde.Data = Xtilde_data;

	Montgomery_Multiplication(Xtilde, X, R2modP);

	memcpy(A.Data, R_1248_mod_p, X.Length * 4);
	
	for (i = 11; i >= 0; i--)
	{
		for (j = 31; j >= 0; j--)
		{
			bit = (Exponent.Data[i] & (0x01 << j)) > 0 ? 1 : 0;

			if (firstone == 0 && bit == 1)
			{
				firstone = 1;
			}

			if (firstone == 1)
			{
				Montgomery_Multiplication(A, A, A);

				if (bit == 1)
				{
					Montgomery_Multiplication(A, A, Xtilde);
				}
			}
		}
	}

	memset(Xtilde.Data, 0, X.Length * 4);
	Xtilde.Data[0] = 1;
	
	Montgomery_Multiplication(A, A, Xtilde);

	for (k = 0; k < X.Length; k++)
	{
		Output.Data[k] = A.Data[k];
	}
	
}

void Montgomery_Inverse(BIGINT Output, BIGINT A)
{
	//  Phase I
	//	Input: a ∈[1, p − 1] and p
	//	Output : r ∈[1, p − 1] and k, where r = a−1^2^k (mod p) and n ≤ k ≤ 2n
	//
	//	 1 : u : = p, v : = a, r : = 0, and s : = 1
	//	 2 : k : = 0
	//	 3 : while (v > 0)
	//	 4 :	if u is even then u : = u / 2, s : = 2s
	//	 5 :	else if v is even then v : = v / 2, r : = 2r
	//	 6 :	else if u > v then u : = (u − v) / 2, r : = r + s, s : = 2s
	//	 7 :	else if v ≥ u then v : = (v − u) / 2, s : = s + r, r : = 2r
	//	 8 :	k : = k + 1
	//	 9 : if r ≥ p then r : = r − p
	//	 10 : return r : = p − r and k
	
	// STEP 1, 2
	BIGINT u, v, r, s, p;

	uint32_t u_data[MONT_BUFF_SIZE+1], v_data[MONT_BUFF_SIZE+1], r_data[MONT_BUFF_SIZE+1] = { 0, }, s_data[MONT_BUFF_SIZE+1] = { 0, }, p_data[MONT_BUFF_SIZE+1], k = 0;
	
	uint32_t i;

	p.Length = A.Length + 1;	p.Data = p_data;
	u.Length = A.Length + 1;	u.Data = u_data;
	v.Length = A.Length + 1;	v.Data = v_data;
	r.Length = A.Length + 1;	r.Data = r_data;
	s.Length = A.Length + 1;	s.Data = s_data;

	memcpy(p.Data, Modulus, p.Length * 4);
	memcpy(u.Data, p.Data, (p.Length-1) * 4);
	memcpy(v.Data, A.Data, A.Length * 4);
	p.Data[p.Length - 1] = 0;
	u.Data[p.Length - 1] = 0;
	v.Data[p.Length - 1] = 0;

	s_data[0] = 1;

	// STEP 3
	
	while (isGreaterThanZero(v))
	{
		// STEP 4
		if (u.Data[0] % 2 == 0)
		{
			ShiftRight(u);
			ShiftLeft(s);
		}
		// STEP 5
		else if (v.Data[0] % 2 == 0)
		{
			ShiftRight(v);	
			ShiftLeft(r);
		}
		// STEP 6
		else if (isAlignedGreater(u, v) == 1)
		{
			Subtract(u, u, v); 
			ShiftRight(u);
			Add(r, r, s); 
			ShiftLeft(s);
		}
		// STEP 7
		else							
		{
			Subtract(v, v, u); 
			ShiftRight(v);
			Add(s, s, r); 
			ShiftLeft(r); 
		}

		// STEP 8
		k++;
	}

	// STEP 9
	if (!isAlignedGreater(p, r))	
		Subtract(r, r, p);

	// STEP 10
	Subtract(r, p, r);

	//	Phase II
	//	Input : r ∈[1, p − 1], p, and k from Phase I
	//	Output : x ∈[1, p − 1], where x = a−12n(mod p)
	//	11 : for i = 1 to k − n do
	//	12 :	if r is even then r : = r / 2
	//	13 :	 else then r : = (r + p) / 2
	//	13 : return x : = r

	// STEP 11
	for (i = 1; i <= k - 1376; i++)
	{
		// STEP 12
		if (r.Data[0] % 2 == 0)
		{
			ShiftRight(r);
		}
		// STEP 13
		else
		{
			Add(r, r, p);
			ShiftRight(r);
		}
	}

	memcpy(Output.Data, r.Data, A.Length * 4);
}

void Montgomery_Divide(BIGINT Quotient, BIGINT Divident, BIGINT Divisor)
{
	// This function modifies Divisor

	BIGINT R2modP;

	R2modP.Length = Divisor.Length;
	R2modP.Data = R2_1376_mod_p;

	Montgomery_Inverse(Divisor, Divisor);
	
	Montgomery_Multiplication(Divisor, Divisor, R2modP);

	Montgomery_Multiplication(Quotient, Divisor, Divident);
}