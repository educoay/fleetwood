// 22/11/2015
/******************************************/
/*           Unit Settings                */
/******************************************/
// This unit's MAC address. MAC address is used for OpenHAB to recognise this controller at startup
// and give it the settings it should have, e.g. zone assignment, etc.
// i.e. zero config!
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};  


/******************************************/
/*           Server settings              */
/******************************************/
// MQTT Server IP Address
byte server[] = {192, 168, 0, 21};

/******************************************/
/*           Display settings             */
/******************************************/
// Is the remote multisensor connected, i.e. do we wish to display lux setting?
#define showlightlevel 0

/******************************************/
/*           MQTT Topic Naming            */
/******************************************/
// MQTT topic naming. We are sending commands over MQTT topics like this:
//   rootvector/zonevector/commandvector
//   e.g. home/bedroom/arduinocommands
// ALL commands are sent on this one topic, however values received back over MQTT are
//   received over multiple topics.
// NB The zonevector is determined at startup by "downloading" this from OpenHAB over MQTT.
//   meaning that OpenHAB will determine the assigned zone given this unit's MAC address.
//   This makes it quick to deploy lots of units, as you'd only need to change the MAC above.
// None of the below really need changing! They are 'internal' names. Change only if you want different names for topics.
//   Topic paths are built from the below names.
#define rootvector "home"                          // do not exceed 10 chars, see below sram note
#define commandvector "megacmd"                    // do not exceed 10 chars, see below sram note
#define setupvector "megasetup"                    // do not exceed 10 chars
#define datetimevector "datetime/arduinodisplay"   // hmmm, think about #sram later. This seems ok
#define audiovector "audio"
#define titlevector "title"
#define artistvector "artist"
#define tempvector "temp"
#define boilervector "boiler"
#define currentvector "current" // 
#define volumevector "volume" // 
#define playstatusvector "playstatus" // 
#define settingsvector "settings" // 
#define circuitcountvector "circuit_count" // 
#define circuit_1_namevector "circuit_1_name" // NB don't enter actual circuit names here. This is the vector used to obtain circuit names
#define circuit_2_namevector "circuit_2_name" // NB don't enter actual circuit names here. This is the vector used to obtain circuit names
#define circuit_3_namevector "circuit_3_name" // NB don't enter actual circuit names here. This is the vector used to obtain circuit names
#define circuit_4_namevector "circuit_4_name" // NB don't enter actual circuit names here. This is the vector used to obtain circuit names
#define circuit_5_namevector "circuit_5_name" // NB don't enter actual circuit names here. This is the vector used to obtain circuit names
#define circuit_6_namevector "circuit_6_name" // NB don't enter actual circuit names here. This is the vector used to obtain circuit names
#define light_scene_1_namevector "light_scene_1_name" // NB don't enter actual light scene names here...
#define light_scene_2_namevector "light_scene_2_name" // NB don't enter actual light scene names here...
#define light_scene_3_namevector "light_scene_3_name" // NB don't enter actual light scene names here...
#define light_scene_4_namevector "light_scene_4_name" // NB don't enter actual light scene names here...
#define light_scene_5_namevector "light_scene_5_name" // NB don't enter actual light scene names here...
#define audio_fave_1_namevector "audio_fave_1_name" // NB don't enter actual audio fave names here...
#define audio_fave_2_namevector "audio_fave_2_name" // NB don't enter actual audio fave names here...
#define audio_fave_3_namevector "audio_fave_3_name" // NB don't enter actual audio fave names here...
#define audio_fave_4_namevector "audio_fave_4_name" // NB don't enter actual audio fave names here...
#define audio_fave_5_namevector "audio_fave_5_name" // NB don't enter actual audio fave names here...
#define lightvector "light" //
#define circuit1levelvector "circuit1level" //
#define circuit2levelvector "circuit2level" //
#define circuit3levelvector "circuit3level" //
#define circuit4levelvector "circuit4level" //
#define circuit5levelvector "circuit5level" //
#define circuit6levelvector "circuit6level" //


/******************************************/
/*          SRAM management note         */
/******************************************/
// I have hashtagged possible areas for improvement / troubleshooting regarding SRAM. So if we see
//   problems such as random text on screen then search for #sram.
// Most likely culprit would be buff[43]. To my calculations, assuming a topic root of
//   no more than 10 chars plus null terminator, buff[43] should be safe!
// Light circuit names, scene names, and audio favourites should be 11 chars max. This could be increased
//   by changing the struct 

/******************************************/
/*          Include libraries             */
/******************************************/

#include <ClickButton.h>
#include <Encoder.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "U8glib.h"
#include "DHT.h" // DHT11/22 temp / humidity
#include <Wire.h>
#include <BH1750.h>            // #lux
#include "Adafruit_MPR121.h"   // #prox


/******************************************/
/*               Defines                  */
/******************************************/

// #sram memory management
// Maximum length for strings. This is the actual number of chars, we account for null terminator
//   on top of this. Keep this as LOW as poss.
#define nameLength 11      // Light circuit, scene, and audio fave names. (suggest 12)
#define trackLength 30     // Now Playing track: max length to accept / display onto Arduino. (suggest 60)
#define artistLength 30    // Now Playing artist: as above. (suggest 60)

// comment below line out if not debugging
#define DEBUG      // code step-through

// Pins
#define btn1Pin 		54	// illuminated tact switch - Switch part
#define led1Pin 		55	// illuminated tact switch - LED
#define btn2Pin 		56	// illuminated tact switch - Switch part
#define led2Pin 		57	// illuminated tact switch - LED
#define btn3Pin 		58	// illuminated tact switch - Switch part
#define led3Pin 		59	// illuminated tact switch - LED
#define btn4Pin 		60	// illuminated tact switch - Switch part
#define led4Pin 		61	// illuminated tact switch - LED
#define btn5Pin 		62	// illuminated tact switch - Switch part
#define led5Pin 		63	// illuminated tact switch - LED
#define btnMode 		64	// illuminated tact switch - Switch part
#define ledMode 		65	// illuminated tact switch - LED
#define btnAllOff               67
#define ledAllOff               68
#define encoder1 		3	// rotary encoder - right hand pin in row of 3 as you look down
#define encoder2 		2	// rotary encoder - left hand pin in row of 3 as you look down
#define encodersw 		8	// rotary encoder - switch part
#define oledClock		4	// OLED
#define oledData		5	// OLED
#define chipSelect		6	// OLED
#define commandDataSelect	7	// OLED
#define resetCtl                66      // don't ask. (separate w5100 reset)
#define ardReset                69      // hardwired to reset pin of Arduino - physical reset by MQTT
#define DHTPIN                  32      // was pin 11. With latest version of PCB, this is an external connector


