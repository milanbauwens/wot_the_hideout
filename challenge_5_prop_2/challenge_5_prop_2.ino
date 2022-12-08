#include "ArduinoProps.h"
#include <ezButton.h>


/* ---------------
 * Configure your WiFi Prop here
 * --------------- */

WifiProp prop(u8"challenge_5_prop_2", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/challenge_5_prop_2/inbox",
                  u8"Room/My room/Props/challenge_5_prop_2/outbox",
                  "192.168.50.101", // your MQTT server IP address
                  1883); // your MQTT server port;

/* ---------------
 * Define the variables (internal and Xcape.io variables)
 * --------------- */
bool lightsOn = false;
PropDataLogical ventIsOpen(u8"Vent is open", u8"yes", u8"no", false);
PropDataLogical keyFell(u8"Key fell", u8"yes", u8"no", false);
PropDataLogical isReset(u8"isReset");
PropDataText rssi(u8"rssi");

void manageVent(); // define upcoming manageTempSlider method
PropAction vent = PropAction(1, manageVent);

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphrase of the network
const int propNumber = 152;

// vent variables
const int RELAY_PIN = A5;  // the Arduino pin, which connects to the IN pin of relay
const int BUTTON_PIN = 7; // Arduino pin connected to button's pin

bool open_lock = false;
bool lock_is_opened = false;
unsigned long currentTime = 0;

ezButton button(BUTTON_PIN); // create ezButton object that attach to pin 12;


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

  Serial.println("Starting vent program!");

  prop.addData(&ventIsOpen);
  prop.addData(&keyFell);
  prop.addData(&rssi);
  prop.addData(&isReset);
  isReset.setValue(true);
  prop.sendAllData();

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages

  // At this point, the broker is not connected yet

  // vent
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
  // initialize digital pin 3 as an output.
  pinMode(RELAY_PIN, OUTPUT);

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
  rssi.setValue(WiFi.RSSI() + String(" dBm")); // https://www.metageek.com/training/resources/understanding-rssi.html

  if (lightsOn) {
    vent.check();
  }
}

/* ---------------
 * The Xcape.io logic/code
 * --------------- */

void manageVent() {
  button.loop(); // MUST call the loop() function first


  if (open_lock){
    if (!lock_is_opened) {
      currentTime = millis();

      digitalWrite(RELAY_PIN, HIGH);
      do {
        // Waiting for time to pass...
        // Light on for 1s
      } while (millis() < currentTime + 1000);

      digitalWrite(RELAY_PIN, LOW);

      lock_is_opened = true;
    }
    if(button.isPressed()) {
      prop.sendOver("Vent");
      Serial.println("The key fell");
      keyFell.setValue(true);
      lock_is_opened = false;
      open_lock = false;
    }
  }
}

void InboxMessage::run(String a) {

  if (a == u8"app:startup")
  {
    prop.sendAllData();
    prop.sendDone(a);
  }
  else if (a == u8"reset")
  {
    open_lock = false;
    lock_is_opened = false;
    lightsOn = false;
    isReset.setValue(true);
    prop.sendAllData();
    }
  else if (a == "thermostat_solved"){
    open_lock = true;
  }
  
    else if (a == "lightsOn:true")
  {
    lightsOn = true;
  }
  else if (a == "ventOpen:true")
  {
    ventIsOpen.setValue(true);

    open_lock = true;

    prop.sendAllData(); // all data change, we don't have to be selective then
    prop.sendDone(a); // acknowledge prop command action
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}
