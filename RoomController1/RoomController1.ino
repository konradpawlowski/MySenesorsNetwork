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
#include <AsyncTaskLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define NODE_ID		32

#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RS485
#define MY_RS485_DE_PIN 7
#define MY_NODE_ID	NODE_ID
#define MY_INCLUSION_MODE_FEATURE
#define MY_RS485_BAUD_RATE 9600

#define MS_BOARD_NAME               "PatrykRooms"
#define MS_SOFTWARE_VERSION         "2.0"
#define MS_RELAY1_CHILD_ID          1
#define MS_RELAY2_CHILD_ID          2
#define MS_RELAY3_CHILD_ID          3
#define MS_TEMP_CHILD_ID            4
#define ACK							false


//#define MY_TRANSPORT_WAIT_READY_MS 5000
//#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>

#define ONE_WIRE_BUS 12 // Pin where dallase sensor is connected 

#define RELAY_ON 1
#define RELAY_OFF 0


#define COMPARE_TEMP 1 // Send temperature only if changed? 1 = Yes 0 = No
#define ONE_WIRE_BUS 12 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 1
#define TEMPERATURE_PRECISION 10



int oldValue[3] = { HIGH, HIGH, HIGH };
bool state[3];
unsigned long lastTime;
bool connection_broken = false;

int RELAY_PIN[3] = { 5, 6, 11 }; // Arduino Digital I/O pin number for relay 
int BUTTON_PIN[3] = { 2, 3, 4 };  // Arduino Digital I/O pin number for button 

bool first_message_sent = false;
unsigned long SLEEP_TIME = 1000; // Sleep time between reads (in milliseconds)


AsyncTask dsRead (SLEEP_TIME, []() { ReadDs();   }); 
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 

float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors = 0;
bool receivedConfig = false;
bool metric = true;
Bounce debouncer[3]
= {
	Bounce(),
	Bounce(),
	Bounce()
};


MyMessage msg[4] = {
  MyMessage(MS_RELAY1_CHILD_ID,V_LIGHT),
  MyMessage(MS_RELAY2_CHILD_ID,V_LIGHT),
  MyMessage(MS_RELAY3_CHILD_ID,V_LIGHT),
  MyMessage(MS_TEMP_CHILD_ID,V_TEMP)
};



void before()
{
	for (size_t i = 0; i < 3; i++)
	{
		// Setup the button
		// Activate internal pull-up
		pinMode(BUTTON_PIN[i], INPUT_PULLUP);
		pinMode(RELAY_PIN[i], OUTPUT);

		debouncer[i].attach(BUTTON_PIN[i]);
		debouncer[i].interval(5);

		state[i] = loadState(i);
		// Make sure relays are off when starting up
		digitalWrite(RELAY_PIN[i], RELAY_OFF);
	}
	pinMode(LED_BUILTIN, OUTPUT);
}

void setup()
{
	for (size_t i = 0; i < 3; i++)
	{

		// Set relay to last known state (using eeprom storage) 
		//request(i + 1, V_LIGHT, NODE_ID);

		digitalWrite(RELAY_PIN[i], state[i] ? HIGH : LOW);

	}
	// requestTemperatures() will not block current thread
	SetupDs182b();
	dsRead.Start();

	BlinkLed(5,200);

}

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo(MS_BOARD_NAME, MS_SOFTWARE_VERSION, ACK);

	// Register all sensors to gw (they will be created as child devices)

	present(MS_RELAY1_CHILD_ID, S_LIGHT);
	present(MS_RELAY2_CHILD_ID, S_LIGHT);
	present(MS_RELAY3_CHILD_ID, S_LIGHT);
	present(MS_TEMP_CHILD_ID, S_TEMP);

}

/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop()
{

	if (first_message_sent == false) {
		Serial.println("Sending initial state...");
		send_startup();
		
		first_message_sent = true;
	}

	//if (!isTransportReady())
	//{
	//	connection_broken = true;
	//	BlinkLed(2, 500);
	//}
	//else if (connection_broken)
	//{
	//	for (size_t i = 0; i < 3; i++)
	//	{

	//		// Set relay to last known state (using eeprom storage) 
	//		request(i + 1, V_LIGHT, NODE_ID);
	//				
	//	}
	//}



	dsRead.Update();
	for (size_t i = 0; i < 3; i++)
	{


		debouncer[i].update();
		// Get the update value
		int value = debouncer[i].read();

		if (value != oldValue[i]) {
	
			send(msg[i].set(state[i] ? false : true), ACK);
			digitalWrite(RELAY_PIN[i], value);
			oldValue[i] = value;
		}

		

	}

}
void send_startup() {

	Serial.println("Sending initial state... ");

	send(msg[0].set(state[0]), ACK);
	send(msg[1].set(state[1]), ACK);
	send(msg[2].set(state[2]), ACK);
	ReadDs();
}
void receive(const MyMessage& message) {
	// We only expect one type of message from controller. But we better check anyway.
	if (message.isAck()) {
		Serial.println("This is an ack from gateway");
	}

	if (message.type == V_LIGHT) {
		int i = message.sensor - 1;
		// Change relay state
		state[i] = message.getBool();
		/*Serial.print("Pin: ");
		Serial.print(RELAY_PIN[message.sensor]);
		Serial.print(", New status: ");
		Serial.println(state[message.sensor] ? HIGH : LOW);*/
		digitalWrite(RELAY_PIN[i], state[i] ? HIGH : LOW);
		// Store state in eeprom
		saveState(i, state[i]);

		//// Write some debug info
		/*Serial.print("Incoming change for sensor:");
		Serial.print(message.sensor);
		Serial.print(", New status: ");
		Serial.println(message.getBool());*/
	}
}

void SetupDs182b() {
	sensors.begin();						  // locate devices on the bus
	numSensors = sensors.getDeviceCount();
	//Serial.print("Temp sensor: ");
	//Serial.println(numSensors);

	//ustawienia rozdzielczosci
	for (uint8_t i = 0; i < numSensors; i++)
	{
		DeviceAddress  add;
		if (!sensors.getAddress(add, i)) Serial.print("Unable to find address for Device ");
		sensors.setResolution(add, TEMPERATURE_PRECISION);
		sensors.setWaitForConversion(false);
		//printResolution(sensors, add);
	}

}
void ReadDs()
{
	// Fetch temperatures from Dallas sensors
	sensors.requestTemperatures();

	// query conversion time and sleep until conversion completed
	int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
	// sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
	sleep(conversionTime);

	// Read temperatures and send them to controller 
	for (int i = 0; i < numSensors && i < MAX_ATTACHED_DS18B20; i++) {

		// Fetch and round temperature to one decimal
		float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric ? sensors.getTempCByIndex(i) : sensors.getTempFByIndex(i)) * 10.)) / 10.;
		Serial.print("Temperature: ");
		Serial.println(temperature);
		// Only send data if temperature has changed and no error
#if COMPARE_TEMP == 1
		if (lastTemperature[i] != temperature && temperature != -127.00 && temperature != 85.00) {
#else
		if (temperature != -127.00 && temperature != 85.00) {
#endif

			// Send in the new temperature
			send(msg[3].set(temperature, 1));
			// Save new temperatures for next compare
			lastTemperature[i] = temperature;
		}

	}


}

void BlinkLed(int count, uint32_t time )
{
	digitalWrite(LED_BUILTIN, LOW);
	for (int i = 0; i < count; i++)
	{
		digitalWrite(LED_BUILTIN, HIGH);
		sleep(time);
		digitalWrite(LED_BUILTIN, LOW);
		sleep(time / 2);
	}
}
