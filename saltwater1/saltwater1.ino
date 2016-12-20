#define MY_RF24_CE_PIN 8
#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/arduino-sensors/mysensors_id_list.h"

#define MY_NODE_ID SALTWATER1_NODE_ID

////////////////////////////////////////////////////////
// define some pins
const int BUTTON_PIN = 2;     // the number of the pushbutton pin
const int LED_WHITE1_PIN = 9;    // PWM connected to digital pin
const int LED_WHITE2_PIN = 6;    // PWM connected to digital pin
const int LED_BLUE_PIN   = 5;    // PWM connected to digital pin
const int LED_RED_PIN    = 3;    // PWM connected to digital pin


////////////////////////////////////////////////////////
// define child ids
#define SALTWATER1_TEMP1_ID 10

////////////////////////////////////////////////////////
// Actually do the work...
#include <MySensors.h>
#include <SPI.h>
#include <Bounce2.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;


bool lightSchedule[] = {
  false,
  false, // 1am
  false, // 2am
  false, // 3am
  false, // 4am
  true, // 5am
  true, // 6am
  false, // 7am
  false, // 8am
  false, // 9am
  false, // 10am
  false, // 11am
  false, // 12am
  false, // 1pm
  false, // 2pm
  false, // 3pm
  true, // 4pm
  true, // 5pm
  true, // 6pm
  true, // 7pm
  true, // 8pm
  true, // 9pm
  false, // 10pm
  false, // 11pm
  false, // 12pm
};

int buttonState = LOW;


#define ONE_WIRE_BUS 4 // Pin where dallase sensor is connected 
unsigned long SLEEP_TIME = 3000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

unsigned long previousMillis = 0;
unsigned long nextForceMillis = 0;

float lastTemperature;

// Initialize temperature message
MyMessage msg(SALTWATER1_TEMP1_ID, V_TEMP);

// Instantiate a Bounce object :
Bounce debouncer = Bounce();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  RTC.begin();

  pinMode(LED_WHITE1_PIN, OUTPUT);
  pinMode(LED_WHITE2_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(13, OUTPUT);

  // initialize the pushbutton pin as an input:
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  sensors.setResolution(TEMP_12_BIT); // Genauigkeit auf 12-Bit setzen

}

void presentation() {

  // Startup up the OneWire library
  sensors.begin();
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // Startup and initialize MySensors library. Set callback for incoming messages.

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Saltwater1", "1.0");

  // Present all sensors to controller
  present(SALTWATER1_TEMP1_ID, S_TEMP);
}

void changeLight(int pin, int level) {
  analogWrite(pin, level);
}

void loop() {
     bool forceUpdate = false;

  // Update the Bounce instance :
  debouncer.update();

  // Call code if Bounce fell (transition from HIGH to LOW) :
  if ( debouncer.fell() ) {
    // Toggle LED state :
    buttonState = !buttonState;
  }
  //Serial.println(buttonState);

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

    
    DateTime now = RTC.now();// Getting the current Time and storing it into a DateTime object
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    //Serial.print(lightSchedule[now.hour()]);
    //Serial.println();

    if (buttonState) {
      Serial.print("Enable light because of button");
      changeLight(LED_WHITE1_PIN, 60);
      changeLight(LED_WHITE2_PIN, 60);
      changeLight(LED_BLUE_PIN, 80);
      changeLight(LED_RED_PIN, 30);
    } else {
      if (lightSchedule[now.hour()]) {
        Serial.print("Enable light");
        changeLight(LED_WHITE1_PIN, 60);
        changeLight(LED_WHITE2_PIN, 60);
        changeLight(LED_BLUE_PIN, 80);
        changeLight(LED_RED_PIN, 30);
      } else {
        Serial.print("Disable light");
        changeLight(LED_WHITE1_PIN, 0);
        changeLight(LED_WHITE2_PIN, 0);
        changeLight(LED_BLUE_PIN, 0);
        changeLight(LED_RED_PIN, 0);
      }
    }

    // Fetch temperatures from Dallas sensors
    sensors.requestTemperatures();
    // Read temperatures and send them to controller
    float temperature = sensors.getTempFByIndex(0);

    if ((temperature > -127.00 && temperature < 185.00) && (forceUpdate || offsetDiff(temperature, lastTemperature, 0.20))) {
      Serial.println("sending different temp");
      Serial.println(temperature);
      // Send in the new temperature
      send(msg.setSensor(SALTWATER1_TEMP1_ID).set(temperature, 2));
      // Save new temperatures for next compare
      lastTemperature = temperature;
    }

  }

}


