// Copyright (c) 2025 BahnOtto

#include "Arduino.h"
#include <NmraDcc.h>
#include <MobaTools.h>
#include <EEPROM.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "DSprop.h"

const char Vers[] = "Drescheibe Ds01 VSC 250708 BahnOtto 2x";
//--------------------------------------------------------

//Stepper einrichten ( 800 steps / cycle - 1/4 microstep )
MoToStepper myStepper(800, STEPDIR );

#define DCC_PIN 2
DSprop Prop;

int  front  [49];
int  back   [49];

const int lastposAddr = 50;
int pos;
int lastpos;
bool noTurn = false;
bool isMoving = false;
byte state1;
byte state2;
unsigned long int setTime;

unsigned int startAddr;
unsigned int firstTrackAddr;
unsigned int soundAddr;
unsigned int calibrateAddr;
unsigned int turn180Addr;

#include "Sound.h"
#include "TrackManager.h"
#include "StepperControl.h"

//--------------------------------------------------
NmraDcc Dcc;
void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
//	unsigned int startAddr = Prop.StartAddr; //229; //225  wahrscheinlich doppelt vgl. Stepper
	startAddr      = Prop.StartAddr; //229; //225  wahrscheinlich doppelt vgl. Stepper
    soundAddr      = startAddr;      //225
    firstTrackAddr = startAddr + 4;  // 229
    calibrateAddr  = startAddr + 2;
    turn180Addr    = startAddr + 1;


	int tpos = 0;
    //if (Addr == 225) { // ??
    if (Addr == soundAddr) { // ??
//    	Serial.println(startAddr);
    	if (OutputPower == 0) {
    		soundOff();
    		delay(800); }
		else {
    		soundOn();
    		delay(800); }
    }

	if ((Addr == turn180Addr) and (OutputPower == 0)) {
		delay(500);
		turn180();
	}
	if ((Addr == calibrateAddr) and (OutputPower == 0)) {
		calibrate();
	}

//	if ((Addr >= firstTrackAddr) and (Addr <= firstTrackAddr+24) and (OutputPower == 0) ) {
	if ((Addr >= firstTrackAddr) and (Addr <= firstTrackAddr+24) and (OutputPower == 0) ) {

		Serial.println();
		Serial.print(" A "); Serial.print (Addr);
		Serial.print(" D "); Serial.print (Direction);
		tpos = 2 * (Addr - firstTrackAddr); //???????????????????????????????????????????????
		if (Direction == 1) tpos = tpos+1;
		Serial.print(" tpos ");	Serial.println(tpos);
		Serial.println(" --------------------");
		turnToPos(tpos);
    }
}



void setup()
{
	 Serial.begin(9600); //19200
	 delay(200);
	 Serial.println("setup Start");
	 Serial.println(Vers);
	 delay(5000);
	 setupDFPlayer();
	 delay(1000);
	 setupStepper();

	 Prop = DSprop(); //muss nach Stepper

     EEPROMtoList();

     state1 = 0;
     state2 = 255;

     Dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, false);
     Dcc.init(MAN_ID_DIY, 1, FLAGS_DCC_ACCESSORY_DECODER | FLAGS_OUTPUT_ADDRESS_MODE, 0);

     delay (200); //must be
     lastpos = readEEPROM(lastposAddr);
     delay (200);
 	 Serial.print("m lastpos "); Serial.println (lastpos);
 	 Serial.print("m pos     "); Serial.println(pos);
     Serial.println("m ---");
     if (lastpos != 0) {
    	 int sZero = -front[lastpos];
		 Serial.println(front[lastpos]);
		 Serial.println(back[0]);
		 Serial.println (sZero);
		 myStepper.setZero(sZero);
     }
     else {
    	 myStepper.setZero(-back[0]);
     }
     checkRelay(lastpos);
}

void menu() {
	  if (Serial.available()) {
		 String data = Serial.readString();
		 char d1 = data.charAt(0);
	     switch (d1) {
	         case 'R':  Reset();
	        	 	 	break;
	         case 'E':  EEPROMtoList();
	        			break;
	         case 'e':  listToEEPROM();
	                    break;
			 case 'l':  listPositions();
						break;
			 case 'a':  relayOff();
			 	 	 	break;
	    	 case 'A':  relayOn();
	    	            break;
			 case 'd':  turnToPos(data.substring(1).toInt());
			 	 	 	break;
			 case 'D': 	turn180();
					 	break;
			 case 'p':  adjustment(data.charAt(1), 10);
	                    break;
			 case 'P':  adjustment(data.charAt(1), 30);
						break;
			 case 'm':  adjustment(data.charAt(1), -10);
						break;
			 case 'M':  adjustment(data.charAt(1), -30);
						break;
			 case 'o':  writeNewposEEPROM();
			 	 	 	break;
			 case 'c':  calibrate();
						break;
			 case 'f':  forward();
						break;
			 case 'b':  backward();
						break;
			 case 'x':  front[pos] = data.substring(1).toInt();
				 	 	break;
			 case 'y':  back[pos] = data.substring(1).toInt();
			 			break;
			 case 's':  soundOn();
			 		    break;
			 case 'S':  soundOff();
			           	break;
			 case 't':  myStepper.rotate(-1);
					 	break;
			 case 'u':  myStepper.rotate(1);
						break;
			 case 'T':  myStepper.stop();
			           	break;
			 case 'v':  Prop.readProps();
						break;
			 case 'V':  Prop.saveProps(data.substring(1)); //"#225#1700#12#15#300#300"
				 	 	//prop.saveProps("#225#1700#12#15#300#300");
						break;
			 default:   Serial.println ("no command");
				 	 	break;
	     }
	  }

}


void loop()
{
   Dcc.process();
   menu();

   if (myStepper.moving() > 0) {
   	 digitalWrite(movePin, 0);
	 bitWrite(state2, 1, 0);
	 isMoving = true;
    }

//   if (noTurn == true) {
//	   Serial.println (noTurn);
//   }

   if (
		   (isMoving and myStepper.moving() == 0) or (noTurn and myStepper.moving() == 0) )
		   //Klammer entfernt ....!!
	   {
       isMoving = false;
       setTime = setReached();
       bitWrite(state2, 1, 1);
       delay(200);
       Serial.print ((String) "msetReached lastPos " + lastpos + "\n"); //???????????????
   }

   if (digitalRead(dirInPin) == HIGH) {
   	  bitWrite(state2, 0, 1);
   	  digitalWrite (dirLedPin, HIGH);
   	  }
   else {
   	  bitWrite(state2, 0, 0);
   	  digitalWrite (dirLedPin, LOW);
   }

   if (millis() > setTime)  resetReached() ;

   if (state1 != state2) {
      	  String ledByteStr = String(state2, BIN);
      	  Serial.print((String) "l"+ledByteStr+"\n");
      	  state1 = state2;
   }
}


