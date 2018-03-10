
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
* The ArduinoGateway prints data received from sensors on the serial link.
* The gateway accepts input on seral which will be sent out on radio network.
*
* The GW code is designed for Arduino Nano 328p / 16MHz
*
* Wire connections (OPTIONAL):
* - Inclusion button should be connected between digital pin 3 and GND
* - RX/TX/ERR leds need to be connected between +5V (anode) and digital pin 6/5/4 with resistor 270-330R in a series
*
* LEDs (OPTIONAL):
* - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs
* - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
* - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
* - ERR (red) - fast blink on error during transmission error or recieve crc error
*
*/


#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_BMP085.h>

#include <Wire.h>
#ifdef MY_DEBUG

#define Sprintln(a) (Serial.println(a))
#define Sprint(a) (Serial.println(a))
#else
#define Sprintln(a)
#define Sprint(a) 
#endif
// Enable debug prints to serial monitor
#define MY_DEBUG

//
//// Enable and select radio type attached
//#define MY_RADIO_NRF24
//
//// Set LOW transmit power level as default, if you have an amplified NRF-module and
//// power your radio separately with a good regulator you can turn up PA level.
//#define MY_RF24_PA_LEVEL RF24_PA_LOW
//

//
//// Define a lower baud rate for Arduino's running on 8 MHz (Arduino Pro Mini 3.3V & SenseBender)
//#if F_CPU == 8000000L
//#define MY_BAUD_RATE 38400
//#endif




// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 9

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600


//// Enable serial gateway
#define MY_GATEWAY_SERIAL
// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE

// Inverses behavior of inclusion button (if using external pullup)
//#define MY_INCLUSION_BUTTON_EXTERNAL_PULLUP

// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3

// Set blinking period
#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Inverses the behavior of leds
//#define MY_WITH_LEDS_BLINKING_INVERSE

// Flash leds on rx/tx/err
// Uncomment to override default HW configurations
#define MY_DEFAULT_ERR_LED_PIN 4  // Error led pin
#define MY_DEFAULT_RX_LED_PIN  6  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  5  // the PCB, on board LED



#define MY_NODE_ID 1

#define DHT_DATA_PIN 7
#define ONE_WIRE_BUS 8 // Pin where dallase sensor is connected 
#define TEMPERATURE_PRECISION 10


#include <MySensors.h>
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 

// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0.25

static const uint64_t UPDATE_INTERVAL = 60000;
static const uint8_t FORCE_UPDATE_N_READS = 10;

uint8_t nNoUpdates[7];

bool metric = true;

DHT dht;
Adafruit_BMP085 bmp;

int DsId[7] = { 0,1,2,3,4,5,6 };
char SensorName[] = { 'Woda', 'Piec', 'Spaliny', 'Zewnetrzny', 'Wilgotnosc', 'Zewnetrzny2', 'Cisnienie' };
mysensor_sensor SensorType[] = { S_TEMP, S_TEMP, S_TEMP, S_TEMP, S_HUM, S_TEMP, S_BARO };
MyMessage SensorMsg[7] = {
		MyMessage(0, V_TEMP),
		MyMessage(1, V_TEMP),
		MyMessage(2, V_TEMP),
		MyMessage(3, V_TEMP),
		MyMessage(4, V_HUM),
		MyMessage(5, V_TEMP),
		MyMessage(6, V_PRESSURE)
};

int SensorsCount;
DeviceAddress SensorAddress[3] = {
	{0x28, 0xFC, 0x57, 0x28, 0x00, 0x00, 0x80, 0x35},
	{0x28, 0x7C, 0x35, 0x28, 0x00, 0x00, 0x80, 0x4D},
	{0x28, 0x0B, 0x23, 0x28, 0x00, 0x00, 0x80, 0xB0}
	};
float SensorValue[7] = { 0,0,0,0,0,0,0 };
float SensorLastValue[7] = { 0,0,0,0,0,0,0 };



unsigned long iDhtStop = 0;
unsigned long iDsStop = 0;
unsigned long iSendStop = 0;
unsigned long iBmpStop = 0;



void setup()
{
	SetupDs182b();
//	SetupDht();
//	SetupBmp180();

}

void presentation()
{
	sendSketchInfo("Kotlownia", "1.1");

	// Register all sensors to gw (they will be created as child devices)
	for (int i = 0; i < 7; i++)
	{
		present(i, SensorType[i], SensorName[i]);
	}
	metric = getControllerConfig().isMetric;
}

void loop()
{
	//ReadBmp(1000);
	//ReadDht(1000);
	ReadDs(1000);
	Send(1000);
}

