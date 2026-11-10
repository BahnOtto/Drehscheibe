// Copyright (c) 2025 BahnOtto

#ifndef TrackManager_H
#define TrackManager_H


#include "Arduino.h"

int readEEPROM(int addr) {
	return EEPROM.read(addr) + ((EEPROM.read(addr+1) << 8)&0xFF00);
}

void writeEEPROM(int addr, int wert)
{
  EEPROM.write(addr, wert&0xFF);
  EEPROM.write(addr + 1, (wert>>8)&0xFF);
}

void writeNewposEEPROM(){
   Serial.print((String) "m " +pos + " " + front[pos] + " : " + back[pos] + "   saved\n");
   writeEEPROM(pos * 4 + 100, front[pos]);
   writeEEPROM(pos * 4 + 100+ 2, back[pos]);
}

void initPositions() {
	///        unused int x = readEEPROM(6);
	front[0] = 0;
	back[0] = 0;
	float f = 0;
	for(int i = 0; i < 49; i++)  {   //<
		f = i * 333.333; //800 * 20 : 48 !!!!
		front[i] = f;
		back[i] =  f;
		Serial.print(i);
		Serial.print(F(" Front: "));
		Serial.print(front[i]);
		Serial.print(F(" Back: "));
		Serial.println(back[i]);
	}
	Serial.println("m-initPositions");
}


void listToEEPROM()
{
	int  e = 100;
	for(int i = 0; i < 49; i++)
	{
		Serial.print((String)  i +" "+front[i]+" "+back[i]+"\n"); //war pos
		writeEEPROM(e, front[i]);
	    e = e + 2;
		writeEEPROM(e, back[i]);
		e = e + 2;
	}
	Serial.println("m-listToEEPROM");
}

void EEPROMtoList(){
	int e = 100;
	int tmp;
	for(int i = 0; i < 49; i++)
	{
		tmp = e;
		front[i] = readEEPROM(e);
		e = e + 2;
		back[i]  = readEEPROM(e);
		e = e + 2;
		Serial.print((String)i+" "+tmp+" Front: "+front[i]+" Back: "+back[i]+"\n");
	}
	Serial.println("EEPROMtoList");
}
void listPositions() {
//	Serial.println("mlistPositions");
//	delay(100);
	for(int i = 0; i < 49; i++) {
    	delay(50);  //30 geht 20 nicht
		Serial.print((String)"m"+i+" Front: "+front[i]+" Back: "+back[i]+"\r\n"); //\n
	}
}

void Reset(){
	Serial.println("R Reset");
 	initPositions();
  listToEEPROM();
}

void adjustment (char dir, int steps){
	 switch (dir) {
		 case '1':  Serial.print((String) "m" + front[pos] + "   f " + steps +  "\n");
		 	 	 	front[pos] = front[pos]+steps; // steps kann auch negativ sein
			    	myStepper.moveTo (front[pos]);
					break;
		 case '2':  Serial.print((String) "m" + back[pos]  + "   b " + steps + "\n");
					back[pos] = back[pos]+steps;
					myStepper.moveTo(back[pos]);
					break;
		 default:   Serial.println ("m command error");
		 	 	 	break;
	 }
	 Serial.print((String) "p"+pos+":"+front[pos]+":"+back[pos]+"\n");
}

#endif
