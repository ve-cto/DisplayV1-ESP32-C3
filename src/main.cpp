#include <Arduino.h>
#include <Adafruit_Neopixel.h>
#include <SECRETS.h> // wifi ssid & pass
#include <array>
#include <vector>

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

/*
*   A
* F   B
*   G
* E   C
*   D
*    DP
*     H
*/
// Each of the segment pins to their corresponding HVout pin on the HV5812...
#define SEG_A  3
#define SEG_B  4
#define SEG_C  5
#define SEG_D  6
#define SEG_E  17 
#define SEG_F  19
#define SEG_G  18
#define SEG_H  2
#define SEG_DP 16

// Each of the digit pins to their HVout pin...
#define DIG_0  1
#define DIG_1  15
#define DIG_2  14
#define DIG_3  13
#define DIG_4  12
#define DIG_5  11
#define DIG_6  10
#define DIG_7  9
#define DIG_8  8
#define DIG_9  7

#include <patterns.h>

// one buffer per digit
uint32_t digitBuffers[10] = {PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR, PATTERN_CLEAR};

/* we can only assume the segments' order until we actually test the board
*   _
* |   |
*   _
* |   |
*   _
*    .
*     ,
*
*   A
* F   B
*   G
* E   C
*   D
*    DP
*     H
*/

struct SpinnerEntry {
  long _start = millis();
  long _lastUpdate = 0;
  uint8_t digit;
  bool clockwise = true;
  uint8_t speed = 100; // milliseconds per segment
  uint8_t iterations = 0;
  uint8_t _count = 0;
  uint8_t _segment = 0; // 0 -> 5
};

struct Spinners {
  std::vector<SpinnerEntry> entries;
};
Spinners spinners;

void writePacket(uint32_t packet) {
  digitalWrite(BLK, HIGH);
  delayMicroseconds(1);

  for (int j = 0; j < 20; j++) {
    bool bitValue = (packet & (1UL << j)) != 0;
    digitalWrite(DIN, bitValue ? HIGH : LOW);
    delayMicroseconds(1);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(CLK, LOW);
  }

  digitalWrite(DIN, LOW);
  delayMicroseconds(1);
  digitalWrite(STR, HIGH);
  delayMicroseconds(1);
  digitalWrite(STR, LOW);
  delayMicroseconds(1);
  digitalWrite(BLK, LOW);
  delayMicroseconds(1);
}
void updateDisplay(uint32_t delayus = 10000) {
  uint32_t start = micros();
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (delayus > 0) {
      while (micros() - start < (delayus * (i + 1)) / NUM_DIGITS) {
        // wait until the next digit
      }
    }
    writePacket(digitBuffers[i]);
  }
}

bool setDigitBuffer(uint8_t digit, uint32_t packet) {
  // bool t = false;
  // for (const auto &spinner : spinners.entries) {
  //   if (t) {continue;}
  //   if (spinner.digit == digit) {
  //     t = true;
  //     break;
  //   }
  // }
  // if (t) {
  //   // the digit is being controlled by a spinner, don't write to it to avoid conflicts
  //   return 0;
  // } else {
    digitBuffers[digit] = (packet | (1UL << DIGITS[digit]));
    return 1;
  // }
  // return 0;
}

void setAllDigitsBuffer(uint32_t pack) {
  for (int i = 0; i < NUM_DIGITS; i++) {
    setDigitBuffer(i, pack);
  }
}
void clearDisplay(bool update = false) {
  setAllDigitsBuffer(PATTERN_CLEAR);
  if (update) {updateDisplay();}
}
void fillDisplay(bool update = false) {
  setAllDigitsBuffer(PATTERN_FULL);  
  if (update) {updateDisplay();}
}

std::array<uint, 3> getTimestamp() {
  long totalSeconds = millis() / 1000;
  uint seconds = totalSeconds % 60;
  uint minutes = (totalSeconds / 60) % 60;
  uint hours = (totalSeconds / 3600) % 24;
  return {seconds, minutes, hours};
}
void showTime(int style, bool update = false) {
  auto timestamp = getTimestamp();
  uint seconds = timestamp[0];
  uint minutes = timestamp[1];
  uint hours = timestamp[2];
  // style 1
  // -, H1, H2, -, M1, M2, -, S1, S2, -
  
  // style 2
  // -, -, H1, H2, -, -, M1, M2, -, -

  uint h1 = (hours / 10) % 10;
  uint h2 = hours - h1*10;
  uint m1 = (minutes / 10) % 10;
  uint m2 = minutes - m1*10;
  uint s1 = (seconds / 10) % 10;
  uint s2 = seconds - s1*10;

  if (style == 1) {
    setDigitBuffer(0, 0);
    setDigitBuffer(1, PATTERN_NUMBERS[h1]);
    setDigitBuffer(2, PATTERN_NUMBERS[h2]);
    setDigitBuffer(3, 0);
    setDigitBuffer(4, PATTERN_NUMBERS[m1]);
    setDigitBuffer(5, PATTERN_NUMBERS[m2]);
    setDigitBuffer(6, 0);
    setDigitBuffer(7, PATTERN_NUMBERS[s1]);
    setDigitBuffer(8, PATTERN_NUMBERS[s2]);
    setDigitBuffer(9, 0);
  } else if (style == 2) {
    setDigitBuffer(0, 0);
    setDigitBuffer(1, 0);
    setDigitBuffer(2, PATTERN_NUMBERS[h1]);
    setDigitBuffer(3, PATTERN_NUMBERS[h2]);
    setDigitBuffer(6, 0);
    setDigitBuffer(7, 0);
    setDigitBuffer(4, PATTERN_NUMBERS[m1]);
    setDigitBuffer(5, PATTERN_NUMBERS[m2]);
    setDigitBuffer(8, 0);
    setDigitBuffer(9, 0);
  }
  if (update) {updateDisplay();}
}

