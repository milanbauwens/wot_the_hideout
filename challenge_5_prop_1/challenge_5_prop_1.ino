#include "ArduinoProps.h"
#include <Wire.h>
#include "rgb_lcd.h"

/* ---------------
 * Configure your WiFi Prop here
 * --------------- */

WifiProp prop(u8"challenge_5_prop_1", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/challenge_5_prop_1/inbox",
                  u8"Room/My room/Props/challenge_5_prop_1/outbox",
                  "192.168.50.101", // your MQTT server IP address
                  1883); // your MQTT server port;

/* ---------------
 * Define the variables (internal and Xcape.io variables)
 * --------------- */

PropDataLogical ventIsOpen(u8"Vent is open");
PropDataLogical isReset(u8"isReset");
PropDataText rssi(u8"rssi");

void manageTempSlider(); // define upcoming manageTempSlider method
PropAction tempSlider = PropAction(0, manageTempSlider);

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphrase of the network
const int propNumber = 162;

// temp slider variables
bool lightsOn = false;
rgb_lcd lcd;
int adcPin = A0; // select the input pin for the potentiometer
int adcPin2 = A1; // select the input pin for potentiometer 2
int temp1 = 0;
int temp2 = 0;

const int ledPin = 3;      // the number of the LED pin, D3
const int buttonPin = 4;    // the number of the pushbutton pin, D4
const boolean breathMode = true;
int ledState = LOW;  


const int colorR = 255;
const int colorG = 255;
const int colorB = 255;

int ledFadeValue = 0;
int ledFadeStep = 5;
int ledFadeInterval = 20;   //milliseconds
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH; 
 
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long lastLedFadeTime = 0;

unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;

const long interval = 1000;
const long interval2 = 100;


/* ---------------
 * Sketch Helpper Functions
 * --------------- */

/**
 * Writes a header in the serial output
 */
void writeHeader(String title) {
  Serial.println("--------");
  Serial.println(title);
  Serial.println("--------");
  Serial.println("");
}

/* ---------------
 * Do the setup process
 * --------------- */

void setup()
{
  Serial.begin(9600);

  Serial.println("Starting temp slider program!");

  prop.addData(&ventIsOpen);
  prop.addData(&isReset);
  prop.addData(&rssi);

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages

  // At this point, the broker is not connected yet

  // temp slider
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setCursor(5,0);
  lcd.setRGB(colorR,colorG,colorB);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, ledState);
  
  // Print a message to the LCD.
  pinMode(adcPin, INPUT); // set ledPin to OUTPUT

}

/* ---------------
 * Start the Arduino loop process
 * --------------- */

/**
 * Validate the Wifi (do this in every loop)
 */
bool validateWifi() {
  // if the Wifi was not initialized yet
  if (!wifiBegun) {
    writeHeader("WiFi information");
    WiFi.begin(ssid, passphrase); // start connecting
    Serial.print("WiFiNNA firmware version: \t");
    Serial.println(WiFi.firmwareVersion());
    delay(250); // acceptable freeze for this prop (otherwise use PropAction for async-like behavior)
    // do static IP configuration disabling the dhcp client, must be called after every WiFi.begin()

  // uncomment this, to set the static IP address
    WiFi.config(IPAddress(192, 168, 50, propNumber), // local_ip
    IPAddress(192, 168, 50, 221),  // dns_server (sets to default gateway)
    IPAddress(192, 168, 50, 221),  // gateway
    IPAddress(255, 255, 255, 0)); // subnet

    // if the Wifi is in connected state
    if (WiFi.status() == WL_CONNECTED) {
      wifiBegun = true;
      Serial.print("Your local IP address: \t\t");
      Serial.println(WiFi.localIP());
      Serial.print("Your subnet mask: \t\t");
      Serial.println(WiFi.subnetMask());
      Serial.print("Gateway IP address: \t\t");
      Serial.println(WiFi.gatewayIP());
    } else {
      WiFi.end();
    }
  } else if (wifiBegun && WiFi.status() != WL_CONNECTED) { 
    WiFi.end();
    wifiBegun = false;
  }
}

