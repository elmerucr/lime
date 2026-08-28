#include <stdio.h>
#include <stdint.h>

uint8_t rnda = 0;
uint8_t rndb = 0;
uint8_t rndc = 0;
uint8_t rndx = 0;

uint8_t prng()
{
	rndx++;
	rnda = (rnda ^ rndc) ^ rndx;
	rndb = rndb + rnda;
	rndc = (rndc + ((rndb >> 1) | (rndb << 7))) ^ rnda;
	return rndc;
}

int main()
{
	for (int i=0; i<10; i++) {
		printf("%02x\n", prng());
	}
	return 0;
}
