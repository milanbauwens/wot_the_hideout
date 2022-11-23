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

WifiProp prop(u8"Fusebox", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/Fusebox/inbox",
                  u8"Room/My room/Props/Fusebox/outbox",
                  "192.168.50.101", // your MQTT server IP address
                  1883); // your MQTT server port;

/* ---------------
 * Define the variables (internal and Xcape.io variables)
 * --------------- */

PropDataLogical led(u8"led");
PropDataLogical lamp(u8"lamp");
PropDataLogical isReset(u8"isReset");

void fusebox();
void blink();
PropAction fuseboxAction = PropAction(1000, fusebox);
PropAction blinkAction = PropAction(1000, blink);

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphras of the network
const int propNumber = 111;

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
boolean SENDPROPS = false;
boolean LIGHT = false;

void setup()
{
  Serial.begin(9600);

  #define REDLED_PIN 3
  #define INPUT_PIN1 11
  #define INPUT_PIN2 12
  #define INPUT_PIN3 13

  writeHeader("Welcome to the Web Of Things Arduino Prop library!");

  prop.addData(&led);
  prop.addData(&lamp);
  prop.addData(&isReset);

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages

  pinMode(REDLED_PIN, OUTPUT);
  pinMode(INPUT_PIN1, INPUT);
  pinMode(INPUT_PIN2, INPUT);
  pinMode(INPUT_PIN3, INPUT);
  // At this point, the broker is not connected yet
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

void loop()
{
  validateWifi(); // validate the Wifi connection
  prop.loop(); // start looping the prop (superpowers gottin from xcape.io)
  led.setValue(digitalRead(REDLED_PIN)); // read I/O
  lamp.setValue(LIGHT); // read I/O
  fuseboxAction.check();
  blinkAction.check();
}

void fusebox()
{
  if(digitalRead(INPUT_PIN1) == HIGH && digitalRead(INPUT_PIN2) == HIGH && digitalRead(INPUT_PIN3) == HIGH) {
    SOLVED = true;
  } else {
    SOLVED = false;
  }

  if (!SOLVED) {
    isReset.setValue(true);
    if (LIGHT) {
      prop.sendOver("Disconnect");
      prop.sendOver("Disconnect");
      prop.sendOver("Disconnect");
      prop.sendOver("Disconnect");
      prop.sendOver("Disconnect");
      LIGHT = false;
      lamp.setValue(LIGHT);
      prop.sendAllData();
    }
  }

  if (SOLVED) {
    digitalWrite(REDLED_PIN, LOW);
    led.setValue(digitalRead(REDLED_PIN));
    isReset.setValue(false);
    if (!LIGHT) {
      prop.sendOver("Fusebox");
      prop.sendOver("Fusebox");
      prop.sendOver("Fusebox");
      prop.sendOver("Fusebox");
      prop.sendOver("Fusebox");
      LIGHT = true;
      lamp.setValue(LIGHT);
      prop.sendAllData();
    }
    if (!SENDPROPS) {
      SENDPROPS = true;
      prop.sendOver("lightsOn");
    }
  }
}

void blink()
{
  if (!SOLVED) {
    digitalWrite(REDLED_PIN, digitalRead(REDLED_PIN) ? LOW : HIGH);
    led.setValue(digitalRead(REDLED_PIN));
  }
}


/* ---------------
 * The Xcape.io logic/code
 * --------------- */

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
  else if (a == "1")
  {
    Serial.println("The lamp is on");
    prop.sendDone(a);
  }
  else if (a == "0")
  {
    Serial.println("The lamp is off");
    prop.sendRequ("lamp -> lampoff");
    prop.sendDone(a);
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}
