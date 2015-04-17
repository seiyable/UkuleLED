/***************************************************
  This is an example for our Adafruit 24-channel PWM/LED driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1429

  These drivers uses SPI to communicate, 3 pins are required to
  interface: Data, Clock and Latch. The boards are chainable

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_TLC5947.h"
#include <cmath>

// How many boards do you have chained?
#define NUM_TLC5974 1

#define data   4
#define clock   5
#define latch   6
#define oe  -1  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock, data, latch);

int rgbLEDs[8][3] = {
  {0, 0, 0}, //channel 1
  {0, 0, 0}, //channel 2
  {0, 0, 0}, //channel 3
  {0, 0, 0}, //channel 4
  {0, 0, 0}, //channel 5
  {0, 0, 0}, //channel 6
  {0, 0, 0}, //channel 7
  {0, 0, 0}  //channel 8
};

//================ setup() ================
void setup() {
  Serial.begin(9600);
  //Serial.setTimeout(100);

  //Serial.println("TLC5974 test");
  tlc.begin();
  if (oe >= 0) {
    pinMode(oe, OUTPUT);
    digitalWrite(oe, LOW);
  }

  turnOnOffAllLEDs(0);
  //Serial.println("turned All LEDs on");
}

//================ loop() ================
void loop() {
  lightUpLED();
  //parceIncomingStringForOneLED();
  parceIncomingCharForOneChord();

}

//================ parceIncomingStringForOneLED() ================
void parceIncomingStringForOneLED() {
  if (Serial.available() > 0) {
    // read the incoming byte:
    String s = Serial.readString();

    int firstIndex = s.indexOf(',');
    String stringNum = s.substring(0, firstIndex);
    //Serial.println("stringNum: " + stringNum);
    s.remove(0, firstIndex + 1);

    int secondIndex = s.indexOf(',');
    String fretNum = s.substring(0, secondIndex);
    //Serial.println("fretNum: " + fretNum);
    s.remove(0, secondIndex + 1);

    String state = s;
    //Serial.println("state: " + state);

    updateLED(stringNum.toInt(), fretNum.toInt(), state.toInt() * 4095);
  }
}

//================ parceIncomingCharForOneChord() ================
void parceIncomingCharForOneChord() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    //Serial.print("I got ");
    //Serial.println(c);

    switch(c){
      case 'A':
        turnOnOffAllLEDs(0);
        updateLED(3, 1, 4095);
        updateLED(4, 2, 4095); 
        break;
      
      case 'B':
        turnOnOffAllLEDs(0);
        updateLED(1, 2, 4095);
        updateLED(2, 2, 4095);
        updateLED(3, 3, 4095);
        updateLED(4, 4, 4095);
        break;
        
      case 'C':
        turnOnOffAllLEDs(0);
        updateLED(1, 3, 4095);
        break;
        
      case 'D':
        turnOnOffAllLEDs(0);
        updateLED(2, 2, 4095);
        updateLED(3, 2, 4095);
        updateLED(4, 2, 4095);
        break;
        
      case 'E':
        turnOnOffAllLEDs(0);
        updateLED(1, 2, 4095);
        updateLED(2, 4, 4095);
        updateLED(3, 4, 4095);
        updateLED(4, 4, 4095);
        break;
        
      case 'F':
        turnOnOffAllLEDs(0);
        updateLED(2, 1, 4095);
        updateLED(4, 2, 4095);
        break;
        
      case 'G':
        turnOnOffAllLEDs(0);
        updateLED(1, 2, 4095);
        updateLED(2, 3, 4095);
        updateLED(3, 2, 4095);
        break;
        
      case '1':
        turnOnOffAllLEDs(4095);
        break;
          
      case '0':
        turnOnOffAllLEDs(0);
        break;
    }
    
  }
}

//================ turnOnOffAllLEDs() ================
void turnOnOffAllLEDs(int _brightness) {
  for (int stringNum = 0; stringNum < 4; stringNum++) {
    for (int fretNum = 0; fretNum < 6; fretNum++) {
      updateLED(stringNum + 1, fretNum + 1, _brightness);
//      Serial.print(stringNum);
//      Serial.print(fretNum);
//      Serial.println(_state * 4095);
    }
  }
}

//================ lightUpLED() ================
void lightUpLED() {
  //light up all the LEDs
  for (int i = 0; i < NUM_TLC5974 * 8; i++) {
    tlc.setLED(i, rgbLEDs[i][0], rgbLEDs[i][1], rgbLEDs[i][2]);
    tlc.write();
    delay(100);
  }
}

//================ updateLED() ================
int updateLED(int stringNum, int fretNum, int state) {
  //get the actual pin number on the LED driver board
  int pinNum;
  if (stringNum == 1) {
    pinNum = 22 - (fretNum - 1) * 2;
  } else if (stringNum == 2) {
    pinNum = 23 - (fretNum - 1) * 2;
  } else if (stringNum == 3) {
    pinNum = 1 + (fretNum - 1) * 2;
  } else if (stringNum == 4) {
    pinNum = 0 + (fretNum - 1) * 2;
  }

  //update the rgbLEDs array
  int channel = floor(pinNum / 3);
  int targetPin = pinNum % 3;
  rgbLEDs[channel][targetPin] = state;
}
