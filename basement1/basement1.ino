#define MY_RADIO_NRF24
#define MY_DEBUG    // Enables debug messages in the serial log
//#define MY_REPEATER_NODE

#define MY_NODE_ID 6

#define ENABLE_TEMP_PROBE  1
#define ENABLE_RELAY_PROBE 1
#define ENABLE_FLOAT_PROBE 1

////////////////////////////////////////////////////////
// define some pins
#define FLOAT_SWITCH1_PIN        14
#define FLOAT_SWITCH2_PIN        15

#define LED_PIN                  13

////////////////////////////////////////////////////////
// define child ids
#define RELAY_FIRST_ID        0
#define TEMPERATURE_FIRST_ID 10

#define FLOAT_SWITCH1_ID     30
#define FLOAT_SWITCH2_ID     31

char TEMPERATURE_DESC[10][20] = {
  { "Test Temperature1" },
  { "Holding Tank" },
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
  { "Fish Room Lights" },
  { "Zone 1" },
  { "Zone 2" },
  { "Zone 3" },
  { "Unused" },
  { "Water Fill" },
  { "Pump" },
  { "Holding Heaters" },
};

////////////////////////////////////////////////////////
// Actually do the work...
#include <MySensors.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Bounce2.h>

#define SHIFT_REGISTER_DATA_PIN   4
#define SHIFT_REGISTER_CLOCK_PIN  5

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

// Instantiate a Bounce object
Bounce debouncer1 = Bounce();

// Instantiate another Bounce object
Bounce debouncer2 = Bounce();


// Initialize temperature message
MyMessage msgTemperature(TEMPERATURE_FIRST_ID, V_TEMP);

MyMessage msgFloatSwitch1(FLOAT_SWITCH1_ID, V_STATUS);
MyMessage msgFloatSwitch2(FLOAT_SWITCH2_ID, V_STATUS);

MyMessage msgRelay(RELAY_FIRST_ID, V_LIGHT);

bool initialValueSent = false;
bool initialValueRecv = false;

byte relayState = B11111111;

bool offsetDiff(float a, float b, float offset){
   Serial.print("a: ");
   Serial.print(a);
   Serial.print(" b: ");
   Serial.print(b);
   Serial.print(" diff: ");
   float x = abs(a - b);
   Serial.println(x);
   if(x > offset){
      return(true);
   }else{
      return(false);
   }
}
void before() {
  pinMode(SHIFT_REGISTER_DATA_PIN, OUTPUT); // make the data pin an output
  pinMode(SHIFT_REGISTER_CLOCK_PIN, OUTPUT); // make the clock pin an output

  // start with all of the relays off
  shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState);

}
void setup()
{
  Serial.begin(115200);
  Serial.println("Startup");

  // setup the float switches
  pinMode(FLOAT_SWITCH1_PIN, INPUT_PULLUP);
  pinMode(FLOAT_SWITCH2_PIN, INPUT_PULLUP);

  debouncer1.attach(FLOAT_SWITCH1_PIN);
  debouncer1.interval(5); // interval in ms

  debouncer2.attach(FLOAT_SWITCH2_PIN);
  debouncer2.interval(5); // interval in ms

  sensors.setResolution(TEMP_12_BIT);

  pinMode(LED_PIN, OUTPUT);
}

void presentation() {

  // Startup up the OneWire library
  sensors.begin();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Basement1", "1.0", false);
  wait(500);

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
    for (int i = 0; i < 8; i++) {
      present(i, S_LIGHT, RELAY_DESC[i]);
      wait(500);

//      // tell the controller the current state
//      send(msgRelay.setSensor(i).set(1));
//      wait(500);
    }
  }

  if (ENABLE_FLOAT_PROBE) {
    present(FLOAT_SWITCH1_ID, S_BINARY, "float1");
    wait(500);
    send(msgFloatSwitch1.set(0));
    present(FLOAT_SWITCH2_ID, S_BINARY, "float2");
    wait(500);
    send(msgFloatSwitch1.set(0));
    wait(500);
  }
}


void loop()
{
  bool forceUpdate = false;

  // Update the Bounce instances :
  debouncer1.update();
  debouncer2.update();

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

    if (ENABLE_FLOAT_PROBE) {
      // Get the updated value :
      int floatState1 = debouncer1.read();
      int floatState2 = debouncer2.read();

      Serial.print("float 1:");
      Serial.println(floatState1);
      Serial.print("float 2:");
      Serial.println(floatState2);

      if (forceUpdate || (lastFloatState[0] != floatState1)) {
        Serial.print("float state send: 1 => ");
        Serial.println(floatState1);

        send(msgFloatSwitch1.set(floatState1));
        lastFloatState[0] = floatState1;
      }
      if (forceUpdate || (lastFloatState[1] != floatState2)) {
        Serial.print("float state send: 2 => ");
        Serial.println(floatState2);

        send(msgFloatSwitch2.set(floatState2));
        lastFloatState[1] = floatState2;
      }
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
        for (int i = 0; i < 8; i++) {
          bool s = bitRead(relayState, i);

          wait(300);
          send(msgRelay.setSensor(i).set(s));
        }
      }
    }
    initialSend = true;
  }

}


void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_LIGHT) {
    int bit2flip = message.sensor;
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

    bitWrite(relayState, bit2flip, state);
    shiftOut(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, LSBFIRST, relayState); // send this binary value to the shift register
  }
}