//BREADBOARD:
/*
#define btnMode 		54	// illuminated tact switch - Switch part
#define ledMode 		55	// illuminated tact switch - LED
#define btn1Pin 		56	// illuminated tact switch - Switch part
#define led1Pin 		57	// illuminated tact switch - LED
#define btn2Pin 		58	// illuminated tact switch - Switch part
#define led2Pin 		59	// illuminated tact switch - LED
#define btn3Pin 		60	// illuminated tact switch - Switch part
#define led3Pin 		61	// illuminated tact switch - LED
#define btn4Pin 		62	// illuminated tact switch - Switch part
#define led4Pin 		63	// illuminated tact switch - LED
#define btn5Pin 		64	// illuminated tact switch - Switch part
#define led5Pin 		65	// illuminated tact switch - LED
#define encoder1 		2	// rotary encoder - right hand pin in row of 3 as you look down
#define encoder2 		3	// rotary encoder - left hand pin in row of 3 as you look down
#define encodersw 		8	// rotary encoder - switch part
#define oledClock		4	// OLED
#define oledData		5	// OLED
#define chipSelect		6	// OLED
#define commandDataSelect	7	// OLED
#define resetCtl                66      // don't ask. (separate w5100 reset)
#define ardReset                69      // hardwired to reset pin of Arduino - physical reset by MQTT
#define DHTPIN                  11      // DHT11 sensor (DHT22 is better, I had stock of the 11...)
*/

// Modes
#define lightMode 1
#define audioMode 2
#define tempMode 3
#define mvhrMode 4
#define otherMode 5
#define diagsMode 6
#define settingsMode 7

// Proximity sense. We're calibrating this manually, using output from Serial Monitor to show us continuous values...
// Set baseline to lowest value you read from Serial Monitor (when no hand is near sensor)
// Set initial threshold to the number which, when capacitance goes below this number, the "touch" is sensed and controller wakes up etc.
// This initial threshold simply sets a value which can subsequently be overwritten by OpenHAB if we want to adjust this later
// 14/5/2015 - 656. Had to wait a good minute.

#define capBaseline 656                // Lowest value for Mat's wallplates is 670 #prox
#define capInitialThreshold 5          // Decent safety margin #prox

// Timeouts
#define w5100resetdelay 800            // Definitely safe: 1500. Probably ok 500?
#define startuphold 0                  // How long should we delay startup so allow us to view
                                       //   the final page of status messages? (set to 0 for now as added big delays to settings request routine)
                                       //   ... i.e. no point in delaying even more at startup
#define button_leds_off_timeout 5000
#define default_mode_timeout 7500
#define display_dim_timeout 7000
#define display_off_timeout 14000
#define heartbeat_timeout 61000 // we expect an MQTT message at least every 60 seconds
#define initialmqttconnecttimeout 5000 // should be 35000
#define preset_stored_msg_timeout 1500


// Delay between requesting each setting / value over MQTT to allow OH to process
// On this front, Arduino is really reliable with fast processing of MQTT, OpenHAB is not so!
#define mqttsettingsreqdelay 400
#define mqttvalreqdelay 400

#define DHTTYPE DHT22   // DHT 22
#define dhtpolldelay 20000    // Take reading and send every e.g. 20 seconds
#define lightSensorDelay 1000 // #lux

/******************************************/
/*            Define settings             */
/******************************************/

// #sram memory hungry this struct is! mainly because of the char text[30]
struct serverData
{
  char name[nameLength+1];    // for storing a name of the server setting (e.g. "Ceiling")
  int level;                  // for storing a numerical value for the setting if applicable (e.g. 30)
//  char text[30];            // for storing a text value for the setting if applicable (e.g. audio track name)
  int gotName;                // (e.g. if the name was downloaded successfully, this is set to 1)
  int gotLevel;               // (e.g. if a settingTextVal OR settingNumberVal was downloaded successfully, this is set to 1)
};

typedef struct serverData ServerData;

ServerData lightCircuit[6];  // store circuit names and light levels for 6 light circuits in a room
ServerData lightScene[5];
ServerData audioFave[5];

// server values
boolean play_status;
char track[trackLength+1];
char artist[artistLength+1];
char serverdatetime[17];
int temp; // #sram We aren't this desperate, but if we were, this could be stored as a char or byte
int volume; // #sram
int temp_target; // #sram
int mvhr_speed; // #sram
char zonename[15] = {'<','d','e','f','a','u','l','t','>','\0'};        // Zone name (14 chars max to be received from OpenHAB)
char zonevect[15] = {'<','d','e','f','a','u','l','t','>','\0'};        // Zone vector (14 chars max to be received from OpenHAB)
char zonepmsg[30] = {' ','\0'};        // Zone pager message (29 chars max to be received from OpenHAB)
int capThreshAdjust = capInitialThreshold;     // Zone controller proximity sense adjustment
int lowestCapVal = 9999;  // initialise with high number
boolean boiler_status;

// settings
int circuitCount = 6; // This is the initial circuit count number. This is overwritten when Arduino receives setting from OpenHAB
                      //  set to 6 so that we catch all circuit values in the case where values arrive before the setting itself
                      //  If we see circuit value of -1, this means we think there is a circuit but haven't got the value yet
char commandtopic[41]; // array size: topicroot/zonevector/commandvector (max 11 + 15 + 11 respectively, plus 2x "/" strings)

// OPTIMISE THIS
char buff[43]; // general buffer. char array. Assuming topicroot is max 10 chars with null terminator
               // this should not ever exceed 43. #sram
char ipString[16];  // char array for displaying the IP address at startup (and any other time, diagnostics?)
char macString[13]; // 12 digit MAC address displayed at startup plus null terminator...
char *modeText[] = {"LIGHT", "AUDIO", "TEMP", "MVHR", "OTHER", "DIAG", "SETTING"}; // #sram

char* settingsNames[] = {"Download", "Slow downld", "Diags", "Reboot", "Exit"}; //#sram


/******************************************/
/*         Define global vars             */
/******************************************/

int oldPosition = 0;                 // This is for the rotary encoder
int currentMode = lightMode;         // default mode
int currentCircuit = 1;              // default circuit
unsigned long defaultmodetimer;      // used for sleep delay, capsense delay, set mode to default
unsigned long preset_stored_msg_timer;   // How long to display the message "preset stored"
unsigned long heartbeatsense = 0;        // We expect an MQTT message every 1 minute (60000 millis) at least
                                         // There's an OH rule sending datetime at least every minute.
                                         // We use this as our heartbeat
unsigned long dhtpollcounter;
unsigned long lightSensorCounter; //#lux
boolean gotinfofromserver = false;
int mqttresponsetime = 0;            // Boot sequence: show OpenHAB response time, i.e. difference in time between
                                     //  sending an MQTT request message and receiving back a value

uint16_t lasttouched = 0;  // #prox
uint16_t currtouched = 0;  // #prox
uint16_t lux;              // #lux
/******************************************/
/*       Define library functions         */
/******************************************/

ClickButton btnSelect(btnMode, LOW, CLICKBTN_PULLUP);        // change this name to btnMode #tidy
ClickButton btnOrf(btnAllOff, LOW, CLICKBTN_PULLUP);
ClickButton btnTransport(encodersw, LOW, CLICKBTN_PULLUP);
ClickButton btnF1(btn1Pin, LOW, CLICKBTN_PULLUP);
ClickButton btnF2(btn2Pin, LOW, CLICKBTN_PULLUP);
ClickButton btnF3(btn3Pin, LOW, CLICKBTN_PULLUP);
ClickButton btnF4(btn4Pin, LOW, CLICKBTN_PULLUP);
ClickButton btnF5(btn5Pin, LOW, CLICKBTN_PULLUP);
Encoder myEnc(encoder1, encoder2);
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_MPR121 cap = Adafruit_MPR121();  // #prox
BH1750 lightMeter;                        // #lux

