/*
	Name:       HeaterController.ino
	Created:	21.09.2019 19:08:43
	Author:     DESKTOP-R29CEKM\konra
*/



#include <AsyncTaskLib.h>
#include <OneWire.h>
#include <Bounce2.h>
#include <DallasTemperature.h>

#define MY_DEBUG

// MySencsor deifnition
#define MY_RS485
#define MY_NODE_ID  31
// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2
#define MY_RS485_BAUD_RATE 9600
#define ACK false





#include <MySensors.h>
#pragma region InitalTemp
// DS18b20 temp sensor
#define PIN_IN_TEMP  4
#define COMPARE_TEMP 1 // Send temperature only if changed? 1 = Yes 0 = No
#define MAX_ATTACHED_DS18B20 6

unsigned long TEMP_SLEEP = 30000; // Sleep time between reads (in milliseconds)
OneWire oneWire(PIN_IN_TEMP); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature tempSensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors = 0;
bool receivedConfig = false;
bool metric = true;
// Initialize temperature message
MyMessage msg(0, V_TEMP);
AsyncTask asyncReadTemp(TEMP_SLEEP, true, []() { ReadTemperature(); });
DeviceAddress defTempSensors[7] = {
	{0x28, 0xE3, 0x1D, 0x77, 0x91, 0x13, 0x02, 0x56},//1 
	{0x28, 0x49, 0x48, 0x45, 0x92, 0x0A, 0x02, 0x77},//2
	{0x28, 0xFC, 0xA4, 0x45, 0x92, 0x10, 0x02, 0x6B},//3
	{0x28, 0xFF, 0xC6, 0xF4, 0x40, 0x18, 0x03, 0xC0}, //4
	{0x28, 0xFF, 0xA4, 0x91, 0x41, 0x18, 0x03, 0xE8 }, //5
	{0x28, 0x7C, 0x35, 0x28, 0x00, 0x00, 0x80, 0x4D},  //6 - 2
	{0x28, 0x0B, 0x23, 0x28, 0x00, 0x00, 0x80, 0xB0} //7 - 3
};


//DeviceAddress defTempSensors[6] = {
//	{0x28, 0xE3, 0x1D, 0x77, 0x91, 0x13, 0x02, 0x56},//1 
//	{0x28, 0x49, 0x48, 0x45, 0x92, 0x0A, 0x02, 0x77},//2
//	{0x28, 0xFC, 0xA4, 0x45, 0x92, 0x10, 0x02, 0x6B},//3
//	{0x28, 0xFC, 0x57, 0x28, 0x00, 0x00, 0x80, 0x35}, //5
//	{0x28, 0x7C, 0x35, 0x28, 0x00, 0x00, 0x80, 0x4D},  //6
//	{0x28, 0x0B, 0x23, 0x28, 0x00, 0x00, 0x80, 0xB0} //7
//};
#pragma endregion



#pragma region InitalInput
#define PIN_IN_SMOKE 5
#define PIN_IN_POMPA_CUW 6
#define PIN_IN_PODAJNIK 7
#define PIN_IN_WENTYLATOR 12
#define PIN_IN_POMPA_CO 11 
#define PIN_IN_ALARM 3 

#define NUM_INPUT_SENSORS 6
const uint8_t INPUT_SENSORS_PINS[NUM_INPUT_SENSORS] = {
	PIN_IN_PODAJNIK,
	PIN_IN_WENTYLATOR,
	PIN_IN_POMPA_CUW,
	PIN_IN_POMPA_CO,
	PIN_IN_ALARM,
	PIN_IN_SMOKE
	
};

Bounce* inputSensorsBounce = new Bounce[NUM_INPUT_SENSORS];
bool inputLastValues[NUM_INPUT_SENSORS] = {};
MyMessage inmputMessages[NUM_INPUT_SENSORS] = {
	MyMessage(6, V_TRIPPED),
	MyMessage(7, V_TRIPPED),
	MyMessage(8, V_TRIPPED),
	MyMessage(9, V_TRIPPED),
	MyMessage(10, V_ARMED),
	MyMessage(11, V_ARMED)
};

AsyncTask asyncReadInput(200, true, []() { ReadInputs(); });

#pragma endregion

#pragma region InitalOutput
#define PIN_OUT_REG_POK 19
#define PIN_OUT_BOJLER 18
#define PIN_OUT_POMPA_CUW 13
#define PIN_OUT_POMPA_CO 17
#define PIN_OUT_POMPA_OBIEG 16
#define PIN_OUT_WENTYLATOR 15
#define PIN_OUT_PODAJNIK 14

#define NUM_OUTPUT_SENSORS 7


const uint8_t OUTPUT_SENSORS_PINS[NUM_OUTPUT_SENSORS] = {
	PIN_OUT_PODAJNIK,
	PIN_OUT_WENTYLATOR,
	PIN_OUT_POMPA_CUW,
	PIN_OUT_POMPA_CO,
	PIN_OUT_POMPA_OBIEG,
	PIN_OUT_REG_POK,
	PIN_OUT_BOJLER,

	
	
	
};

MyMessage outputMessages[NUM_OUTPUT_SENSORS] = {
	MyMessage(12, V_STATUS),
	MyMessage(13, V_STATUS),
	MyMessage(14, V_STATUS),
	MyMessage(15, V_STATUS),
	MyMessage(16, V_STATUS),
	MyMessage(17, V_STATUS),
	MyMessage(18, V_STATUS)
	
};
#pragma endregion

bool first_message_sent = false;



void before() {
	tempSensors.begin();
}


void presentation() {
	sendSketchInfo("HeaterController ", "2.0", ACK);
	presentSensor();
}

