#define BLINK_TIME_MS 350

Adafruit_NeoPixel strip = Adafruit_NeoPixel(50, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define NUM_COLORS 7
uint32_t colorPalette[NUM_COLORS] = {
  strip.Color(255, 255, 255), // white
  strip.Color(0,   0,   255), // blue
  strip.Color(255, 0,   255), // purple
  strip.Color(0,   255, 0),   // green
  strip.Color(0,   255, 255), // teal
  strip.Color(255, 255, 0),   // yellow
  strip.Color(255, 0,   0)    // red
};

#define STATE_IDLE 0
#define STATE_LETTER 1
#define STATE_DUPE_LETTER 2

#define STATE_IDLE_MS 7000
#define STATE_LETTER_MS 800
#define STATE_DUPE_LETTER_PAUSE_MS 200

uint8_t state = STATE_IDLE;
unsigned long nextTransition = 0;
char *curLetterPtr = readTweet();

// For use when a new message comes in -- reset to idle state
void resetBlinkControllerFsm() {
  state = STATE_IDLE;
  nextTransition = millis() + STATE_LETTER_MS; // (not STATE_IDLE_MS... do a short delay)
}

void blinkFsmLoop() {
  if (millis() > nextTransition) {
    switch(state) {
      // Was waiting to show next message
      case STATE_IDLE:
        Serial.println("[blinkFSM] Starting to read next tweet");
        curLetterPtr = readTweet();
        state = STATE_LETTER;
        break;

      // Was displaying a letter
      case STATE_LETTER: 
      {
        // transition back to idle if done displaying stuff
        if (! *curLetterPtr) { // assume done when null termination
          Serial.println("[blinkFSM] Done with tweet, back to idle");
          illuminateLed(UNKNOWN_CHAR_I, 0);
          state = STATE_IDLE;
          nextTransition = millis() + STATE_IDLE_MS;
          break;
        }

        Serial.print("[blinkFSM] Blinking letter: ");
        Serial.print(*curLetterPtr);
        Serial.println();
        
        // display the current letter
        uint8_t ledI = char2LedI(curLetterPtr);
        illuminateLed( 
          ledI, 
          colorPalette[ ledI % NUM_COLORS ]
        );

        // wait until next one
        state = STATE_LETTER;
        nextTransition = millis() + STATE_LETTER_MS;
        
        // add some more time if we're displaying a space
        if (*curLetterPtr == ' ') {
          Serial.println("  (pausing, space)");
          nextTransition += STATE_LETTER_MS;
        }

        curLetterPtr += 1;

        // pause with blank if next letter is the same
        if (*curLetterPtr == *(curLetterPtr - 1)) {
          Serial.println("  (next letter same as before, next is pause)");
          state = STATE_DUPE_LETTER;
        }
      }
      break;
        
      case STATE_DUPE_LETTER:
        illuminateLed(UNKNOWN_CHAR_I, 0);
        nextTransition = millis() + STATE_DUPE_LETTER_PAUSE_MS;
        state = STATE_LETTER;
        break;
    }
  }
}

#define LETTER_OFFSET 10
#define NUMBER_OFFSET 0
char *emoji = "9876543210"; // emojis are encoded as numbers
char *lowercaseStr = "zyxrstuvwqponmlfghijkedcba";
char *uppercaseStr = "ZYXRSTUVWQPONMLFGHIJKEDCBA";


uint8_t char2LedI(char *cmd) {
  for (uint8_t i = 0; i < 26; i++) {
    if (lowercaseStr[i] == *cmd) return LETTER_OFFSET + i;
    if (uppercaseStr[i] == *cmd) return LETTER_OFFSET + i;
  }
  for (uint8_t i=0; i < 12; i++) {
    if (emoji[i] == *cmd) return NUMBER_OFFSET + i;
  }
  if (*cmd == '#') return LETTER_OFFSET + 26;
  return UNKNOWN_CHAR_I;
}

// Illuminates a single LED with a color
void illuminateLed(uint8_t ledI, uint32_t color) {
  for (uint8_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, i == ledI ? color : 0);
  }
  strip.show();
}