// Blue display
// WORKING: U8GLIB_NHD31OLED_GR u8g(4, 5, 6, 7);	// SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9

// White display
U8GLIB_NHD31OLED_2X_GR u8g(oledClock, oledData, chipSelect, commandDataSelect);	// SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9


/******************************************/
/*        Pre-Setup Functions             */
/******************************************/

void(* resetFunc) (void) = 0; //declare reset function @ address 0


/******************************************/
/*               SETUP                    */
/*               -----                    */
/******************************************/

void setup()
{
  digitalWrite(ardReset, HIGH); // don't move this line :)
  
  pinMode(ardReset, OUTPUT);   // Set that pin as output, if we didn't already know
  // Setup U8G
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) u8g.setColorIndex(255);                  // white
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) u8g.setColorIndex(3);             // max intensity
  else if ( u8g.getMode() == U8G_MODE_BW ) u8g.setColorIndex(1);                   // pixel on
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) u8g.setHiColorByRGB(255,255,255);

  // MAC string used for MQTT setup and startup display
  sprintf (macString, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  startupState(1);
  
  Serial.begin(9600);
  dht.begin(); // poll counter set at end of setup
  // MPR121 touch / proximity sense #prox
  lightMeter.begin();
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
  }
  Serial.println("MPR121 found!");


  Serial.println("Ready...");

  // Placeholder text and populate structs before getting data from server
                                             // Light CIRCUIT
  for (int i=0;i<6;i++) {
    strcpy(lightCircuit[i].name,"--");       // Value not yet received from server
    lightCircuit[i].level = -1;        // Init this value to -1 to show it hasn't been set yet
    lightCircuit[i].gotName = 0;           // We don't yet have the light name
    lightCircuit[i].gotLevel = 0;            // We don't yet have the light level
  }
                                             // Light SCENE
//strcpy(lightScene[0].name,"--");
  for (int i=0;i<5;i++) {
    strcpy(lightScene[i].name,"--");       // Value not yet received from server
    lightScene[i].level = -1;        //  "is this the currently set light scene": 0=no, 1=yes (or in this case, -1 saying "we don't know")
    lightScene[i].gotName = 0;           // We don't yet have the light name
    lightScene[i].gotLevel = 0;            // We don't yet have the light level
  }
                                             // Audio FAVE
  for (int i=0;i<5;i++) {
    strcpy(audioFave[i].name,"--");       // Value not yet received from server
    audioFave[i].level = -1;        // "is this the currently set audio fave": 0=no, 1=yes (or in this case, -1 saying "we don't know")
    audioFave[i].gotName = 0;           // We don't yet have the light name
    audioFave[i].gotLevel = 0;            // We don't yet have the light level
  }
  strcpy(serverdatetime,"[date time]     "); // initial date time placeholder

  pinMode(resetCtl, OUTPUT);   // Reset W5100
  digitalWrite(resetCtl, LOW); // artificial reset
  delay(w5100resetdelay);
  digitalWrite(resetCtl, HIGH); // artificial reset

  startupState(2);
  

  // Click library
  btnSelect.debounceTime    = 20;
  btnSelect.multiclickTime  = 0;
  btnSelect.longClickTime   = 1000;
  btnOrf.debounceTime    = 20;
  btnOrf.multiclickTime  = 0;
  btnOrf.longClickTime   = 1000;
  btnTransport.debounceTime    = 20;
  btnTransport.multiclickTime  = 200;
  btnTransport.longClickTime   = 1000;
  btnF1.debounceTime    = 20;
  btnF1.multiclickTime  = 200;
  btnF1.longClickTime   = 5000; // long-hold = reset
  btnF2.debounceTime    = 20;
  btnF2.multiclickTime  = 200;
  btnF2.longClickTime   = 1000;
  btnF3.debounceTime    = 20;
  btnF3.multiclickTime  = 200;
  btnF3.longClickTime   = 1000;
  btnF4.debounceTime    = 20;
  btnF4.multiclickTime  = 200;
  btnF4.longClickTime   = 1000;
  btnF5.debounceTime    = 20;
  btnF5.multiclickTime  = 200;
  btnF5.longClickTime   = 1000;
  // LEDs
  pinMode(ledMode, OUTPUT);
  pinMode(ledAllOff, OUTPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);
  pinMode(led5Pin, OUTPUT);
  // Startup LED sequence
  flash(ledMode);
  flash(ledAllOff);
  flash(led1Pin);
  flash(led2Pin);
  flash(led3Pin);
  flash(led4Pin);
  flash(led5Pin);
  flash(led5Pin);
  flash(led4Pin);
  flash(led3Pin);
  flash(led2Pin);
  flash(led1Pin);
  flash(ledAllOff);
  flash(ledMode);
  
  startupState(3);
    
//  Ethernet.begin(mac, ip);
  if (Ethernet.begin(mac) == 0) {
    startupState(40);
  } else {
    startupState(4);
  }

  delay(150); // this was 720 and stable - before then it was 150 and fine!
  client.connect("arduinoClient");
  delay(50); // change back to 50

  startupState(5);
  
  /******************************************/
  /*          Setup: ZeroConfig             */
  /******************************************/
  // First subscribe to a topic consisting of this arduino's MAC address
  sprintf (buff, "%s/%s/%02X%02X%02X%02X%02X%02X/#", rootvector, setupvector, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  client.subscribe(buff); // subscribing to e.g. home/megasetup/MAC/#
  // Now publish this MAC address to a generic topic
  sprintf (buff, "%s/%s", rootvector, setupvector);
  client.publish(buff, macString);
  unsigned long mqttconnecttimeout = millis();
  while (zonename[0] == '<' | zonevect[0] == '<') { // whilst we haven't finished downloading controller startup stuff
    if (millis() - mqttconnecttimeout > initialmqttconnecttimeout) {
      break;
    }
  client.loop(); // This is usually found in the main loop, but we are checking for incoming MQTT messages at this stage too
  }
  mqttresponsetime = millis() - mqttconnecttimeout;
  // Set up commandtopic, which is the whole path e.g. home/bedroom/arduinocmd
  sprintf(commandtopic, "%s%s%s%s%s", rootvector, "/", zonevect, "/", commandvector);

  // Subscribe to our topics, for getting data back from the server
  sprintf(buff, "%s/%s/#", rootvector, zonevect);
  client.subscribe(buff);  // e.g. subscribe to home/bedroom/#
  sprintf(buff, "%s/%s/#", rootvector, datetimevector);
  client.subscribe(buff);  // e.g. subscribe to home/datetime/arduinodisplay/#

  startupState(6);

  delay(startuphold);            // Keep the startup display on the screen... so we can see last message
  turnLEDs(1);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  digitalWrite(2, HIGH);          // Tie rotary encoder interrupt pins high
  digitalWrite(3, HIGH);          // Tie rotary encoder interrupt pins high

  defaultmodetimer = millis();    // The important bit
  dhtpollcounter = millis();

}

