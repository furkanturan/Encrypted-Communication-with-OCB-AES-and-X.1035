#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int uint32_t;

#define SIZE 128

//uint32_t p[33], q[33], sp[33], Ra[13], Rb[13], PW[5];

typedef struct {
	uint8_t		Type;
	uint16_t	Counter;
	uint16_t	Length;
	uint32_t	*Data;
} PACKET;

#endif