#include "BigInteger.h"

uint32_t cnt = 0;

void BIGINT_from_String(uint32_t *destination, int length, char* num)
{
	//str1 is the raw data in hex without 0x.
	uint8_t counter = 0;
	uint32_t val;
	int i = 0;
	char hex[8];
	char *ptr;

	for (i = length * 2 - 1; i >= 0; i = i - 8)
	{
		hex[0] = num[i - 7];
		hex[1] = num[i - 6];
		hex[2] = num[i - 5];
		hex[3] = num[i - 4];
		hex[4] = num[i - 3];
		hex[5] = num[i - 2];
		hex[6] = num[i - 1];
		hex[7] = num[i];

		val = (uint32_t)strtoll(hex, &ptr, 16);

		destination[counter] = val;

		counter++;
	}
}

void Display_BIGD(BIGINT Data)
{
	uint8_t i = 0;

	printf("%d - ", Data.Length);

	printf("0x");

	for (i = 0; i < Data.Length; i++)
	{
		printf("%08x", Data.Data[Data.Length - i - 1]);
	}
}

uint8_t Add(BIGINT Output, BIGINT Input1, BIGINT Input2)
{
	
	uint8_t i;
	uint8_t carry = 0;

	for (i = 0; i < Input1.Length; i++)
	{
		Input1.Data[i] += Input2.Data[i];
		Input1.Data[i] += carry;

		carry = (Input1.Data[i] < Input2.Data[i]);
	}

	return carry;
}

uint8_t Subtract(BIGINT Output, BIGINT Input1, BIGINT Input2)
{
	uint8_t i;
	uint32_t tempResult = 0;

	for (i = 0; i < Input1.Length * 2; i++)
	{
		tempResult += *(((uint16_t*)Input1.Data) + i) - *(((uint16_t*)Input2.Data) + i);

		*(((uint16_t*)Output.Data) + i) = tempResult;

		if ((tempResult & 0x80000000) == 0)
			tempResult >>= 16;
		else
			tempResult = 0xFFFFFFFF;

	}

	return ((uint8_t)tempResult == 0xFF);
}

uint8_t Subtract2(BIGINT Output, BIGINT Input1, BIGINT Input2)
{
	uint8_t i;
	uint8_t carry = 0;

	for (i = 0; i < Input1.Length; i++)
	{
		Input1.Data[i] -= carry;

		if ((int32_t)Input1.Data[i] < (int32_t)Input2.Data[i])
			carry = 1;
		else
			carry = 0;

		Input1.Data[i] -= Input2.Data[i];
	}

	return carry;
}

void Multiply(BIGINT Output, BIGINT Input1, BIGINT Input2)
{
	uint8_t k, l;
	uint32_t tmp;

	for (k = 0; k < Input1.Length + Input2.Length; k++)
	{
		Output.Data[k] = 0;
	}

	for (k = 0; k < Input1.Length * 2; k++)
	{
		tmp = 0;

		for (l = 0; l < Input2.Length * 2; l++)
		{
			tmp = *(((uint16_t*)Input1.Data) + k) * *(((uint16_t*)Input2.Data) + l) + *(((uint16_t*)Output.Data) + k + l) + tmp;

			*(((uint16_t*)Output.Data) + k + l) = tmp;
			tmp >>= 16;
		}

		*(((uint16_t*)Output.Data) + k + l) += tmp;
	}
}

