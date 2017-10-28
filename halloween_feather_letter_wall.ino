#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <bluefruit.h>

#define PIXEL_PIN   16  /* Pin used to drive the NeoPixels */
#define START_LED   0  /* First LED of the letter wall*/
#define UNKNOWN_CHAR_I  255 /* LED Index code for an unrecognized character */

#define DEBUG


// BLE Service
BLEDis  bledis;
BLEUart bleuart;

// Packet buffer
extern char packetbuffer[];

extern Adafruit_NeoPixel strip;

// Misc
uint8_t current_state = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("DLo's Letter Wall");
  Serial.println("--------------------");
  Serial.println();
  Serial.println("Please connect using Adafruit's Bluefruit Connect LE application");

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Init Bluefruit
  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("DLo's Halloween Costume");
  Bluefruit.setConnectCallback(connect_callback);

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();  

  // Configure and start BLE UART service
  bleuart.begin();

  // Set up and start advertising
  startAdv();
}

void loop() {
  if ( Bluefruit.connected() && bleuart.notifyEnabled() ) {
    //uint8_t command = bleuart.read();
    uint8_t len = readPacket(&bleuart, 500);

    switch(packetbuffer[1]) { // [0] is the start-packet char


      // Blink the packet buffer LEDs
      case 't':
        Serial.println("Command _t_weet! Saving current tweet...");
        saveTweet(packetbuffer, len);
        rainbowCycle(2);
        
        break;

      case 'd':
        Serial.println("Command _d_ump messaage...");
        dumpMessagesToUart();
        break;

        
      case 'a':
      case 'b':
      case 'c':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      case 'z':
        current_state = packetbuffer[1];
        Serial.print("Got command:");
        Serial.print(current_state);
        Serial.println("");
        break;
    }
  }

  blinkFsmLoop();
  switch(current_state) {
    case 'a':
      colorWipe(strip.Color(255, 0, 0), 20); // Red
      break;
    case 'b':
      colorWipe(strip.Color(0, 255, 0), 20); // Green
      break;
    case 'c':
      colorWipe(strip.Color(0, 0, 255), 20); // Blue
      break;
    case 'd':
      theaterChase(strip.Color(255, 0, 0), 20); // Red
      break;
    case 'e':
      theaterChase(strip.Color(0, 255, 0), 20); // Green
      break;
    case 'f':
      theaterChase(strip.Color(255, 0, 255), 20); // Green
      break;
    case 'g':
      rainbowCycle(5);
      break;
    case 'h':
      rainbow(5);
      break;
    case 'i':
      theaterChaseRainbow(100);
      break;
  }
}


