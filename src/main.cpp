#include <Arduino.h>
#include <Adafruit_Neopixel.h>
#include <map>

#define NEOPIXEL D4
Adafruit_NeoPixel pixel(1, NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Display dimensions
#define NUM_DIGITS 10
#define NUM_SEGMENTS 9

// HV5812 Interface Pins
#define DIN D0 // Input
#define CLK D1 // Clock
#define STR D2 // Strobe (Display)
#define BLK D3 // Blank (Clear)

// Each of the segment pins to their corresponding HVout pin on the HV5812...
#define SEG_A  17
#define SEG_B  16
#define SEG_F  1
#define SEG_G  2
#define SEG_C  15
#define SEG_E  3
#define SEG_D  14
#define SEG_DP 13
#define SEG_H  5

// Each of the digit pins to their HVout pin...
#define DIG_1  4
#define DIG_2  13
#define DIG_3  12
#define DIG_4  11
#define DIG_5  10
#define DIG_6  9
#define DIG_7  8
#define DIG_8  7
#define DIG_9  6
#define DIG_10 5

#include <patterns.h>

/* We can only assume the segments' order until we actually test the board.
*   _
* |   |
*   _
* |   |
*   _
*    .
*     ,
*
*   A
* B   F
*   G
* C   E
*   D
*    DP
*     H
*/

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initialising...");
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(STR, OUTPUT);
  pinMode(BLK, OUTPUT);
  pixel.begin();
  pixel.clear();
  pixel.show();
  Serial.println("Initialisation finished.");
}

void loop() {
  for (int i = 0; i < NUM_DIGITS; i++) {
    digitalWrite(BLK, HIGH);
    uint32_t packet = PATTERN_FULL;
    packet |= (1UL << DIGITS[i]);
    delay(2);
    
    // clock an extra 0 because there's only 19 display segments
    digitalWrite(DIN, LOW);
    delay(2);
    digitalWrite(CLK, HIGH);
    delay(2);
    digitalWrite(CLK, LOW);
    for (int j = 0; j < 19; j++) {
      // int k = 18 - j; 
      bool bitValue = (packet & (1U << j) != 0);
      digitalWrite(DIN, bitValue ? HIGH : LOW);
      delay(2);
      digitalWrite(CLK, HIGH);
      delay(2);
      digitalWrite(CLK, LOW);
    }

    digitalWrite(STR, HIGH);
    delay(2);
    digitalWrite(STR, LOW);
    delay(2);
    digitalWrite(BLK, LOW);
    delay(2);
  }
}