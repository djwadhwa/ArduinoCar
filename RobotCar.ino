// DJ Wadhwa
// 7/13/2016
#include "C:\Users\s-dwadhwa\Desktop\Arduino\Lesson10SimpleTones\pitches.h"
#include <C:\Users\s-dwadhwa\Desktop\Arduino\Adafruit_NeoPixel-master\Adafruit_NeoPixel.h>
#include <C:\Users\s-dwadhwa\Desktop\Arduino\Adafruit_NeoPixel-master\Adafruit_NeoPixel.cpp>

// smart led strip
#define led 7
#define led2 9
Adafruit_NeoPixel myStrip = Adafruit_NeoPixel (5, led, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel mybulb = Adafruit_NeoPixel (5, led2, NEO_RGB + NEO_KHZ800);
// audio
#define soundPin 8

//Address
#define ADDRESS_TV 1		// TV
#define ADDRESS_VCR 11		// VCR
#define ADDRESS_DVD 26		// DVD 
#define ADDRESS_SAT 4		// Satellite box
#define ADDRESS_CBL 2           // second VCR

// initialize to desired address
uint8_t addressUs = ADDRESS_VCR;

//Reading the remote
#define DEBUG 0  // comment out to turn off
#define IRInputPin0 1
#define IRInputPin1 2
#define IRInputPin2 4
#define IRInputPin3 8
#define IRInputPin4 16
#define IRInputPin5 32
#define IRInputPin6 64
#define IRInputPin7 128

#define IRPin_PIN  PIND

// Identify the pin of the IR sensor.
#define IRPin      IRInputPin6	// use left sensor

///////////////// UNDERSTANDING THE SONY IR PROTOCOL
// Sony protocol has 12 data bits in its message.
#define NUMBITS 12
// When a button is pushed an encoded message is transmitted
// via infrared. The message is sent in groups of three.
// Each message begins with a special pulse to identify
// the start. This is followed by a set of 12 pulses.
// Each of these is either 600 or 1200 microseconds long.
// If 1200 the value of that pulse is considered to be 1.
// If 600 the value is considered to be 0.
// Contained in this set of pulses is an identification of
// what type of device is to be controlled (TV, Cable box, etc.)
// This is followed by a number indicating which button is
// being pushed (volume up, channel down, the number 6, etc.)
// See the IR Decoding lesson for diagrams and a more
// detailed explanation of the Sony IR format.

#define RES 50  // resolution


#define LOWUS 600
#define HIGHUS 1200

#define MAXPULSE 5000 // longest pulse we'll accept

#define MAXREST 20000 // longest we'll wait for a pulse

uint16_t currentMessage = 0;
uint8_t command = 0;	// command portion of message
uint8_t previousCommand;	// holds last acted upon command
uint16_t address = 0;	// address portion of message

#define HIGHP true
#define LOWP false
//Remote Declerations
#define MOTORS_STOP 37    	// 5 on remote
#define MOTORS_FORWARD 1	// 2 on remote
#define MOTORS_REVERSE 7	// 8 on remote
#define MOTORS_SPINLEFT 3	// 4 on remote
#define MOTORS_PIVOT_RIGHT_FORWARD 2 //3 on remote
#define MOTORS_PIVOT_LEFT_FORWARD 0 //1 on remote
#define MOTORS_PIVOT_RIGHT_REVERSE 6 //7 on remote
#define MOTORS_PIVOT_LEFT_REVERSE 8 //9 on remote
#define MOTORS_SPINRIGHT 5	// 6 on remote

#define MOTORS_STOP_BUTTON 11		// center button
#define MOTORS_FORWARD_ARROW 121	//up arrow
#define MOTORS_REVERSE_ARROW 122	//down arrow 
#define MOTORS_SPINLEFT_ARROW 123	//left arrow
#define MOTORS_SPINRIGHT_ARROW 124	//right arrow

//// END OF IR //////

/////CONTROLLING MOTORS ON H-BRIDGE /////
#define MC1LEFT 3
#define MC2LEFT 2
#define MC1RIGHT 5
#define MC2RIGHT 4

#define MOTORSTOP 0
#define MOTORCW 2
#define MOTORCCW 1

#define MOTORLEFT 0
#define MOTORRIGHT 1

////smart led colors\\\\
#define RPLUS 48
#define GPLUS1 92
#define GPLUS2 20
#define BPLUS 49

#define RMINUS 51
#define GMINUS 50 
#define BMINUS 52
#define RESET 57

int r=155;
int g=155;
int b=155;
void setup()
{
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("In debug mode");
#endif

//Theme song
  playTheme();
  
  //leds
  myStrip.begin();
mybulb.begin();

//led colors 
Color(r,g,b);

  // Initialize the bridge control pins
  pinMode(MC1LEFT, OUTPUT);
  pinMode(MC2LEFT, OUTPUT);
  pinMode(MC1RIGHT, OUTPUT);
  pinMode(MC2RIGHT, OUTPUT);

  // Initialize IR receiver pin
  pinMode(IRPin, INPUT);
  pinMode(led, OUTPUT);
  pinMode (soundPin, OUTPUT);
  // Initialize the command and previous command to
  // reflect the motors are stopped. Then stop them
  command = MOTORS_STOP;
  previousCommand = command;
  setDirection();
  //Play the theme song of the team

}

// MANDATORY Method. This method is run over and over and over.
void loop()
{
  if (checkForMessage())
  {
    Serial.println("Messsage");  // debug
    processMessage();	// content is in currMessage
  } 
  else {
//    delay (27);
  allStop();
Color(r,g,b);
  }
}

// wait until the pulse of type TYPE ends. returns -1 if timedout,
// otherwise returns length of pulse
int wait(boolean type, int timeout)
{
  int num = 0;  // we wait in increments of RES in microseconds.
  while ( ( type == HIGHP && (IRPin_PIN & IRPin) )
          ||
          ( type == LOWP && !(IRPin_PIN & IRPin ) ) )
  {
    // pin is still HIGH
    delayMicroseconds(RES);
    num++;
    // if the pulse is too long we 'timed out'
    if (num * RES >= timeout)
    {
      return -1;
    }
  }

  return num * RES; // return the duration of the pulse
}

// Data is collected here.
boolean checkForMessage()
{
  currentMessage = 0;

  if ( wait(HIGHP, MAXREST) == -1)
  { //timeout ( before sees a low pulse)
    return false;
  }

  if (wait(LOWP, MAXPULSE) == -1) //start pulse
  {
    return false; //timeout
  }
  //start decoding message

  for (int i = 0; i < NUMBITS; i++)
  {
    if (wait(HIGHP, MAXPULSE) == -1)
    {
      return false;
    }
    int data = wait(LOWP, MAXPULSE);
    if (data == -1)
    {
      return false;  //timeout
    }

    // Here the pulse is put in it's
    // appropriate bit position.
    if (data * 2 > LOWUS + HIGHUS)
    {
      // set bit to one
      currentMessage = currentMessage | (1 << i);
    }
  }

  return true;

}

///////////// PROCESS MESSAGE /////////////////
void processMessage()
{
  // break into command and address
  command = currentMessage & 127;
  address = (currentMessage >> 7);

  // debug
#ifdef DEBUG
  Serial.print("Message Address: ");
  Serial.print(address, DEC);
  Serial.print("   Us: ");
  Serial.println(addressUs);
#endif

  // return if message not for us
  if (address != addressUs) {
#ifdef DEBUG
    Serial.println("Message not for us");
#endif
return;
      }
  // debug

  Serial.print("Previous cmd: ");
  Serial.print(previousCommand);
  Serial.print("   Cmd: ");
  Serial.println(command);


  // do nothing if command is not different from
  // the previous command 
   setDirection();
}

void setDirection() {
#ifdef DEBUG
  Serial.print("command: ");
  // Serial.println(command);
#endif
  switch (command) {
    //case MOTORS_STOP_BUTTON:
    case MOTORS_STOP:
#ifdef DEBUG
      Serial.println("Stop Motors");
#endif
      allStop();
      break;

    //case MOTORS_FORWARD_ARROW:
    case MOTORS_FORWARD:
#ifdef DEBUG
      Serial.println("forward");
#endif
      forward();
  delay (100);
  Color(0,255,0);
      break;
    //case MOTORS_REVERSE_ARROW:
    case MOTORS_REVERSE:
#ifdef DEBUG
      Serial.println("reverse");
#endif
  Color(0,0,255);  
      reverse();
      reverseAudio();
delay(40);

      break;
  //  case MOTORS_SPINLEFT_ARROW:
    case MOTORS_SPINLEFT:
#ifdef DEBUG
      Serial.println("spin left");
#endif
  Color(255,0,255);
      spinLeft();
delay(40);
  
      break;
//    case MOTORS_SPINRIGHT_ARROW:
case MOTORS_SPINRIGHT:
#ifdef DEBUG
      Serial.println("spin right");
#endif
      spinRight();
      delay(50);
      Color(255,0,255);
  break;

    case MOTORS_PIVOT_RIGHT_FORWARD:
      pivotRightForward();
      delay(70);
      Color(0,255,255);
    break;

    case MOTORS_PIVOT_LEFT_FORWARD:
      pivotLeftForward();
      delay(50);
      Color(0,255,255);
    break;

    case MOTORS_PIVOT_LEFT_REVERSE:
      pivotRightReverse();
      delay(50);
      Color(0,255,255);
      break;

    case MOTORS_PIVOT_RIGHT_REVERSE:
      pivotLeftReverse();
      delay(50);
      Color(0,255,255);
      break;

//Aesthetics
case RPLUS:
    r+=5;
    break;
    
    case GPLUS1:
    case GPLUS2: 
    g+=5;
    break;
    
    case BPLUS:
    b+=5;
    break;
    
    case RMINUS:
    r-=5;
    break;
    
    case GMINUS: 
    g-=5;
    break;

    case BMINUS:
    b-=5;
    break;

    case RESET:
    r=155;
    g=155; 
    b=155;
    break;
    
    default:
#ifdef DEBUG
      Serial.println("unrecognized command");
#endif

  }
  previousCommand = command;
}
//Movement and maneuverability
void reverse() {
  spinMotor(MOTORLEFT, MOTORCCW);
  spinMotor(MOTORRIGHT, MOTORCW);
}

void forward() {
  spinMotor(MOTORLEFT, MOTORCW);
  spinMotor(MOTORRIGHT, MOTORCCW);
}

void spinRight() {
  spinMotor(MOTORLEFT, MOTORCW);
  spinMotor(MOTORRIGHT, MOTORCW);
}

void spinLeft() {
  spinMotor(MOTORLEFT, MOTORCCW);
  spinMotor(MOTORRIGHT, MOTORCCW);
}
void pivotRightForward() {

  spinMotor (MOTORLEFT, MOTORCW);
  //spinMotor (MOTORRIGHT, MOTORSTOP);
}
void pivotLeftForward() {
  spinMotor (MOTORRIGHT, MOTORCCW);
  //spinMotor (MOTORLEFT, MOTORSTOP);
}

void pivotLeftReverse() {
  spinMotor (MOTORRIGHT, MOTORCW);
//  spinMotor (MOTORLEFT, MOTORSTOP);
}

void pivotRightReverse() {
  spinMotor (MOTORLEFT, MOTORCCW);
//spinMotor (MOTORRIGHT, MOTORSTOP);
}
void allStop() {
  spinMotor(MOTORLEFT, MOTORSTOP);
  spinMotor(MOTORRIGHT, MOTORSTOP);
}
void spinMotor(int motor, int dir){
  switch(motor){
    case MOTORLEFT:
      switch(dir){
        case MOTORCW:
          #ifdef DEBUG
            Serial.println("Inside MOTORCW for MOTORLEFT");
          #endif
          digitalWrite(MC1LEFT, HIGH);
          digitalWrite(MC2LEFT, LOW);
          break;
        case MOTORCCW:
          digitalWrite(MC1LEFT, LOW);
          digitalWrite(MC2LEFT, HIGH);
          break;
        default:
          digitalWrite(MC1LEFT, LOW);
          digitalWrite(MC2LEFT, LOW);
          break;
      }
      break;
    case MOTORRIGHT:
      switch(dir){
        case MOTORCW:
          digitalWrite(MC1RIGHT, HIGH);
          digitalWrite(MC2RIGHT, LOW);
          break;
        case MOTORCCW:
          digitalWrite(MC1RIGHT, LOW);
          digitalWrite(MC2RIGHT, HIGH);
          break;
        default:
          digitalWrite(MC1RIGHT, LOW);
          digitalWrite(MC2RIGHT, LOW);
          break;
      }
      break;
    default:
      break;
  }
}

//audio for theme
void playTheme() {
  play(NOTE_B4, 300);
  play(NOTE_D5, 300);
  play(NOTE_B4, 300);
  play(NOTE_A4, 600);
  play(NOTE_D5, 900);
  play(NOTE_G4, 300);
  play(NOTE_A4, 300);
  play(NOTE_G4, 300);
  play(NOTE_FS4, 600);
  play(NOTE_D4, 900);
  noTone(soundPin);
}
void Color(int r, int g, int b){
myStrip.setBrightness(255);
mybulb.setBrightness(255);
for (int i = 0; i<7; i++){
myStrip.setPixelColor(i,r, g, b);
}
mybulb.setPixelColor(0, r,g,b);
mybulb.show();
myStrip.show();
}
//audio for reversing
void reverseAudio (){
play (NOTE_FS5,10);
}

//play functions to make writing audio methods easier.
void play(int i, int time) {
  tone(soundPin, i, time);
  delay(time);
}
//////////// END OF PROGRAM /////////////////
