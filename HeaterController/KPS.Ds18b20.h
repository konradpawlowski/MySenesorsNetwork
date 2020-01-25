// KPS.Ds18b20.h

#ifndef _KPS.DS18B20_h
#define _KPS.DS18B20_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class KPS.Ds18b20Class
{
 protected:


 public:
	void init();
};

extern KPS.Ds18b20Class KPS.Ds18b20;

#endif

