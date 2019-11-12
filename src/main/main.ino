#include <Si446x.h>
#include <string.h>

uint16_t  gen_pack(char * in, size_t len, uint8_t * out) //caller function has to guarantee that out has length == len + 3 (1 for size, 2 for CRC) and also free the allocated memory
{
	uint16_t crc = 0xffff ; //init
	uint16_t poly = 0x1021 ; // CRC Polynomial, 0001 0000 0010 0001 (0, 5, 12)
	uint8_t lim = in[len - 1] == '\0' ? len - 1 : len ; //change limits to handle null termination properly
    out [0] = lim ; //prepend packet by message length
    memcpy( out+1 , in, lim) ; //append message to packet
	for ( uint8_t i = 0 ; i < lim + 1 ; i++ ) { //increase length by 1 to address the prepended length
		for ( uint8_t j = 0 ; j < 8 ; j++ ) { //for each bit
			uint8_t bit = (out[i] >> (7-j) & 1) ;
			uint8_t c15 = (crc >> 15      & 1) == 1 ? 1 : 0 ;
			//printf("In CRC: i: %d, j: %d, bit: %d, c15: %d, crc: 0x%x\n", i, j, bit, c15, crc);
			crc <<= 1 ;
			if ( c15 ^ bit ) crc ^= poly ;
		}
	}
    crc &= 0xffff; //is this necessary?
    out [ lim + 1 ] = (uint8_t)(crc>>8) ;
    out [ lim + 2 ] = (uint8_t)(crc) ;
    return crc ; //return the CRC for sanity checking
}

void setup ()
{
    Serial.begin(115200);
    Serial.println("Init: Serial");
    Si446x_init() ;
    Serial.println("Init: Radio");
    delay(1500);
    uint8_t * buf ;
    char cmd [] = "ES+W22003322\r" ; //tunnel mode
    buf = (uint8_t *) malloc (strlen(cmd)+3) ;
    gen_pack(cmd, strlen(cmd), buf) ;
    uint8_t status ;
    Si446x_TX(buf, strlen(cmd)+3, 0, status);
    free (buf);
    Serial.print("TX Status: 0x");Serial.println(status,HEX);
    delay(1000);
    char msg [] = "Hello world!";
    buf = (uint8_t *) malloc (strlen(msg)+3) ;
    uint16_t temp = gen_pack(msg, strlen(msg), buf);
    Si446x_TX(buf, strlen(msg)+3, 0, status);
    free(buf);
    Serial.print("TX Status: 0x");Serial.println(status,HEX);
}

void loop ()
{

}
