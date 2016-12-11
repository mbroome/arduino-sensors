#define MY_RF24_CE_PIN 8
#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_NODE

#include "C:/Users/mbroome/Documents/Arduino/sensors/mysensors_id_list.h"

#define MY_NODE_ID SALTWATER1_NODE_ID


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

const int buttonPin = 2;     // the number of the pushbutton pin
int buttonState = LOW;

const int ledWhite1 = 9;    // PWM connected to digital pin
const int ledWhite2 = 6;    // PWM connected to digital pin
const int ledBlue   = 5;    // PWM connected to digital pin
const int ledRed    = 3;    // PWM connected to digital pin

#define ONE_WIRE_BUS 4 // Pin where dallase sensor is connected 
unsigned long SLEEP_TIME = 3000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

boolean receivedConfig = false;
boolean metric = true;

float lastTemperature;

// Initialize temperature message
MyMessage msg(SALTWATER1_TEMP1, V_TEMP);

// Instantiate a Bounce object :
Bounce debouncer = Bounce();

unsigned long previousMillis = 0;

void blink() {
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  RTC.begin();

  pinMode(ledWhite1, OUTPUT);
  pinMode(ledWhite2, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(13, OUTPUT);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(buttonPin);
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
  present(SALTWATER1_TEMP1, S_TEMP);
}

void changeLight(int pin, int level) {
  analogWrite(pin, level);
}

void loop() {
  unsigned long currentMillis = millis();

  // Update the Bounce instance :
  debouncer.update();

  // Call code if Bounce fell (transition from HIGH to LOW) :
  if ( debouncer.fell() ) {
    // Toggle LED state :
    buttonState = !buttonState;
  }
  //Serial.println(buttonState);

  if (currentMillis - previousMillis >= SLEEP_TIME) {
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
      changeLight(ledWhite1, 60);
      changeLight(ledWhite2, 60);
      changeLight(ledBlue, 80);
      changeLight(ledRed, 30);
    } else {
      if (lightSchedule[now.hour()]) {
        Serial.print("Enable light");
        changeLight(ledWhite1, 60);
        changeLight(ledWhite2, 60);
        changeLight(ledBlue, 80);
        changeLight(ledRed, 30);
      } else {
        Serial.print("Disable light");
        changeLight(ledWhite1, 0);
        changeLight(ledWhite2, 0);
        changeLight(ledBlue, 0);
        changeLight(ledRed, 0);
      }
    }

    // Fetch temperatures from Dallas sensors
    sensors.requestTemperatures();
    // Read temperatures and send them to controller
    float temperature = sensors.getTempFByIndex(0);

    if (lastTemperature != temperature && temperature != -127.00) {
      Serial.println("sending different temp");
      Serial.println(temperature);
      // Send in the new temperature
      send(msg.setSensor(SALTWATER1_TEMP1).set(temperature, 2));
      // Save new temperatures for next compare
      lastTemperature = temperature;
    }

    //    blink();
  }


}


