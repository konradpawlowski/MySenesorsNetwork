/**
   Name:     Smarthome: LED RGBW Controller
   Autor:    Alberto Gil Tesa
   Web:      https://giltesa.com/smarthome
   License:  CC BY-NC-SA 4.0
   Version:  1.0.2
   Date:     2018/04/05

*/


/**
	Pinout V1.0
*/
#define pLED_RED    5 //PWM
#define pLED_GREEN  6 //PWM
#define pLED_BLUE   9 //PWM
#define pLED_WHITE  3 //PWM
#define pNRF_CE     7
#define pNRF_CS    10
#define pFLASH_CS   8
#define pDS18B20   A0
#define pATSHA204A A3



/**
   MySensor configuration:
*/
#define MY_DEBUG                                // Enable debug prints to serial monitor
#define MY_BAUD_RATE                9600        // Serial output baud rate
//#define MY_TRANSPORT_WAIT_READY_MS  1000        // Set how long to wait for transport ready in milliseconds
#define MY_RADIO_NRF24                          // Enable and select radio type attached
#define MY_RF24_CE_PIN              pNRF_CE     // Define this to change the chip enable pin from the default
#define MY_RF24_CS_PIN              pNRF_CS     // Define this to change the chip select pin from the default
#define MY_REPEATER_FEATURE                   // Enable repeater functionality for this node
#define MY_OTA_FIRMWARE_FEATURE                 // Define this in sketch to allow safe over-the-air firmware updates
#define MY_OTA_FLASH_SS             pFLASH_CS   // Slave select pin for external flash. (The bootloader must use the same pin)
#define MY_OTA_FLASH_JDECID         0xEF30      // https://forum.mysensors.org/topic/4267/w25x40clsnig-as-flash-for-ota
#define MY_RF24_PA_LEVEL            RF24_PA_HIGH
#define MY_NODE_ID                  1

#define MS_BOARD_NAME               "LED RGBW Controller"
#define MS_SOFTWARE_VERSION         "1.0.2"
#define MS_STATUS_CHILD_ID          1
#define MS_RGB_CHILD_ID             1
#define MS_RGBW_CHILD_ID            1
#define MS_WHITE_CHILD_ID           1
#define MS_TEMP_CHILD_ID            1

#include <MySensors.h>
#include <DallasTemperature.h>
#include <AsyncTaskLib.h>

MyMessage msgSTAT(MS_STATUS_CHILD_ID, V_STATUS);

MyMessage msgRGB(MS_RGB_CHILD_ID, V_RGB);
//MyMessage msgRGBW( MS_RGBW_CHILD_ID, V_RGBW );
MyMessage msgWhite(MS_WHITE_CHILD_ID, V_PERCENTAGE);

MyMessage msgTEMP(MS_TEMP_CHILD_ID, V_TEMP);

bool first_message_sent = false;
int CurrentLevel = 100;
DallasTemperature ds18b20(new OneWire(pDS18B20));
AsyncTask* taskOn, * taskOff, * taskTemp;
bool recived = false;
bool CurrentStatus;
struct RGBW {
	byte r, g, b, w;
} led;




/**
   For initialisations that needs to take place before MySensors transport has been setup (eg: SPI devices).
*/
void before()
{
	pinMode(pLED_RED, OUTPUT);
	pinMode(pLED_GREEN, OUTPUT);
	pinMode(pLED_BLUE, OUTPUT);
	pinMode(pLED_WHITE, OUTPUT);

	setLedColor('0');
}



/**
   Called once at startup, usually used to initialize sensors.
*/
void setup()
{
	ds18b20.begin();

	taskOn = new AsyncTask(4000, []() {
		setLedColor('B');
		});
	taskOff = new AsyncTask(1000, []() {
		setLedColor('0');
		});

#ifdef MY_DEBUG
	taskTemp = new AsyncTask(60000, true, []() {
		sendTemperature();
		});
	taskTemp->Start();
#endif
	loadRgbwLed();
	CurrentLevel = loadState(6);

}