// Store pretty icons to progmem
static unsigned char light_bits[] U8G_PROGMEM = {
   0x00, 0x3e, 0x00, 0x80, 0xff, 0x00, 0xc0, 0xc1, 0x01, 0x60, 0x00, 0x03,
   0x60, 0x20, 0x03, 0x30, 0x40, 0x06, 0x30, 0x40, 0x06, 0x30, 0x00, 0x06,
   0x30, 0x00, 0x06, 0x30, 0x00, 0x06, 0x60, 0x00, 0x03, 0xc0, 0x80, 0x01,
   0x80, 0x80, 0x00, 0x80, 0xc1, 0x00, 0x00, 0x41, 0x00, 0x00, 0x63, 0x00,
   0x00, 0x22, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x22, 0x00, 0x00, 0x3e, 0x00,
   0x00, 0x22, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x08, 0x00 };

static unsigned char audio_bits[] U8G_PROGMEM = {
   0x00, 0x00, 0x0e, 0x00, 0x80, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xf0, 0x09,
   0x00, 0xfe, 0x0c, 0x80, 0x3f, 0x0b, 0x80, 0xcf, 0x08, 0x80, 0x31, 0x08,
   0x80, 0x0f, 0x08, 0x80, 0x00, 0x18, 0x80, 0x00, 0x10, 0x80, 0x00, 0x10,
   0x80, 0x00, 0x10, 0x80, 0x01, 0x1c, 0x00, 0x01, 0x1f, 0x00, 0x81, 0x1f,
   0x00, 0xc1, 0x1f, 0xc0, 0xc1, 0x0f, 0xf0, 0x81, 0x07, 0xf8, 0x01, 0x00,
   0xfc, 0x01, 0x00, 0xfc, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00 };

static unsigned char temp_bits[] U8G_PROGMEM = {
   0x00, 0x1c, 0x00, 0x00, 0x26, 0x00, 0x00, 0x26, 0x00, 0x00, 0xe6, 0x00,
   0x00, 0x26, 0x00, 0x00, 0xe6, 0x01, 0x00, 0x26, 0x00, 0x00, 0xee, 0x00,
   0x00, 0x2e, 0x00, 0x00, 0xee, 0x01, 0x00, 0x2e, 0x00, 0x00, 0xee, 0x00,
   0x00, 0x2e, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x3e, 0x00,
   0x00, 0x63, 0x00, 0x80, 0xc1, 0x00, 0x80, 0x80, 0x00, 0x80, 0xc1, 0x00,
   0x80, 0xff, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x1c, 0x00 };

static unsigned char mvhr_bits[] U8G_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x6e, 0x00,
   0x00, 0x8f, 0x00, 0x00, 0x0f, 0x00, 0x80, 0x0f, 0x00, 0x80, 0x0f, 0x00,
   0x88, 0x8f, 0x03, 0x04, 0xcf, 0x0f, 0x04, 0xee, 0x1f, 0x00, 0xfc, 0x3f,
   0xfe, 0xff, 0x3f, 0xfe, 0x1f, 0x00, 0xfc, 0x3b, 0x10, 0xf8, 0x79, 0x10,
   0xe0, 0xf8, 0x08, 0x00, 0xf8, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x78, 0x00,
   0x80, 0x78, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00 };

static unsigned char other_bits[] U8G_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x20, 0xfb, 0xff, 0x6f,
   0xfb, 0xff, 0x6f, 0xf2, 0xff, 0x27, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07,
   0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07,
   0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x07,
   0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x36, 0x00,
   0x00, 0x22, 0x00, 0x00, 0x36, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00 };

static unsigned char alloff_bits[] U8G_PROGMEM = {
   0x80, 0x00, 0xc0, 0x01, 0xc0, 0x01, 0xd8, 0x0d, 0xdc, 0x1d, 0xc6, 0x31,
   0xc6, 0x31, 0x83, 0x60, 0x03, 0x60, 0x03, 0x60, 0x03, 0x60, 0x03, 0x60,
   0x06, 0x30, 0x06, 0x30, 0x1c, 0x1c, 0xf8, 0x0f, 0xe0, 0x03 };
   
static unsigned char degree_bits[] U8G_PROGMEM = {
   0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0x66, 0x3c };
   
static unsigned char play_bits[] U8G_PROGMEM = {
   0x01, 0x00, 0x07, 0x00, 0x0f, 0x00, 0x3f, 0x00, 0x7f, 0x00, 0xff, 0x01,
   0xff, 0x03, 0xff, 0x01, 0x7f, 0x00, 0x3f, 0x00, 0x0f, 0x00, 0x07, 0x00,
   0x01, 0x00 };

static unsigned char pause_bits[] U8G_PROGMEM = {
   0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00,
   0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00,
   0xcc, 0x00 };

// Define element sizes
#define icon_width 24           // Mode icon
#define icon_height 24          // Mode icon

#define alloff_width 15
#define alloff_height 17

#define playpause_width 10      // Play & pause icons
#define playpause_height 13     // Play & pause icons

#define degree_width 8          // Degree symbol (temp)
#define degree_height 7         // Degree symbol (temp)

// Define draw locations
#define icon_x_coord 5
#define icon_y_coord 13

#define alloff_x_coord 230
#define alloff_y_coord 14


#define light_statustext_x_coord 35
#define light_statustext_y_coord 39
#define audio_statustext_x_coord 35
#define audio_statustext_y_coord 39

#define major_text_x_coord 60
#define major_text_y_coord 26
#define minor_text_x_coord 60
#define minor_text_y_coord 40 // (was 39)

#define preset_y_coord 61        // i.e. change this if text height needs adjusting


