
#include "Globals.h"
#include "OCB.h"
#include "x1035.h"
#include "Montgomery.h"

#define PRINT 1

// # of blocks are processed at each packet
#define BLOCK_COUNT 4				// 4		

// # of plaintext bytes processed at each packet
#define LENGTH (16*BLOCK_COUNT)		// 64 bytes	

PACKET Channel;
uint32_t Channel_data[(LENGTH + TAG_LENGTH) / 4];

char pt[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer ornare rhoncus est, sed accumsan dui mollis sit amet. Etiam vel tortor vitae turpis porta interdum. Ut dignissim libero quis felis volutpat volutpat. Nam quis nunc ultrices, eleifend est ut, facilisis arcu. Suspendisse risus justo, aliquet non erat a, ornare varius ex. Nam ac felis vel elit pretium iaculis quis eget justo. Donec consequat placerat risus vel ultrices. In hac habitasse platea dictumst. In hac habitasse platea dictumst. Praesent dignissim turpis nec ante pellentesque hendrerit quis vel odio. Vestibulum gravida nibh eu ex porta, vel elementum sapien scelerisque. Maecenas sed viverra arcu, vel eleifend orci. Proin tristique augue sed augue lobortis tristique. Curabitur suscipit tempus lorem, nec posuere arcu. Aliquam erat volutpat. Cras condimentum sem ac felis condimentum consectetur. Etiam a nibh eu quam porta cursus. Praesent diam eros, tempor at risus non, ultrices fringilla ipsum. Praesent id consectetur est. Praesent sed velit a est mollis congue. Morbi sit amet iaculis lectus. Aliquam molestie, neque a semper fringilla, erat felis egestas metus, eu porttitor diam velit eu turpis. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque euismod sed turpis ac fringilla. Ut vulputate quis diam at interdum. Curabitur sit amet condimentum dui. Maecenas in viverra leo. Cras viverra quam eget dolor interdum tempor. Aliquam vitae viverra risus, eget imperdiet eros. Sed id eros sem. Duis at felis et arcu eleifend vulputate nec vitae eros. Proin feugiat dui vel velit pharetra, quis facilisis lorem euismod. Suspendisse ultrices dolor ligula. Maecenas vitae aliquam neque. Suspendisse pellentesque velit nisi. Nulla nec blandit felis, nec rutrum dui. Curabitur consectetur vitae velit sit amet vehicula. Sed condimentum pharetra mi porttitor suscipit. Maecenas condimentum risus metus, vitae dictum est consectetur eu. Curabitur ut diam fermentum, sollicitudin sem a, suscipit velit. Duis posuere.";
char np[2500] = { 0, };
uint16_t bufferlength = sizeof(pt);

int main()
{
	uint32_t AssData[4] = { 0x55, };
	uint32_t nonce[3] = { 0, };
	uint32_t KeyMaster[4], KeySlave[4];

	uint8_t ByteCount;
	uint16_t i;

#ifdef PRINT
	printf("\n\nStarted\n\n");
#endif

	// GENERATE SESSION KEYS

	while (!EstablishKey(KeyMaster, KeySlave))
	{
		#ifdef PRINT
				printf("\n\n Key Establishment error! - Re-attempting\n");
		#endif
	}

#ifdef PRINT
	printf("Key Establishment is Successful!\n");
	printf("Key of Master is \t0x%08x%08x%08x%08x\n", 
		KeyMaster[3], 
		KeyMaster[2], 
		KeyMaster[1], 
		KeyMaster[0]);
	printf("Key of Slave is \t0x%08x%08x%08x%08x\n\n", 
		KeySlave[3], 
		KeySlave[2], 
		KeySlave[1], 
		KeySlave[0]);
#endif

	// ENCRYPT -> CHANNEL -> DECRYPT

	// Prepare Channel
	Channel.Type = 16;
	Channel.Counter = 0;
	Channel.Data = Channel_data;

	// Display plain text to be encrypted
#ifdef PRINT
	printf("Plain Text = Length %d\n%s\n\n", bufferlength, pt);

	printf("ENCRYPTION -> CHANNEL -> DECRYPTION\n\n");
#endif

	// Loop until there are data to be sent
	i = 0;
	while (bufferlength > 0)
	{
		// ByteCount is variable that sets how many encrypted data bytes
		// 	will be put in a packet
		// LENGTH is desired # of plaintext bytes processed at each packet
		// 	but if there is less than LENGTH bytes, adjust ByteCount accordingly
		if (bufferlength > LENGTH)	ByteCount = LENGTH;
		else				ByteCount = bufferlength;

		bufferlength -= ByteCount;

		// Set Nonce

		nonce[0] = i;

		// ENCRYPT -> Channel

		// Put cipher text into Channel.Data
		// Update Channel.Length info
		// Update Channel.Counter
		OCB((uint8_t*)Channel.Data, KeyMaster, nonce, 
			AssData, 4, pt + i * LENGTH, ByteCount, OCB_ENCRYPT);
		Channel.Length = 2 + 2 + ByteCount + TAG_LENGTH;
		Channel.Counter++;

		// Channel -> DECRYPT

		// Use data from Channel.Data
		// Use data length from Channel.Length
		// Put decrypted text into p buffer
		if (OCB(np + i * LENGTH, KeySlave, nonce, AssData, 
			4, (uint8_t*)Channel.Data, Channel.Length - (2 + 2 + TAG_LENGTH), 
			OCB_DECRYPT) == 0)
		{
#ifdef PRINT
			printf("\n\n Validation error!");
#endif
			getchar();
			return 0;
		}

		i++;
	}

#ifdef PRINT
	printf("All the data is decrypted and validated successfully.\n\n");

	printf("Decrypted Text \n%s\n\n", np);
#endif

	getchar();
	return 0;
}

