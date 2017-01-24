#define MY_RADIO_NRF24
#define MY_DEBUG    // Enables debug messages in the serial log
#define MY_REPEATER_NODE

#define MY_NODE_ID 7

#define ENABLE_TEMP_PROBE  1
#define ENABLE_RELAY_PROBE 1

////////////////////////////////////////////////////////
// define some pins
#define RELAY1_PIN         4
#define RELAY2_PIN         5

#define LED_PIN            13

////////////////////////////////////////////////////////
// define child ids
#define RELAY_ZONE5_ID        0
#define TEMPERATURE_FIRST_ID     10

////////////////////////////////////////////////////////
// define mappings
char RELAY_DESC[2][20] = {
  { "Zone 4" },
  { "Zone 5" },
};

int RELAY_PINS[2] = {
  4,
  5
};

char TEMPERATURE_DESC[10][20] = {
  { "Rams Temperature" },
};

#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay


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

int lastFloatState[2];
unsigned long previousMillis = 0;
unsigned long nextForceMillis = 0;

bool initialSend = false;

int connectedTempuratureProbes = 0;

// Initialize messages
MyMessage msgTemperature(TEMPERATURE_FIRST_ID, V_TEMP);
MyMessage msgRelay(RELAY_ZONE5_ID, V_LIGHT);

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

void before() {
  pinMode(RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, RELAY_OFF);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY2_PIN, RELAY_OFF);

}
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
  if (ENABLE_TEMP_PROBE) {
    delay(200);
    connectedTempuratureProbes = sensors.getDeviceCount();
    for (int i = TEMPERATURE_FIRST_ID; i < TEMPERATURE_FIRST_ID + connectedTempuratureProbes; i++) {
      present(i, S_TEMP, TEMPERATURE_DESC[i - TEMPERATURE_FIRST_ID]);
      wait(500);
    }
  }

  // now we do all of the relays
  if (ENABLE_RELAY_PROBE) {
    for (int i = 0; i < 2; i++) {
      present(i, S_LIGHT, RELAY_DESC[i]);
      wait(500);

      // tell the controller the current state
      //send(msgRelay.setSensor(i).set(1));
      wait(500);
    }
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

  }
}


void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_LIGHT) {
    int bit2flip = RELAY_PINS[message.sensor];
    int state = message.getBool() ? 0 : 1;

    // Write some debug info
    Serial.print("message type: ");
    Serial.println(message.type);
    Serial.print("Incoming change for sensor:");
    Serial.println(message.sensor);
    Serial.print(" from node:");
    Serial.println(message.sender);
    Serial.print("New status: ");
    Serial.println(message.getBool());
    Serial.print(" bit to flip: ");
    Serial.print(bit2flip);
    Serial.print(" state change: ");
    Serial.println(state);

    //bitWrite(relayState, bit2flip, state);
    //shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
    digitalWrite(bit2flip, state);
  }
}