/******************************************/
/*          CONTROL DRAWING               */
/*          ---------------               */
/******************************************/
// Cool fonts: profont10, 5x7, 6x12, helvB08, helvB10
void drawControl(void) {
  if (millis() - heartbeatsense > heartbeat_timeout) {
    u8g.drawStr(245, 10, ":("); // Draw sad face if no signal received
  }
  if (millis() - preset_stored_msg_timer < preset_stored_msg_timeout) {
      u8g.setFont(u8g_font_helvB12);
      u8g.drawStr(major_text_x_coord - 20, major_text_y_coord, "Preset stored");
  }
  else if (currentMode == diagsMode) {
    // Draw some stuff!

  }
  else if (millis() - defaultmodetimer > display_off_timeout) { 
      // Draw nothing - display off
     } else {

    // Draw icon, mode text, and ALL OFF
    u8g.setFont(u8g_font_5x8);
    int modeStringWidth = u8g.getStrWidth(modeText[currentMode-1]);
    u8g.drawStr(17-(modeStringWidth/2), 9, modeText[currentMode-1]);
    u8g.drawStr(220, 9, "ALL OFF");

    u8g.drawXBMP(alloff_x_coord, alloff_y_coord, alloff_width, alloff_height, alloff_bits);

    if (currentMode == lightMode) u8g.drawXBMP(icon_x_coord, icon_y_coord, icon_width, icon_height, light_bits);
    if (currentMode == audioMode) u8g.drawXBMP(icon_x_coord, icon_y_coord, icon_width, icon_height, audio_bits);
    if (currentMode == tempMode) u8g.drawXBMP(icon_x_coord, icon_y_coord, icon_width, icon_height, temp_bits);
    if (currentMode == mvhrMode) u8g.drawXBMP(icon_x_coord, icon_y_coord, icon_width, icon_height, mvhr_bits);
    if (currentMode == otherMode) u8g.drawXBMP(icon_x_coord, icon_y_coord, icon_width, icon_height, other_bits);
    // Draw state & value
    if (currentMode == audioMode) {
      if (play_status == true) {
        u8g.drawXBMP( 40, 13, playpause_width, playpause_height, play_bits);
      } else {
        u8g.drawXBMP( 40, 13, playpause_width, playpause_height, pause_bits);
      }
      u8g.setFont(u8g_font_6x10);
      sprintf (buff, "%3i%", volume);
      u8g.drawStr(audio_statustext_x_coord, audio_statustext_y_coord, buff);
    }
    else if (currentMode == lightMode) {
    }
    else if (currentMode == tempMode) {
      u8g.setFont(u8g_font_helvB18);
      //sprintf (buff, "%3i", temp);
      dtostrf(dht.readTemperature(), 4, 1, buff);
      u8g.drawStr( 22, 32, buff);
      u8g.drawXBMP( 80, 13, degree_width, degree_height, degree_bits);
    }
    else if (currentMode == mvhrMode) {
      // no idea what to put here yet
    }
    else if (currentMode == otherMode) {
      // no idea what to put here yet
    }

    // Draw major and minor text
    if (currentMode == lightMode) {
      // Write date and time
      u8g.setFont(u8g_font_helvB12);
      u8g.drawStr(major_text_x_coord - 20, major_text_y_coord, serverdatetime);

      // Write name of current circuit
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(minor_text_x_coord - 20, minor_text_y_coord, lightCircuit[currentCircuit-1].name);

      #define circuitSpacing 24

      // Display values
      int circuit_display_x_coord = minor_text_x_coord + 35;
      u8g.setFont(u8g_font_6x10);
      for (int i = 0; i < circuitCount; i++) {
        sprintf (buff, "%i", lightCircuit[i].level);
        u8g.drawStr (circuit_display_x_coord+(circuitSpacing/2) - (u8g.getStrWidth(buff)/2), minor_text_y_coord, buff);
        if (i == currentCircuit-1) { // indicate current circuit being controlled
          // dot -- u8g.drawStr (circuit_display_x_coord+(circuitSpacing/2) - (u8g.getStrWidth(buff)/2) - 7, minor_text_y_coord-3, ".");
          u8g.drawLine(circuit_display_x_coord+(circuitSpacing/2) - (u8g.getStrWidth(buff)/2), minor_text_y_coord+4, circuit_display_x_coord+(circuitSpacing/2) + (u8g.getStrWidth(buff)/2) -2, minor_text_y_coord+4);
        }
        circuit_display_x_coord = circuit_display_x_coord + circuitSpacing;
      }
    }



    else if (currentMode == audioMode) {
      u8g.setFont(u8g_font_helvB12);
      u8g.drawStr(major_text_x_coord, major_text_y_coord, track);
      u8g.setFont(u8g_font_helvB08);
      u8g.drawStr(minor_text_x_coord, minor_text_y_coord, artist);
    }
    
    else if (currentMode == tempMode) {
      u8g.setFont(u8g_font_helvB12);
      if (boiler_status) sprintf (buff, "%s%s", "Boiler: ", "ON");
        else sprintf (buff, "%s%s", "Boiler: ", "OFF");
      u8g.drawStr(major_text_x_coord, major_text_y_coord, buff);

      u8g.setFont(u8g_font_helvB08);
      u8g.drawStr(minor_text_x_coord, minor_text_y_coord, artist);
    }

    // Draw preset button labels
    u8g.setFont(u8g_font_5x8);
    int presetStringWidth;
    int presetXPos = 25;
    for (int i = 0; i < 5; i++) {
      if (currentMode == lightMode) {
        presetStringWidth = u8g.getStrWidth(lightScene[i].name);
        u8g.drawStr(presetXPos - (presetStringWidth /2), preset_y_coord, lightScene[i].name);
      } else if (currentMode == audioMode) {
        presetStringWidth = u8g.getStrWidth(audioFave[i].name);
        u8g.drawStr(presetXPos - (presetStringWidth /2), preset_y_coord, audioFave[i].name);
      } else if (currentMode == settingsMode) {
        presetStringWidth = u8g.getStrWidth(settingsNames[i]);
        u8g.drawStr(presetXPos - (presetStringWidth /2), preset_y_coord, settingsNames[i]);          
      }
      presetXPos = presetXPos + 51;
    } // end of for loop
    
    #define lux_val_x_coord 150
    #define lux_val_y_coord 30


    // Draw current room light level
    if (showlightlevel == 1) {
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(178, 30, "Currently");
      if (lux < 20) {
        sprintf (buff, "%i%s", lux, " (dark)");
      } else if (lux < 100) {
        sprintf (buff, "%i%s", lux, " (med dark)");
      } else if (lux < 400) {
        sprintf (buff, "%i%s", lux, " (med bright)");
      } else if (lux < 10000) {
        sprintf (buff, "%i%s", lux, " (bright)");
      }
      u8g.drawStr(178, 40, buff);
    }

    
  } // end of "if display is displaying stuff"
} // end of drawControl function

/******************************************/
/*               LOOP                     */
/*               ----                     */
/******************************************/


