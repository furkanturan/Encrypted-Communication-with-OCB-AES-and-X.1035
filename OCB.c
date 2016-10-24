#include "Globals.h"
#include "OCB.h"
#include "aes.h"

void XOR(uint32_t* Output, uint32_t* Input1, uint32_t*  Input2);
void DOUBLE(uint32_t* Output, uint32_t* Input);
void CALCULATE_L(uint32_t* l, uint32_t* ldollar, uint8_t i); 
void AUTH(uint32_t *authenticationdata, uint32_t *associateddata, uint8_t associateddata_length, uint32_t *key);

int OCB(uint8_t *ciphertext, uint32_t *key, uint32_t *nonce, uint32_t *associateddata, uint8_t ad_block_count, uint8_t *plaintext, uint8_t plaintext_length, uint8_t mode)
{
	AES_KEY aes_encrypt_key, aes_decrypt_key;
	uint8_t i,j;
	uint32_t tmp[4], lstar[4], ldollar[4], top[4], bottom, ktop[4], stretch[8], delta[4], checksum[4], tag[4], pad[4];
	// SETUP AES AND STRIP CIPHERTEXT OF ITS TAG

	if (!mode)
	{
		aes_set_decrypt_key(&aes_decrypt_key, (uint8_t*)key, 128);
	}

	aes_set_encrypt_key(&aes_encrypt_key, (uint8_t*)key, 128);
		
	// KEY DEPENPENDENT VARIABLES

	// Calculate L_start = Ek(0^128) = Ek(tmp)
	memset(tmp, 0, 16);
	aes_encrypt(&aes_encrypt_key, (uint8_t*)tmp, (uint8_t*)lstar);

	// Calculate L_dollar = double(L_star)
	DOUBLE(ldollar, lstar);


	// TOP DEPENPENDENT VARIABLES

	// Calculate Top = 0x00000001 | N (last 6 bits of N is zero)
	top[3] = 0x0000001;
	top[2] = nonce[2];
	top[1] = nonce[1];
	top[0] = nonce[0] & 0xFFFFFFC0;

	// Calculate Bottom = Last 6 bits of N
	bottom = nonce[0] & 0x0000003F;

	// Calculate K_top = Ek(Top)
	aes_encrypt(&aes_encrypt_key, (uint8_t*)top, (uint8_t*)ktop);

	// Calculate Stretch = K_top | (K_top xor (K_top << 8))
	stretch[7] = ktop[3];
	stretch[6] = ktop[2];
	stretch[5] = ktop[1];
	stretch[4] = ktop[0];
	stretch[3] = ktop[3] ^ (ktop[3] << 8 | ktop[2] >> 24);
	stretch[2] = ktop[2] ^ (ktop[2] << 8 | ktop[1] >> 24);
	stretch[1] = ktop[1] ^ (ktop[1] << 8 | ktop[0] >> 24);
	stretch[0] = ktop[0] ^ (ktop[0] << 8);

	// Calculate delta = Init(N)
	// The initial value for delta, is the first 128 bits of Stretch <<	Bottom	
	if (bottom != 0)
		for (i = 7; i>3; i--)
			delta[i-4] = (stretch[i] << bottom) | (stretch[i - 1] >> (32 - bottom));
	else
		for (i = 7; i > 3; i--)
			delta[i-4] = stretch[i];


	// ENCRYPTION BEGINS

	// Initialise checksum to 0
	memset(checksum, 0, 16);

	// Traverse all the plain text in 128 bit (16 byte) blocks
	
	for (i = 1; i <= plaintext_length / 16; i++)
	{
		// Increment Delta
		CALCULATE_L(tmp, ldollar, i);
		XOR(delta, delta, tmp);

		// Xor delta with plain text
		XOR(tmp, delta, (uint32_t*)plaintext);

		if (mode)
		{
			// Encrypt 
			aes_encrypt(&aes_encrypt_key, (uint8_t*)tmp, (uint8_t*)tmp);

			// Xor encryption output and delta, output is one 128 bit (16 byte) cypther text block
			XOR((uint32_t*)ciphertext, delta, tmp);

			// Calculate Checksum = M1 xor ... xor Mm in each iteration					
			XOR(checksum, (uint32_t*)plaintext, checksum);
		}
		else
		{
			// Decrypt (same procedure above, use input and output inversely)
			aes_decrypt(&aes_decrypt_key, (uint8_t*)tmp, (uint8_t*)tmp);

			XOR((uint32_t*)ciphertext, delta, tmp);

			// Calculate Checksum = M1 xor ... xor Mm in each iteration
			XOR(checksum, (uint32_t*)ciphertext, checksum);
		}

		// Update block pointers for the next loop
		plaintext = plaintext + 16;
		ciphertext = ciphertext + 16;
	}

	
	// Process any final partial block and compute raw tag

	// Byte count in final block 
	plaintext_length = plaintext_length % 16;  

	if (plaintext_length > 0)
	{
		// Increment Delta
		CALCULATE_L(tmp, ldollar, i);
		XOR(delta, delta, tmp);

		// Calculate Pad = Ek(Delta)
		aes_encrypt(&aes_encrypt_key, (uint8_t*)delta, (uint8_t*)pad);

		if (mode)
		{
			// Pad 10*'s to remaining data bytes
			// 10* means; append a single 1 - bit and then the minimum number of 0 - bits to get the string to be 128 bits
			for (j = 0; j < 16; j++)
			{
				if (j < plaintext_length)
					*((uint8_t*)tmp + (15-j)) = plaintext[j];
				else if (j == plaintext_length)
					*((uint8_t*)tmp + (15 - j)) = 0x80;
				else
					*((uint8_t*)tmp + (15 - j)) = 0x00;
			}
			
			// Xor pad and padded message
			XOR(pad, tmp, pad);
			
			// Store entire encrypted last block (not just padded part) to output
			for (j = 0; j < plaintext_length; j++)
			{
				*ciphertext = *((uint8_t*)pad + (15 - j));
				ciphertext++;
			}

			// Update Checksum
			XOR(checksum, tmp, checksum);
		}
		else
		{
			// Read remaining bytes from plaintext, and place them to the MSB's of tmp
			// Pad zero's to remaining bytes of the block
			for (j = 0; j < 16; j++)
			{
				if (j < plaintext_length)
					*((uint8_t*)tmp + (15 - j)) = plaintext[j];
				else
					*((uint8_t*)tmp + (15 - j)) = 0x00;
			}
			
			// Xor pad and padded message
			XOR(pad, (uint32_t*)tmp, pad);
				
			for (j = 0; j < 16; j++)
			{
				if (j < plaintext_length)
				{
					// Store only data (not padded) part of encrypted message to output
					*((uint8_t*)ciphertext + j) = *((uint8_t*)pad + (15 - j));
					// Update offset one block (will be used in validation)
					plaintext++;
				}
				// Remaining part should be 10* padded again so that checksum will match
				else if (j == plaintext_length)
					*((uint8_t*)pad + (15 - j)) = 0x80;
				else
					*((uint8_t*)pad + (15 - j)) = 0x00;
			}

			// Update Checksum
			XOR(checksum, pad, checksum);
		}
	}
	

	// FINAL COLUMN OF ENCRYPTION: ENCRYPTION WITH CHECKSUM TO CREATE TAG

	// Increment Delta
	CALCULATE_L(tmp, ldollar, i);
	XOR(delta, delta, tmp);

	XOR(tmp, checksum, delta);

	// Encrypt 
	aes_encrypt(&aes_encrypt_key, (uint8_t*)tmp, (uint8_t*)tag);

	// Calculate AUTH in hash function
	AUTH(tmp, associateddata, ad_block_count, key);

	// Calculate Tag by xoring encryption result with AUTH
	XOR(tag, tmp, tag);

	if (mode)
	{
		// Put the tag at the end of cypthertext
		memcpy(ciphertext, tag, TAG_LENGTH);
		return 0;
	}
	else
		// Check for validity of the received and calculated tag
		return (memcmp(plaintext, tag, TAG_LENGTH) == 0 ? 1 : 0);

}