void Divide(BIGINT Quotient, BIGINT Divident, BIGINT Divisor)
{
	uint8_t j, i, l;
	uint32_t tmp = 0;
	uint16_t temp1[3] = { 0, };

	BIGINT temp2;
	uint32_t temp2data[MAX_ARRAY_SIZE] = { 0, };
	temp2.Data = temp2data;

	uint8_t n = Divident.Length * 2 - 1;
	uint8_t t = Divisor.Length * 2 - 1;

	/*
	BIGINT Quotient;
	uint32_t Qdata[MAX_ARRAY_SIZE] = { 0, };
	Quotient.Length = Quotient2.Length;
	Quotient.Data = Qdata;
	*/
	
	// STEP 1.
	// For j from 0 to (n-t) do: Quotient[j] = 0

	for (j = 0; j < n - t; j++)
		*((uint16_t*)Quotient.Data + j) = 0;

	// STEP 2.
	// While Divident > Divisor * b^(n-t) :
	//		Quotient[n-t] = Quotient[n-t] + 1
	//		Divident = Divident - Divisor * b^(n-t)
	// 
	// Note: Multiplying Divisor with b^(n-t) means, aligning MSB of Divisor and Divident.

	while (isAlignedGreater(Divident, Divisor))
	{
		*((uint16_t*)Quotient.Data + n - t) = *((uint16_t*)Quotient.Data + n - t) + 1;
		SubtractAligned(Divident, Divident, Divisor);
	}

	// STEP 3.
	// For i from n down to (t+1):
	//	
	//	STEP 3.1
	//		if Divident[i] == Divisor[t] then:
	//			Quotient[i-t-1] = b - 1
	//		else
	//			Quotient[i-t-1] = ( Divident[i] || Divident[i-1] ) / Divisor[t]
	//	
	//	STEP 3.2
	//		While Quotient[i-t-1] * ( Divisor[t] || Divisor[t-1]) > ( Divident[i] || Divident[i-1] || Divident[i-2] ) :
	//			Quotient[i-t-1] = Quotient[i-t-1] - 1;
	//	
	//	STEP 3.3
	//		Divident = Divident - Quotient[i-t-1] * Divisor * b^(i-t-1)
	//	
	//	STEP 3.4
	//		if Divident < 0 then:
	//			Divident = Divident + Divisor * b^(i-t-1)
	//			Quotient[i-t-1] = Quotient[i-t-1] - 1

	for (i = n; i > t; i--)
	{
		//	STEP 3.1
		if (*((uint16_t*)Divident.Data + i) == *((uint16_t*)Divisor.Data + i))
			*((uint16_t*)Quotient.Data + i - t - 1) = 0xFFFF;
		else
			*((uint16_t*)Quotient.Data + i - t - 1) = ((uint32_t)(*((uint16_t*)Divident.Data + i) << 16) | (uint32_t)*((uint16_t*)Divident.Data + i - 1)) / *((uint16_t*)Divisor.Data + t);

		//	STEP 3.2

		tmp = ((uint32_t)(*((uint16_t*)Divident.Data + i) << 16) | (uint32_t)*((uint16_t*)Divident.Data + i - 1)) / *((uint16_t*)Divisor.Data + t);

		// Calculate	 Quotient[i-t-1] * ( Divisor[t] || Divisor[t-1])	into	 temp1[]
		tmp = *((uint16_t*)Divisor.Data + t - 1) * *((uint16_t*)Quotient.Data + i - t - 1);
		temp1[0] = tmp;
		tmp >>= 16;
		tmp = *((uint16_t*)Divisor.Data + t) * *((uint16_t*)Quotient.Data + i - t - 1) + tmp;
		temp1[1] = tmp;
		temp1[2] = tmp >> 16;

		while (isGreater16(temp1, (uint16_t*)Divident.Data + i - 2, 3))
		{
			*((uint16_t*)Quotient.Data + i - t - 1) = *((uint16_t*)Quotient.Data + i - t - 1) - 1;

			// Calculate	 Quotient[i-t-1] * ( Divisor[t] || Divisor[t-1])	again for next while loop
			tmp = *((uint16_t*)Divisor.Data + t - 1) * *((uint16_t*)Quotient.Data + i - t - 1);
			temp1[0] = tmp;
			tmp >>= 16;
			tmp = *((uint16_t*)Divisor.Data + t) * *((uint16_t*)Quotient.Data + i - t - 1) + tmp;
			temp1[1] = tmp;
			temp1[2] = tmp >> 16;
		}

		//	STEP 3.3
		//	Calculate	Quotient[i-t-1] * Divisor * b^(i-t-1)	into	temp[2]
		tmp = 0;

		for (l = 0; l < t + (i - t - 1) + 1; l++)
		{
			if (l<(i - t - 1))
				tmp = *((uint16_t*)temp2.Data + l) + tmp;
			else
				tmp = *((uint16_t*)Divisor.Data + l - (i - t - 1)) * *((uint16_t*)Quotient.Data + i - t - 1) + tmp;

			*((uint16_t*)temp2.Data + l) = tmp;
			tmp >>= 16;
		}

		*((uint16_t*)temp2.Data + l) = tmp;
		*((uint16_t*)temp2.Data + l + 1) = 0;

		// Now perform the subtraction:
		//		Divident = Divident - Quotient[i-t-1] * Divisor * b^(i-t-1)
		// Also goto next step;

		//	STEP 3.4
		if (Subtract(Divident, Divident, temp2))
		{
			// Calculate	Divisor * b^(i-t-1)		into temp2[]

			for (l = 0; l < t + (i - t - 1) + 1; l++)
				if (l < (i - t - 1))
					*((uint16_t*)temp2.Data + l) = 0;
				else
					*((uint16_t*)temp2.Data + l) = *((uint16_t*)Divisor.Data + l - (i - t - 1));

			*((uint16_t*)temp2.Data + l) = 0;

			Add(Divident, Divident, temp2);

			// Quotient[i-t-1] = Quotient[i-t-1] - 1

			*((uint16_t*)Quotient.Data + i - t - 1) = *((uint16_t*)Quotient.Data + i - t - 1) - 1;
		}

		tmp = 0;
	}

	// STEP 4.
	// Remainder = Divident

}

