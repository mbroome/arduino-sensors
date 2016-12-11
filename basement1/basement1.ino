#define MY_RADIO_NRF24
#define MY_DEBUG    // Enables debug messages in the serial log
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/sensors/mysensors_id_list.h"

#define MY_NODE_ID BASEMENT1_NODE_ID

#define ENABLE_TEMP_PROBE  0
#define ENABLE_AMP_PROBE   0
#define ENABLE_RELAY_PROBE 1

#include <MySensors.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
EnergyMonitor emon2;                   // Create an instance

#define SHIFT_REGISTER_DATA_PIN   4
#define SHIFT_REGISTER_CLOCK_PIN  5
#define AMPCLAMP1_CLAMP1_PIN      6
#define AMPCLAMP1_CLAMP2_PIN      7

#define ONE_WIRE_BUS 3 // Pin where dallase sensor is connected 

unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

#define MAX_ATTACHED_TEMP_SENSORS 10

float lastTemperature[MAX_ATTACHED_TEMP_SENSORS];
unsigned long previousMillis = 0;
bool initialSend = false;

// Initialize temperature message
MyMessage msgTemperature(BASEMENT1_TEMP1, V_TEMP);

MyMessage msgClamp1(BASEMENT1_CLAMP1, V_WATT);
MyMessage msgClamp2(BASEMENT1_CLAMP2, V_WATT);

MyMessage msgRelay0(BASEMENT1_RELAY0, V_STATUS);
MyMessage msgRelay1(BASEMENT1_RELAY1, V_STATUS);
MyMessage msgRelay2(BASEMENT1_RELAY2, V_STATUS);
MyMessage msgRelay3(BASEMENT1_RELAY3, V_STATUS);
MyMessage msgRelay4(BASEMENT1_RELAY4, V_STATUS);
MyMessage msgRelay5(BASEMENT1_RELAY5, V_STATUS);
MyMessage msgRelay6(BASEMENT1_RELAY6, V_STATUS);
MyMessage msgRelay7(BASEMENT1_RELAY7, V_STATUS);


bool initialValueSent = false;
bool initialValueRecv = false;

byte relayState = B11111111;

void setup()
{
  Serial.begin(115200);
  Serial.println("Startup");

  sensors.setResolution(TEMP_12_BIT);

  emon1.current(AMPCLAMP1_CLAMP1_PIN, 32.1);             // Current: input pin, calibration.
  emon2.current(AMPCLAMP1_CLAMP2_PIN, 32.1);             // Current: input pin, calibration.

  pinMode(SHIFT_REGISTER_DATA_PIN, OUTPUT); // make the data pin an output
  pinMode(SHIFT_REGISTER_CLOCK_PIN, OUTPUT); // make the clock pin an output

  // start with all of the relays off
  shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState);

  /*
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
    delay(1000);
    bitWrite(relayState, 3, LOW);
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
    delay(1000);
    bitWrite(relayState, 2, LOW);
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
    delay(1000);
    bitWrite(relayState, 1, LOW);
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
    delay(1000);
    bitWrite(relayState, 3, HIGH);
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
    delay(1000);
    bitWrite(relayState, 2, HIGH);
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
  */
}

void presentation() {

  // Startup up the OneWire library
  sensors.begin();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Basement1", "1.0");

  // Present all sensors to controller
  for (int i = BASEMENT1_TEMP1; i < BASEMENT1_TEMP1 + MAX_ATTACHED_TEMP_SENSORS; i++) {
    present(i, S_TEMP);
  }

  // Register all sensors to gw (they will be created as child devices)
  present(BASEMENT1_CLAMP1, S_POWER);
  present(BASEMENT1_CLAMP2, S_POWER);

  // now we do all of the relays
  present(BASEMENT1_RELAY0, S_BINARY, "relay0");
  present(BASEMENT1_RELAY1, S_BINARY, "relay1");
  present(BASEMENT1_RELAY2, S_BINARY, "relay2");
  present(BASEMENT1_RELAY3, S_BINARY);
  present(BASEMENT1_RELAY4, S_BINARY);
  present(BASEMENT1_RELAY5, S_BINARY);
  present(BASEMENT1_RELAY6, S_BINARY);
  present(BASEMENT1_RELAY7, S_BINARY);

  send(msgRelay0.set(0));
  send(msgRelay1.set(0));
  send(msgRelay2.set(0));
  send(msgRelay3.set(0));
  send(msgRelay4.set(0));
  send(msgRelay5.set(0));
  send(msgRelay6.set(0));
  send(msgRelay7.set(0));

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
    for (int i = BASEMENT1_TEMP1; i < BASEMENT1_TEMP1 + MAX_ATTACHED_TEMP_SENSORS; i++) {
      float temperature = sensors.getTempFByIndex(i);

      // Only send data if temperature has changed and no error
      if (lastTemperature[i] != temperature && temperature > -127.00 && temperature < 185.00) {
        if (ENABLE_TEMP_PROBE) {
          Serial.print("sending different temp: ");
          Serial.println(temperature);
          // Send in the new temperature
          send(msgTemperature.setSensor(i).set(temperature, 2));
        }
        // Save new temperatures for next compare
        lastTemperature[i] = temperature;
      } else {
        Serial.print("not sending temp: ");
        Serial.println(temperature);
      }
    }
    // collect watts
    double Irms1 = emon1.calcIrms(1480);  // Calculate Irms only
    double Irms2 = emon2.calcIrms(1480);  // Calculate Irms only

    double watts1 = Irms1 * 115.0;
    double watts2 = Irms2 * 115.0;

    if (initialSend) {
      Serial.print("watts 1: ");
      Serial.println(watts1);
      Serial.print("watts 2: ");
      Serial.println(watts2);

      // and send the data
      if (ENABLE_AMP_PROBE) {
        send(msgClamp1.set(watts1, 1));
        send(msgClamp2.set(watts2, 1));
      }
    }
    initialSend = true;
  }

}


void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    /*
        switch (message.sensor) {
          case 1:
            stateA = message.getBool();
            digitalWrite(message.sensor + 4, stateA ? RELAY_ON : RELAY_OFF);

            break;
          case 2:
            stateB = message.getBool();
            digitalWrite(message.sensor + 4, stateB ? RELAY_ON : RELAY_OFF);

            break;

        }
    */
    int bit2flip = message.sensor - 40;

    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.println(message.sensor);
    Serial.print("from node:");
    Serial.println(message.sender);
    Serial.print("New status: ");
    Serial.println(message.getBool());
    Serial.print("bit to flip: ");
    Serial.println(bit2flip);
  }
}


