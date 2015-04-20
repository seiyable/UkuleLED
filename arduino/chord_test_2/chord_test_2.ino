//================ import libraries ================
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <SimpleTimer.h>
#include <cmath>

//================ define ================
#ifndef _BV
#define _BV(bit) (1<<(bit))
#endif

#define testLEDpin 2

//================ global variables ================
//an instance storing bit map
Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

//an array storing a sequence of chords received from phone
//each chord is expressed like in this array: [2, 3, 4, 0]
//this means "pick the 2nd fret on the first string, the 3rd fret on the second string.."
//the maximum number of the stored chords is 40 for now
int chordSequence[40][4];
int chordOnDuty = 0;

//variables for microphone
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

//variables for timer & animation
SimpleTimer timer; // the timer object
int anim_pileUp_timerId;
int anim_pileDown_timerId;
int anim_random_timerId;
int anim_pickDown_timerId;
int anim_pickUp_timerId;

//================ setup() ================
void setup() {
  Serial.begin(9600);

  //for debug use
  pinMode(testLEDpin, OUTPUT);

  //reset chord sequence
  resetChordSequence();

  //set timers
  anim_pileUp_timerId = timer.setInterval(100, anim_pileUp);
  anim_pileDown_timerId = timer.setInterval(100, anim_pileDown);
  anim_random_timerId = timer.setInterval(100, anim_random);
  anim_pickDown_timerId = timer.setInterval(20, anim_pickDown);
  anim_pickUp_timerId = timer.setInterval(20, anim_pickUp);
  timer.disable(anim_pileUp_timerId);
  timer.disable(anim_pileDown_timerId);
  timer.disable(anim_random_timerId);
  timer.disable(anim_pickDown_timerId);
  timer.disable(anim_pickUp_timerId);

  matrix.begin(0x70);  // pass in the address to the matrix instance
  turnOffAllLEDs(); //turn off all LEDs at first
}

//================ loop() ================
void loop() {
  //check the input vlotage from the attached microphone
  checkMicrophone();

  //parse the incoming string from Serial Port
  parseIncomingString();

  //check the chord sequence array for debug use
  //checkChordSequence();

  //update LED status
  updateLEDstatus();

  //run timer
  timer.run();

  // write the changes we just made to the display
  matrix.writeDisplay();

}


//================ checkMicrophone() ================
//Check the input voltage from the attached microphone
//to notify that the string has been picked to the connected phone
//by sending back a signal.
//
//*This function will be replaced by sound analyzing function on the phone.
void checkMicrophone() {
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(0);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double volts = (peakToPeak * 3.3) / 1024;  // convert to volts

  //Serial.println(volts);

  //if the input voltage is greater than a threshold amount,
  //send a signal back to the connected phone
  //to notify that the string has been picked
  if (volts > 2.0) {
    digitalWrite(testLEDpin, HIGH);
    chordOnDuty++; //move to next chord
    Serial.print(">>");
  } else {
    digitalWrite(testLEDpin, LOW);
  }

}

//================ resetChordSequence() ================
//reset all the values in the chord sequence array
void resetChordSequence() {
  for (int chordIndex = 0; chordIndex < 40; chordIndex++) {
    for (int stringNum = 0; stringNum < 4; stringNum++) {
      chordSequence[chordIndex][stringNum] = 9; //set an unreal number to represent this is blank
    }
  }
}

//================ checkChordSequence() ================
//print out all the values in the chord sequence array
void checkChordSequence() {
  for (int chordIndex = 0; chordIndex < 40; chordIndex++) {
    Serial.print("Chord Index: ");
    Serial.println(chordIndex);
    for (int stringNum = 0; stringNum < 4; stringNum++) {
      Serial.print(chordSequence[chordIndex][stringNum]);
      Serial.print(',');
    }
    Serial.println("---------------");
  }
}

//================ parseIncomingString() ================
//parse incoming string from Serial Port.
//The format of the string is like this: "command/content/"
//so parse the command first, and check the content next.
void parseIncomingString() {
  if (Serial.available() > 0) {

    //get the whole string at first
    String s = Serial.readString();
    Serial.println(s);

    int firstSlashIndex = s.indexOf('/');
    String command = s.substring(0, firstSlashIndex);
    int secondSlashIndex = s.indexOf('/', firstSlashIndex + 1);
    String content = s.substring(firstSlashIndex + 1, secondSlashIndex);

    Serial.print("command: ");
    Serial.println(command);
    Serial.print("content: ");
    Serial.println(content);

    // Prepare the character array (the buffer) for the content string
    char char_array_content[content.length() + 1]; //last char is null representing end of a string

    // Copy it over
    content.toCharArray(char_array_content, content.length() + 1);

    //------------------- showChordSeq -------------------
    if (command == "showChordSeq") {
      resetChordSequence(); //reset the array first
      int chordIndex = 0;
      int stringNum = 0;

      for (int i = 0; i < content.length() - 1; i++) {

        //iterate through all characters
        if (content[i] != '[' && content[i] != ']' && content[i] != ',' && content[i] != ' ') {
          //if it is not a delimeter ('[' or ']' or ',' or ' '),
          //store it to the chord sequence array
          chordSequence[chordIndex][stringNum] = (int)content[i] - 48 - 1;//convert to char and subtract 1
          //          Serial.print("chordIndex: ");
          //          Serial.print(chordIndex);
          //          Serial.print(", ");
          //          Serial.print("stringNum: ");
          //          Serial.print(stringNum);
          //          Serial.print(", ");
          //          Serial.println(content[i]);

        } else if (content[i] == ',') {
          stringNum++; //move to next string

        } else if (content[i] == ']') {
          chordIndex++; //move to next chord
          stringNum = 0; //reset
        }
      }
    }

    //------------------- moveTo -------------------
    else if (command == "moveTo") {
      String str(content);//turn it to string object
      int moveTo = content.toInt(); //convert it to int
      chordOnDuty = moveTo; //move to the index
    }




  }
}