void loop()
{
  validateWifi(); // validate the Wifi connection
  prop.loop(); // start looping the prop (superpowers gottin from xcape.io)
  rssi.setValue(WiFi.RSSI() + String(" dBm")); // https://www.metageek.com/training/resources/understanding-rssi.html

  if (lightsOn) {
    isReset.setValue(false);
    prop.sendAllData();
    tempSlider.check();
  }
}

/* ---------------
 * The Xcape.io logic/code
 * --------------- */

void manageTempSlider() {
  // Turn off the display:
  // Turn on the display:


  temp1 = floor(analogRead(adcPin) /33.8);
  temp2 = floor(analogRead(adcPin2) /10.3);

  #define MY_SIZE 17
  int temp2Options[MY_SIZE] ={0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96};
  unsigned long currentMillis2 = millis();

  int i = 0;
  for(int j = 1; j < MY_SIZE +1; j++){
    if(temp2Options[i] <= temp2 && temp2 < temp2Options[j]) {
    temp2 = temp2Options[i];
      if( currentMillis2 - previousMillis2 >= interval2){
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print(String(temp1) + "," + String(temp2) + " *C");
        previousMillis2 = currentMillis2;          
      }
    } else if(temp2 > 98){
      temp2 = 99;
    }
    i++;
  }

  if(currentMillis2 - previousMillis2 >= interval2){
    if( temp1 != floor(analogRead(adcPin)/33.8)){
     lcd.clear();
     lcd.setCursor(5,0);
     lcd.print(String(temp1) + "," + String(temp2) + " *C");
   }
   previousMillis2 = currentMillis2;   
  }

  if (temp1 <= 13){
    lcd.setRGB(135,206,250);
  }
  else if(temp1 <= 23){
    lcd.setRGB(150,150,0);
  }
  else{
    lcd.setRGB(240,10,10);
  }

  
 
  // If the switch changed, due to noise or pressing:
  
  lastDebounceTime = millis();
  unsigned long currentMillis = millis();

  if( currentMillis - previousMillis >= interval){
    if (temp1 == 19 && temp2 == 84) {
      // reset the debouncing timer
      if(digitalRead(buttonPin) == 0){
        prop.sendOver("Tempslider");
        Serial.println("de vent is open");
        ventIsOpen.setValue(true);
        previousMillis = currentMillis;
      }
    }
    else{
      if(digitalRead(buttonPin) == 0){
        Serial.println("Niet correct");
        previousMillis = currentMillis;
      }
    }
  }
 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
 
    // if the button state has changed:
  
    ledState = !ledState;
    ledFadeValue = 0;
    lastLedFadeTime = millis();
    
  }
 
  // set the LED:
  if (breathMode) {
    if (millis() - lastLedFadeTime > ledFadeInterval) {
      lastLedFadeTime = millis();
      analogWrite(ledPin, ledFadeValue);
      ledFadeValue += ledFadeStep;
      if (ledFadeValue > 255){
        ledFadeValue = 255 - ledFadeStep;
        ledFadeStep = -ledFadeStep;
      } else if (ledFadeValue < 0) {
        ledFadeValue = 0;
        ledFadeStep = -ledFadeStep;
      }
    }
  } else {
    digitalWrite(ledPin, ledState);
  }
}

void InboxMessage::run(String a) {

  if (a == u8"app:startup")
  {
    prop.sendAllData();
    prop.sendDone(a);
  }
  else if (a == u8"reset-mcu")
  {
    prop.resetMcu();
  }
  else if (a == "lightsOn:true")
  {
    lightsOn = true;
  } 
  else if (a == "reset") 
  {
    lightsOn = false;
    isReset.setValue(true);
    prop.sendAllData();
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}
