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

WifiProp prop(u8"Posters", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/Posters/inbox",
                  u8"Room/My room/Props/Posters/outbox",
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
PropAction blinkAction = PropAction(1000, blink); // this action will repeat every 1 second and will NOT block the code

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphras of the network
const int propNumber = 121;

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
#include <ezButton.h>

const int BUTTON_PIN = 12; // Arduino pin connected to button's pin
const int RELAY_PIN = A5; // Arduino pin connected to relay's pin
int red_light_pin = 11;
int green_light_pin = 10;
int blue_light_pin = 9;

ezButton button(BUTTON_PIN); // create ezButton object that attach to pin 12;

boolean lightsOn = false; 

void setup()
{
  Serial.begin(9600);

  
  writeHeader("Welcome to the Web Of Things Arduino Prop library!");

  prop.addData(&blinking);
  prop.addData(&led);
  prop.addData(&connection);
  prop.addData(&rssi);

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages


  // At this point, the broker is not connected yet
  pinMode(RELAY_PIN, OUTPUT); // set arduino pin to output mode
  button.setDebounceTime(50); // set debounce time to 50 milliseconds

  digitalWrite(RELAY_PIN, LOW); // lock the desk slide

  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);
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
    IPAddress(192, 168, 50, 1),  // dns_server (sets to default gateway)
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

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}
void loop()
{
  validateWifi(); // validate the Wifi connection
  prop.loop(); // start looping the prop (superpowers gottin from xcape.io)
  rssi.setValue(WiFi.RSSI() + String(" dBm")); // https://www.metageek.com/training/resources/understanding-rssi.html
  led.setValue(digitalRead(LED_BUILTIN)); // read I/O
  connection.setValue(digitalRead(!green_light_pin)); // read I/O
  
  if(lightsOn){

    button.loop(); // MUST call the loop() function first

    if (SOLVED) {
      Serial.println("The button is pressed");
      digitalWrite(RELAY_PIN, HIGH); // unlock the desk slide in 10 seconds
      RGB_color(0, 255, 0); // Green
      prop.sendOver("Posters");
    }

    if (!SOLVED) {
      digitalWrite(RELAY_PIN, LOW); // unlock the desk slide in 10 seconds
      RGB_color(255, 0, 0); // Red
    }

    if(button.isPressed()) {
      SOLVED = true;
    }
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
  else if (a == "posters:1")
  {
    SOLVED = true;
    Serial.println("Schuif open");
    prop.sendOver("Posters");
  }
  else if (a == "posters:0")
  {
    SOLVED = false;
    Serial.println("Schuif gesloten");
    prop.sendOver("Posters");
  }
  else if (a == "lightsOn:true")
  {
    lightsOn = true;
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}