void loop()
{
  u8g.firstPage();  // START U8G
  do {              // START U8G
  drawControl();  // START U8G

  /******************************************/
  /*        touch sensor #prox              */
  /******************************************/

  // Get the currently touched pads
  currtouched = cap.touched();
  if (currtouched) defaultmodetimer = millis();
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  }
  // reset our state
  lasttouched = currtouched;

  /******************************************/
  /*        Take Light Readings             */
  /******************************************/
  if (millis() - lightSensorCounter > lightSensorDelay) {
    lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    lightSensorCounter = millis();
  }

  /******************************************/
  /*        Get values from server          */
  /******************************************/

  // If values are yet to be downloaded
  // e.g. if they have never been downloaded, or if they need to be downloaded again
  if (gotinfofromserver == false) {
    requestSettingsFromServer();
    requestValuesFromServer();
    // Insert code here to check if all settings / vals downloaded from server, if so...
    gotinfofromserver=true;
  }
  
  /******************************************/
  /*        Take DHT Readings               */
  /******************************************/
  if (millis() - dhtpollcounter > dhtpolldelay) {
//    float h = dht.readHumidity();
//    float t = dht.readTemperature();
//    float f = dht.readTemperature(true);
//    if (isnan(h) || isnan(t) || isnan(f)) {
//      Serial.println("Failed to read from DHT sensor!");
//      return;
//    }
//    float hi = dht.computeHeatIndex(f, h);
    dtostrf(dht.readTemperature(), 4, 1, buff);  //4 is mininum width, 3 is precision; float value is copied onto buff
    client.publish("home/bedroom/megacmd/temp", buff);
    dhtpollcounter = millis();
  }

  /******************************************/
  /*  Dim display, lights, sleep mode etc.  */
  /*  This needs a bit of tidying, was lazy */
  /******************************************/
    if (millis() - defaultmodetimer > button_leds_off_timeout) { // Dim the lights
      turnLEDs(0);
    } else {
      turnLEDs(1);
    }
    if (currentMode == diagsMode) {
      turnLEDs(1);
    }

    if (millis() - defaultmodetimer > display_dim_timeout) { // Dim the lights
      u8g.setColorIndex(1);         // min intensity
    } else {
      u8g.setColorIndex(3);         // max intensity
    }

  if (millis() - defaultmodetimer > default_mode_timeout) { // Default back to light control mode
      if (currentMode == diagsMode) {
        // Do nothing - stay in this mode forever
      }
      else if (currentMode != 1) {
        currentMode = 1;
      }
      if (currentCircuit != 1) {
        currentCircuit = 1;
      }
    }
    
  /******************************************/
  /*       Respond to button inputs         */
  /******************************************/
  btnSelect.Update();
  btnOrf.Update();
  btnTransport.Update();
  btnF1.Update();
  btnF2.Update();
  btnF3.Update();
  btnF4.Update();
  btnF5.Update();
  if (btnSelect.clicks == 1) buttonFunc ("cyclemodes");        // Cycle through modes
  if (btnTransport.clicks == 1) buttonFunc ("rotarySingle");   // Press rotary 
  if (btnTransport.clicks == 2) buttonFunc ("rotaryDouble");   // Double press rotary
  if (btnTransport.clicks == 3) buttonFunc ("rotaryTriple");   // Triple press rotary
  if (btnF1.clicks == 1) buttonFunc ("Button1");               // Press Favourite button 1
  if (btnF1.clicks == -1) buttonFunc ("Store1");               // Store Favourite 1
  if (btnF1.clicks == 2) buttonFunc ("enterSettings");            // Double press Fave 2 button to download from server again
  if (btnF2.clicks == 1) buttonFunc ("Button2");               // Press Favourite button 2
  if (btnF2.clicks == -1) buttonFunc ("Store2");               // Press Favourite button 2
  if (btnF3.clicks == 1) buttonFunc ("Button3");               // Press Favourite button 3
  if (btnF3.clicks == -1) buttonFunc ("Store3");               // Press Favourite button 3
  if (btnF4.clicks == 1) buttonFunc ("Button4");               // Press Favourite button 4
  if (btnF4.clicks == -1) buttonFunc ("Store4");               // Press Favourite button 4
  if (btnF5.clicks == 1) buttonFunc ("Button5");               // Press Favourite button 5
  if (btnF5.clicks == -1) buttonFunc ("Store5");               // Press Favourite button 5
  if (btnOrf.clicks == 1) buttonFunc ("Orf");                  // Press Favourite button 5

  /******************************************/
  /*       Respond to encoder inputs        */
  /******************************************/
  int newPosition = myEnc.read();
  if (newPosition > oldPosition) {
    oldPosition = newPosition;
    rotaryFunc("up");
    Serial.println("Up");
  }
  else if (newPosition < oldPosition) {
    oldPosition = newPosition;
    rotaryFunc("down");
    Serial.println("Down");
  }
  client.loop();
  } // END U8G
  while( u8g.nextPage() ); // END U8G
} // end of loop


/******************************************/
/*       Supporting Functions             */
/******************************************/

void buttonFunc(char* buttonName) // Function to handle button presses, including presets in various modes 
{
  defaultmodetimer = millis(); // reset last action time - this brings device out of sleep
  
  // Set the mode

  // Cycle through all modes except last two, settings and diags
  if (buttonName == "cyclemodes") {if (currentMode < 5) currentMode++; else currentMode = 1;}
  
  // Enter settings if double press happened. Diags mode is entered only through settings mode, so no
  // need for this to be done here
  if (buttonName == "enterSettings") currentMode = settingsMode;

  if (currentMode == lightMode) {
    if (buttonName == "Orf") client.publish(commandtopic, "Light0");
    if (buttonName == "Button1") client.publish(commandtopic, "Light1");
    if (buttonName == "Button2") client.publish(commandtopic, "Light2");
    if (buttonName == "Button3") client.publish(commandtopic, "Light3");
    if (buttonName == "Button4") client.publish(commandtopic, "Light4");
    if (buttonName == "Button5") client.publish(commandtopic, "Light5");
    if (buttonName == "Store1") {
    	client.publish(commandtopic, "StoreScene1");
    	preset_stored_msg_timer = millis();
    }
    if (buttonName == "Store2") {
    	client.publish(commandtopic, "StoreScene2");
    	preset_stored_msg_timer = millis();
    }
    if (buttonName == "Store3") {
    	client.publish(commandtopic, "StoreScene3");
    	preset_stored_msg_timer = millis();
    }
    if (buttonName == "Store4") {
    	client.publish(commandtopic, "StoreScene4");
    	preset_stored_msg_timer = millis();
    }
    if (buttonName == "Store5") {
    	client.publish(commandtopic, "StoreScene5");
    	preset_stored_msg_timer = millis();
    }
    if (buttonName == "rotarySingle") {if (currentCircuit < circuitCount) currentCircuit++; else currentCircuit = 1;}
    if (buttonName == "rotaryDouble"); // Do something here, e.g. toggle light state
  }
  if (currentMode == audioMode) {
    if (buttonName == "Button1") client.publish(commandtopic, "Audio1");
    if (buttonName == "Button2") client.publish(commandtopic, "Audio2");
    if (buttonName == "Button3") client.publish(commandtopic, "Audio3");
    if (buttonName == "Button4") client.publish(commandtopic, "Audio4");
    if (buttonName == "Button5") client.publish(commandtopic, "Audio5");
    if (buttonName == "rotarySingle") client.publish(commandtopic, "playpause");
    if (buttonName == "rotaryDouble") client.publish(commandtopic, "next");
    if (buttonName == "rotaryTriple") client.publish(commandtopic, "prev");
  }
  if (currentMode == tempMode) {
    if (buttonName == "Button1") ; // Do something here
    if (buttonName == "Button2") ; // Do something here
    if (buttonName == "Button3") ; // Do something here
    if (buttonName == "Button4") ; // Do something here
    if (buttonName == "Button5") ; // Do something here
  }
  if (currentMode == mvhrMode) {
    if (buttonName == "Button1") ; // Do something here
    if (buttonName == "Button2") ; // Do something here
    if (buttonName == "Button3") ; // Do something here
    if (buttonName == "Button4") ; // Do something here
    if (buttonName == "Button5") ; // Do something here
  }
  if (currentMode == otherMode) {
    if (buttonName == "Button1") ; // Do something here
    if (buttonName == "Button2") ; // Do something here
    if (buttonName == "Button3") ; // Do something here  
    if (buttonName == "Button4") ; // Do something here
    if (buttonName == "Button5") ; // Do something here
  }
  if (currentMode == settingsMode) {
    if (buttonName == "Button1") gotinfofromserver = false; // Re-fetch server data
    if (buttonName == "Button2") {     // Re-fetch server data ... slooowly (delay between requests, gives time for OH to respond
      gotinfofromserver = false;
    }
    if (buttonName == "Button3") currentMode = diagsMode;   // Go into diagnostic mode. (When in this mode, it stays forever until button press, this is set up in loop)
    if (buttonName == "Button4") resetFunc();     // Reboot
    if (buttonName == "Button5") {                // Exit
      currentCircuit = 1; 
      currentMode = 1;
    }
  }
}

