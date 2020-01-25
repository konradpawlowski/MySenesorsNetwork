/*
    Name:       GarageController.ino
    Created:	24.01.2020 18:53:30
    Author:     DESKTOP-R29CEKM\konra
*/




#include <Bounce2.h>
#include <AsyncTaskLib.h>


#define MY_DEBUG

// MySencsor deifnition
#define MY_RS485
#define MY_NODE_ID  33
// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2
#define MY_RS485_BAUD_RATE 9600
#define ACK false
#include <MyConfig.h>
#include <MySensors.h>
#pragma region InitalInput

#define PIN_IN_POMPA_CUW 6
#define PIN_IN_PODAJNIK 7
#define PIN_IN_WENTYLATOR 12
#define PIN_IN_POMPA_CO 11 

#define PIN_OUT_REG_POK 19

#define PIN_OUT_POMPA_CUW 13
#define PIN_OUT_POMPA_CO 17
#define PIN_OUT_POMPA_OBIEG 16
#define PIN_OUT_WENTYLATOR 15
#define PIN_OUT_PODAJNIK 14

#define PIN_OUT_OSW_ZEW 18 //out bojler
#define PIN_OUT_GARAGE_DOOR 19 
#define PIN_IN_GARAGE_DOOR 3 
#define PIN_IN_SMOKE 5

#define SMOKE_SENSOR 1
#define GARAGE_IN 2
#define GARAGE_OUT 3
#define OUTSIDE_LIGHT 4

Bounce door = Bounce(); ;
Bounce smoke = Bounce();
MyMessage mySmoke = MyMessage(SMOKE_SENSOR, V_ARMED);
MyMessage myDoorIn = MyMessage(GARAGE_IN, V_TRIPPED);
MyMessage myDoorOut = MyMessage(GARAGE_OUT, V_STATUS);
MyMessage myLightOutside = MyMessage(OUTSIDE_LIGHT, V_STATUS);

bool first_message_sent = false;
bool smokeLast = false;
bool doorInLast = false;


void before() {
	
}

void presentation() {
	sendSketchInfo("GarageController", "1.0", ACK);
	

	present(SMOKE_SENSOR, S_SMOKE, "Czujnik dymu");
	present(GARAGE_IN, S_DOOR, "Brama gara¿owa");
	present(GARAGE_OUT, S_BINARY, "Brama gara¿owa");
	present(OUTSIDE_LIGHT, S_BINARY, "Oœwietlnie zewn¹trzne");

}


void setup()
{
	
	door.attach(PIN_IN_GARAGE_DOOR, INPUT_PULLUP);       //setup the bounce instance for the current button
	door.interval(5);              // interval in ms
	smoke.attach(PIN_IN_SMOKE, INPUT);
	smoke.interval(5);



	pinMode(PIN_OUT_OSW_ZEW, OUTPUT);
	pinMode(PIN_OUT_GARAGE_DOOR, OUTPUT);
	pinMode(PIN_OUT_POMPA_CUW, OUTPUT);
	
}

void loop()
{
	if (first_message_sent == false) {
		Serial.println("Sending initial state...");
		send_startup();
		first_message_sent = true;
	}

	UpdateGarageDoorStatus();
	UpdateSmokeStatus();

	
}
void send_startup() {
	door.update();
	doorInLast = !door.read();
	send(myDoorIn.set(doorInLast));

	smoke.update();
	smokeLast = smoke.read();
	send(mySmoke.set(smokeLast));



}

void UpdateGarageDoorStatus() {
	bool value;

	door.update();
	value = !door.read();

	if (doorInLast != value) {
		send(myDoorIn.set(value));
		doorInLast = value;
	}

}
void UpdateSmokeStatus() {
	bool value;

	smoke.update();
	value = smoke.read();
	if (smokeLast != value)
	{
		send(mySmoke.set(value));
		smokeLast = value;
	}
	
}
void receive(const MyMessage& message)
{
	Serial.print(message.fValue);


	if (message.isAck()) {
		Serial.println("This is an ack from gateway");
	}
	// We only expect one type of message from controller. But we better check anyway.
	if (message.type == V_STATUS) {
		// Change relay state
		//digitalWrite(OUTPUT_SENSORS_PINS[message.sensor], message.getBool);
		SetOutput(message.sensor, message.getBool());
		// Store state in eeprom
				// Write some debug info
		Serial.print("Incoming change for sensor:");
		Serial.print(message.sensor);
		Serial.print(", New status: ");
		Serial.println(message.getBool());
	}
}
void SetOutput(int sensor_id, bool status) {
	uint8_t sensor; 
	switch (sensor_id)
	{
	case GARAGE_OUT:
		Serial.print(", New status: Brama ");
		if (status)
		{
			digitalWrite(PIN_OUT_GARAGE_DOOR, HIGH);
			sleep(200);
			digitalWrite(PIN_OUT_GARAGE_DOOR, LOW);
		}
		break;
	case OUTSIDE_LIGHT:
		Serial.print(", New status: osw ");
		digitalWrite(PIN_OUT_OSW_ZEW, status ? HIGH : LOW);
		//send(myLightOutside.set(status), ACK);
		saveState(OUTSIDE_LIGHT, status);
		break;
	default:
		
		break;
	}

}