#include <string.h>
#include <Arduino.h>
#include <bluefruit.h>

/**
   exposes readPacket(), which fills in the (global) packetBuffer char array.
   Based off of Adafruit Feather 'peripheals > controller' example.
*/

//    READ_BUFSIZE            Size of the read buffer for incoming packets
#define READ_BUFSIZE          (142) // 140 (max search API length) + 2 for start and end
#define PACKET_START_CHAR     '^'
#define PACKET_END_CHAR       '$' // TODO: end with a \n?

/* (Global) buffer to hold incoming characters */
char packetbuffer[READ_BUFSIZE + 1]; // +1 to hold final null term

/**
   Waits timeoutCycles to read a packet of data from the uart.

   Message packets expected to start with a '^' and end with a '$' TODO: end with a \n?

   Returns the length of the message (how much of packetbuffer was filled in) -- use that to find out where to stop on packetBuffer
*/
uint8_t readPacket(BLEUart *ble_uart, uint16_t timeoutCycles) {
  uint16_t origtimeout = timeoutCycles, len = 0;

  // Clear the message buffer
  memset(packetbuffer, 0, READ_BUFSIZE + 1);

  while (timeoutCycles--) {
    if (timeoutCycles == 0)
      break;
    if (len >= READ_BUFSIZE)
      break;
    if (packetbuffer[len - 1] == PACKET_END_CHAR)
      break;
      
    while (ble_uart->available() && len < READ_BUFSIZE) {
      char c = ble_uart->read();
      if (c == PACKET_START_CHAR) {
        len = 0;
      }
      packetbuffer[len] = c;
      len++;

      if (c == PACKET_END_CHAR || c == '\n')
        break;
      
      timeoutCycles = origtimeout;
    }

    if (packetbuffer[len - 1] == PACKET_END_CHAR || packetbuffer[len - 1] == '\n') {
      break;
    }

    delay(1);
  }

  packetbuffer[len] = 0;  // null term

  if (!len) {  // no data or timeout 
    // Serial.println("[readPacket] No data/timeout reached");
    return 0;
  }
  if (packetbuffer[0] != PACKET_START_CHAR) {  // doesn't start with '^' packet beginning, ie never got valid data
//    Serial.println("[readPacket] Invalid start char, returning len 0");
    return 0;
  }
    

  // TODO: Checksum?

  return len;
}
