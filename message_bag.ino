#include <Arduino.h>

#define MAX_MSG_LEN 140 // READ_BUFSIZE - 2 (start/end chars)
#define MAX_NUM_TWEETS 8

extern BLEUart bleuart;

char tweets[MAX_NUM_TWEETS][MAX_MSG_LEN + 1] = { // +1 for null term
  "tweet me #danogorgon",
  "run",
  "boo",
  "spoooky",
  "binge stranger things #netflix",
  "not safe",
  "send beer",
  "friends dont lie"
};

uint8_t tweetLens[MAX_NUM_TWEETS];

uint8_t curReadTweetI = 0; // circular buffer
uint8_t curWriteTweetI = 0;


void saveTweet(char *packet, uint8_t len) {
  len = min(len, MAX_MSG_LEN);

  // clear out the current tweet
  memset(tweets[curWriteTweetI], 0, MAX_MSG_LEN);

  Serial.print("Copying tweet into buffer: ");

  // copy packet into the message buffer
  uint8_t i=0;
  for (; i < len - 2; i++) {
    tweets[curWriteTweetI][i] = packet[i + 2]; // +2 to skip initial '^t'
    Serial.print(tweets[curWriteTweetI][i]);
    Serial.print(",");
  }
  tweets[curWriteTweetI][i] = 0; // null term

  Serial.print("\n");

  // Display the new tweet when it comes in
  curReadTweetI = (curWriteTweetI - 1) % MAX_NUM_TWEETS; // -1 bc resetBCFsm() puts into idle stage, and transition reads next
  resetBlinkControllerFsm();
  
  curWriteTweetI = (curWriteTweetI + 1) % MAX_NUM_TWEETS;
}


char *numberChars = "0123456789";

void printAndUartLine(char *strPtr) {
  while(*strPtr) {
    Serial.print(*strPtr);
    bleuart.write(*strPtr);
    strPtr++;
  }
  Serial.print('\n');
  bleuart.write('\n');
}

void dumpMessagesToUart() {
  printAndUartLine("Dumping tweets");
  
  for (uint8_t twI=0; twI < MAX_NUM_TWEETS; twI++) {
    Serial.print(twI);
    Serial.print(":");
    
    bleuart.write(numberChars[twI]);
    bleuart.write(':');

    if (twI == curReadTweetI) {
      Serial.print('r');
      bleuart.write('r');
    }

    if (twI == curWriteTweetI) {
      Serial.print('w');
      bleuart.write('w');
    }

    Serial.print(' ');
    bleuart.write(' ');

    printAndUartLine(tweets[twI]);
  }
}

char* readTweet() {
  uint8_t tweetI = curReadTweetI;
  curReadTweetI = (curReadTweetI + 1) % MAX_NUM_TWEETS;
  return tweets[ curReadTweetI ];
}


