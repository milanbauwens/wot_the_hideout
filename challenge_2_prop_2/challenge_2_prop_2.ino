/* BlinkOnWifiProp.ino
   MIT License (c) Faure Systems <dev at faure dot systems>

   Adapt the Blink example (https://www.arduino.cc/en/tutorial/blink) as a
   simple MQTT prop. Avoid delay() calls (except short ones) in loop() to
   ensure CPU for MQTT protocol. Use PropAction checks instead.

   Copy and change it to build your first Arduino connected prop, you will
   only be limited by your imagination.

   Requirements:
   - install ArduinoProps2.zip library and dependencies (https://github.com/gdm-webofthings/ArduinoProps2)
   - help: https://github.com/gdm-webofthings/ArduinoProps2/blob/main/help/ArduinoProp_sketch.md
*/

#include "ArduinoProps.h"

// If you're running xcape.io Room software you have to respect prop inbox/outbox
// topic syntax: Room/[escape room name]/Props/[propsname]/inbox|outbox
// https://xcape.io/go/room

/* ---------------
 * Configure your WiFi Prop here
 * --------------- */

WifiProp prop(u8"Frequency", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/Frequency/inbox",
                  u8"Room/My room/Props/Frequency/outbox",
                  "192.168.50.101", // your MQTT server IP address
                  1883); // your MQTT server port;

/* ---------------
 * Define the variables (internal and Xcape.io variables)
 * --------------- */

PropDataLogical blinking(u8"blink", u8"yes", u8"no", true);
PropDataLogical led(u8"led");
PropDataLogical connection(u8"connection");
PropDataText rssi(u8"rssi");

void blink(); // define your upcoming blink method
void frequency(); // define your upcoming blink method
PropAction blinkAction = PropAction(1000, blink); // this action will repeat every 1 second and will NOT block the code
PropAction frequencyAction = PropAction(100, frequency); // this action will repeat every 1 second and will NOT block the code

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphras of the network
const int propNumber = 122;

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

boolean SOLVED = false;
boolean RESET = false;
boolean PostersSolved = false;
boolean SEND_OVER = false;

#include <Wire.h>     // include Arduino Wire library
#include "rgb_lcd.h"  // include Seeed Studio LCD library

#define ROTARY_ANGLE_SENSOR A0
#define ROTARY_ANGLE_SENSOR_DECIMAL A1
const int buttonPin = 2;

rgb_lcd lcd;  // create an instance of the rgb_lcd class
int freq = 0; // variable to store the frequency
int freq_dec = 0; // variable to store the frequency decimals
int buttonState = 0; // variable for reading the pushbutton status
int resetPin = 12;

void setup()
{
  Serial.begin(9600);

  
  writeHeader("Welcome to the Web Of Things Arduino Prop library!");

  prop.addData(&blinking);
  prop.addData(&led);
  prop.addData(&connection);
  prop.addData(&rssi);

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages

  pinMode(buttonPin, INPUT); // initialize the pushbutton pin as an input:

  // lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
  // lcd.setRGB(255, 150, 0);  // set the backlight color to green
  // lcd.print("Frequency");  // print a message to the LCD
  // lcd.setCursor(4, 1);  // set the cursor to column 0, line 1

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
    IPAddress(192, 168, 50, 101),  // dns_server (sets to default gateway)
    IPAddress(192, 168, 50, 1),  // gateway
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
  buttonState = digitalRead(buttonPin);
  if(PostersSolved){
    frequencyAction.check();
  }
  if(!PostersSolved){
    lcd.clear();
    lcd.setRGB(0, 0, 0);
  }
  if(SEND_OVER){
    prop.sendOver("Frequency");
  }

}

/* ---------------
 * The Xcape.io logic/code
 * --------------- */

void frequency()
{
  lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.setRGB(255, 150, 0);  // set the backlight color to green

  freq = map(analogRead(ROTARY_ANGLE_SENSOR), 0, 1023, 108, 88);
  freq_dec = map(analogRead(ROTARY_ANGLE_SENSOR_DECIMAL), 0, 1023, 0, 99);

  if(SOLVED){
      lcd.setRGB(0, 255, 0);
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("Screen on");
      SEND_OVER = true;
  }

  if(!SOLVED){
    if(buttonState == HIGH){
      if((freq == 100) && (freq_dec == 15)){
        SOLVED = true;
      }
      else{
        lcd.setRGB(255, 0, 0);
        while(buttonState == HIGH){
          buttonState = digitalRead(buttonPin);
        }
      }
    }
    else{
      lcd.setRGB(255, 150, 0);
    }

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Frequency");
    lcd.setCursor(4, 1);
    lcd.print(freq);
    lcd.print(".");
    lcd.print(freq_dec);
    lcd.print(" MHz        ");
    lcd.setCursor(4, 3);
  }

}
void blink()
{
  if (blinking.value()) {
    led.setValue(!led.value());
    digitalWrite(LED_BUILTIN, led.value() ? HIGH : LOW);
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
  else if (a == "blink:1")
  {
    blinking.setValue(true);

    prop.sendAllData(); // all data change, we don't have to be selctive then
    prop.sendDone(a); // acknowledge prop command action
  }
  else if (a == "blink:0")
  {
    blinking.setValue(false);

    prop.sendAllData(); // all data change, we don't have to be selctive then
    prop.sendDone(a); // acknowledge prop command action
  }
  else if (a == "frequency:1")
  {
    Serial.println("De frequency is correct");
    SOLVED = true;
    prop.sendOver("Frequency");
  }
  else if (a == "frequency:0")
  {
    Serial.println("Frequency is resetting...");
    SOLVED = false;
    frequencyAction.check();
    PostersSolved = false;
  }
  
  else if (a == "postersSolved")
  {
    PostersSolved = true;
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}