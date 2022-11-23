#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <SPI.h>

#include "ArduinoProps.h"
#include <PubSubClient.h>


// Connect with Xcape.io
WifiProp prop(u8"RFID_Prikbord", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/RFID_Prikbord/inbox",
                  u8"Room/My room/Props/RFID_Prikbord/outbox",
                  "192.168.50.101", // your MQTT server IP address
                  1883); // your MQTT server port;

bool wifiBegun(false); // this is an internal variable to use
const char* ssid = "WOT"; // the SSID of the network
const char *passphrase = "enterthegame"; // the passphrase of the network
const int propNumber = 131; //131

// Amount of RFID Readers
const byte AMOUNT_READERS = 3; 

// Array of Slave Select (SS-pins)
const byte SS_PINS[]= { 10, 11, 12};

// Reset pin (RST)
const byte RESET_PIN = 9;

// Initialise an array of readers 
MFRC522 mfrc522[AMOUNT_READERS];  

// Correct RFID tag UID's
const String CORRECT_IDS[]= {"392112EF", "D9324BD9", "797A4BD9"}; 

// ID's collected by the readers
String current_ids[AMOUNT_READERS];

// Ligths on

bool frequencySolved = false;
bool SENDPROPS =  false;

PropDataLogical isReset(u8"isReset");

void rfid();
PropAction rfidAction = PropAction(1000, rfid);


void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  // Initialize the SPI
  SPI.begin();

  prop.addData(&isReset);
  isReset.setValue(true);


  // Initialize all defined readers and show their versions
  for (uint8_t i=0; i<AMOUNT_READERS; i++) {
    mfrc522[i].PCD_Init(SS_PINS[i], RESET_PIN);
    mfrc522[i].PCD_DumpVersionToSerial();
  }
  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages
}

/**
 * Writes a header in the serial output
 */
void writeHeader(String title) {
  Serial.println("--------");
  Serial.println(title);
  Serial.println("--------");
  Serial.println("");
}

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

void loop() {
  // validate the Wifi connection
  validateWifi(); 
  prop.loop();

  // check rfid prop
  if (frequencySolved) {
    rfidAction.check();
  }
}


String dump_byte_array(byte *buffer, byte bufferSize) {
  String str = "";
  for (byte i = 0; i < bufferSize; i++) {
    str = str + ((((buffer[i] & 0xF0) >> 4) <= 9) ? (char)(((buffer[i] & 0xF0) >> 4) + '0') : (char)(((buffer[i] & 0xF0) >> 4) + 'A' - 10));
    str = str + (((buffer[i] & 0x0F) <= 9) ? (char)((buffer[i] & 0x0F) + '0') : (char)((buffer[i] & 0x0F) + 'A' - 10));
  }
  return str;
}

void rfid()
{
// Boolean if the correct combination has been made 
  boolean isChanged = false;
  boolean isSolved = false;

  for (uint8_t i=0; i<AMOUNT_READERS; i++) {
    mfrc522[i].PCD_Init();
     
    // Container to store the last reading of a RFID tag
    String readRFID = "";

    // Reading sensor data from an RFID tag
    if(mfrc522[i].PICC_IsNewCardPresent() && mfrc522[i].PICC_ReadCardSerial()) {
      // Read the ID of the tag  
      readRFID = dump_byte_array(mfrc522[i].uid.uidByte, mfrc522[i].uid.size);
    }
    
    // Checking if the last reading is different from the last stored one
    if(readRFID != current_ids[i]) {
      // Update the stored value
       isChanged = true;
      current_ids[i] = readRFID;
    }

    if(isChanged) {
       for (uint8_t i=0; i<AMOUNT_READERS; i++) {
         Serial.println(current_ids[i]);
       }
    }

    // Halt PICC 
    mfrc522[i].PICC_HaltA();
  } 

  // Check if the readings are correct
  if((current_ids[0] == CORRECT_IDS[0]) && (current_ids[1] == CORRECT_IDS[1]) && (current_ids[2] == CORRECT_IDS[2])) {
    isSolved = true;
    if (!SENDPROPS) {
      prop.sendOver("RFID_Prikbord");
      SENDPROPS = true;
    }
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
  } else if (a == "frequencySolved") {
    frequencySolved = true;
    isReset.setValue(false);
    prop.sendAllData();
  } else if (a == "reset") {
    frequencySolved = false;
    SENDPROPS = false;
    isReset.setValue(true);
    prop.sendAllData();
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}
