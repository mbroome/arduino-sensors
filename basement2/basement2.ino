#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_NODE

#define MY_NODE_ID 7

#define ENABLE_TEMP_PROBE  1
#define ENABLE_RELAY_PROBE 1

////////////////////////////////////////////////////////
// define some pins
#define LED_PIN            13

////////////////////////////////////////////////////////
// define child ids
#define TEMPERATURE_FIRST_ID     10

////////////////////////////////////////////////////////
// define mappings


char TEMPERATURE_DESC[10][20] = {
  { "Ghost & Gold Temp" },
  { "Discus Temp" },
  { "Koi2 Temp" },
  { "Blacks Temp" },
  { "Test Temperature4" },
  { "Test Temperature5" },
  { "Test Temperature6" },
  { "Test Temperature7" },
  { "Test Temperature8" },
  { "Test Temperature9" },
};


////////////////////////////////////////////////////////
// define some pins
#define HUMIDITY_SENSOR_DIGITAL_PIN 3


////////////////////////////////////////////////////////
// define child ids
#define FISHROOM1_HUM1_ID  0
#define FISHROOM1_TEMP1_ID 9


////////////////////////////////////////////////////////
// Actually do the work...
#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 4 // Pin where dallase sensor is connected 

unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

#define MAX_ATTACHED_TEMP_SENSORS 10
float lastTemperature[MAX_ATTACHED_TEMP_SENSORS];

unsigned long previousMillis = 0;
unsigned long nextForceMillis = 0;

bool initialSend = false;

int connectedTempuratureProbes = 0;

// Initialize messages
MyMessage msgTemperature(TEMPERATURE_FIRST_ID, V_TEMP);

DHT dht;


float lastTemp;
float lastHum;
boolean metric = true;
MyMessage msgHum(FISHROOM1_HUM1_ID, V_HUM);
MyMessage msgTemp(FISHROOM1_TEMP1_ID, V_TEMP);

bool offsetDiff(float a, float b, float offset) {
  Serial.print("a: ");
  Serial.print(a);
  Serial.print(" b: ");
  Serial.print(b);
  Serial.print(" diff: ");
  float x = abs(a - b);
  Serial.println(x);
  if (x > offset) {
    return (true);
  } else {
    return (false);
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Startup");

  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);

}

void presentation()
{
  // Startup up the OneWire library
  sensors.begin();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Send the Sketch Version Information to the Gateway
  sendSketchInfo("Basement2", "1.0");

  // Present all sensors to controller
  if (ENABLE_TEMP_PROBE) {
    delay(200);
    connectedTempuratureProbes = sensors.getDeviceCount();
    for (int i = TEMPERATURE_FIRST_ID; i < TEMPERATURE_FIRST_ID + connectedTempuratureProbes; i++) {
      present(i, S_TEMP, TEMPERATURE_DESC[i - TEMPERATURE_FIRST_ID]);
      wait(500);
    }
  }

  // Register all sensors to gw (they will be created as child devices)
  present(FISHROOM1_HUM1_ID, S_HUM, "FishRoom Humidity");
  wait(500);
  present(FISHROOM1_TEMP1_ID, S_TEMP, "FishRoom Temperature");
  wait(500);
}

void loop()
{
  bool forceUpdate = false;

  // wait at least SLEEP_TIME until we poll again
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= SLEEP_TIME) {
    // force the update if we go over a long time
    if (currentMillis > nextForceMillis) {
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

    if (ENABLE_TEMP_PROBE) {
      // Fetch temperatures from Dallas sensors
      sensors.requestTemperatures();
      Serial.print("connected probes: ");
      Serial.println(connectedTempuratureProbes);

      // Read temperature
      for (int i = 0; i < connectedTempuratureProbes; i++) {
        float temperature = sensors.getTempFByIndex(i);
        Serial.print("temp probe: ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(temperature);

        // Only send data if temperature has changed and no error
        if ((temperature > -127.00 && temperature < 185.00) && (forceUpdate || offsetDiff(lastTemperature[i], temperature, 0.20))) {
          Serial.print("sending different temp: ");
          Serial.println(temperature);
          // Send in the new temperature
          send(msgTemperature.setSensor(i + TEMPERATURE_FIRST_ID).set(temperature, 2));
          // Save new temperatures for next compare
          lastTemperature[i] = temperature;
        } else {
          Serial.print("not sending temp: ");
          Serial.println(temperature);
        }
      }
    }

  }
}