void startSpinner(uint8_t digit, uint8_t iterations = 1, uint8_t speed = 100, bool clockwise = true) {
  SpinnerEntry spinner;
  spinner.digit = digit;
  spinner.clockwise = clockwise;
  spinner.speed = speed;
  spinner.iterations = iterations;
  spinners.entries.push_back(spinner);
}

// find and delete any spinners that are occupying a digit
void stopSpinner(uint8_t digit, bool clear = true) {
  for (auto si = spinners.entries.begin(); si != spinners.entries.end();) {
    auto &spinner = *si;

    if (digit == spinner.digit) {
      si = spinners.entries.erase(si);
      setDigitBuffer(digit, 0);
      continue;
    }
    ++si;
  }
}

bool isSpinner(uint8_t digit = 255) {
  if (digit == 255) {return !spinners.entries.empty();}
  
  for (auto si = spinners.entries.begin(); si != spinners.entries.end();) {
    auto &spinner = *si;

    if (digit == spinner.digit) {
      return true;
    }
    ++si;
  }
  return false;
}

void pollSpinners(boolean blankIfFinished = true) {
  // if there's no spinners, do nothing
  if (spinners.entries.empty()) {return;}
  
  // loop through all the spinners
  for (auto si = spinners.entries.begin(); si != spinners.entries.end();) {
    auto &spinner = *si;

    // if the time since last update is greater than the speed of the spinner, change the segment shown on the spinners' digit
    if ((millis() - spinner._lastUpdate) > spinner.speed) {
      setDigitBuffer(spinner.digit, SEGMENT_SPINNER_MASKS[spinner.clockwise ? spinner._segment : 5 - spinner._segment]);
      spinner._lastUpdate = millis();

      spinner._segment = (spinner._segment + 1) % 6; // increment and make sure it's a valid segment (from 0 to 5)
      if (spinner._segment == 0) {
        spinner._count += 1;
      }
    } else {
      updateDisplay();
    }

    // if the number of times the spinner has looped (_count) is >= to the target iterations of the spinner, delete it from memory
    if (spinner._count >= spinner.iterations) {
      if (blankIfFinished) {
        setDigitBuffer(spinner.digit, 0);
      }
      si = spinners.entries.erase(si);
      continue;
    }

    ++si;
  }
}

uint8_t serialBuffer[10] = {};

// position in the range of 0 <= p <= 7, where p corresponds to the bit from right to left, IE: {00000000 -> 76543210}
bool getBit(u_char byte, int position) {
  return (byte >> position) & 1; // shift the target bit to the first position, then compare it against 1 via bitwise AND
}

void checkSerial() {
  // each full display pattern is 3 bytes - that's 24 bits and we're only ever going to use up to 20 of them for the HV
  // we'll say that the first bytes' first four bits are reserved to identify the digit index so a stream of 24 bits becomes:
  
  //  first    second   third
  // PPPPRRRR PPPPPPPP PPPPPPPP
  // 76543210 76543210 76543210
  // 00000000 00000000 00000000
  if (Serial.available() >= 3) {
    uint8_t bytes[3] = {};
    Serial.readBytes(bytes, 3);
    
    // 0x0F -> 00001111
    uint8_t targetDigit = bytes[0] & 0x0F;
    
    // 00001111 -> 1+2+4+8=15 -> 0x0F
    // 11110000 -> 16+32+64+128=240 -> 0xF0
    
    // AND the first byte with 11110000 to only get the four bits of the packet, then move it to its' position in the 32 bit stream, the other bytes can stay as-is but they also have to be moved 
    uint32_t final = ((uint32_t)(bytes[0] & 0xF0) << 12) | ((uint32_t)bytes[1] << 8) | (uint32_t)bytes[2];

    if (setDigitBuffer(targetDigit, final)) {
      Serial.println("OK");
    } else {
      Serial.println("ERR");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Initialising...");
  delay(10);
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(STR, OUTPUT);
  pinMode(BLK, OUTPUT);
  digitalWrite(DIN, LOW);
  digitalWrite(CLK, LOW);
  digitalWrite(STR, LOW);
  digitalWrite(BLK, HIGH); // so it remains blanked until we write to it
  clearDisplay();
  for (int i = 0; i < 11; i++) {
    startSpinner(i, 3, 50, true);
  }

  pixel.begin();
  pixel.clear();
  pixel.show();
  delay(10);
  Serial.println("Initialisation finished.");
}

void testSegments() {
  for (int d = 0; d < 1; d++) {  // test only digit 0 first
    for (int s = 0; s < 9; s++) {
      clearDisplay();
      setDigitBuffer(d, SEGMENT_MASKS[s]);
      writePacket(digitBuffers[d]);
      Serial.print(F("Writing segment: "));
      Serial.println(s);
      delay(5000);
    }
  }
}

void testDigits() {
  for (int d = 0; d < 10; d++) {
    clearDisplay();
    setAllDigitsBuffer(0);
    setDigitBuffer(d, PATTERN_8);
    writePacket(digitBuffers[d]);
    Serial.print(F("Writing digit: "));
    Serial.println(d);
    delay(5000);
  }
}

void loop() {
  pollSpinners(true);
  if (!isSpinner()) {
    showTime(/* hhmmss */ 1);
  }
  checkSerial();
  // testDigits();
  updateDisplay(); 
  // Serial.println(digitBuffers[0], BIN); // prints 11111100000000011110, but nothing is displayed on the HV... It isn't a hardware problem.
  // delay(100);
}