/**
   This allows controller to re-request presentation and do re-configuring node after startup.
*/
void presentation()
{
	sendSketchInfo(MS_BOARD_NAME, MS_SOFTWARE_VERSION, false);

	// present(MS_STATUS_CHILD_ID, S_BINARY,     "LED On/Off");
	present(MS_RGB_CHILD_ID, S_RGB_LIGHT, "LED RGB");
	//present(MS_RGBW_CHILD_ID,   S_RGBW_LIGHT, "LED RGBW");
	//present(MS_WHITE_CHILD_ID,  S_DIMMER,     "LED White");
	//present(MS_TEMP_CHILD_ID,   S_TEMP,       "Temp. Int.");
}



/**
   This will be called continuously after setup.
*/
void loop()
{

	if (first_message_sent == false) {
		Serial.println("Sending initial state...");
		send_startup();
		first_message_sent = true;
	}


#ifdef MY_DEBUG
	taskTemp->Update();
#endif


	//In case of problems in the wireless connection, the LED flashes blue.
	if (!isTransportReady())
	{
		if (!taskOn->IsActive()) {
			taskOn->Start();
		}
		taskOn->Update(*taskOff);
		taskOff->Update(*taskOn);





	}
	else if (taskOn->IsActive() || taskOff->IsActive())
	{
		taskOn->Stop();
		taskOff->Stop();
		//    setLedColor('0');
		fadeLed(CurrentStatus, led, CurrentLevel);

	}



}


void send_startup() {
	//char str[32] = "";
	// array_to_string(led, str);
	Serial.println("Sending initial state... rgb");
	//send(msgRGB.set(str), false);
	send(msgRGB.set("f1e2d3"), true);
	//send(msgWhite.set(CurrentLevel), true);
	send(msgWhite.set(100), true);
	Serial.println("Sending initial state... stat");
	send(msgSTAT.set(1), true);
}

/**

*/
void receive(const MyMessage& message)
{
	if (!recived) {
		recived = true;
		bool statusLed = false;
		RGBW ledIn;
		if (message.isAck()) {
			Serial.println("This is an ack from gateway");
		}
		if (message.sensor == MS_STATUS_CHILD_ID || message.sensor == MS_RGB_CHILD_ID || message.sensor == MS_RGBW_CHILD_ID || message.sensor == MS_WHITE_CHILD_ID)
		{
			if (message.type == V_STATUS)
			{

				statusLed = message.getBool();
				CurrentStatus = statusLed;
				fadeLed(statusLed, led, CurrentLevel);

			}
			else if (message.type == V_RGB || message.type == V_RGBW)
			{
				String hex = message.getString();
				if (hex.startsWith("#")) {
					hex = hex.substring(1, hex.length());
				}

				if (hex.length() == 6 || hex.length() == 8)
				{
					ledIn.r = strHexToDec(hex.substring(0, 2));
					ledIn.g = strHexToDec(hex.substring(2, 4));
					ledIn.b = strHexToDec(hex.substring(4, 6));
					ledIn.w = hex.length() == 8 ? strHexToDec(hex.substring(6, 8)) : 0;

					if (ledIn.r == 0 && ledIn.g == 0 && ledIn.b == 0 && ledIn.w == 0) {
						statusLed = false;

					}
					else {
						statusLed = true;
					}

				}
				fadeLed(statusLed, ledIn, CurrentLevel);
				send(msgSTAT.set(statusLed), true);
				CurrentStatus = statusLed;
			}
			else if (message.type == V_DIMMER)
			{
				setDimLevel(message.getInt());
			}

#ifndef MY_DEBUG
			sendTemperature();
#endif
		}
		recived = false;
	}
}
RGBW CalculateColor(RGBW ledIn, int level) {
	RGBW rLed;
	level = level > 100 ? 100 : level;
	level = level < 0 ? 0 : level;
	rLed.r = (int)(level / 100. * ledIn.r);
	rLed.b = (int)(level / 100. * ledIn.b);
	rLed.g = (int)(level / 100. * ledIn.g);
	return rLed;
}
void setDimLevel(int level) {

	if (level == 0)
	{
		fadeLed(false, led, level);
	}
	else {
		fadeLed(true, led, level);
		CurrentLevel = level;
		saveState(6, level);
	}
}

