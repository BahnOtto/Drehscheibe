// Copyright (c) 2025 BahnOtto

#ifndef StepperControl_H
#define StepperControl_H


#include "Arduino.h"

//const int STEPS_UMDREHUNG = 800; //inkl.  microsteps   800 auf keinen Fall 8000
////Stepper einrichten ( 800 Schritte / Umdrehung - 1/4 Microstep / 800 * 20 : 48 = 333,33)
//
//MoToStepper myStepper( STEPS_UMDREHUNG, STEPDIR );

const byte dirPin  = 5;  //to attach
const byte stepPin = 6;  //to attach
const byte enaPin  = 7;  //to attach


const byte relayPin   = 3;
const byte refPin 	  = 4; // Referenzswitch
const byte dirInPin   = A0;

const byte movePin    = A1;
const byte reachedPin = A2;
const byte dirLedPin  = A4;

// Steppermodul TMC 2208 ----------------------------------
#define MOTOR_FULL_STEPS_PER_REVOLUTION 200
#define REDUCTION_GEARBOX_RATIO 1
#define STEPS_PER_REVOLUTION (MOTOR_FULL_STEPS_PER_REVOLUTION * REDUCTION_GEARBOX_RATIO)
#define FULL_TURN_STEPS (STEPS_PER_REVOLUTION * 4)  // 1/4  steps - MS1=OFF, MS2=ON,  MS3=OFF

//#define MAX_TURNOUT_POSITIONS  48 //Otto sollte verwendet werden
//#define POSITION_01_DCC_ADDRESS 225 // +  200 !!!!!!!!!!!!!!!!

#define HALF_TURN_STEPS (FULL_TURN_STEPS / 2)
#define POSITION_01 (HALF_TURN_STEPS / 10)

void setupStepper()
{
	 pinMode(dirInPin,  INPUT);
	 pinMode(relayPin,  OUTPUT);
	 pinMode(movePin,   OUTPUT);
	 pinMode(reachedPin,OUTPUT);
	 pinMode(dirLedPin, OUTPUT);
	 pinMode(refPin, INPUT);

	 digitalWrite(dirPin, HIGH);
	 digitalWrite(movePin, HIGH);
	 digitalWrite(reachedPin, HIGH);
	 digitalWrite(relayPin, HIGH);

	 myStepper.attach(stepPin, dirPin);
	 myStepper.setSpeed( 300 );     //300          // = 60 rpm
	 myStepper.setRampLen(300);     //600          // 100 steps to achive set speed
	 myStepper.attachEnable( enaPin, 100, LOW );   // if you want to switch off power when stepper reached position
}

void relayOn(){
	digitalWrite(relayPin, LOW);
	bitWrite(state2, 3, 0);
}

void relayOff(){
	digitalWrite(relayPin, HIGH);
 	bitWrite(state2, 3, 1);
}

void checkRelay (int pos) {
	if (pos  > Prop.Relaytrack and pos <=  24 + Prop.Relaytrack)
		relayOff();
	else
		relayOn();
}

void calibrate() {
	if (sound) myDFPlayer.play(1);
	Serial.print("mcalibrate\n");
	Serial.print("lastpos "); Serial.println (lastpos);
	Serial.println("m---");
    Serial.println("mturn right");
    myStepper.setSpeedSteps(4000, 0); //0, 100); //5000 7000 nicht mehr als 8000
    myStepper.rotate(-1); //-1
	while ( digitalRead(refPin) != LOW);
    myStepper.rotate(0);
    while ( myStepper.moving()) {};   // Bremsrampe abwarten;

	Serial.println("mstop return"); // Langsam und ohne Rampe zurueck zum Schaltpunkt des Refpunktes fahren
	myStepper.setSpeedSteps(200);   // besser langsamer
	myStepper.setRampLen(0); // !!!
	myStepper.rotate( 1 );
	while (digitalRead( refPin) == LOW);
    myStepper.rotate(0);
    while ( myStepper.moving() );

    Serial.println("mto track 0");
    myStepper.setSpeed(150 );    //150 200
    myStepper.setRampLen(100 ); //100 300
	myStepper.move(Prop.StepsToTrack0);
	while ( myStepper.moving() );
	myStepper.setZero();
    myStepper.setRampLen(300 ); //400 600
	myStepper.setSpeed(300);
	pos = 0;
	lastpos = 0;
    writeEEPROM(lastposAddr, lastpos);
	if (sound) myDFPlayer.play(3);
	Serial.println("mfinished calibrate");
	checkRelay(pos);
}

void turnToPos (int mypos) {
    pos = mypos;
    noTurn = pos == lastpos;
	Serial.print((String) "mpos "+pos+" "+"  lpos "+lastpos+ "\n");
	if (sound) {
		if (not noTurn) {
			myDFPlayer.volume(Prop.Volume); // ???
			myDFPlayer.play(1);
			delay(2000);
			myDFPlayer.volume(Prop.Volume + 5);
			myDFPlayer.play(2); }
	}
	if (pos != lastpos and pos <= 48) {
		if (pos > lastpos) {
			Serial.println ("mforward ");
			myStepper.moveTo(front[pos]);
		}
	    else {
	    	Serial.println ("mbackward ");
	    	myStepper.moveTo(back[pos]);
	    }
	}

	checkRelay(pos);

    noTurn = pos == lastpos;
	lastpos = pos;

	writeEEPROM(lastposAddr, lastpos);
	Serial.print((String) "p"+pos+":"+front[pos]+":"+back[pos]+"\n");
}

void turn180(){
	int newpos;
 	if (pos >= 24) {
		newpos = pos - 24;
	}
	else {
		 newpos = pos + 24;
	}
	if (newpos == 48) {pos = 0; newpos = 0;}
	Serial.print((String) "m180 Grad "+pos+ " -> "+ newpos +"\n");
	turnToPos(newpos);
}

void forward() {
	Serial.print("mforward\n");
	if (pos < 48) {  // max verwenden
		pos = pos +1;
        turnToPos(pos);
	}
}

void backward() {
	Serial.print("mbackward\n");
	if (pos > 0) {
		pos = pos -1;
		turnToPos(pos);
	}
}

unsigned long int setReached()
{
	noTurn = false;
	digitalWrite(movePin, HIGH);
	digitalWrite(reachedPin, LOW);
	bitWrite(state2, 2, 0);
	if (lastpos == 0) {
		Serial.println("msetZero " );
		myStepper.setZero(-back[0]);
	}
///////////////	Serial.println("reached " );
	if (sound) {
		myDFPlayer.volume(Prop.Volume);
		myDFPlayer.play(3);
	}
	return millis() + 2000;
}

void resetReached()
{
  digitalWrite(reachedPin, HIGH);
  bitWrite(state2, 2, 1);
 /// noTurn = false;
}

#endif
