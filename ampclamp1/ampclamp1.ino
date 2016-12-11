#define MY_RADIO_NRF24
//#define MY_DEBUG    // Enables debug messages in the serial log
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/sensors/mysensors_id_list.h"

#define MY_NODE_ID AMPCLAMP1_NODE_ID

#include <SPI.h>
#include <MySensors.h>  
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
EnergyMonitor emon2;                   // Create an instance

#define AMPCLAMP1_CLAMP1 6
#define AMPCLAMP1_CLAMP2 7

unsigned long lastSend;
unsigned long SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
bool initialSend = false;

MyMessage msgClamp1(AMPCLAMP1_CLAMP1, V_WATT);
MyMessage msgClamp2(AMPCLAMP1_CLAMP2, V_WATT);

void setup()
{  
  Serial.begin(115200);
  Serial.println("Startup");

  emon1.current(0, 32.1);             // Current: input pin, calibration.
  emon2.current(1, 32.1);             // Current: input pin, calibration.
}

void presentation()  
{ 
  // Send the Sketch Version Information to the Gateway
  sendSketchInfo("AmpClamp1", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(AMPCLAMP1_CLAMP1, S_POWER);
  present(AMPCLAMP1_CLAMP2, S_POWER);
}

void loop()
{
  unsigned long now = millis();
  bool sendTime = now - lastSend > SEND_FREQUENCY;

  double Irms1 = emon1.calcIrms(1480);  // Calculate Irms only
  double Irms2 = emon2.calcIrms(1480);  // Calculate Irms only
  
  if (sendTime || !initialSend) {
    send(msgClamp1.set(Irms1*115.0, 1));
    send(msgClamp2.set(Irms2*115.0, 1));
    lastSend = now;
    initialSend = true;
  }
}

