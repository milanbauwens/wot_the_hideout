#include "ArduinoProps.h"


/* ---------------
 * Configure your WiFi Prop here
 * --------------- */

WifiProp prop(u8"challenge_4_prop_1", // as MQTT client id, should be unique per client for given broker
                  u8"Room/My room/Props/challenge_4_prop_1/inbox",
                  u8"Room/My room/Props/challenge_4_prop_1/outbox",
                  "192.168.50.101", // your MQTT server IP address
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
const int propNumber = 141;

// morse variables
bool lightsOn = false;
bool videoIsSynced = false;
bool rfidSolved = false;
const int relay = 7;
bool ventilationIsOpen = false;
bool gameStopped = false;

//Variables (Niels)
unsigned long FRIDA[][4] = {{500, 500, 2000, 500}, {500, 2000,500,0}, {500,500,0,0},{2000,500,500,0},{500,2000,0,0}}; //The sequence of the word Frida
unsigned int silence = 500; //time between intervals
unsigned int nextLetter = 5000; // time between letters
unsigned int nextWord = 5000; //time between words ( + between letters = 7000)
int numOfFlashes = 5; //amount of flashes for modulo
int numOfLetters = 5; //amount of letters for modulo
unsigned int index = 0; //index for Frida[index][]
unsigned int index2 = 0; // index for Frida [][index2]
unsigned long lastMillis;
unsigned long waitTime;
unsigned long currentMillis;
bool flash;
unsigned long currentMillis2;
unsigned long lastMillis2;
unsigned long waitTimeVideo = 52000;

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

  prop.begin(InboxMessage::run); // this will start a loop to check for MQTT messages

  // At this point, the broker is not connected yet

  // morse
  pinMode(relay, OUTPUT);

  // wait till synced with timing in video
  // if (videoSync) {
  //   currentTime = millis();
  //   do {
  //     // Waiting for time to pass...
  //   } while (millis() < currentTime + syncTime);
  //   videoIsSynced = true;
  // }

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

  // morse
  // loop morse code till the ventilation closes

// if(rfidSolved){
//   currentMillis2 = millis();

//   if(currentMillis2 - lastMillis2 >= waitTimeVideo){
//       videoIsSynced = true;
//       lastMillis2 = currentMillis2;
//     }
// }
  
  if(lightsOn && rfidSolved &&!ventilationIsOpen && videoIsSynced) {
    morse.check();
  }
}

/* ---------------
 * The Xcape.io logic/code
 * --------------- */

void playMorse() {

  currentMillis = millis(); //sets time to currentTime
  

  if (currentMillis - lastMillis >= waitTime) { //checks if time is greater or equal to the waittime of the letter
    lastMillis = currentMillis; //resets time to current

    if (flash) { // if the light is turned on, turn off again and wait for the duration of the silence variable
      flash = false;
      digitalWrite(relay, LOW);
      waitTime = silence;
    } else { // if the light is off, check conditions or else turn the led on
      if((index2 + 1) % numOfFlashes == 0){ // if the index is at it's end -> no light and put a waittime for the next letter
        digitalWrite(relay, LOW);
        waitTime = nextLetter;
        index = (index + 1) % numOfLetters; //increase index for the next letter
      }

      flash = true; //set flash to true (see above)
      waitTime = FRIDA[index][index2]; // set the waittime equal to the array value of FRIDA [][]
      index2 = (index2 + 1) % numOfFlashes; // increase the index to go to the next dot or dash
      
      if(index == 4 && index2 == 4){ // if we are at the end of the word and interval -> add waitTime to start again
          waitTime = nextWord;
      }
      else if(index2 == 0){ // if we are at the end of a letter sequence, set light off and waitTime for next letter
        digitalWrite(relay, LOW);
        waitTime = nextLetter;
      }
      else if(waitTime == 0){ // if the waitTime in FRIDA [][] == 0 , No light should be turned on
        digitalWrite(relay, LOW);
      }
      else{
        digitalWrite(relay, HIGH); //turns on the led if other conditions are not fulfilled -> sequence
      }
    
    }
  }

  // get out of do while loop when the vent opens
  if (ventilationIsOpen)
  {
    exit(0);
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
  else if (a == "videoSync:true")
  {
    propVideoSync.setValue(true);

    videoIsSynced = true;

    prop.sendAllData(); // all data change, we don't have to be selective then
    prop.sendDone(a); // acknowledge prop command action
  }
  else if (a == "ventOpen:true")
  {
    propVentilationIsOpen.setValue(true);
    digitalWrite(relay,LOW);
    ventilationIsOpen = true;
    rfidSolved = false;
    lightsOn = false;
    prop.sendAllData(); // all data change, we don't have to be selective then
    prop.sendDone(a); // acknowledge prop command action
  }
  else if (a == "rfidSolved:true"){
    
    propVideoSync.setValue(true);
    rfidSolved = true;
    prop.sendAllData();
    prop.sendDone(a);
  }
    else if (a == "videoEnded"){
    videoIsSynced = true;
    prop.sendAllData();
    prop.sendDone(a);
  }
  else if (a == "reset"){
    isReset.setValue(true);
    index = 0;
    index2 = 0;
    videoIsSynced = false;
    rfidSolved = false;
    lightsOn = false;
    ventilationIsOpen = false;
    digitalWrite(relay,LOW);
    prop.sendAllData();
  }
  else if (a == u8"game:end" || a == u8"app:exit")
  {
    gameStopped = true;

    prop.sendAllData();
    prop.sendDone(a);
  }
  else
  {
    // acknowledge omition of the prop command
    prop.sendOmit(a);
  }
}