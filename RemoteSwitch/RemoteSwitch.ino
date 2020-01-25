/**
* The MySensors Arduino library handles the wireless radio link and protocol
* between your home built sensors/actuators and HA controller of choice.
* The sensors forms a self healing radio network with optional repeaters. Each
* repeater and gateway builds a routing tables in EEPROM which keeps track of the
* network topology allowing messages to be routed to nodes.
*
* Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
* Copyright (C) 2013-2015 Sensnology AB
* Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
*
* Documentation: http://www.mysensors.org
* Support Forum: http://forum.mysensors.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
*******************************
*
* REVISION HISTORY
* Version 1.0 - Henrik Ekblad
*
* DESCRIPTION
* Example sketch for a "light switch" where you can control light or something
* else from both HA controller and a local physical button
* (connected between digital pin 3 and GND).
* This node also works as a repeader for other nodes
* http://www.mysensors.org/build/relay
*/

// Enable debug prints to serial monitor


#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_PA_LEVEL RF24_PA_LOW

//#define MY_RADIO_RFM69
#define MY_INCLUSION_MODE_FEATURE

// Enabled repeater feature for this node
//#define MY_REPEATER_FEATURE

//#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>




int BUTTON_PIN[3] = { 3,4,5 };  // Arduino Digital I/O pin number for button 

#define RELAY_ON 1
#define RELAY_OFF 0
int oldValue[3] = { LOW, LOW, LOW };
bool state[3];
unsigned long lastTime;

unsigned long SLEEP_TIME = 1000; // Sleep time between reads (in milliseconds)

int numSensors = 0;
bool receivedConfig = false;
bool metric = true;
bool isSending = false;
Bounce debouncer[3] = {
	Bounce(),
	Bounce(),
	Bounce()
};


MyMessage msg[4] = {
	MyMessage(0,V_LIGHT),
	MyMessage(1,V_LIGHT),
	MyMessage(2,V_LIGHT)

};
//void before()
//{
//	SetupDs182b();
//
//}
void setup()
{
	for (size_t i = 0; i < 3; i++)
	{
		state[i] = loadState(i);
		// Setup the button
		pinMode(BUTTON_PIN[i], INPUT_PULLUP);
		// Activate internal pull-up
		//digitalWrite(BUTTON_PIN[i], HIGH);                                                        

		// After setting up the button, setup debouncer
		debouncer[i].attach(BUTTON_PIN[i]);
		debouncer[i].interval(5);

	}

}

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("RemoteSwitch", "1.0", false);

	// Register all sensors to gw (they will be created as child devices)
	for (size_t i = 0; i < 3; i++)
	{
		present(i, S_LIGHT);
	}


}

/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop()
{
	//if (!isSending) {
	for (size_t i = 0; i < 3; i++)
	{

		isSending = true;
		debouncer[i].update();
		// Get the update value
		int value = !debouncer[i].read();

		if (value != oldValue[i] ) {
			/*Serial.println("*********************************************");
			Serial.print("Button: ");
			Serial.print(i);
			Serial.print(" - Pin: ");
			Serial.print(BUTTON_PIN[i]);
			Serial.print("Stan: ");
			Serial.println(value);
			Serial.println("*********************************************");*/
			state[i] = !state[i];
			
			send(msg[i].set(state[i] ? false : true));

		}
		oldValue[i] = value;
		
	}
	//}

}

void receive(const MyMessage &message) {

}
