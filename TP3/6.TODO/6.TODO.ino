/*********
 Based on Rui Santos work :
 https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
 Modified by GM
*********/
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "ArduinoJson.h"
#include "classic_setup.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "sensors.h"


/*===== MQTT broker/server ========*/
const char* mqtt_server = "192.168.1.39"; 
//const char* mqtt_server = "public.cloud.shiftr.io"; // Failed in 2021
// need login and passwd (public,public) mqtt://public:public@public.cloud.shiftr.io
//const char* mqtt_server = "broker.hivemq.com"; // anynomous Ok in 2021 
//const char* mqtt_server = "test.mosquitto.org"; // anynomous Ok in 2021
//const char* mqtt_server = "mqtt.eclipseprojects.io"; // anynomous Ok in 2021

/*===== MQTT TOPICS ===============*/
#define TOPIC_SENSORS "esp/sensors" // 
#define TOPIC_FIRE_EMERGENCY  "emergency/fire"  // Topic pour communication entre ESP et pompiers
#define TOPIC_FIREFIGHTERS_EMERGENCY "emergency/firefighters"
#define TOPIC_COMMAND_CENTER "command/center" // Topic pour communication entre ESP et centre de commande hors sensors


/*===== ESP is MQTT Client =======*/
WiFiClient espClient;           // Wifi 
PubSubClient client(espClient); // MQTT client

/*============= GPIO ==============*/
const int ledPin = 19; // LED Pin

/* ---- TEMP ---- */
OneWire oneWire(23); // Pour utiliser une entite oneWire sur le port 23
DallasTemperature tempSensor(&oneWire) ; // Cette entite est utilisee par le capteur de temperature
const int LightPin = A5;


String temperature = "";
String light = "";
boolean isOnFire = false;
String state = "NORMAL";
String location = "5eme etage";
boolean HelpOnTheWay = false;
const char* emergencyStatus = "";
StaticJsonDocument<256> jdoc;
char jpayload[256];

/*===== Arduino IDE paradigm : setup+loop =====*/
void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for a serial connection. Needed for native USB port only
   
  connect_wifi(); // Connexion Wifi  
  print_network_status();
  
  // Initialize the output variables as outputs
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);// Set outputs to LOW
  
  // Init temperature sensor 
  tempSensor.begin();

  // set server of our client
  client.setServer(mqtt_server, 1883);
  // set callback when publishes arrive for the subscribed topic
  client.setCallback(mqtt_pubcallback); 
}

/*============= TO COMPLETE ===================*/
void set_LED(int v){
  digitalWrite(ledPin, v);
}

void blinding_LED(){
  if(digitalRead(ledPin)){
     set_LED(LOW);
  } else {
     set_LED(HIGH);
  }
}

/*============== CALLBACK ===================*/
void mqtt_pubcallback(char* topic, 
                      byte* message, 
                      unsigned int length) {
  /* 
   * Callback if a message is published on this topic.
   */
  Serial.print("Message arrived on topic : ");
  Serial.println(topic);

  StaticJsonDocument<256> doc;
  deserializeJson(doc, message, length);

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic,
  // you check if the message is either "normal" or "onFire".
  // Changes the output state according to the message

   if (String(topic) == TOPIC_FIREFIGHTERS_EMERGENCY) {
    Serial.print("Message received from firefighters ");
    if (doc["status"] == "en route" && doc["state"] == "fire") {
      HelpOnTheWay = true;
      emergencyStatus = doc["status"];
      createJsonCommandCenter();
      client.publish(TOPIC_COMMAND_CENTER,jpayload);
    } else if(doc["status"] == "sur place" && doc["state"] == "fire"){
      emergencyStatus = doc["status"];
      createJsonCommandCenter();
      client.publish(TOPIC_COMMAND_CENTER,jpayload);
    } else if(doc["status"] == "retour en caserne" && doc["state"] == "normal"){
      emergencyStatus = doc["status"];
      state = "NORMAL";
      createJsonCommandCenter();
      client.publish(TOPIC_COMMAND_CENTER,jpayload);
      set_LED(LOW);
      HelpOnTheWay = false;
      emergencyStatus = "";
    }
   }
}

void createJsonSensors(){
   jdoc.clear();
   jdoc["temperature"] = temperature;
   jdoc["light"] = light;
   jdoc["location"] = location;
   jdoc["ip"] = WiFi.localIP().toString();
   serializeJson(jdoc, jpayload);
}

void createJsonEmergency(){
   jdoc.clear();
   jdoc["emergency"] = "FIRE EMERGENCY";
   jdoc["location"] = location;
   jdoc["ip"] = WiFi.localIP().toString();
   serializeJson(jdoc, jpayload);
}

void createJsonCommandCenter(){
   jdoc.clear();
   jdoc["state"] = state;
   jdoc["emergencyStatus"] = emergencyStatus;
   jdoc["location"] = location;
   jdoc["ip"] = WiFi.localIP().toString();
   serializeJson(jdoc, jpayload);
}

// On utilise la lumière pour détecter un feu, car plus complexe de changer la température 
void detectFire(){
  if(light.toInt()>2200){
    state = "ONFIRE";
  }
}

// Si un feu est détecté ou si le centre de commande prévient d'un incendie, l'esp envoit ses coordonnées au pompiers
void callForHelp(){
    createJsonEmergency();
    client.publish(TOPIC_FIRE_EMERGENCY, jpayload);
}

/*============= SUBSCRIBE =====================*/
void mqtt_mysubscribe(char *topic) {
  /*
   * Subscribe to a MQTT topic 
   */
  while (!client.connected()) { // Loop until we're reconnected

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect => https://pubsubclient.knolleary.net/api
    if (client.connect("esp32", /* Client Id when connecting to the server */
		                    NULL,    /* No credential */ 
		                    NULL)) {
      Serial.println("connected");
      // then Subscribe topic
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

/*================= LOOP ======================*/
void loop() {
  int32_t period = 5000; // 5 sec
  unsigned long startTime = 0;
  startTime = millis();

  while(millis() < startTime + period){
    delay(1000);
    if(state == "ONFIRE"){
    blinding_LED();
  }
  }
  /*--- subscribe to TOPIC_LED if not yet ! */
  if (!client.connected()) { 
    mqtt_mysubscribe((char *)(TOPIC_FIREFIGHTERS_EMERGENCY));
  }
  /*--- Publish Temperature periodically   */
  light = get_light(LightPin);
  temperature = get_temperature(tempSensor);
  // Serial info
 // Serial.print("Published Temperature : "); Serial.println(temperature);
  //Serial.print("Published Light : "); Serial.println(light);
  Serial.println(state);
  // MQTT Publish
  createJsonSensors();
  client.publish(TOPIC_SENSORS,jpayload);
  detectFire();
  if(state == "ONFIRE"){
    if(!HelpOnTheWay){
      callForHelp();
    }
    blinding_LED();
  }


  /* Process MQTT ... une fois par loop() ! */
  client.loop(); 
}
