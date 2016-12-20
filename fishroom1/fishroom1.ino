#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/arduino-sensors/mysensors_id_list.h"

#define MY_NODE_ID FISHROOM1_NODE_ID


////////////////////////////////////////////////////////
// define some pins
#define HUMIDITY_SENSOR_DIGITAL_PIN 3


////////////////////////////////////////////////////////
// define child ids
#define FISHROOM1_HUM1_ID  0
#define FISHROOM1_TEMP1_ID 10


////////////////////////////////////////////////////////
// Actually do the work...
#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>

unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)

DHT dht;

unsigned long previousMillis = 0;
unsigned long nextForceMillis = 0;


float lastTemp;
float lastHum;
boolean metric = true;
MyMessage msgHum(FISHROOM1_HUM1_ID, V_HUM);
MyMessage msgTemp(FISHROOM1_TEMP1_ID, V_TEMP);

void setup()
{
  Serial.begin(115200);
  Serial.println("Startup");

  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);

}

void presentation()
{
  // Send the Sketch Version Information to the Gateway
  sendSketchInfo("FishRoom1", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(FISHROOM1_HUM1_ID, S_HUM);
  wait(500);
  present(FISHROOM1_TEMP1_ID, S_TEMP);
  wait(500);
}

void loop()
{
   bool forceUpdate = false;
   
  // wait at least SLEEP_TIME until we poll again
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= SLEEP_TIME) {
   // force the update if we go over a long time
   if(currentMillis > nextForceMillis){
      nextForceMillis = currentMillis + 300000;
      Serial.println("### need to force update");
      forceUpdate = true;
   }
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    delay(dht.getMinimumSamplingPeriod());

    // Fetch temperatures from DHT sensor
    float temperature = dht.getTemperature();
    temperature = dht.toFahrenheit(temperature);
    if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
    } else if ((temperature > -127.00 && temperature < 185.00) && (forceUpdate || offsetDiff(temperature, lastTemp, 0.20))) {
      Serial.println(temperature);
      send(msgTemp.set(temperature, 2));
      lastTemp = temperature;
    }

    // Fetch humidity from DHT sensor
    float humidity = dht.getHumidity();
    if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
    } else if (forceUpdate || offsetDiff(humidity, lastHum, 0.50)) {
      Serial.println(humidity);
      send(msgHum.set(humidity, 2));
      lastHum = humidity;
    }

  }
}

