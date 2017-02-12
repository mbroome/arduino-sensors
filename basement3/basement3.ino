//#define MY_RF24_CHANNEL  124

#define MY_RADIO_NRF24
#define MY_DEBUG    // Enables debug messages in the serial log
#define MY_REPEATER_NODE

#define MY_NODE_ID 8

#define ENABLE_TEMP_PROBE  1
#define ENABLE_RELAY_PROBE 1

////////////////////////////////////////////////////////
// define some pins
#define RELAY_1_PIN         4
#define RELAY_2_PIN         5
#define LED_PIN            13

////////////////////////////////////////////////////////
// define child ids
#define RELAY_1_ID               0
#define RELAY_2_ID               1
#define TEMPERATURE_FIRST_ID     10

////////////////////////////////////////////////////////
// define mappings


char TEMPERATURE_DESC[10][20] = {
  { "Rams" },
  { "Test Temperature1" },
  { "Test Temperature2" },
  { "Test Temperature3" },
  { "Test Temperature4" },
  { "Test Temperature5" },
  { "Test Temperature6" },
  { "Test Temperature7" },
  { "Test Temperature8" },
  { "Test Temperature9" },
};

char RELAY_DESC[10][20] = {
  { "Zone 4" },
  { "Zone 5" },
};

#define RELAYSTATE(x) ( x ? 0 : 1)

#include <MySensors.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 3 // Pin where dallase sensor is connected 

unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
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

MyMessage msgRelay(RELAY_1_ID, V_LIGHT);
int relayState[2] = {false, false};

bool initialValueSent = false;
bool initialValueRecv = false;

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

  pinMode(RELAY_1_PIN, OUTPUT); // make the clock pin an output
  digitalWrite(RELAY_1_PIN, relayState[RELAY_1_ID]);
  pinMode(RELAY_2_PIN, OUTPUT); // make the clock pin an output
  digitalWrite(RELAY_2_PIN, relayState[RELAY_2_ID]);

  sensors.setResolution(TEMP_12_BIT);
}

void presentation() {

  // Startup up the OneWire library
  sensors.begin();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Basement3", "1.0");

  // Present all sensors to controller
  if (ENABLE_TEMP_PROBE) {
    delay(200);
    connectedTempuratureProbes = sensors.getDeviceCount();
    for (int i = TEMPERATURE_FIRST_ID; i < TEMPERATURE_FIRST_ID + connectedTempuratureProbes; i++) {
      present(i, S_TEMP, TEMPERATURE_DESC[i - TEMPERATURE_FIRST_ID]);
      wait(500);
    }
  }

  if (ENABLE_RELAY_PROBE) {
     present(RELAY_1_ID, S_LIGHT, RELAY_DESC[RELAY_1_ID]);
     wait(500);
     present(RELAY_2_ID, S_LIGHT, RELAY_DESC[RELAY_2_ID]);
     wait(500);
  }
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
        //        if (lastTemperature[i] != temperature && temperature > -127.00 && temperature < 185.00) {
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
    if (ENABLE_RELAY_PROBE) {
      if (forceUpdate) {
          wait(300);
          send(msgRelay.setSensor(RELAY_1_ID).set(RELAYSTATE(relayState[RELAY_1_ID])));
          wait(300);
          send(msgRelay.setSensor(RELAY_2_ID).set(RELAYSTATE(relayState[RELAY_2_ID])));
      }
    }

  }
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_LIGHT) {
    int pin;
    int state = RELAYSTATE(message.getBool());

    if(message.sensor == RELAY_1_ID){
      pin = RELAY_1_PIN;
    }else if(message.sensor == RELAY_2_ID){
      pin = RELAY_2_PIN;
    }

    digitalWrite(pin, state);
    relayState[message.sensor] = state;
    send(msgRelay.setSensor(message.sensor).set(RELAYSTATE(relayState[message.sensor])));

  }
}


