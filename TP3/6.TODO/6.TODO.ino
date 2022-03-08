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
#include "WiFiClientSecure.h"

const char* CA_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDETCCAfmgAwIBAgIUKQr9bBxBGhOJGTgca9H5HE0fWCAwDQYJKoZIhvcNAQEL\n" \
"BQAwGDEWMBQGA1UEAwwNVHJ1c3RlZENBLm5ldDAeFw0yMjAzMDcxODM5NTNaFw0y\n" \
"MzAzMDcxODM5NTNaMBgxFjAUBgNVBAMMDVRydXN0ZWRDQS5uZXQwggEiMA0GCSqG\n" \
"SIb3DQEBAQUAA4IBDwAwggEKAoIBAQC1w7Bt3psB1bQSlOJRz2cLmsEAJqv2cEsz\n" \
"E+7PFX/UjC6n8FShL+cGtbND0Yb0++GyApJ/POVAiBePs7Z3yyBMeZ5hnu7sdGhN\n" \
"ue+Wacn5shftbl/WXkY6QfuFXM7PdNl9nf7zXkArCCwNo1K0gnYC/ibxEooiClib\n" \
"54jB3ae5ASlCaq038AmANp6WCLFIEFMi89R15S4shnHLinFN+4QKBtbLkWRgEYV8\n" \
"8IpyKQx7+ki8CA3BsmwHSuNtNSiajMFiuRZu/hl/rI5H5iUHC8WiWk3IIxb6syMf\n" \
"riA6xd6gSMfkSJrtiBH8QHLSA1xMiLtTGRwgsQ8QxIS4LIuWs7DLAgMBAAGjUzBR\n" \
"MB0GA1UdDgQWBBSAxD5hfRV077bZ43D9YWhtCxoQrjAfBgNVHSMEGDAWgBSAxD5h\n" \
"fRV077bZ43D9YWhtCxoQrjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCNyFXNz8+6WrtIfGv68N27jI1FU3PNnj0Yu4SeD1FrpYL2MojGPiEgP1Gs\n" \
"MNTsv+mWwYqZgiNH76zVBQ5yT0ndGHxiM/TII8Yh71dD54xByYK94I2nAxDWZh5f\n" \
"tUFpwBJHfdZvIpcNYxFfXHG5uMXsXyXkc03qMqPxO3BNeJbFelGja/kPWFkTjYJ8\n" \
"ew0mp55zbsTt1nRm224UOYld/B7dlU+DL/iwx/7g00s7fX8VlVRjlaFauXSH1QvB\n" \
"Dx6IXtrrs53E5ekC/ZlKVee0NYGD6nHwwSEREWilVcRxDO5qnAvnZcQUsg26w4uX\n" \
"eWOv7f78XBB7sM2nz6SdqjIlMMch\n" \
"-----END CERTIFICATE-----";

const char* ESP_CA_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIC7zCCAdcCFDjVVV2MeXpsNoR68t6Xm6Lz0NKBMA0GCSqGSIb3DQEBCwUAMBgx\n" \
"FjAUBgNVBAMMDVRydXN0ZWRDQS5uZXQwHhcNMjIwMzA3MjMyMTE0WhcNMjMwMzA3\n" \
"MjMyMTE0WjBQMQswCQYDVQQGEwJGUjETMBEGA1UECAwKY290ZSBkYXp1cjEPMA0G\n" \
"A1UEBwwGU29waGlhMQ4wDAYDVQQKDAVNSUFHRTELMAkGA1UECwwCTTEwggEiMA0G\n" \
"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCqmMbeZaqv3qaqaW3wvHSPSU4nYYyl\n" \
"zkPe7lwTmlwp35twXlfkq+A6SouxVHIfFCXSibYobu32zdaaL82GMHvGAMbclQQb\n" \
"hERaCKgSsEBdx9qbQ+GSKIhrx4M/9rrsAntbnV+KMshs16fqYjO1fEPr8WB1kFzA\n" \
"BzsG6RFVH5F28vTTIwZaQvUMWOOGPbpit0pP82juUPjRf+rZCHuEhXXK9bUzebS/\n" \
"e2/4F/FjsdomKGmvrM7n2BR/WtQo6zmWahz+zDonpWFBD4SWoJHAwY9UlXVbVldt\n" \
"tF1sI+t0eWsgqo0Y/s/j6HCM1H/E51XhIRCgMRs1nDIqU5zl7LycQTU/AgMBAAEw\n" \
"DQYJKoZIhvcNAQELBQADggEBALDbQJxfpx7IzBlBZU3bfZ5Qgj8ABB8ZDpE+ORhw\n" \
"Tn4Smus3w7N6wTrDuXqH7uZvhafWWkLc/Huxp+bWHr4I0Qon20YGtcEQvlz4BQ9v\n" \
"TTMpUJbLf5AN6KVRAwqOkfoqSs4DCCaf4ZbM18JiYHbYKP6DLZPwxrwtsQC44lMn\n" \
"Y65UBkuzprD9FnAdgBi7mP4p1qciWwfTTT6142ieVOto5VBy5NiVhXOuptUwiD1Z\n" \
"DNlXeya6A5T/3SciTySc7SrJNujdvT7V8nq3OpgscGiGyUfGMeszj0X0EL5ZaGhs\n" \
"BmIzpcf1UpExQ1cqNm2dDCZifQvtypze0k1wdDrtBWy08VA=\n" \
"-----END CERTIFICATE-----";