void requestSettingsFromServer()
{
  client.publish(commandtopic, "circuit_count_please");                  // Request how many light circuits there are
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "circuit_names_please");                  // Request light circuit names
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "light_scene_1_name_please");             // Request light scene 1 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "light_scene_2_name_please");             // Request light scene 2 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "light_scene_3_name_please");             // Request light scene 3 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "light_scene_4_name_please");             // Request light scene 4 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "light_scene_5_name_please");             // Request light scene 5 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "audio_fave_1_name_please");              // Request audio fave 1 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "audio_fave_2_name_please");              // Request audio fave 2 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "audio_fave_3_name_please");              // Request audio fave 3 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "audio_fave_4_name_please");              // Request audio fave 4 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "audio_fave_5_name_please");              // Request audio fave 5 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "heat_profile_1_name_please");             // Request heating profile 1 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "heat_profile_2_name_please");             // Request heating profile 2 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "heat_profile_3_name_please");             // Request heating profile 3 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "heat_profile_4_name_please");             // Request heating profile 4 name
  turnLEDs(0);
  delay(mqttsettingsreqdelay);
  client.publish(commandtopic, "heat_profile_5_name_please");             // Request heating profile 5 name
  turnLEDs(1);
  delay(mqttsettingsreqdelay);

  defaultmodetimer = millis(); // reset last action time - this brings device out of sleep

}

void requestValuesFromServer()
{      client.publish(commandtopic, "datetime_please");           // Request date (one-off... OH to send new value at top of each minute)
  turnLEDs(0);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "light_scene_please");        // Request current light scene
  turnLEDs(1);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "light_levels_please");       // Request light circuit levels
  turnLEDs(0);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "volume_please");             // Request volume
  turnLEDs(1);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "play_status_please");        // Request play status
  turnLEDs(0);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "track_please");              // Request track
  turnLEDs(1);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "artist_please");             // Request album
  turnLEDs(0);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "node_settemp_please");       // Request current room temp
  turnLEDs(1);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "temp_target_please");        // Request room temp setting
  turnLEDs(0);
      delay(mqttvalreqdelay);
      client.publish(commandtopic, "mvhr_speed_please");         // Request MVHR speed seeting
  turnLEDs(1);
      delay(mqttvalreqdelay);
  defaultmodetimer = millis(); // reset last action time - this brings device out of sleep
}

void rotaryFunc(char direction[5]) // array with 5 elements to cope with the word "down" and a null terminator
{
  char directioncommand[13]; // to cope with direction array and the rest of the command (MQTT message)
  if (currentMode == lightMode) sprintf(directioncommand, "%s%i%s%s", "light_", currentCircuit, "_", direction);
  if (currentMode == audioMode) sprintf(directioncommand, "%s%s", "audio_", direction);
  client.publish(commandtopic, directioncommand);
  defaultmodetimer = millis(); // reset last action time
}

void flash(int pin)
{
  digitalWrite(pin, HIGH);
  delay(120);
  digitalWrite(pin, LOW);
}

