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
/*
 * Apparently the radio suppports CRC-ing
 * The code right now has a CRC function, but the packets only include the messages themselves.
 */

#define _BV(bit) (1 << (bit))

#define PACKET_NONE 0
#define PACKET_OK 2
#define MAX_PACKET_SIZE 64

void SI446X_CB_RXCOMPLETE(uint8_t length, int16_t rssi)
{
  if(length > MAX_PACKET_SIZE)
    length = MAX_PACKET_SIZE;
  //Serial.print("Received. Len: "); Serial.print(length); Serial.print(" | RSSI: ") ; Serial.println(rssi); 
  char buf[MAX_PACKET_SIZE] ;

  Si446x_read((uint8_t*)buf, length);
  Serial.print(buf);  
  if ( buf[length-1] == '\r' )
    Serial.println();
  return ;
}
 
void setup ()
{
    Serial.begin(115200);
    Serial.println("Init: Serial");
    Si446x_init() ;
    si446x_info_t info ;
    Serial.println("Init: Radio");
    delay(1000);
    Serial.print("State: ") ;
    Serial.println((uint8_t)Si446x_getState(),HEX);
    delay(100);
    Serial.print("RSSI: ");
    Serial.println((uint16_t)Si446x_getRSSI(),HEX);
    Si446x_setupCallback(SI446X_CBS_RXBEGIN, 1);
    uint8_t * buf ;
    Serial.println("Turning on tunnel mode: " ) ;
    char cmd [] = "ES+W22003320\r" ; //tunnel mode
    buf = (uint8_t *) malloc (strlen(cmd)+3) ;
    gen_pack(cmd, strlen(cmd), buf) ;
    si446x_state_t status ;
    Si446x_TX(cmd, strlen(cmd), 0, SI446X_STATE_RX);
    free (buf);
//    Serial.print("TX Status: 0x");Serial.println(status,HEX);
    delay(100);
    while ( true ){
//      Serial.println("LOOP");
//      if ( pingInfo.ready ) {
//        char temp [MAX_PACKET_SIZE] ;
//        memcpy(temp, (char*)pingInfo.buffer,pingInfo.length);
//        String recvmsg = (String)temp ;
//        //Serial.write((uint8_t*)pingInfo.buffer, sizeof(pingInfo.buffer));
//        Serial.flush();
//        Serial.print(temp);
//        pingInfo.ready = PACKET_NONE ;
//        continue ;
//      }
      String msg_str ;
      while ( Serial.available() <= 0 ) ;
      msg_str = Serial.readStringUntil('\n');
      char * msg ;
      if ( msg_str == "EN+PIPE") {
        msg_str = "ES+W22003323\r" ;
      }
      else
        msg_str += "\n\r" ;
      msg = (char * )msg_str.c_str() ;
      Si446x_TX(msg, strlen(msg), 0, SI446X_STATE_RX);
//      SI446X_NO_INTERRUPT()
//      {
//        // Print the message
//        Serial.print("TX Status: 0x");Serial.println(status,HEX);
//      }
      delay ( 100 ) ;
    }
}

void loop ()
{

}