uint8_t isGreaterThanZero(BIGINT Input)
{
	uint8_t i;

	//if ((Input.Data[Input.Length - 1] & 0x80000000) != 0) return 0;

	for (i = 0; i < Input.Length; i++)
	{
		if (Input.Data[i] > 0) return 1;
		else return 0;
	}

	return 0;
}

void ShiftLeft(BIGINT Input)
{
	uint8_t i, oldMSB = 0, newMSB;

	for (i = 0; i < Input.Length; i++)
	{
		newMSB = (Input.Data[i] & 0x80000000) != 0;

		Input.Data[i] = Input.Data[i] << 1 | oldMSB;

		oldMSB = newMSB;
	}
}


void ShiftRight(BIGINT Input)
{
	uint8_t oldLSB = 0, newLSB;
	int i;

	for (i = Input.Length-1; i >= 0; i--)
	{
		newLSB = (Input.Data[i] & 0x00000001) != 0;

		if (oldLSB)	
			Input.Data[i] = 0x80000000 | Input.Data[i] >> 1;
		else		
			Input.Data[i] = Input.Data[i] >> 1;

		oldLSB = newLSB;
	}
}

uint8_t isAlignedGreater(BIGINT Input1, BIGINT Input2)
{
	uint8_t i;

	for (i = 0; i < Input2.Length; i++)
	{
		if (Input1.Data[Input1.Length - i - 1] > Input2.Data[Input2.Length - i - 1])	return 1;
		else if (Input1.Data[Input1.Length - i - 1] < Input2.Data[Input2.Length - i - 1]) return 0;
	}

	while (i < Input1.Length)
	{
		if (Input1.Data[Input1.Length - i - 1] > 0)
			return 1;

		i++;
	}

	return 0;
}

uint8_t SubtractAligned(BIGINT Output, BIGINT Input1, BIGINT Input2)
{
	uint8_t i;
	uint32_t tempResult = 0;

	for (i = 0; i < Input1.Length * 2; i++)
	{
		if (i < (Input1.Length - Input2.Length) * 2)
			tempResult += *((uint16_t*)Input1.Data + i);
		else
			tempResult += *((uint16_t*)Input1.Data + i) - *((uint16_t*)Input2.Data + i - (Input1.Length - Input2.Length) * 2);

		*((uint16_t*)Output.Data + i) = tempResult;

		if ((tempResult & 0x80000000) == 0)
			tempResult >>= 16;
		else
			tempResult = 0xFFFFFFFF;
	}

	return ((uint8_t)tempResult == 0xFF);
}

uint8_t isGreater16(uint16_t *Input1, uint16_t *Input2, uint8_t Length)
{
	uint8_t i;

	for (i = 0; i < Length; i++)
	{
		if (Input1[Length - i - 1] > Input2[Length - i - 1])	return 1;
		else if (Input1[Length - i - 1] < Input2[Length - i - 1]) return 0;
	}

	return 0;
}