//================ updateLEDstatus() ================
//update values in the bitmap array to show current chord
void updateLEDstatus() {

  if (chordSequence[chordOnDuty][0] == 9) {
    //if there is no value in the slot,
    resetChordSequence(); //reset the chord sequence array
    chordOnDuty = 0; //reset the current chord index
    turnOffAllLEDs(); //turn off all LEDs
    return; //bail out from this function
  }

  //overwrite the values in the bitmap array
  for (int stringNum = 0; stringNum < 4; stringNum++) {
    if (chordSequence[chordOnDuty][stringNum] == -1) {
      //don't light up any LEDs on the string
      matrix.displaybuffer[stringNum] = 0;
    } else {
      //light up one LED on the string
      matrix.displaybuffer[stringNum] = _BV(chordSequence[chordOnDuty][stringNum]);
    }
  }
}

//================ turnOnAllLEDS() ================
//turn on all the LEDs on the fingerboards
void turnOnAllLEDs() {
  for (int stringNum = 0; stringNum < 4; stringNum++) {
    for (int fretNum = 0; fretNum < 14; fretNum++) {
      matrix.displaybuffer[stringNum] |= _BV(fretNum);
    }
  }
}

//================ turnOffAllLEDS() ================
//turn off all the LEDs on the fingerboards
void turnOffAllLEDs() {
  for (int stringNum = 0; stringNum < 4; stringNum++) {
    matrix.displaybuffer[stringNum] = 0;
  }
}

//================ anim_pileUp() ================
//lighting up four LEDs on each frets in sequence from top to bottom
void anim_pileUp() {
  static int counter = 0;

  matrix.displaybuffer[0] |= _BV(counter);
  matrix.displaybuffer[1] |= _BV(counter);
  matrix.displaybuffer[2] |= _BV(counter);
  matrix.displaybuffer[3] |= _BV(counter);

  counter++;
  if (counter > 13) {
    counter = 0;
    timer.disable(anim_pileUp_timerId);
  }
}

//================ anim_pileDown() ================
//turning off four LEDs on each frets in sequence from bottom to top
void anim_pileDown() {
  static int counter = 13;

  matrix.displaybuffer[0] &= ~_BV(counter);
  matrix.displaybuffer[1] &= ~_BV(counter);
  matrix.displaybuffer[2] &= ~_BV(counter);
  matrix.displaybuffer[3] &= ~_BV(counter);

  counter--;
  if (counter < 0) {
    counter = 13;
    timer.disable(anim_pileDown_timerId);
  }
}

//================ anim_random() ================
//light up LEDs randomly for 2 seconds
void anim_random() {
  static int counter = 0;

  matrix.displaybuffer[0] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  matrix.displaybuffer[1] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  matrix.displaybuffer[2] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  matrix.displaybuffer[3] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));

  counter++;
  if (counter > 20) {
    counter = 0;
    timer.disable(anim_random_timerId);
    turnOffAllLEDs();
  }

}

//================ anim_pickDown() ================
//animation for picking strings from up to down
void anim_pickDown() {
  static int counter = 0;

  matrix.displaybuffer[counter] |= _BV(14);

  counter++;
  if (counter > 4) {
    counter = 0;
    timer.disable(anim_pickUp_timerId);
    turnOffStringLEDs();
  }
}

//================ anim_pickUp() ================
//animation for picking strings from down to up
void anim_pickUp() {
  static int counter = 3;

  matrix.displaybuffer[counter] |= _BV(14);

  counter--;
  if (counter < -1) {
    counter = 3;
    timer.disable(anim_pickDown_timerId);
    turnOffStringLEDs();
  }
}

//================ turnOffStringLEDs() ================
//turning off all the LEDs for strings
void turnOffStringLEDs() {
  for (int stringNum = 0; stringNum < 4; stringNum++) {
    matrix.displaybuffer[stringNum] &= ~_BV(14);
  }
}