void turnLEDs(int LEDstatus)
{
  if (LEDstatus == 1) {
    digitalWrite(ledMode, HIGH);
    digitalWrite(ledAllOff, HIGH);
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, HIGH);
    digitalWrite(led3Pin, HIGH);
    digitalWrite(led4Pin, HIGH);
    digitalWrite(led5Pin, HIGH);
  } else {
    digitalWrite(ledMode, LOW);
    digitalWrite(ledAllOff, LOW);
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, LOW);
    digitalWrite(led3Pin, LOW);
    digitalWrite(led4Pin, LOW);
    digitalWrite(led5Pin, LOW);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  heartbeatsense = millis();    // reset heartbeat sensor. i.e. "we are receiving messages, all's okay"
  payload[length] = '\0';

  // Controller Setup / paging functionality
  sprintf (buff, "%s/%s/%02X%02X%02X%02X%02X%02X", rootvector, setupvector, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  if(strcmp(topic, buff) == 0) {
    // ZONENAME Master Bedroom (ascii code for N = 78)
    // ZONEVECT home/lg_bedroom (ascii code for V = 86)
    // ZONEPMSG Time for dinner! (i.e. pager message)(ascii code for P = 80)
    // ZONESENS Sensitivity of proximity sensor. To allow fine tune adjustment of proximity from OpenHAB
    // we are taking payload from 9 chars in
    if (payload[4]==78) {
      for (int i=0; i<15; i++) zonename[i] = payload[i+9]; // zone name
    }
    if (payload[4]==86) {
      for (int i=0; i<15; i++) zonevect[i] = payload[i+9]; // zone vector
    }
    if (payload[4]==80) {
      for (int i=0; i<30; i++) zonepmsg[i] = payload[i+9]; // zone pager message
    }
    if (payload[4]==83) {                                  // capacitance adjustment value
      char capValRead[3];
      for (int i=0; i<2; i++) capValRead[i] = payload[i+9]; 
      capThreshAdjust = atoi( (const char*) capValRead);                   // zone controller proximity adjustment
    }
    if (payload[4]==82) {                                  // Remote reboot
      resetFunc();
      //digitalWrite(ardReset, LOW);
    }
    if (payload[4]==85) {                                  // Update controller (i.e. send MQTT requests to download settings / data)
      gotinfofromserver = false;
      defaultmodetimer = millis(); // reset last action time - this brings device out of sleep

    }
  }
  
  // perhaps here instead we should get all audio strings into one vector like this:
  // home/bedroom/audio -> artist: Jeith Karrett
  // home/bedroom/audio -> title: Sca Lala
  // think about OpenHAB performance implications of doing this
  
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, audiovector, titlevector);
  if(strcmp(topic, buff) == 0)                  // found a title
  {
    for (int i=0; i<trackLength+1; ++i)
    track[i] = payload[i];
/*
    titlelength = u8g.getStrWidth(track);

    Serial.print("Title length: ");
    Serial.println(titlelength);
    Serial.print("Title width: ");
 
    Serial.println(titlelength);
    scrollcounter = 200;
    x_pos = 3;
*/
  }
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, audiovector, artistvector); // home/bedroom/audio/artist
    if(strcmp(topic, buff) == 0) for (int i=0; i<artistLength+1; ++i) artist[i] = payload[i];
  sprintf (buff, "%s/%s", rootvector, datetimevector); // home/datetime/arduinodisplay
    if(strcmp(topic, buff) == 0) for (int i=0; i<17; ++i) serverdatetime[i] = payload[i];


  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, tempvector, currentvector); // home/bedroom/temp/current
  if(strcmp(topic, buff) == 0) temp = atoi( (const char*) payload );
  
  sprintf (buff, "%s/%s", rootvector, boilervector); // home/bedroom/temp/current
  if(strcmp(topic, buff) == 0) {
    if (payload[2]==78) {
      boiler_status=true;
    }
  }
  
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, audiovector, volumevector); // home/bedroom/audio/volume
  if(strcmp(topic, buff) == 0) volume = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, audiovector, playstatusvector); // home/bedroom/audio/playstatus
  if(strcmp(topic, buff) == 0) if (payload[0]==48) play_status = false; else play_status = true;

  // Settings
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuitcountvector); // home/bedroom/settings/circuit_count
  if(strcmp(topic, buff) == 0) circuitCount = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuit_1_namevector); // home/bedroom/settings/circuit_1_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightCircuit[0].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuit_2_namevector); // home/bedroom/settings/circuit_2_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightCircuit[1].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuit_3_namevector); // home/bedroom/settings/circuit_3_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightCircuit[2].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuit_4_namevector); // home/bedroom/settings/circuit_4_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightCircuit[3].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuit_5_namevector); // home/bedroom/settings/circuit_5_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightCircuit[4].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, circuit_6_namevector); // home/bedroom/settings/circuit_6_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightCircuit[5].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, light_scene_1_namevector); // home/bedroom/settings/light_scene_1_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightScene[0].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, light_scene_2_namevector); // home/bedroom/settings/light_scene_2_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightScene[1].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, light_scene_3_namevector); // home/bedroom/settings/light_scene_3_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightScene[2].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, light_scene_4_namevector); // home/bedroom/settings/light_scene_4_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightScene[3].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, light_scene_5_namevector); // home/bedroom/settings/light_scene_5_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) lightScene[4].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, audio_fave_1_namevector); // home/bedroom/settings/audio_fave_1_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) audioFave[0].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, audio_fave_2_namevector); // home/bedroom/settings/audio_fave_2_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) audioFave[1].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, audio_fave_3_namevector); // home/bedroom/settings/audio_fave_3_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) audioFave[2].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, audio_fave_4_namevector); // home/bedroom/settings/audio_fave_4_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) audioFave[3].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, settingsvector, audio_fave_5_namevector); // home/bedroom/settings/audio_fave_5_name
  if(strcmp(topic, buff) == 0) for (int i=0; i<nameLength+1; ++i) audioFave[4].name[i] = payload[i];
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, lightvector, circuit1levelvector); // home/bedroom/light/circuit1level
  if(strcmp(topic, buff) == 0) lightCircuit[0].level = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, lightvector, circuit2levelvector); // home/bedroom/light/circuit2level
  if(strcmp(topic, buff) == 0) lightCircuit[1].level = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, lightvector, circuit3levelvector); // home/bedroom/light/circuit3level
  if(strcmp(topic, buff) == 0) lightCircuit[2].level = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, lightvector, circuit4levelvector); // home/bedroom/light/circuit4level
  if(strcmp(topic, buff) == 0) lightCircuit[3].level = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, lightvector, circuit5levelvector); // home/bedroom/light/circuit5level
  if(strcmp(topic, buff) == 0) lightCircuit[4].level = atoi( (const char*) payload );
  sprintf (buff, "%s/%s/%s/%s", rootvector, zonevect, lightvector, circuit6levelvector); // home/bedroom/light/circuit6level
  if(strcmp(topic, buff) == 0) lightCircuit[5].level = atoi( (const char*) payload );
}

void startupState(int state) {
  #define status_x 155
  #define msg_x 40

  u8g.firstPage();  // START U8G
  do {
      u8g.setFont(u8g_font_helvB12);
      u8g.drawStr(15, 15, F("Welcome to casa Mat & Anna")); // Welcome to casa Mat & Anna!
      u8g.setFont(u8g_font_6x12);

      if (state == 1) {
          u8g.drawStr(msg_x, 30, F("Resetting W5100... "));
      }
      if (state == 2) {
        u8g.drawStr(msg_x, 30, F("Resetting W5100... DONE"));
        u8g.drawStr(msg_x, 40, F("Testing LEDs... "));
      }
      if (state == 3) {
        u8g.drawStr(msg_x, 30, F("Resetting W5100... DONE"));
        u8g.drawStr(msg_x, 40, F("Testing LEDs...    DONE"));
        u8g.drawStr(msg_x, 50, F("MAC Address..."));
        u8g.drawStr(msg_x, 60, F("Getting IP..."));
        u8g.drawStr(status_x, 50, macString);
      }
      if (state == 4) {
        sprintf (ipString, "%i.%i.%i.%i", Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
        u8g.drawStr(msg_x, 30, F("Testing LEDs...    DONE"));
        u8g.drawStr(msg_x, 40, F("MAC Address..."));
        u8g.drawStr(msg_x, 50, F("Getting IP..."));
        u8g.drawStr(msg_x, 60, F("Subscribing MQTT..."));
        u8g.drawStr(status_x, 40, macString);
        u8g.drawStr(status_x, 50, ipString);
      }
      if (state == 40) {
        u8g.drawStr(msg_x, 30, F("Testing LEDs...    DONE"));
        u8g.drawStr(msg_x, 40, F("MAC Address..."));
        u8g.drawStr(msg_x, 50, F("Getting IP..."));
        u8g.drawStr(msg_x, 60, F("Subscribing MQTT..."));
        u8g.drawStr(status_x, 40, macString);
        u8g.drawStr(status_x, 50, ipString);
        delay(10000);
        // onwards and upwards! (i.e. continue even though we are clearly failing miserably...)
      }
      if (state == 5) {
        u8g.drawStr(msg_x, 30, F("MAC Address..."));
        u8g.drawStr(msg_x, 40, F("Getting IP..."));
        u8g.drawStr(msg_x, 50, F("Subscribing MQTT...DONE"));
        u8g.drawStr(msg_x, 60, F("Getting Zone..."));
        u8g.drawStr(status_x, 30, macString);
        u8g.drawStr(status_x, 40, ipString);
        sprintf(buff, "(%ims)", mqttresponsetime); // #sram save 8 bytes here and below
        u8g.drawStr(status_x + 30, 50, buff);
      }
      if (state == 6) {
        u8g.drawStr(msg_x, 30, F("MAC Address..."));
        u8g.drawStr(msg_x, 40, F("Getting IP..."));
        u8g.drawStr(msg_x, 50, F("Subscribing MQTT...DONE"));
        u8g.drawStr(msg_x, 60, F("Getting Zone..."));
        u8g.drawStr(status_x, 30, macString);
        u8g.drawStr(status_x, 40, ipString);
        u8g.drawStr(status_x, 60, zonename);
        sprintf(buff, "(%ims)", mqttresponsetime);
        u8g.drawStr(status_x + 30, 50, buff);
      }

  } // END U8G
  while (u8g.nextPage()); // END U8G
}

