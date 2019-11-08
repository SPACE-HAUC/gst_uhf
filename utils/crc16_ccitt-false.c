#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint16_t crc16(char * in, size_t len)
{
	uint16_t crc = 0xffff ; //init
	uint16_t poly = 0x1021 ; // CRC Polynomial, 0001 0000 0010 0001 (0, 5, 12)
	uint8_t lim = in[len - 1] == '\0' ? len - 1 : len ; //change limits to handle null termination properly
	for ( uint8_t i = 0 ; i < lim ; i++ ) {
		for ( uint8_t j = 0 ; j < 8 ; j++ ) {
			uint8_t bit = (in[i] >> (7-j) & 1) == 1 ? 1 : 0 ;
			uint8_t c15 = (crc >> 15      & 1) == 1 ? 1 : 0 ;
			//printf("In CRC: i: %d, j: %d, bit: %d, c15: %d, crc: 0x%x\n", i, j, bit, c15, crc);
			crc <<= 1 ;
			if ( c15 ^ bit ) crc ^= poly ;
		}
	}
	return crc & 0xffff; //is this necessary?
}

int main(int argc, char * argv[])
{
	//char str[] = "Hello world!" ;
	
	if ( argc != 2 )
	{
		printf("Usage: ./crc16 <string>\n\n");
		return 0 ;
	}
	#define str argv[1]
	
	printf("String: %s, length: %d, CRC16-CCITT-False: 0x%x\n", str, strlen(str), crc16(str, strlen(str)));
	return 0;
}
