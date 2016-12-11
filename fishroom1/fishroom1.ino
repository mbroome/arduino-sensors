//#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/sensors/mysensors_id_list.h"

#define MY_NODE_ID FISHROOM1_NODE_ID

#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>

#define FISHROOM1_HUM1 0
#define FISHROOM1_TEMP1 1
#define HUMIDITY_SENSOR_DIGITAL_PIN 3
unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)

DHT dht;
float lastTemp;
float lastHum;
boolean metric = true;
MyMessage msgHum(FISHROOM1_HUM1, V_HUM);
MyMessage msgTemp(FISHROOM1_TEMP1, V_TEMP);

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
  present(FISHROOM1_HUM1, S_HUM);
  present(FISHROOM1_TEMP1, S_TEMP);
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  // Fetch temperatures from DHT sensor
  float temperature = dht.getTemperature();
  temperature = dht.toFahrenheit(temperature);
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
   Serial.println(temperature);
    send(msgTemp.set(temperature, 1));
    lastTemp = temperature;
  }

  // Fetch humidity from DHT sensor
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
   Serial.println(humidity);
    send(msgHum.set(humidity, 1));
    lastHum = humidity;
  }

  sleep(SLEEP_TIME); //sleep a bit
}

