// Visual Micro is in vMicro>General>Tutorial Mode
//
/*
    Name:       RemoteRelay.ino
    Created:	29.03.2019 09:23:17
    Author:     Konrad Paw�owski
*/

// Define User Types below here or use a .h file

#include <Bounce2.h>
#include <NRFLite.h>
#include <SPI.h>
#include <C:\Users\konra\source\repos\MySenesorsNetwork\RemoteRelay\Definiction.h>
//


// Define Function Prototypes that use User Types below here or use a .h file
//
#define Led 4
#define BUTTON_1 9
#define BUTTON_2 10
#define SSR_1 5
#define SSR_2 6


const static uint8_t RADIO_ID = 0;       // Our radio's id.  The transmitter will send to this id.
const static uint8_t PIN_RADIO_CE = 7;
const static uint8_t PIN_RADIO_CSN = 8;

// Define Functions below here or use other .ino or cpp files
//

NRFLite _radio;
RadioPacket _radioData;


Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
bool ssr1_state = false;
bool ssr2_state = false;

bool value1 = true;
bool value2 = true;
bool initialStart = true;
unsigned long tempTime;


void SetSSR1() {

  ssr1_state = !ssr1_state;
  SetState(SSR_1, ssr1_state);

  //Blink(Led, 1);

}

void SetSSR2() {

  ssr2_state = !ssr2_state;
  SetState(SSR_2, ssr2_state);
 
  //Blink(Led, 2);


}


// The setup() function runs once each time the micro-controller starts
void setup()
{
  Serial.begin(9600);
  pinMode(Led, OUTPUT);
  pinMode(SSR_1, OUTPUT);
  pinMode(SSR_2, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  debouncer1.attach(BUTTON_1);
  debouncer1.interval(5); // interval in ms

  debouncer2.attach(BUTTON_2);
  debouncer2.interval(5); // interval in ms

  SetState(SSR_1, false);
  SetState(SSR_2, false);

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
  {
    Serial.println("Cannot communicate with radio");
    while (1); // Wait here forever.
  }

}

// Add the main program code into the continuous loop() function
void loop()
{
//  GetTemp();

  // Update the Bounce instances :
  debouncer1.update();
  debouncer2.update();

  // Get the updated value :
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




  while (_radio.hasData())
  {
    _radio.readData(&_radioData); // Note how '&' must be placed in front of the variable name.

    if (_radioData.Pb1) {
      SetSSR1();
    }

    if (_radioData.Pb2) {
      SetSSR2();
    }

  }

}


