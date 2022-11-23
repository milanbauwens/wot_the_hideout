#include "ArduinoProps.h"


/* ---------------
 * Configure your WiFi Prop here
 * --------------- */

WifiProp prop(u8"challenge_4_prop_1", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/challenge_4_prop_1/inbox",
                  u8"Room/My room/Props/challenge_4_prop_1/outbox",
                  "192.168.50.101", // your MQTT server IP address1
                  1883); // your MQTT server port;

/* ---------------
 * Define the variables (internal and Xcape.io variables)
 * --------------- */

PropDataLogical propVideoSync(u8"Synced with timing in video", u8"yes", u8"no", false);
PropDataLogical propVentilationIsOpen(u8"ventilation is open", u8"yes", u8"no", false);
PropDataLogical isReset(u8"isReset");
PropDataText rssi(u8"rssi");

void playMorse(); // define upcoming playMorse method
PropAction morse = PropAction(0, playMorse);

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphrase of the network
const int propNumber = 161;

// morse variables
bool videoIsSynced = false;
const int syncTime = 1000; // time that needs to pass till program is synced with video
const int relay = 7;
char morseCode[] = "--- .-. .-- . .-.. .-.."; // ORWELL in morse code
unsigned long currentTime = 0;
bool videoSync = false;
bool ventilationIsOpen = false;

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

  Serial.println("Starting morse program!");

  prop.addData(&propVideoSync);
  prop.addData(&propVentilationIsOpen);
  prop.addData(&rssi);
  prop.addData(&isReset);
  isReset.setValue(true);
  prop.sendAllData();

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages

  // At this point, the broker is not connected yet

  // morse
  pinMode(relay, OUTPUT);



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

  // wait till synced with timing in video
  if (videoSync) {
    isReset.setValue(false);
    prop.sendAllData();
    currentTime = millis();
    do {
      // Waiting for time to pass...
    } while (millis() < currentTime + syncTime);
    videoIsSynced = true;
  }
  
  // morse
  // loop morse code till the ventilation closes
  if(videoIsSynced && !ventilationIsOpen) {
    morse.check();
  }
}

/* ---------------
 * The Xcape.io logic/code
 * --------------- */

void playMorse() {
  for (int i = 0; i < strlen(morseCode); i++) {
    currentTime = millis();
    Serial.println(i);

    switch(morseCode[i]) {    
      // dot
      case 46:
        Serial.print("Dot\n");
        prop.sendAllData();

        // get out of do while loop when the vent opens
        if(ventilationIsOpen) {
          exit(0);
        }

        digitalWrite(relay, HIGH);

        do {
          // Waiting for time to pass...
          // Light on for 1s
        } while (millis() < currentTime + 1000);

        currentTime = millis();

        digitalWrite(relay, LOW);

        do {
          // Waiting for time to pass...
          // Light off for 1s
        } while (millis() < currentTime + 1000);

        currentTime = millis();

        break;

      // dash
      case 45:
        Serial.print("Dash\n");
        prop.sendAllData();

        // get out of do while loop when the vent opens
        if(ventilationIsOpen) {
          exit(0);
        }

        digitalWrite(relay, HIGH);

        do {
          // Waiting for time to pass...
          // Light on for 3s
        } while (millis() < currentTime + 3000);

        currentTime = millis();

        digitalWrite(relay, LOW);

        do {
          // Waiting for time to pass...
          // Light off for 1s
        } while (millis() < currentTime + 1000);

        currentTime = millis();

        break;
      // space
      case 32:
        // space between words is 3s -> 1s is already default after every dot or dash
        Serial.print("Space\n");
        prop.sendAllData();

        // get out of do while loop when the vent opens
        if(ventilationIsOpen) {
          exit(0);
        }

        do {
          // Waiting for time to pass...
          // Light off for 2s
        } while (millis() < currentTime + 2000);

        currentTime = millis();

        break;
      default:     
        exit(0);
    }
  }
  // space between words is 7s -> 1s is already default after every dot or dash
  Serial.print("Between letters\n");
  prop.sendAllData();
  do {
    // Waiting for time to pass...
    // Light on for 3s
  } while (millis() < currentTime + 6000);
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
  else if (a == "rfidSolved:true")
  {
    propVideoSync.setValue(true);

    videoSync = true;

    prop.sendAllData(); // all data change, we don't have to be selective then
    prop.sendDone(a); // acknowledge prop command action
  }
  else if (a == "ventOpen:true")
  {
    propVentilationIsOpen.setValue(true);

    ventilationIsOpen = true;

    prop.sendAllData(); // all data change, we don't have to be selective then
    prop.sendDone(a); // acknowledge prop command action
  }
  else if (a == "reset")
  {
    videoIsSynced = false;
    videoSync = false;
    ventilationIsOpen = false;
    isReset.setValue(true);
    prop.sendAllData();
    digitalWrite(relay, LOW);
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}