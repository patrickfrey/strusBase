#include <stdint.h>

int main(void)
{
	uint32_t idx = 8;
	uint32_t result;
	asm(" bsf %1, %0 \n" : "=r"(result) : "r"(idx) );
	return (result == 3) ? 0 : 1;
}

