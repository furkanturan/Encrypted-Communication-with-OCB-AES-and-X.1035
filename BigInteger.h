#ifndef _BIGINT_H_
#define _BIGINT_H_

#include "Globals.h"

typedef struct {
	uint8_t		Length;
	uint32_t	*Data;
} BIGINT;

#define MAX_ARRAY_SIZE 69

void BIGINT_from_String(uint32_t *destination, int length, char* num);
void Display_BIGD(BIGINT Data);
uint8_t Add(BIGINT Output, BIGINT Input1, BIGINT Input2);
uint8_t Subtract(BIGINT Output, BIGINT Input1, BIGINT Input2);
void Multiply(BIGINT Output, BIGINT Input1, BIGINT Input2);
void Divide(BIGINT Quotient, BIGINT Divident, BIGINT Divisor);


uint8_t isAlignedGreater(BIGINT Input1, BIGINT Input2);
uint8_t SubtractAligned(BIGINT Output, BIGINT Input1, BIGINT Input2);
uint8_t isGreater16(uint16_t *Input1, uint16_t *Input2, uint8_t Length);

uint8_t isGreaterThanZero(BIGINT Input);
void ShiftLeft(BIGINT Input);
void ShiftRight(BIGINT Input);

#endif