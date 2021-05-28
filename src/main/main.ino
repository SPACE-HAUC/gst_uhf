
/* Written by Sunip K. Mukherjee (c) 2019 @ UML
 *  
 *  Utilizes the Si446x library by Zak Kemble (https://github.com/zkemble/Si446x)
 *  
 *  Connection Layout:
 *  
 *  ARDUINO              Si446x
 *  ----------------------------
 *  3.3V                 Power
 *  ----                 GPIO0
 *  ----                 GPIO1
 *  MISO                 SDO
 *  MOSI                 SDI
 *  SCLK                 SCLK
 *  10                   SPI SS
 *  2                    NIRQ
 *  5                    SDN
 *  GND                  GND
 *  ============================
 *  
 *  ARDUINO              RPi
 *  ----------------------------
 *  19 (RX)              14 (TX)
 *  18 (TX)              15 (RX)
 *  14 (PIPE_IRQ)        11 (GPIO17)
 *  GND                  GND
 *  ============================
 *  
 * 9.6 kbps, 2.4 kHz deviation, single channel, 2GFSK (Frequency not finalized)
 * Packet structure: 0xAAAAAAAAAA | 0x7E | F1 L | F1 (upto 128 bytes) | CRC16 [F1L + F1]
 * CRC: CRC16 CCITT-False (Init: 0xFFFF), enabled over F1L and F1 and transmitted at end of F1
 * NO DATA WHITENING
 * NO MANCHESTER ENCODING
 * 
 * This program essentially enables a half-duplex style Serial terminal.
 * End goal: To enable a PPP over Serial, over which SSH access can be granted.
 * SSH takes care of packet encryption (RSA-2048)
 * PPP and SSH not tested (2019-11-19)
 * 
 * The program is intended to run on an Arduino Due with dual serial port (USB-Serial or Serial for
 * data and Serial1 for debug).
 * 
 * Pipe mode is enabled using an ISR attached to a GPIO pin.
 * 
 * This makes the code easier to maintain. 
 * 
 */

#include "Si446x.h"
#include <string.h>

#define _BV(bit) (1 << (bit)) //_BV macro for bit representation

#define PACKET_NONE 0
#define PACKET_OK 2
#define MAX_PACKET_SIZE 64

#define PIPE_INTERRUPT 14

volatile byte RECV_STAT = 1 ;

void SI446X_CB_RXCOMPLETE(uint8_t length, int16_t rssi) //Receive interrupt function
{
  if(length > MAX_PACKET_SIZE)
    length = MAX_PACKET_SIZE;
  char buf[MAX_PACKET_SIZE] ;
//  Serial.print("Length: "); Serial.print(length);Serial.print(" | RSSI: "); Serial.println(rssi);
  Si446x_read((uint8_t*)buf, length);
  if ( RECV_STAT ){ // if we are printing the enable message, we want to put it on the debug serial instead of the data serial
//    RECV_STAT-- ; // this is not in the if statement to avoid buffer underflow
//    if ( buf[length-1] != '\r' ) // no need to print the carriage return
      Serial.print(buf); // this is affordable because PIPE mode is rarely enabled
    return ;
  }
  Serial1.print(buf);  
  return ;
}
/* 
 *    UART Speed               PIPE command
 * ===========================================
 *     1200                    ES+W22001320
 *     9600                    ES+W22000320
 *     115200                  ES+W22003320
 */
void EN_PIPE(void) // ISR to enable PIPE mode
{
  RECV_STAT = 8 ; // going to receive 8 bytes (OK+3323\r) that need to be put out of the debug port instead of the data port
  char cmd [] = "ES+W22000320\r" ; //turn on tunnel mode at 9600 bps UART
  Serial.println("Sending PIPE command...") ;
  delay(1000);
  Si446x_TX(cmd, strlen(cmd), 0, SI446X_STATE_RX); // transmit the tunnel mode packet and fall back to RX mode
  Serial.println("Sent PIPE command...") ;
  return ;
}

void EN_IRQ(void)
{
    return;
}

void setup ()
{
    Serial.begin(9600); // Debug transmission
    Serial.println("Init: Serial -- Done");
    delay(100);
    Si446x_init() ;
    Serial.println("Init: Radio -- Done");
    delay(1000); // wait after initializing radio
    attachInterrupt(digitalPinToInterrupt(PIPE_INTERRUPT), EN_IRQ, RISING) ; // Attach the EN_PIPE handler to PIPE_INTERRUPT on the RISING edge
    EN_PIPE();
    Serial.print("State: 0x") ;
    Serial.println((uint8_t)Si446x_getState(),HEX); // get radio state
    delay(100); // wait
    Serial.print("RSSI: 0x");
    Serial.println((uint16_t)Si446x_getRSSI(),HEX); // last RSSI
    Si446x_setupCallback(SI446X_CBS_RXBEGIN, 1); // enable receive interrupt
//    EN_PIPE() ; // Enable PIPE in setup without an actual interrupt, FOR DEBUG ONLY
//    Serial1.print("TX Status: 0x");Serial1.println(status,HEX);
    delay(100);
    Serial1.begin(9600); // Data transmission, at 9.6 kbaud to relate to the 9.6 kbaud radio rate
}

/*
 * There should be a shell script that enables pipe mode first and then creates the PPP
 * and then enables SSH. This should alleviate frustration.
 */

void loop ()
{
  while ( Serial.available() < 64 ) ; // wait till we have data on the transmission line
  char txbuf[MAX_PACKET_SIZE];
  for (int i = 0; i < MAX_PACKET_SIZE; i++)
    txbuf[i] = Serial.read();
  Si446x_TX(txbuf, MAX_PACKET_SIZE, 0, SI446X_STATE_RX); // transmit byte
  // delay ( 100 ) ; //Is a delay necessary?
}