/**

*/
byte strHexToDec(String str)
{
	char charbuf[3];
	str.toCharArray(charbuf, 3);
	return strtol(charbuf, 0, 16);
}



/**

*/
void sendTemperature()
{
	ds18b20.requestTemperatures();
	send(msgTEMP.set(getControllerConfig().isMetric ? ds18b20.getTempCByIndex(0) : ds18b20.getTempFByIndex(0), 1), false);
}



/**
   Turn on the led in the indicated color.
*/
void setLedColor(char color)
{
	switch (color)
	{
	case 'R':   //RED
		digitalWrite(pLED_RED, HIGH);
		digitalWrite(pLED_GREEN, LOW);
		digitalWrite(pLED_BLUE, LOW);
		digitalWrite(pLED_WHITE, LOW);
		break;
	case 'G':   //GREEN
		digitalWrite(pLED_RED, LOW);
		digitalWrite(pLED_GREEN, HIGH);
		digitalWrite(pLED_BLUE, LOW);
		digitalWrite(pLED_WHITE, LOW);
		break;
	case 'B':   //BLUE
		digitalWrite(pLED_RED, LOW);
		digitalWrite(pLED_GREEN, LOW);
		digitalWrite(pLED_BLUE, HIGH);
		digitalWrite(pLED_WHITE, LOW);
		break;
	case 'W':   //WHITE
		digitalWrite(pLED_RED, HIGH);
		digitalWrite(pLED_GREEN, HIGH);
		digitalWrite(pLED_BLUE, HIGH);
		// digitalWrite(pLED_WHITE, HIGH);
		break;
	default:    //OFF
		digitalWrite(pLED_RED, LOW);
		digitalWrite(pLED_GREEN, LOW);
		digitalWrite(pLED_BLUE, LOW);
		digitalWrite(pLED_WHITE, LOW);
		break;
	}
}



/**
   Activate or deactivate the light gradually.
   The code is not perfect...
*/
void fadeLed(bool statusIn, RGBW ledIn, int levelIn)
{
	bool statusOld = loadState(MS_STATUS_CHILD_ID);
	char str[32] = "";
	printRgbw(ledIn);

	if (statusIn) {
		if (statusOld) {
			ledChange(ledIn, levelIn);
		}
		else {
			ledOn();

			send(msgSTAT.set(statusIn), true);
		}
	}
	else {
		if (statusOld) {
			ledOff();
			send(msgSTAT.set(statusIn), true);
		}
	}
	if (statusOld != statusIn)
	{
		saveState(MS_STATUS_CHILD_ID, statusIn);
	}
	saveRgbwLed();
	array_to_string(led, str);

	send(msgRGB.set(str), false);

}

void ledChange(RGBW ledIn, int levelIn) {
	// Serial.println("Zmiana");
	//    Serial.println("===led==");
	//   printRgbw(led);
	RGBW ledSet = CalculateColor(led, CurrentLevel);
	//   Serial.println("LedSet");
	//   printRgbw(ledSet);
	//
	//
	//   Serial.println("***ledIn****");
	//   printRgbw(ledIn);

	RGBW ledIn2 = CalculateColor(ledIn, levelIn);
	// Serial.println("ledIn2");
	//   printRgbw(ledIn2);


	while (ledSet.r != ledIn2.r || ledSet.g != ledIn2.g || ledSet.b != ledIn2.b || ledSet.w != ledIn2.w)
	{
		analogWrite(pLED_RED, (ledSet.r = ledSet.r < ledIn2.r ? ledSet.r + 1 : (ledSet.r > ledIn2.r ? ledSet.r - 1 : ledIn2.r)));
		analogWrite(pLED_GREEN, (ledSet.g = ledSet.g < ledIn2.g ? ledSet.g + 1 : (ledSet.g > ledIn2.g ? ledSet.g - 1 : ledIn2.g)));
		analogWrite(pLED_BLUE, (ledSet.b = ledSet.b < ledIn2.b ? ledSet.b + 1 : (ledSet.b > ledIn2.b ? ledSet.b - 1 : ledIn2.b)));
		analogWrite(pLED_WHITE, (ledSet.w = ledSet.w < ledIn2.w ? ledSet.w + 1 : (ledSet.w > ledIn2.w ? ledSet.w - 1 : ledIn2.w)));
		wait(10);
	}
	led = ledIn;
}

