/*
 Name:		OutsideSensors.ino
 Created:	4/15/2018 5:31:53 PM
 Author:	Konrad
*/

// Enable debug prints to serial monitor

#include <AsyncTaskLib.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>

#define MY_DEBUG

// Enable RS485 transport layer
#define MY_RS485
#define MY_NODE_ID  30
// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600
#define DHT_PIN 5
#define SENSOR_TEMP_OFFSET 0
#define LIGHT_SENSOR 3


#define CHILD_ID_LIGHT 5
#define CHILD_ID_HUM 1
#define CHILD_ID_TEMP 2
#define CHILD_ID_PRES 3
#define CHILD_ID_TEMP2 4

bool metric = true;
bool first_message_sent = false;

#include <MySensors.h>
MyMessage msg(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgBar(CHILD_ID_PRES, V_PRESSURE);
MyMessage msgTemp2(CHILD_ID_TEMP2, V_TEMP);


static const uint8_t FORCE_UPDATE_N_READS = 10;

int lastLightLevel;



// Enable this if RS485 is connected to a hardware serial port
//#define MY_RS485_HWSERIAL Serial1


struct BmpValue
{
	float temperature;
	int presure;
	uint8_t nNoUpdatesTemp;
	uint8_t nNoUpdatesHum;

};
struct DhtValue
{
	float temperature;
	int humidity;
	uint8_t nNoUpdatesTemp;
	uint8_t nNoUpdatesHum;

};
BmpValue lastBmpValue;
DhtValue lastDhtValue;





bool lightValue;
Adafruit_BMP085 bmp;
DHT dht;

AsyncTask asyncReadLight(10000, true, []() { readLight(); });
AsyncTask asyncReadDht(10000, true, []() { readDht(); });
AsyncTask asyncReadBmp(10000, true, []() { readBmp(); });


void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Outside Sensor", "2.0");

	// Register all sensors to gateway (they will be created as child devices)
	present(CHILD_ID_LIGHT, S_BINARY, "Czujnik oœwietlenia");
	present(CHILD_ID_HUM, S_HUM, "Wilgotnoœæ");
	present(CHILD_ID_TEMP, S_TEMP, "Temperatura 1");
	present(CHILD_ID_PRES, S_BARO,"Ciœnienie");
	present(CHILD_ID_TEMP2, S_TEMP, "Temperatura 2");
	metric = getControllerConfig().isMetric;
}

void before() {
	if (!bmp.begin()) {
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
		while (1) {}
	}
	dht.setup(DHT_PIN); // data pin 2
	sleep(dht.getMinimumSamplingPeriod());
}


// the setup function runs once when you press reset or power the board
void setup() {
	

	asyncReadLight.Start();
	asyncReadDht.Start();
	asyncReadBmp.Start();


}

// the loop function runs over and over again until power down or reset
void loop() {

	if (first_message_sent == false) {
		Serial.println("Sending initial state...");
		readBmp();
		readDht();
		readLight();

		first_message_sent = true;
	}


	asyncReadLight.Update();
	asyncReadBmp.Update();
	asyncReadDht.Update();
}


void readBmp() {
	
	float temperature = bmp.readTemperature();
	if (isnan(temperature)) {
		Serial.println("Failed reading temperature from BMP180!");
	}
	else if (temperature != lastBmpValue.temperature || lastBmpValue.nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
		// Only send temperature if it changed since the last measurement or if we didn't send an update for n times
		lastBmpValue.temperature = temperature;

		// apply the offset before converting to something different than Celsius degrees
		temperature += SENSOR_TEMP_OFFSET;

		if (!metric) {
			temperature = dht.toFahrenheit(temperature);
		}
		// Reset no updates counter
		lastBmpValue.nNoUpdatesTemp = 0;
		send(msgTemp2.set(temperature, 1));

#ifdef MY_DEBUG
		Serial.print("T: ");
		Serial.println(temperature);
#endif
	}
	else {
		// Increase no update counter if the temperature stayed the same
		lastBmpValue.nNoUpdatesTemp++;
	}

	// Get humidity from DHT library
	//float presure = bmp.readPressure();
	float presure = bmp.readSealevelPressure(272);

	if (isnan(presure)) {
		Serial.println("Failed reading humidity from DHT");
	}
	else {
		presure = roundf(presure / 100);
	}
		
		
		if (presure != lastBmpValue.presure || lastBmpValue.nNoUpdatesHum == FORCE_UPDATE_N_READS) {
		// Only send humidity if it changed since the last measurement or if we didn't send an update for n times
		lastBmpValue.presure = presure;
		// Reset no updates counter
		lastBmpValue.nNoUpdatesHum = 0;
		send(msgBar.set(presure, 1));

#ifdef MY_DEBUG
		Serial.print("H: ");
		Serial.println(presure);
#endif
	}
	else {
		// Increase no update counter if the humidity stayed the same
		lastDhtValue.nNoUpdatesHum++;
	}





}
void readDht() {
	dht.readSensor(true);

	// Get temperature from DHT library
	float temperature = dht.getTemperature();
	if (isnan(temperature)) {
		Serial.println("Failed reading temperature from DHT!");
	}
	else if (temperature != lastDhtValue.temperature || lastDhtValue.nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
		// Only send temperature if it changed since the last measurement or if we didn't send an update for n times
		lastDhtValue.temperature = temperature;

		// apply the offset before converting to something different than Celsius degrees
		temperature += SENSOR_TEMP_OFFSET;

		if (!metric) {
			temperature = dht.toFahrenheit(temperature);
		}
		// Reset no updates counter
		lastDhtValue.nNoUpdatesTemp = 0;
		send(msgTemp.set(temperature, 1));

#ifdef MY_DEBUG
		Serial.print("T: ");
		Serial.println(temperature);
#endif
	}
	else {
		// Increase no update counter if the temperature stayed the same
		lastDhtValue.nNoUpdatesTemp++;
	}

	// Get humidity from DHT library
	float humidity = dht.getHumidity();
	if (isnan(humidity)) {
		Serial.println("Failed reading humidity from DHT");
	}
	else if (humidity != lastDhtValue.humidity || lastDhtValue.nNoUpdatesHum == FORCE_UPDATE_N_READS) {
		// Only send humidity if it changed since the last measurement or if we didn't send an update for n times
		lastDhtValue.humidity = humidity;
		// Reset no updates counter
		lastDhtValue.nNoUpdatesHum = 0;
		send(msgHum.set(humidity, 1));

#ifdef MY_DEBUG
		Serial.print("H: ");
		Serial.println(humidity);
#endif
	}
	else {
		// Increase no update counter if the humidity stayed the same
		lastDhtValue.nNoUpdatesHum++;
	}

}

void readLight() {

	bool lightLevel = digitalRead(LIGHT_SENSOR);
	Serial.println(lightLevel);
	if (lightLevel != lastLightLevel) {
		send(msg.set(lightLevel));
		lastLightLevel = lightLevel;
	}


}

