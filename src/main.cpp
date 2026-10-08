#include <Arduino.h>
#include <Adafruit_Neopixel.h>
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

// Each of the segment pins to their corresponding HVout pin on the HV5812...
#define SEG_A  17
#define SEG_B  16
#define SEG_F  1
#define SEG_G  2
#define SEG_C  15
#define SEG_E  3
#define SEG_D  14
#define SEG_DP 4
#define SEG_H  18

// Each of the digit pins to their HVout pin...
#define DIG_0  19
#define DIG_1  13
#define DIG_2  12
#define DIG_3  11
#define DIG_4  10
#define DIG_5  9
#define DIG_6  8
#define DIG_7  7
#define DIG_8  6
#define DIG_9  5

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
  delay(2);
  // clock in an extra bit because we're only using 19 of them (and there are 20 total)
  digitalWrite(DIN, LOW);
  delay(2);
  digitalWrite(CLK, HIGH);
  delay(2);
  digitalWrite(CLK, LOW);
  for (int j = 0; j < 19; j++) {
    // int k = 18 - j; 
    bool bitValue = (packet & (1U << j)) != 0; // get if the bit at position j is 1
    digitalWrite(DIN, bitValue ? HIGH : LOW);
    delay(2);
    digitalWrite(CLK, HIGH);
    delay(2);
    digitalWrite(CLK, LOW);
  }
  digitalWrite(DIN, LOW);
  delay(2);
  digitalWrite(STR, HIGH);
  delay(2);
  digitalWrite(STR, LOW);
  delay(2);
  digitalWrite(BLK, LOW);
  delay(2);
}
void updateDisplay(uint32_t delayus = 1000) {
  long lastUpdate = micros();
  for (int i = 0; i < 10; i++) {
    if (micros() < (lastUpdate + delayus)) {continue;}
    writePacket(digitBuffers[i]);
    lastUpdate = micros();
  }
}

void setDigitBuffer(uint8_t digit, uint32_t packet) {
  bool t = false;
  for (const auto &spinner : spinners.entries) {
    if (t) {continue;}
    if (spinner.digit == digit) {
      t = true;
      break;
    }
  }
  if (t) {
    // the digit is being controlled by a spinner, don't write to it to avoid conflicts
  } else {
    digitBuffers[digit] = (packet | (1UL << DIGITS[digit]));
  }
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
    setDigitBuffer(2, PATTERN_NUMBERS[h1]);
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

bool isSpinner(uint8_t digit = -1) {
  if (digit == -1) {return !spinners.entries.empty();}
  
  for (auto si = spinners.entries.begin(); si != spinners.entries.end();) {
    auto &spinner = *si;

    if (digit == spinner.digit) {
      return true;
    }
    ++si;
  }
  return false;
}

void pollSpinners(boolean blankStopped = true) {
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
      continue;
    }

    // if the number of times the spinner has looped (_count) is >= to the target iterations of the spinner, delete it from memory
    if (spinner._count >= spinner.iterations) {
      if (blankStopped) {
        setDigitBuffer(spinner._segment, 0);
      }
      si = spinners.entries.erase(si);
      continue;
    }

    ++si;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initialising...");
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(STR, OUTPUT);
  pinMode(BLK, OUTPUT);
  startSpinner(0, 3);
  pixel.begin();
  pixel.clear();
  pixel.show();
  Serial.println("Initialisation finished.");
}

void loop() {
  if (!isSpinner()) {
    showTime(/* hhmmss */ 1);
  }
  pollSpinners(true);
  updateDisplay(); 
}