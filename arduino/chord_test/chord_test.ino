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
Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

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
  
  pinMode(testLEDpin, OUTPUT);
  
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

  //parce the incoming chars from Serial Port
  parceIncomingChar();
  
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
void checkMicrophone(){
 unsigned long startMillis= millis();  // Start of sample window
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
   if(volts > 2.0){
     digitalWrite(testLEDpin, HIGH);
   } else {
     digitalWrite(testLEDpin, LOW);
   }
  
}

//================ parceIncomingChar() ================
//parce incoming characters from Serial Port.
//This function will be re-written with a different protocol.

void parceIncomingChar() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    
    switch(c){
      case 'A':
        turnOffAllLEDs();
        matrix.displaybuffer[2] = _BV(0);
        matrix.displaybuffer[3] = _BV(1);
        break;
      
      case 'B':
        turnOffAllLEDs();
        matrix.displaybuffer[0] = _BV(1);
        matrix.displaybuffer[1] = _BV(1);
        matrix.displaybuffer[2] = _BV(2);
        matrix.displaybuffer[3] = _BV(3);
        break;
        
      case 'C':
        turnOffAllLEDs();
        matrix.displaybuffer[0] = _BV(2);
        break;
        
      case 'D':
        turnOffAllLEDs();
        matrix.displaybuffer[1] = _BV(1);
        matrix.displaybuffer[2] = _BV(1);
        matrix.displaybuffer[3] = _BV(1);
        break;
        
      case 'E':
        turnOffAllLEDs();
        matrix.displaybuffer[0] = _BV(1);
        matrix.displaybuffer[1] = _BV(3);
        matrix.displaybuffer[2] = _BV(3);
        matrix.displaybuffer[3] = _BV(3);
        break;
        
      case 'F':
        turnOffAllLEDs();
        matrix.displaybuffer[1] = _BV(0);
        matrix.displaybuffer[3] = _BV(1);
        break;
        
      case 'G':
        turnOffAllLEDs();
        matrix.displaybuffer[0] = _BV(1);
        matrix.displaybuffer[1] = _BV(2);
        matrix.displaybuffer[2] = _BV(1);
        break;
        
      case '1':
        turnOnAllLEDs();
        break;
          
      case '0':
        turnOffAllLEDs();
        break;
        
      case '+':
        if(!timer.isEnabled(anim_pileDown_timerId)){
          timer.enable(anim_pileUp_timerId);
        }
        break;
        
      case '-':
        if(!timer.isEnabled(anim_pileUp_timerId)){
          timer.enable(anim_pileDown_timerId);
        }
        break;
      
      case 'r':
        timer.enable(anim_random_timerId);
        break;
        
      case 'u':
        if(!timer.isEnabled(anim_pickUp_timerId)){
          timer.enable(anim_pickDown_timerId);
        }
        
      case 'd':
        if(!timer.isEnabled(anim_pickDown_timerId)){
          timer.enable(anim_pickUp_timerId);
        }
        
      //default:
        //do nothing
    }
    
  }
}

//================ turnOnAllLEDS() ================
//turn on all the LEDs on the fingerboards
void turnOnAllLEDs(){
  for(int stringNum = 0; stringNum < 4; stringNum++){
    for(int fretNum = 0; fretNum < 14; fretNum++){
      matrix.displaybuffer[stringNum] |= _BV(fretNum); 
    }
  }
}

//================ turnOffAllLEDS() ================
//turn off all the LEDs on the fingerboards
void turnOffAllLEDs(){
  for(int stringNum = 0; stringNum < 4; stringNum++){
    matrix.displaybuffer[stringNum] = 0; 
  }
}

//================ anim_pileUp() ================
//lighting up four LEDs on each frets in sequence from top to bottom
void anim_pileUp(){
  static int counter = 0;
  
  matrix.displaybuffer[0] |= _BV(counter);
  matrix.displaybuffer[1] |= _BV(counter);
  matrix.displaybuffer[2] |= _BV(counter);
  matrix.displaybuffer[3] |= _BV(counter);
  
  counter++;
  if(counter > 13){
    counter = 0;
    timer.disable(anim_pileUp_timerId);
  }
}

//================ anim_pileDown() ================
//turning off four LEDs on each frets in sequence from bottom to top
void anim_pileDown(){
  static int counter = 13;
  
  matrix.displaybuffer[0] &= ~_BV(counter);
  matrix.displaybuffer[1] &= ~_BV(counter);
  matrix.displaybuffer[2] &= ~_BV(counter);
  matrix.displaybuffer[3] &= ~_BV(counter);
  
  counter--;
  if(counter < 0){
    counter = 13;
    timer.disable(anim_pileDown_timerId);
  }
}

//================ anim_random() ================
//light up LEDs randomly for 2 seconds
void anim_random(){
  static int counter = 0;
  
  matrix.displaybuffer[0] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  matrix.displaybuffer[1] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  matrix.displaybuffer[2] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  matrix.displaybuffer[3] = _BV(round(random(14))) | _BV(round(random(14))) | _BV(round(random(14)));
  
  counter++;
  if(counter > 20){
    counter = 0;
    timer.disable(anim_random_timerId);
    turnOffAllLEDs();
  }
  
}

//================ anim_pickDown() ================
//animation for picking strings from up to down
void anim_pickDown(){
  static int counter = 0;
  
  matrix.displaybuffer[counter] |= _BV(14);
  
  counter++;
  if(counter > 4){
    counter = 0;
    timer.disable(anim_pickUp_timerId);
    turnOffStringLEDs();
  }
}

//================ anim_pickUp() ================
//animation for picking strings from down to up
void anim_pickUp(){  
  static int counter = 3;
  
  matrix.displaybuffer[counter] |= _BV(14);
  
  counter--;
  if(counter < -1){
    counter = 3;
    timer.disable(anim_pickDown_timerId);
    turnOffStringLEDs();
  }
}

//================ turnOffStringLEDs() ================
//turning off all the LEDs for strings
void turnOffStringLEDs(){
  for(int stringNum = 0; stringNum < 4; stringNum++){
    matrix.displaybuffer[stringNum] &= ~_BV(14); 
  }
}

