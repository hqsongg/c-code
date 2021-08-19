#include <stdio.h>


int main (int argc, char *argv[])
{
	union  {
		short val;
		char union_byte[sizeof(short)];
	}test;
	
	test.val = 0x0102;
	
	if ((test.union_byte[0] == 0x02) && (test.union_byte[1] == 0x01)) {
		printf("union_byte[0]=%d little endian  !\n", test.union_byte[0]);
	} else if ((test.union_byte[0] == 0x01) && (test.union_byte[1] == 0x01)) {
		printf("union_byte[0]=%d big endian  !\n", test.union_byte[0]);
	}
	
	return 0;
}