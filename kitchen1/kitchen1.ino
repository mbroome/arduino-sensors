#define MY_RADIO_NRF24
//#define MY_DEBUG    // Enables debug messages in the serial log
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/sensors/mysensors_id_list.h"

#define MY_NODE_ID KITCHEN1_NODE_ID


#include <MySensors.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 3 // Pin where dallase sensor is connected 

unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

float lastTemperature;

// Initialize temperature message
MyMessage msgTemperature(KITCHEN1_TEMP1, V_TEMP);

bool initialValueSent = false;
bool initialValueRecv = false;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Startup"));

  sensors.setResolution(TEMP_12_BIT);
}

void presentation() {

  // Startup up the OneWire library
  sensors.begin();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Kitchen1", "1.0");

  // Present all sensors to controller
  present(KITCHEN1_TEMP1, S_TEMP);
}


void loop()
{
  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures();

  // Read temperature
  float temperature = sensors.getTempFByIndex(0);

  /*  if (!initialValueSent) {
      Serial.println("Sending initial value");
      send(msgTemperature.setSensor(KITCHEN1_TEMP1).set(temperature, 2));
      Serial.println("Requesting initial value from controller");
      request(KITCHEN1_TEMP1, V_TEMP);
      wait(2000, C_SET, V_TEMP);
    } else {
      if (initialValueRecv) {
        // Serial.println("No longer trying to init");
  */
  // Only send data if temperature has changed and no error
  if (lastTemperature != temperature && temperature != -127.00) {
    Serial.println("sending different temp");
    Serial.println(temperature);
    // Send in the new temperature
    send(msgTemperature.setSensor(KITCHEN1_TEMP1).set(temperature, 2));
    // Save new temperatures for next compare
    lastTemperature = temperature;
  }
  //    }
  //  }
  sleep(SLEEP_TIME);
}

void receive(const MyMessage &message) {
  Serial.println("got something back");
  if (message.isAck()) {
    Serial.println("This is an ack from gateway");
  }
  Serial.println(message.getFloat());
  lastTemperature = (float)message.getFloat();
  if (!initialValueSent) {
    Serial.println("Receiving initial value from controller");
    Serial.println(lastTemperature);
    initialValueSent = true;
    initialValueRecv = true;
  }
}

