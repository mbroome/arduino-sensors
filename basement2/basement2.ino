#define MY_RADIO_NRF24
#define MY_DEBUG    // Enables debug messages in the serial log
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/sensors/mysensors_id_list.h"

#define MY_NODE_ID BASEMENT2_NODE_ID


#include <MySensors.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 3 // Pin where dallase sensor is connected 

unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

float lastTemperature;
unsigned long previousMillis = 0;
bool initialSend = false;

// Initialize temperature message
MyMessage msgTemperature(BASEMENT2_TEMP1, V_TEMP);

bool initialValueSent = false;
bool initialValueRecv = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("Startup");

  sensors.setResolution(TEMP_12_BIT);
}

void presentation() {

  // Startup up the OneWire library
  sensors.begin();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Basement2", "1.0");

  // Present all sensors to controller
  present(BASEMENT2_TEMP1, S_TEMP);
}


void loop()
{
  // wait at least SLEEP_TIME until we poll again
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= SLEEP_TIME) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // Fetch temperatures from Dallas sensors
    sensors.requestTemperatures();

    // Read temperature
    float temperature = sensors.getTempFByIndex(0);

    // Only send data if temperature has changed and no error
    if (lastTemperature != temperature && temperature > -127.00 && temperature < 185.00) {
      Serial.print("sending different temp: ");
      Serial.println(temperature);
      // Send in the new temperature
      send(msgTemperature.setSensor(BASEMENT2_TEMP1).set(temperature, 2));
      // Save new temperatures for next compare
      lastTemperature = temperature;
    } else {
      Serial.print("not sending temp: ");
      Serial.println(temperature);
    }
  }
}