void ledOff() {
	//    Serial.println("WyĹ‚Ä…czanie");
	RGBW ledOut = CalculateColor(led, CurrentLevel);
	while (ledOut.r != 0 || ledOut.g != 0 || ledOut.b != 0 || ledOut.w != 0)
	{
		analogWrite(pLED_RED, (ledOut.r = ledOut.r - 1 > 0 ? ledOut.r - 1 : 0));
		analogWrite(pLED_GREEN, (ledOut.g = ledOut.g - 1 > 0 ? ledOut.g - 1 : 0));
		analogWrite(pLED_BLUE, (ledOut.b = ledOut.b - 1 > 0 ? ledOut.b - 1 : 0));
		analogWrite(pLED_WHITE, (ledOut.w = ledOut.w - 1 > 0 ? ledOut.w - 1 : 0));
		wait(10);
		Serial.print("+");
	}

	//  printRgbw(led);
}
void ledOn() {
	//    Serial.println("WĹ‚Ä…czanie");
	RGBW ledOut;
	ledOut.r = 0;
	ledOut.g = 0;
	ledOut.b = 0;
	ledOut.w = 0;
	RGBW ledIn = CalculateColor(led, CurrentLevel);
	Serial.println("Włączenie");
	printRgbw(ledIn);
	printRgbw(ledOut);

	if (led.r > 0 || led.g > 0 || led.b > 0 || led.w > 0)
	{
		while (ledOut.r != ledIn.r || ledOut.g != ledIn.g || ledOut.b != ledIn.b || ledOut.w != ledIn.w)
		{
			analogWrite(pLED_RED, (ledOut.r = ledOut.r < ledIn.r ? ledOut.r + 1 : ledOut.r));
			analogWrite(pLED_GREEN, (ledOut.g = ledOut.g < ledIn.g ? ledOut.g + 1 : ledOut.g));
			analogWrite(pLED_BLUE, (ledOut.b = ledOut.b < ledIn.b ? ledOut.b + 1 : ledOut.b));
			analogWrite(pLED_WHITE, (ledOut.w = ledOut.w < ledIn.w ? ledOut.w + 1 : ledOut.w));
			wait(10);


			// Serial.print(".");
		}
	}
	//printRgbw(led);
}
void printRgbw(RGBW ledIn) {

	Serial.print("R: ");
	Serial.println(ledIn.r);
	Serial.print("G: ");
	Serial.println(ledIn.g);
	Serial.print("B: ");
	Serial.println(ledIn.b);
	Serial.print("W: ");
	Serial.println(ledIn.w);


}

void saveRgbwLed() {
	saveState(2, led.r);
	saveState(3, led.g);
	saveState(4, led.b);
	saveState(5, led.w);
}

void loadRgbwLed() {

	led.r = loadState(2);
	led.g = loadState(3);
	led.b = loadState(4);
	led.w = loadState(5);

	printRgbw(led);
}

void array_to_string(RGBW ledIn, char buffer[])
{
	byte array[4] = { ledIn.r, ledIn.g, ledIn.b, ledIn.w };
	unsigned int len = 4;
	for (unsigned int i = 0; i < len; i++)
	{
		byte nib1 = (array[i] >> 4) & 0x0F;
		byte nib2 = (array[i] >> 0) & 0x0F;
		buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
		buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
	}
	buffer[len * 2] = '\0';
}
