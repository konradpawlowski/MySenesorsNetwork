/**
* Name:     Smarthome: Touch Switch Two Light (MySensors)
* Autor:    Alberto Gil Tesa
* Web:      https://giltesa.com/?p=18460
* License:  CC BY-NC-SA 3.0
* Version:  1.0
* Date:     2018/01/13
*
*/
// Node id
#define NODE_ID		201

/**
*  Pinout Touch Switch Board V2.1 and Relay Switch Board V1.1
*/
//#define pBTN        2 //Interruption
//#define pZERO       3 //Interruption (Not used)
#define pRELAY_1    5 //PWM
#define pRELAY_2    6 //PWM
#define pNRF_CE     7
#define pNRF_CSN    8

#define pLED 4
#define pFLASH_SS  A0
#define pDS18B20   A1
#define pATSHA204A A3

#define pBTN1 9
#define pBTN2 10



/**
* MySensor configuration:
*/
#define MY_DEBUG                                // Enable debug prints to serial monitor
#define MY_BAUD_RATE                9600        // Serial output baud rate
//#define MY_TRANSPORT_WAIT_READY_MS  1           // Set how long to wait for transport ready in milliseconds
#define MY_RADIO_NRF24                          // Enable and select radio type attached
#define MY_RF24_CE_PIN              pNRF_CE     // Define this to change the chip enable pin from the default
#define MY_RF24_CS_PIN              pNRF_CSN    // Define this to change the chip select pin from the default
#define MY_REPEATER_FEATURE                   // Enable repeater functionality for this node
#define MY_OTA_FIRMWARE_FEATURE                 // Define this in sketch to allow safe over-the-air firmware updates
#define MY_OTA_FLASH_SS             pFLASH_SS   // Slave select pin for external flash. (The bootloader must use the same pin)
#define MY_OTA_FLASH_JDECID         0xEF30      // https://forum.mysensors.org/topic/4267/w25x40clsnig-as-flash-for-ota
//#define MY_RF24_PA_LEVEL            RF24_PA_HIGH
#define MY_RF24_PA_LEVEL			RF24_PA_MAX
#define MY_NODE_ID					NODE_ID



#define MS_BOARD_NAME               "Oswitelnie garaz"
#define MS_SOFTWARE_VERSION         "1.0"
#define MS_RELAY1_CHILD_ID          1
#define MS_RELAY2_CHILD_ID          2
#define MS_TEMP_CHILD_ID            3
#define ACK							false

#define TEMP_SLEEP  60000
#include <MySensors.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <AsyncTaskLib.h>
#include <Bounce2.h>

MyMessage msgR1(MS_RELAY1_CHILD_ID, V_LIGHT);
MyMessage msgR2(MS_RELAY2_CHILD_ID, V_LIGHT);
MyMessage msgT1(MS_TEMP_CHILD_ID, V_TEMP);


Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();

OneWire oneWire(pDS18B20);
DallasTemperature ds18b20(&oneWire);
AsyncTask asyncReadTemp(TEMP_SLEEP, true, []() { ReadTemperature(); });
AsyncTask* taskOn, * taskOff;

bool first_message_sent = false;
bool ssr1_state = false;
bool ssr2_state = false;
bool value1 = true;
bool value2 = true;
float lastTemperature;

/**
* For initialisations that needs to take place before MySensors transport has been setup (eg: SPI devices).
*/
void before()
{
	pinMode(pBTN1, INPUT_PULLUP);
	pinMode(pBTN2, INPUT_PULLUP);
	pinMode(pLED, OUTPUT);
	pinMode(pRELAY_1, OUTPUT);
	pinMode(pRELAY_2, OUTPUT);


	digitalWrite(pRELAY_2, LOW);
	digitalWrite(pRELAY_1, LOW);
	digitalWrite(pLED, LOW);


	debouncer1.attach(pBTN1);
	debouncer1.interval(5); // interval in ms

	debouncer2.attach(pBTN2);
	debouncer2.interval(5); // interval in ms
	//ds18b20.begin();

}



/**
* Called once at startup, usually used to initialize sensors.
*/
void setup()
{


	taskOn = new AsyncTask(5000, []() { setLed(HIGH);   });
	taskOff = new AsyncTask(1000, []() { setLed(LOW); });

	//request(MS_RELAY1_CHILD_ID, V_LIGHT);
	//request(MS_RELAY2_CHILD_ID, V_LIGHT);
	// request(MS_LEDPWM_CHILD_ID, V_DIMMER);
	//asyncReadTemp.Start();
	//ds18b20.setWaitForConversion(false);
	//ds18b20.setResolution(12);

}



