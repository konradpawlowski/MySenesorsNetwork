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
* DESCRIPTION
* The RS485 Gateway prints data received from sensors on the serial link.
* The gateway accepts input on seral which will be sent out on
* the RS485 link.
*
* Wire connections (OPTIONAL):
* - Inclusion button should be connected between digital pin 3 and GND
* - RX/TX/ERR leds need to be connected between +5V (anode) and digital pin 6/5/4 with resistor 270-330R in a series
*
* LEDs (OPTIONAL):
* - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
* - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
* - ERR (red) - fast blink on error during transmission error or recieve crc error
*
* If your Arduino board has additional serial ports
* you can use to connect the RS485 module.
* Otherwise, the gateway uses AltSoftSerial to handle two serial
* links on one Arduino. Use the following pins for RS485 link
*
*  Board          Transmit  Receive   PWM Unusable
* -----          --------  -------   ------------
* Teensy 3.0 & 3.1  21        20         22
* Teensy 2.0         9        10       (none)
* Teensy++ 2.0      25         4       26, 27
* Arduino Uno        9         8         10
* Arduino Leonardo   5        13       (none)
* Arduino Mega      46        48       44, 45
* Wiring-S           5         6          4
* Sanguino          13        14         12
*
*/

// Enable debug prints to serial monitor
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Bounce2mcp.h>

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 5

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600

// Enable this if RS485 is connected to a hardware serial port
//#define MY_RS485_HWSERIAL Serial1

// Enable serial gateway
//#define MY_GATEWAY_SERIAL

#define MY_NODE_ID 10

#include <MySensors.h>
//#define VSN "v1.0"
#define noButtons 16
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

Adafruit_MCP23017 mcp;
// Instantiate a Bounce object
BounceMcp debouncer[7];
bool lastValue[7];

MyMessage msg[noButtons] = {
	MyMessage(0,V_TRIPPED),
	MyMessage(1,V_TRIPPED),
	MyMessage(2,V_TRIPPED),
	MyMessage(3,V_TRIPPED),
	MyMessage(4,V_TRIPPED),
	MyMessage(5,V_TRIPPED),
	MyMessage(6,V_TRIPPED),
	MyMessage(7,V_LIGHT),
	MyMessage(8,V_STATUS),
	MyMessage(9,V_STATUS),
	MyMessage(10,V_STATUS),
	MyMessage(11,V_STATUS),
	MyMessage(12,V_STATUS),
	MyMessage(13,V_STATUS),
	MyMessage(14,V_STATUS),
	MyMessage(15,V_STATUS)
};



void setup()
{
	mcp.begin(7);      // use default address 0

					   // Setup locally attached sensors
	for (int pin = 0; pin < noButtons; pin++) {


		//msg[pin](pin, V_TRIPPED);//.sensor = pin;

		Serial.println("Ustawianie pinow:");

		// wejsciowe
		if (pin < 7) {
			//msg[pin].type = V_TRIPPED; //

			Serial.print("Pin wejsciowy nr: ");
			Serial.print(pin);


			mcp.pinMode(pin, INPUT);
			mcp.pullUp(pin, HIGH);  // turn on a 100K pullup internally

			debouncer[pin] = BounceMcp();
			debouncer[pin].attach(mcp, pin, 5);

		}
		else
			// initialize messages
		{
			Serial.print("Pin wyjsciowy nr: ");
			Serial.print(pin);

			//msg[pin].type = V_STATUS;
			mcp.pinMode(pin, OUTPUT);
			// Set relay to last known state (using eeprom storage)
			mcp.digitalWrite(pin, loadState(pin) ? RELAY_ON : RELAY_OFF);
		}
	}
}



void presentation()
{
	// Present locally attached sensors
	sendSketchInfo("MCP23017", "1.0");
	for (int i = 0; i < 16; i++)
	{
		if (i == 0) {
			present(i, S_DOOR);
		}
		else if (i < 7)
		{
			present(i, S_MOTION);
		}
		else if (i == 7)
		{
			present(i, S_LIGHT);
		}
		else
		{
		
			present(i, S_BINARY);
		}
	}
}

void loop()
{
	for (int i = 0; i < 7; i++)
	{
		debouncer[i].update();
		int value = debouncer[i].read();
		if (lastValue[i] != value)
		{
			send(msg[i].set(value == true ? RELAY_ON : RELAY_OFF));
			lastValue[i] = value;
		}
	}
	

	
}

void receive(const MyMessage &message)
{
	// We only expect one type of message from controller. But we better check anyway.
	if (message.type == V_STATUS ) {
		// Change relay state
		mcp.digitalWrite(message.sensor , message.getBool() ? RELAY_ON : RELAY_OFF);
		// Store state in eeprom
		saveState(message.sensor, message.getBool());
		// Write some debug info
		Serial.print("Incoming change for sensor:");
		Serial.print(message.sensor);
		Serial.print(", New status: ");
		Serial.println(message.getBool());
	}
}	