void ReadBmp(unsigned long interval) {
	if (millis() - iBmpStop >= interval)
	{
		// Send locally attached sensor data here
		// Force reading sensor, so it works also after sleep()
		

		// Get temperature from DHT library
		float temperature = bmp.readTemperature();
		if (isnan(temperature)) {
			Serial.println("Failed reading temperature from BMP180!");
		}
		else if (temperature != SensorValue[5]) {
			// Only send temperature if it changed since the last measurement or if we didn't send an update for n times
			SensorValue[5] = temperature;
		}


		Sprint("T: ");
		Sprint(temperature);


		// Get humidity from DHT library
		float pressure = bmp.readPressure();
		if (isnan(pressure)) {
			Serial.println("Failed reading humidity from DHT");
		}
		else if (pressure != SensorValue[6]) {
			// Only send humidity if it changed since the last measurement or if we didn't send an update for n times
			SensorValue[6] = pressure;


			Sprint("H: ");
			Sprintln(pressure);

		}
		iBmpStop = millis();

	}
}
void ReadDht(unsigned long interval) {
	if (millis() - iDhtStop >= interval)
	{
		// Send locally attached sensor data here
		// Force reading sensor, so it works also after sleep()
		dht.readSensor();

		// Get temperature from DHT library
		float temperature = dht.getTemperature();
		if (isnan(temperature)) {
			Serial.println("Failed reading temperature from DHT!");
		}
		else if (temperature != SensorValue[3]) {
			// Only send temperature if it changed since the last measurement or if we didn't send an update for n times
			SensorValue[3] = temperature;
		}


		Sprint("T: ");
		Sprintln(temperature);

		
			// Get humidity from DHT library
			float humidity = dht.getHumidity();
			if (isnan(humidity)) {
				Serial.println("Failed reading humidity from DHT");
			}
			else if (humidity != SensorValue[4] ) {
				// Only send humidity if it changed since the last measurement or if we didn't send an update for n times
				SensorValue[4] = humidity;
				

				Sprint("H: ");
				Sprintln(humidity);

			}
			iDhtStop = millis();

	}
}
void ReadDs(unsigned long interval)
{
	if (millis() - iDsStop >= interval)
	{
		// Fetch temperatures from Dallas sensors
		sensors.requestTemperatures();

		// query conversion time and sleep until conversion completed
		int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
		// sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
		sleep(conversionTime);

		// Read temperatures and send them to controller 
		for (int i = 0; i < 3; i++) {

			// Fetch and round temperature to one decimal
			//float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric ? sensors.getTempC(SensorAddress[i]) : sensors.getTempF(SensorAddress[i])) * 10.)) / 10.;
			float temperature = sensors.getTempC(SensorAddress[i]);
			printData(sensors, SensorAddress[i]);
			// Only send data if temperature has changed and no error

			if (SensorValue[i] != temperature && temperature != -127.00 && temperature != 85.00) {
				SensorValue[i] = temperature;
			}
		}
		iDsStop = millis();

	}
}
void Send(unsigned long interval) 
	{
		if (millis() - iSendStop >= interval)
		{
			for (int i = 0; i < 7; i++)
			{
				if ((SensorLastValue[i] != SensorValue[i] &&
					abs(SensorLastValue[i] - SensorValue[i]) > SENSOR_TEMP_OFFSET
					) || nNoUpdates[i] == FORCE_UPDATE_N_READS)
				{
					send(SensorMsg[i].setSensor(i).set(SensorValue[i], 2));
					SensorLastValue[i] = SensorValue[i];
					nNoUpdates[i] = 0;
				}
				else
				{
					nNoUpdates[i]++;
				}
			}
			iSendStop = millis();
		}
	}
void SetupBmp180() {
	if (!bmp.begin())
	{
		Sprintln("Nie odnaleziono czujnika BMP085 / BMP180");
		//while (1) {}
	}
}
void SetupDs182b() {
	sensors.begin();						  // locate devices on the bus
	SensorsCount = sensors.getDeviceCount();

	Sprint("Locating devices pin8 ...");
	Sprint("Found ");
	Sprint(String(sensors.getDeviceCount(), DEC));
	Sprintln(" devices.");

	//ustawienia rozdzielczosci
	for (uint8_t i = 0; i < SensorsCount; i++)
	{
		DeviceAddress  add;
		if (!sensors.getAddress(add, i)) Sprintln("Unable to find address for Device ");
		sensors.setResolution(add, TEMPERATURE_PRECISION);
		printResolution(sensors, add);
	}

}
void SetupDht() {
	// Setup locally attached sensors
	dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
	if (UPDATE_INTERVAL <= dht.getMinimumSamplingPeriod()) {
		Serial.println("Warning: UPDATE_INTERVAL is smaller than supported by the sensor!");
	}
	// Sleep for the time of the minimum sampling period to give the sensor time to power up
	// (otherwise, timeout errors might occure for the first reading)
	sleep(dht.getMinimumSamplingPeriod());
}
void printData(DallasTemperature sens, DeviceAddress deviceAddress)
{
	Sprint("Device Address: ");
	printAddress(deviceAddress);
	Sprint(" ");
	printTemperature(sens, deviceAddress);
	Sprintln();
}
void printTemperature(DallasTemperature sens, DeviceAddress deviceAddress)
{
	float tempC = sens.getTempC(deviceAddress);
	Sprint("Temp C: ");
	Sprint(tempC);
	Sprint(" Temp F: ");
	Sprint(DallasTemperature::toFahrenheit(tempC));
}
void printResolution(DallasTemperature sens, DeviceAddress deviceAddress)
{
	Sprint("Resolution: ");
	Sprint(sens.getResolution(deviceAddress));
	Sprintln();
}
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) Sprint("0");
		Sprint(String(deviceAddress[i], HEX));
	}
}