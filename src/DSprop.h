
// Copyright (c) 2025 BahnOtto

#ifndef DSPROP_H_
#define DSPROP_H_

#include <EEPROM.h>
#include <Arduino.h>

#define EEPROMAddr 80 

class DSprop {
private:
   int readEEPROM(int addr) {
	  return EEPROM.read(addr) + ((EEPROM.read(addr+1) << 8)&0xFF00);
   }
   
   void writeEEPROM(int addr, int wert) {
	 EEPROM.write(addr, wert&0xFF);
	 EEPROM.write(addr + 1, (wert>>8)&0xFF);
	 }
	
public:
	int StartAddr;
	int StepsToTrack0;
	int Relaytrack;
	int Volume;
	int Speed;
	int RampLength;
		
	DSprop(){
		StartAddr     = readEEPROM(EEPROMAddr);
		StepsToTrack0 = readEEPROM(EEPROMAddr+2);
		Relaytrack    = readEEPROM(EEPROMAddr+4);
		Volume        = readEEPROM(EEPROMAddr+6);
		Speed         = readEEPROM(EEPROMAddr+8);
		RampLength    = readEEPROM(EEPROMAddr+10);
	};
		
	void saveProps (String s) {
		const int propcount = 6;
		int values[propcount];
		char delemiter[] = "#";
		char daten[50];
		s.toCharArray(daten, 50);
		if (s.length() > 0) {
			int i = 0;
			char* token = strtok(daten, delemiter);
			while (token != NULL) {
				values[i] = atoi(token);
				token = strtok(NULL, delemiter);
				i++;
			}
			for (int i=0; i<propcount; i++) {
				Serial.println(values[i]);
			}
			//Beispiel  V#225#1700#12#15#300#301
		StartAddr     = values[0];
		StepsToTrack0 = values[1];
		Relaytrack    = values[2];
		Volume        = values[3];
		Speed         = values[4];
		RampLength    = values[5];

		writeEEPROM (EEPROMAddr,    StartAddr);
		writeEEPROM (EEPROMAddr+ 2, StepsToTrack0);
		writeEEPROM (EEPROMAddr+ 4, Relaytrack);
		writeEEPROM (EEPROMAddr+ 6, Volume);
		writeEEPROM (EEPROMAddr+ 8, Speed);
		writeEEPROM (EEPROMAddr+10, RampLength);
		Serial.println(s = "m" + s);
		Serial.println("min EEPROM gespeichert");
		}
		else Serial.println("mungueltige Werte");
	};

	void readProps() {
		StartAddr     = readEEPROM (EEPROMAddr);
		StepsToTrack0 = readEEPROM (EEPROMAddr+ 2);
		Relaytrack    = readEEPROM (EEPROMAddr+ 4);
		Volume        = readEEPROM (EEPROMAddr+ 6);
		Speed         = readEEPROM (EEPROMAddr+ 8);
		RampLength    = readEEPROM (EEPROMAddr+10);

		String s = String(StartAddr)+"#"+String(StepsToTrack0)+"#"+String(Relaytrack)+"#"
				   +String(Volume)+"#"+String(Speed)+"#"+String(RampLength);
		Serial.println("m"+s);
		delay (100);  ///  !!!!!!!!!!!!!!!!!!!!!!
		Serial.println("#"+s);
	}
};

#endif /* DSPROP_H_ */