/**
* This allows controller to re-request presentation and do re-configuring node after startup.
*/
void presentation()
{
	sendSketchInfo(MS_BOARD_NAME, MS_SOFTWARE_VERSION, ACK);

	present(MS_RELAY1_CHILD_ID, S_LIGHT, "Light 1");
	present(MS_RELAY2_CHILD_ID, S_LIGHT, "Light 2");
	//present(MS_LEDPWM_CHILD_ID, S_DIMMER, "LED brightness");
	//present(MS_TEMP_CHILD_ID, S_TEMP, "Internal temp");
}



/**
* This will be called continuously after setup.
*/
void loop()
{

	if (first_message_sent == false) {
		Serial.println("Sending initial state...");
		send_startup();
		first_message_sent = true;
	}


	//In case of problems in the wireless connection, the LED flashes blue.
	if (!isTransportReady())
	{
		if (!taskOn->IsActive()) {
			taskOn->Start();
		}
		taskOn->Update(*taskOff);
		taskOff->Update(*taskOn);
	}
	else if (taskOn->IsActive())
	{
		taskOn->Stop();
		setLed(LOW);
	}


	debouncer1.update();
	debouncer2.update();

	int ssrval1 = debouncer1.read();
	if (value1 != ssrval1)
	{
		SetSSR1();
		value1 = ssrval1;
	}
	int ssrval2 = debouncer2.read();
	if (value2 != ssrval2)
	{
		SetSSR2();
		value2 = ssrval2;
	}
	//asyncReadTemp.Update();

}

void send_startup() {

	Serial.println("Sending initial state... rgb");

	send(msgR1.set(digitalRead(pRELAY_1)), ACK);
	send(msgR1.set(digitalRead(pRELAY_2)), ACK);
	//ReadTemperature();
}

/**



/**
*
*/
void receive(const MyMessage& message)
{
	if (message.isAck()) {
		Serial.println("This is an ack from gateway");
	}
	if (message.type == V_STATUS)
	{

		switch (message.sensor)
		{
		case MS_RELAY1_CHILD_ID:
			//SetState(pRELAY_1, message.getBool());
			SetSSR1();
			break;

		case MS_RELAY2_CHILD_ID:
			//SetState(pRELAY_2, message.getBool());
			SetSSR2();
			break;
		default:
			break;
		}

	}

}


void ReadTemperature() {
	//Blink(pLED, 1);
	// Fetch temperatures from Dallas sensors
	ds18b20.requestTemperatures();

	// query conversion time and sleep until conversion completed
	int16_t conversionTime = ds18b20.millisToWaitForConversion(ds18b20.getResolution());
	// sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
	//wait(conversionTime);

	// Fetch and round temperature to one decimal
	float temperature = getControllerConfig().isMetric ? ds18b20.getTempC(0) : ds18b20.getTempF(0);

	// Only send data if temperature has changed and no error
#if COMPARE_TEMP == 1
	if (lastTemperature[i] != temperature && temperature != -127.00 && temperature != 85.00) {
#else
	if (temperature != -127.00 && temperature != 85.00) 
	{
#endif
		send(msgT1.set(temperature, 2));
		// Send in the new temperature

		// Save new temperatures for next compare
		lastTemperature = temperature;
	}
}
	


	/**
	* Turn on the led in the indicated color.
	*/
	void setLed(uint8_t status)
	{
		digitalWrite(pLED, status);

	}
	void SetSSR1() {

		ssr1_state = !ssr1_state;
		SetState(pRELAY_1, ssr1_state);
		send(msgR1.set(ssr1_state), ACK);
		//Blink(pLED, 1);

	}

	void SetSSR2() {

		ssr2_state = !ssr2_state;
		SetState(pRELAY_2, ssr2_state);
		send(msgR2.set(ssr2_state), ACK);
		//Blink(pLED, 2);


	}
	void SetState(uint8_t pin, bool state)
	{
		digitalWrite(pin, state ? HIGH : LOW);
	}


	void Blink(uint8_t pin, int count) {
		for (size_t i = 0; i < count; i++)
		{
			SetState(pin, true);
			wait(200);
			SetState(pin, true);
			wait(200);
		}
		
	}