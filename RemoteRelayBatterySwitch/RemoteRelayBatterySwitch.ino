/*
    Name:       RemoteRelayBatterySwitch.ino
    Created:	29.03.2019 11:40:36
    Author:     DESKTOP-M999RU2\konra
*/

#include <avr/sleep.h>//this AVR library contains the methods that controls the sleep modes
#include <SPI.h>
#include <NRFLite.h>
#include <C:\Users\konra\source\repos\MySenesorsNetwork\RemoteRelay\Definiction.h>


#define LedOn1 5
#define LedOn2 4
#define LedOn3 A5

#define LedOff1 6  
#define LedOff2 7
#define LedOff3 A4

#define Touch1 2
#define Touch2 A2
#define Touch3 3

const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;
unsigned long mils;

bool pb1LastStatus;
bool pb2LastStatus;
NRFLite _radio;
RadioPacket _radioData;

void setup()
{
	Serial.begin(9600);

	pinMode(Touch1, INPUT);
	pinMode(Touch2, INPUT);
	pinMode(Touch3, INPUT);

	pinMode(LedOff1, OUTPUT);
	pinMode(LedOff2, OUTPUT);
	pinMode(LedOff3, OUTPUT);
	pinMode(LedOn1, OUTPUT);
	pinMode(LedOn2, OUTPUT);
	pinMode(LedOn3, OUTPUT);

	

	if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
	{
		Serial.println("Cannot communicate with radio");
		while (1); // Wait here forever.
	}

	_radioData.FromRadioId = RADIO_ID;
	setLight(HIGH);

}

void loop()
{
	
	_radioData.Pb1 = digitalRead(Touch1);
	_radioData.Pb2 = digitalRead(Touch3);
	digitalWrite(LedOff1, _radioData.Pb1);
	digitalWrite(LedOn1, !_radioData.Pb1);
	digitalWrite(LedOff3, _radioData.Pb2);
	digitalWrite(LedOn3, !_radioData.Pb2);


	sendData(_radioData);
	//Blink(LedOn2);

	if (millis() - mils > 5000)
		Going_To_Sleep();

}
void Going_To_Sleep() {
	sleep_enable();//Enabling sleep mode
	attachInterrupt(0, wakeUp, RISING);//attaching a interrupt to pin d2
	attachInterrupt(1, wakeUp, RISING);//attaching a interrupt to pin d2
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
	setLight(LOW);
	delay(1000); //wait a second to allow the led to be turned off before going to sleep
	sleep_cpu();//activating sleep mode
	Serial.println("just woke up!");//next line of code executed after the interrupt 
	setLight(HIGH);
	mils = millis();
}

void wakeUp() {
	Serial.println("Interrrupt Fired");//Print message to serial monitor
	sleep_disable();//Disable sleep mode
	detachInterrupt(0); //Removes the interrupt from pin 2;
	detachInterrupt(1); //Removes the interrupt from pin 2;
}
void sendData(RadioPacket data) {
	if (data.Pb1 != pb1LastStatus  || data.Pb2 != pb2LastStatus ) {
		
		
			if (_radio.send(DESTINATION_RADIO_ID, &data, sizeof(data), NRFLite::NO_ACK)) // Note how '&' must be placed in front of the variable name.
			{
				Blink(LedOff2,1);
				pb1LastStatus = data.Pb1;
				pb2LastStatus = data.Pb2;
			}
		
		
		mils = millis();
		//else
		//{
		//	//Blink(LedOff2);

		//}
		//digitalWrite(LedOff2, LOW);
	}


}

void setLight(int status) {
	digitalWrite(LedOn1, status);//turning LED off
	digitalWrite(LedOn3, status);//turning LED off
}
