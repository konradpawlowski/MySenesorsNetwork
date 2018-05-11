/*
 Name:		OutsideSensors.ino
 Created:	4/15/2018 5:31:53 PM
 Author:	Konrad
*/

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600

// Enable this if RS485 is connected to a hardware serial port
//#define MY_RS485_HWSERIAL Serial1

#include <MySensors.h>
// the setup function runs once when you press reset or power the board
void setup() {

}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