const char* ESP_RSA_key= \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEowIBAAKCAQEAqpjG3mWqr96mqmlt8Lx0j0lOJ2GMpc5D3u5cE5pcKd+bcF5X\n" \
"5KvgOkqLsVRyHxQl0om2KG7t9s3Wmi/NhjB7xgDG3JUEG4REWgioErBAXcfam0Ph\n" \
"kiiIa8eDP/a67AJ7W51fijLIbNen6mIztXxD6/FgdZBcwAc7BukRVR+RdvL00yMG\n" \
"WkL1DFjjhj26YrdKT/No7lD40X/q2Qh7hIV1yvW1M3m0v3tv+BfxY7HaJihpr6zO\n" \
"59gUf1rUKOs5lmoc/sw6J6VhQQ+ElqCRwMGPVJV1W1ZXbbRdbCPrdHlrIKqNGP7P\n" \
"4+hwjNR/xOdV4SEQoDEbNZwyKlOc5ey8nEE1PwIDAQABAoIBAH1XswZEfJyrYpli\n" \
"zTxLuvhsS9fzfHt2ibeKOmzMxVqeyH5PUxTYk+9ARlMMUK01CHUgaY5bssvbgQnY\n" \
"hZFKniZk+7emZVYyYGYmZhcVGLXO2Xeq8FmLHnDSLkUY603spOYzQrcviMMv0FgN\n" \
"vyBhJSZ+OvkmEH+m0F/Lbsi0YRSbSkhJvV9U3MLDoURovaZ0gihARl3O4OLgB++/\n" \
"C6VZcIp+nVGicOIRRk1GlLOUPI2O4yv47raPH3kcBZ8BduwDlrhAkoQgUbV9HrrN\n" \
"v/1nSjc+eIlOOu9oirGEuuPqZ933ji/pen0JGmu1KLHwhGSJaszZjE2497AMoGcc\n" \
"O2rUGkECgYEA3lL2q4BrD3nzXb+ucyUMRpJoqNJr8ddJi/X3xsPEc8eodLz8erbI\n" \
"HU2maxNxdWAW+gBBYXw8ms7DEwBoX+GjeyAAmZ4ksKC6hcuN/ijFNxp8mXg3mZfS\n" \
"4JehlLCjQ1NyGy1VzLOqZ3Xs3xn8teobMmsiHmbjMibth8OSLB25Hh0CgYEAxG/9\n" \
"XpaLviWOHoH8yYVHyMQJVTx9IOmeuqwTqk8qoEDkfxxzQsWH8DZDPUZLR/hSl2UY\n" \
"LZUhSpsNkypCgrL56U24+75FElV7YRXHBl7pYJ0QvIrUixCZV5qKhi9JZaQjPfMF\n" \
"H7w9D2aycHDwYCPc/4/BkryTdUS/WtMByWg0cgsCgYEA1OyEDfCpmGLlb43a17BN\n" \
"t3EVB0qv0yiq6X6i1Rw9E93OOi+dZzGTKm1pyvcOHKnwl3KTK3g+2szXC2XCkMGb\n" \
"PpqBuJ466Wal/OncjeQiLQoS9zCpn1JGxo6yU6NwWVSb3ednWutfckMAnxNWcitJ\n" \
"7NmqDY/gW/HoX7QwVEwLNQkCgYAnib7+teCEsjxryZAe47OyJjsWq+knob4wJoiC\n" \
"xBJ5VPZo20oby3R0FgfsomRupP3CuTdnnVepE5Jk56GhevKwUdOE4CQ473m47dA0\n" \
"aRAiQRroSC1FnF6w8kslfQkR6zqosmx6mcHUgsAxv5myUCWgTsgQh+JKPT8iwcvf\n" \
"RRkhRwKBgBowQ1++N0TceUJvnTf+jwXgtXCu5fvddu6jwYR+FA705g9wn00/t24n\n" \
"rPZ55Lu768O63tTqVpBvLNDA4LBAcLYWLtO4qm2ABEXZZodfPu5D+v8E/GqXdXMg\n" \
"PgQt40ZboM71OhDa6C/8S9abVcijwU1JbxahQjZD2mVwwFLXKS6s\n" \
"-----END RSA PRIVATE KEY-----";

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
WiFiClientSecure espClient;           // Wifi 
PubSubClient client(espClient); // MQTT client
const char* mqtt_user = "esp32";
const char* mqtt_pass = "esp32";

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
  espClient.setCACert(CA_cert);
  espClient.setCertificate(ESP_CA_cert);
  espClient.setPrivateKey(ESP_RSA_key);
  client.setServer(mqtt_server, 8883);
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
  while (!espClient.connected()) { // Loop until we're reconnected

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect => https://pubsubclient.knolleary.net/api
    if (client.connect("esp32", /* Client Id when connecting to the server */
		                    mqtt_user,    /* No credential */ 
		                    mqtt_pass)) {
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
