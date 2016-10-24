#ifndef _MONTGOMERY_H_
#define _MONTGOMERY_H_

#include "Globals.h"
#include "BigInteger.h"

void Montgomery_Multiplication(BIGINT Output, BIGINT Input1, BIGINT Input2);
void Montgomery_Exponentiation(BIGINT Output, BIGINT X, BIGINT Exponent);
void Montgomery_Inverse(BIGINT Output, BIGINT A);
void Montgomery_Divide(BIGINT Quotient, BIGINT Divident, BIGINT Divisor);

#endif