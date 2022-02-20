/* 
 * Auteur : G.Menez
 * Fichier : esp_asyncweb_spiffs.ino 
 * Many sources :
 => https://raw.githubusercontent.com/RuiSantosdotme/ESP32-Course/master/code/WiFi_Web_Server_DHT/WiFi_Web_Server_DHT.ino
 => https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 => Kevin Levy 
*/
/*====== Import required libraries ===========*/
#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "classic_setup.h"
#include "sensors.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "SPIFFS.h"
#include <HTTPClient.h>

void setup_OTA(); // from ota.ino

/*===== ESP GPIO configuration ==============*/
/* ---- LED         ----*/
const int LEDpin = 19; // LED will use GPIO pin 19
/* ---- Light       ----*/
const int LightPin = A5; // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
/* ---- Temperature ----*/
OneWire oneWire(23); // Pour utiliser une entite oneWire sur le port 23
DallasTemperature TempSensor(&oneWire) ; // Cette entite est utilisee par le capteur de temperature

/*====== ESP Statut =========================*/
// Ces variables permettent d'avoir une representation
// interne au programme du statut "electrique" de l'objet.
// Car on ne peut pas "interroger" une GPIO pour lui demander !
String LEDState = "off";

/*====== Process configuration ==============*/
// Set timer 
unsigned long loop_period = 10L * 1000; /* =>  10000ms : 10 s */

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Thresholds
short int Light_threshold = 250; // Less => night, more => day
short int sbn = 19 ;   //seuil bas nuit
short int shn = 20 ;   // seuil haut nuit
short int sbj = 21 ;    // seuil bas jour
short int shj = 22 ;    // seuil haut jour

// Host for periodic data report
String target_ip = "";
int target_port = 1880;
int target_sp = 0; // Remaining time before the ESP stops transmitting

// Initialisation du timer
int timer = 0;

/*====== Some functions =====================*/

String processor(const String& var){
  /* Replaces "placeholder" in  html file with sensors values */
  /* accessors functions get_... are in sensors.ino file   */
  //Serial.println(var);
  //int temperatureValue = atoi(get_temperature(TempSensor).c_str());
  //Serial.println(temperatureValue);
  if(var == "TEMPERATURE"){
    return get_temperature(TempSensor);
  }
  else if(var == "LIGHT"){
    return get_light(LightPin);
  }
  else if(var == "UPTIME"){
    return String(timer);
  }
 else if(var == "SSID"){
    return WiFi.SSID();
  }
  else if(var == "MAC"){
    return WiFi.macAddress();
  }
  else if(var == "IP"){
    return WiFi.localIP().toString();
  }
  else if(var == "COOLER"){
    return atoi(get_temperature(TempSensor).c_str()) >= 18? "true" : "false";
  }
  else if(var == "HEATER"){
    return atoi(get_temperature(TempSensor).c_str()) < 18? "true" : "false";
  }
  else if(var == "LT"){
    return String(Light_threshold);
  }
  else if(var == "SBJ"){
    return String(sbj);
  }
  else if(var == "SHJ"){
   return String(shj);
  }
  else if(var == "SBN"){
    return String(sbn);
  }
  else if(var == "SHN"){
    return String(shn);
  }
  return String();
}

void setup_http_server() {
  /* Sets up AsyncWebServer and routes */
  
  // Declaring root handler, and action to be taken when root is requested
  auto root_handler = server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        /* This handler will download statut.html (SPIFFS file) and will send it back */
        request->send(SPIFFS, "/statut.html", String(), false, processor); 
        // cf "Respond with content coming from a File containing templates" section in manual !
        // https://github.com/me-no-dev/ESPAsyncWebServer
        // request->send_P(200, "text/html", page_html, processor); // if page_html was a string .
  });
    
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    /* The most simple route => hope a response with temperature value */ 
    request->send_P(200, "text/plain", get_temperature(TempSensor).c_str());
  });

  server.on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
    /* The most simple route => hope a response with light value */ 
    request->send_P(200, "text/plain", get_light(LightPin).c_str());
  });

  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request){
    /* The most simple route => hope a response with light value */ 
    request->send_P(200, "text/plain", String(timer));
  });


  // This route allows users to change thresholds values through GET params
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
    /* A route with a side effect : this get request has a param and should     
     *  set a new light_threshold ... used for regulation !
     */
        if (request->hasArg("light_threshold")) {
            Light_threshold = atoi(request->arg("light_threshold").c_str());
            request->send_P(200, "text/plain", "Threshold Set !");
        }
  });

  server.on("/target", HTTP_POST, [](AsyncWebServerRequest *request){
    /* A route receiving a POST request with Internet coordinates 
     *  of the reporting target host.
     */
     Serial.println("Receive Request for a ""target"" route !"); 
     HTTPClient http;
     String server = "http://192.168.179.243:1880";
     WiFiClient client;
        if (request->hasArg("ip") &&
        request->hasArg("port") &&
        request->hasArg("sp")) {
            target_ip = request->arg("ip");
            target_port = atoi(request->arg("port").c_str());
            target_sp = atoi(request->arg("sp").c_str());
        }
        http.begin(client,server);
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");
        request->send(SPIFFS, "/statut.html", String(), false, processor);
    });
    
  // If request doesn't match any route, returns 404.
  server.onNotFound([](AsyncWebServerRequest *request){
                   request->send(404);
  });

  // Start server
  server.begin();
}

/*---- Arduino IDE paradigm : setup+loop -----*/
void setup(){
  Serial.begin(9600);
  while (!Serial); // wait for a serial connection. Needed for native USB port only

  // Connexion Wifi   
  connect_wifi(); 
  print_network_status();

  // OTA
  setup_OTA();
    
  // Initialize the LED 
  setup_led(LEDpin, OUTPUT, LOW);
  
  // Init temperature sensor 
  TempSensor.begin();

  // Initialize SPIFFS
  SPIFFS.begin(true);

  setup_http_server();
}
 
void loop(){  
  int tempValue;
  
  ArduinoOTA.handle();
  
  tempValue = atoi(get_temperature(TempSensor).c_str());

  // Use this new temp for regulation updating ?

  
  delay(1000);
  timer++;
}