void XOR(uint32_t* Output, uint32_t* Input1, uint32_t*  Input2)
{
	uint8_t i;
	for (i = 0; i<4; i++)
		Output[i] = Input1[i] ^ Input2[i];
}

void DOUBLE(uint32_t* Output, uint32_t* Input)
{
	// Definition of the function:
	// double(S)	= S << 1			if the MSB bit of Input is 0,
	//				= (S<<1) xor 135	otherwise

	uint8_t temp = (Input[3] & 0x80000000) == 0 ? 0 : 1;

	Output[3] = (Input[3] << 1) | ((Input[2] & 0x80000000) == 0 ? 0 : 1);
	Output[2] = (Input[2] << 1) | ((Input[1] & 0x80000000) == 0 ? 0 : 1);
	Output[1] = (Input[1] << 1) | ((Input[0] & 0x80000000) == 0 ? 0 : 1);
	Output[0] = (Input[0] << 1);

	if (temp)
		Output[0] = Output[0] ^ 135;
}

void CALCULATE_L(uint32_t* l, uint32_t* ldollar, uint8_t i)
{
	DOUBLE(l, ldollar);
	while ((i & 0x01) == 0)
	{
		DOUBLE(l, l);
		i >>= 1;
	}
}

void AUTH(uint32_t *authenticationdata, uint32_t *associateddata, uint8_t ad_block_count, uint32_t *key)
{
	AES_KEY aes_key;
	uint32_t tmp[4], lstar[4], ldollar[4], delta[4];
	uint8_t i;

	aes_set_decrypt_key(&aes_key, (uint8_t*)key, 128);

	// KEY DEPENPENDENT VARIABLES

	// Calculate L_start = Ek(0^128) = Ek(tmp)
	memset(tmp, 0, 16);
	aes_encrypt(&aes_key, (uint8_t*)tmp, (uint8_t*)lstar);

	// Calculate L_dollar = double(L_star)
	DOUBLE(ldollar, lstar);

	// Process blocks

	// Initialise checksum to 0
	memset(authenticationdata, 0, 16);

	// Initialise delta to 0 
	// Different than encryption case, initial value of delta is 0 in auth calculation
	memset(delta, 0, 16);

	for (i = 1; i <= ad_block_count; i++)
	{
		// Increment Delta
		CALCULATE_L(tmp, ldollar, i);
		XOR(delta, delta, tmp);

		// Xor delta with associateddata
		XOR(tmp, delta, associateddata);

		// Encrypt
		aes_encrypt(&aes_key, (uint8_t*)tmp, (uint8_t*)tmp);

		// Xor encryption output and delta,
		// Accumulate result as authenticationdata
		XOR(authenticationdata, authenticationdata, tmp);

		// Update pointer to block for next iteration
		associateddata = associateddata + 4;
	}
}