void setup()
{
	setTempSensors();
	setupInputSensors();
	setupOutpuntSensors();

}

void loop()
{

	if (first_message_sent == false) {
		Serial.println("Sending initial state...");
		send_startup();
		first_message_sent = true;
	}
	asyncReadTemp.Update();
	asyncReadInput.Update();
	//ReadInputs();
	//ReadTemperature();
	//sleep(1000);

}

void presentSensor() {

	// Present all sensors to controller
	for (int i = 0; i < MAX_ATTACHED_DS18B20; i++) {
		present(i, S_TEMP, "temper");

	}

	
	
	present(6, S_MOTION, "Podajnik");
	present(7, S_MOTION, "Wentylator");
	present(8, S_MOTION, "Pompa CUW");
	present(9, S_MOTION, "Pompa CO");
	present(10, S_MOTION, "Alarm Piec");
	present(11, S_SMOKE, "Smoke");
	
	
	present(12, S_BINARY, "Piec podajnik");
	present(13, S_BINARY, "Piec wentylator");
	present(14, S_BINARY, "Pompa CUW");
	present(15, S_BINARY, "Pompa CO");
	present(16, S_BINARY, "Pompa Obieg");
		
	present(17, S_BINARY, "Regulator pokojowy");
	present(18, S_BINARY, "Bojler");

}
void setTempSensors() {
	asyncReadTemp.Start();
	tempSensors.setWaitForConversion(false);
	for (int i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		tempSensors.setResolution(defTempSensors[i], 12);
	}
}

void setupInputSensors() {
	asyncReadInput.Start();
	for (int i = 0; i < NUM_INPUT_SENSORS; i++) {
		inputSensorsBounce[i].attach(INPUT_SENSORS_PINS[i], INPUT_PULLUP);       //setup the bounce instance for the current button
		inputSensorsBounce[i].interval(25);              // interval in ms
	}
}

void setupOutpuntSensors() {
	for (int i = 0; i < NUM_OUTPUT_SENSORS; i++)
	{
		pinMode(OUTPUT_SENSORS_PINS[i], OUTPUT);
	}
}
void ReadTemperature() {
	// Fetch temperatures from Dallas sensors
	tempSensors.requestTemperatures();

	// query conversion time and sleep until conversion completed
	int16_t conversionTime = tempSensors.millisToWaitForConversion(tempSensors.getResolution());
	// sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
	sleep(conversionTime);

	// Read temperatures and send them to controller 
	for (int i = 0; i < MAX_ATTACHED_DS18B20; i++) {

		// Fetch and round temperature to one decimal
		float temperature = getControllerConfig().isMetric ? tempSensors.getTempC(defTempSensors[i]) : tempSensors.getTempF(defTempSensors[i]);

		// Only send data if temperature has changed and no error
#if COMPARE_TEMP == 1
		if (lastTemperature[i] != temperature && temperature != -127.00 && temperature != 85.00) {
#else
		if (temperature != -127.00 && temperature != 85.00) {
#endif

			// Send in the new temperature
		send(msg.setSensor(i).set(temperature, 2), ACK);
		// Save new temperatures for next compare
		lastTemperature[i] = temperature;
		}
	}
}
void ReadInputs() {
	//Serial.println("Weszl read ...");
	//Serial.println(millis());
	for (int i = 0; i < NUM_INPUT_SENSORS; i++) {
		// Update the Bounce instance :
		inputSensorsBounce[i].update();
		//bool value = inputSensorsBounce[i].read();
		//// If it fell, flag the need to toggle the LED
		//if (inputLastValues[i] == value) {
		//	send(inmputMessages[i].set(value));
		//	SetOutputFromInput(i, value);
		//	inputLastValues[i] = value;

		//}


		if (inputSensorsBounce[i].fell()) {
			send(inmputMessages[i].set(HIGH));
			SetOutputFromInput(i, true);
			//saveState(i, true);
		}
		if (inputSensorsBounce[i].rose()) {
			send(inmputMessages[i].set(LOW));
			SetOutputFromInput(i, false);
			//saveState(i, false);
		}
	}
}


void SetOutputFromInput(int inputId, bool status) {
	
	switch (inputId)
	{
	case 0:
		SetOutput(12, status);
		break;
	case 1:
		SetOutput(13, status);
		break;
	case 2:
		SetOutput(14, status);
		break;
	case 3:
		SetOutput(15, status);
		break;

	}


}

void SetOutput(int sensor_id, bool status) {
	/*Serial.println("Set Output");
	Serial.println(sensor_id);
	Serial.println(OUTPUT_SENSORS_PINS[sensor_id]);*/
	
		digitalWrite(OUTPUT_SENSORS_PINS[sensor_id - 12], status ? HIGH : LOW);
		send(outputMessages[sensor_id - 12].set(status), ACK);
		saveState(sensor_id, status);
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
		saveState(message.sensor, message.getBool());
		// Write some debug info
		Serial.print("Incoming change for sensor:");
		Serial.print(message.sensor);
		Serial.print(", New status: ");
		Serial.println(message.getBool());
	}
}
void send_startup() {

	Serial.println("Sending initial state... intput");
	//Serial.println(millis());
	for (int i = 0; i < NUM_INPUT_SENSORS; i++) {
		// Update the Bounce instance :
		inputSensorsBounce[i].update();
		send(inmputMessages[i].set(!inputSensorsBounce[i].read()));
	}

	for (int i = 0; i < NUM_OUTPUT_SENSORS; i++) {
		// Update the Bounce instance :
		
		send(outputMessages[i].set(loadState(i+12)));
	}

}