/*
   Auteur : G.Menez
   Fichier : esp_asyncweb_spiffs.ino
   Many sources :
  => https://raw.githubusercontent.com/RuiSantosdotme/ESP32-Course/master/code/WiFi_Web_Server_DHT/WiFi_Web_Server_DHT.ino
  => https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
  => Kevin Levy
*/

/* FAIRE METHODE POUR CONSTRUIRE JSON + SEND PAGE HTML APRES LE POST*/
/*====== Import required libraries ===========*/
#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include "AsyncJson.h"
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
short int light_threshold = 250; // Less => night, more => day
short int sbn = 19 ;   //seuil bas nuit
short int shn = 20 ;   // seuil haut nuit
short int sbj = 21 ;    // seuil bas jour
short int shj = 22 ;    // seuil haut jour

// Host for periodic data report
String target_ip = "";
int target_port = 1880;
int target_sp = 0; // Remaining time before the ESP stops transmitting
int lastTime = 0;

// Initialisation du timer
int timer = 0;

// Initialisation des variables

bool sendingData = false;
WiFiClient client;
HTTPClient http;
String longitude = "7.055° E";
String latitude = "43.6164° N";
bool cooler = false;
bool heater = false;
String ssid = "";
String esp_mac = "";
String esp_ip = "";
int tempValue = 0;
String light = "";

/*====== Some functions =====================*/

String processor(const String& var) {
  /* Replaces "placeholder" in  html file with sensors values */
  /* accessors functions get_... are in sensors.ino file   */
  if (var == "TEMPERATURE") {
    return String(tempValue);
  }
  else if(var == "LIGHT") {
    return light;
  }
  else if(var == "UPTIME") {
    return String(timer);
  }
  else if(var == "SSID") {
    return ssid;
  }
  else if(var == "MAC") {
    return esp_mac;
  }
  else if(var == "IP") {
    return esp_ip;
  }
  else if(var == "COOLER") {
    return cooler? "ON":"OFF";
  }
  else if (var == "HEATER") {
    return heater? "ON":"OFF";
  }
  else if(var == "LT") {
    return String(light_threshold);
  }
  else if(var == "SBJ") {
    return String(sbj);
  }
  else if(var == "SHJ") {
    return String(shj);
  }
  else if(var == "SBN") {
    return String(sbn);
  }
  else if(var == "SHN") {
    return String(shn);
  }
  else if(var == "WHERE") {
    return latitude+", "+longitude;
  }
  return String();
}

void setup_http_server() {
  /* Sets up AsyncWebServer and routes */

  // Declaring root handler, and action to be taken when root is requested
  auto root_handler = server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* This handler will download statut.html (SPIFFS file) and will send it back */
    request->send(SPIFFS, "/statut.html", String(), false, processor);
    // cf "Respond with content coming from a File containing templates" section in manual !
    // https://github.com/me-no-dev/ESPAsyncWebServer
    // request->send_P(200, "text/html", page_html, processor); // if page_html was a string .
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with temperature value */
    request->send_P(200, "text/plain", get_temperature(TempSensor).c_str());
  });

  server.on("/light", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with light value */
    request->send_P(200, "text/plain", get_light(LightPin).c_str());
  });

  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with light value */
    request->send_P(200, "text/plain", String(timer).c_str());
  });

  // This route allows users to change thresholds values through GET params
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* A route with a side effect : this get request has a param and should
        set a new light_threshold ... used for regulation !
    */
    if (request->hasArg("light_threshold")) {
      light_threshold = atoi(request->arg("light_threshold").c_str());
      request->send_P(200, "text/plain", "Threshold Set !");
    } else if(request->hasArg("sbn")) {
      sbn = atoi(request->arg("sbn").c_str());
      request->send_P(200, "text/plain", "sbn Set !");
    } else if(request->hasArg("shn")) {
      shn = atoi(request->arg("shn").c_str());
      request->send_P(200, "text/plain", "shn Set !");
    } else if(request->hasArg("sbj")) {
      sbj = atoi(request->arg("sbj").c_str());
      request->send_P(200, "text/plain", "sbj Set !");
    } else if(request->hasArg("shj")) {
      shj = atoi(request->arg("shj").c_str());
      request->send_P(200, "text/plain", "shj Set !");
    }
  });

  server.on("/target", HTTP_POST, [](AsyncWebServerRequest * request) {
    /* A route receiving a POST request with Internet coordinates
        of the reporting target host.
    */
    Serial.println("Receive Request for a ""target"" route !");
    if (request->hasArg("ip") &&
        request->hasArg("port") &&
        request->hasArg("sp")) {
      target_ip = request->arg("ip");
      target_port = atoi(request->arg("port").c_str());
      target_sp = atoi(request->arg("sp").c_str());
      sendingData = true;
      request->send(sendDataToServer());
    }
  });

  server.on("/value", HTTP_GET, [](AsyncWebServerRequest * request) {
    StaticJsonDocument<256> jdoc_sent;
    Serial.println("Receive Request for a ""set"" route !");
    int paramsNumber = request->params();
    AsyncWebParameter* p;
    for (int i = 0; i < paramsNumber; i++) {
      p = request->getParam(i);
      if (p->name() == "temperature") {
        jdoc_sent["temperature"] = tempValue;
      } else if (p->name() == "light") {
        jdoc_sent["light"] = light;
      } else if (p->name() == "cooler") {
        jdoc_sent["cooler"] = cooler;
      } else if (p->name() == "heater") {
        jdoc_sent["heater"] = heater;
      } else if (p->name() == "ip") {
        jdoc_sent["ip"] = target_ip;
      } else if (p->name() == "port") {
        jdoc_sent["port"] = target_port;
      } else if (p->name() == "sp") {
        jdoc_sent["sp"] = target_sp;
      } else if (p->name() == "light_threshold") {
        jdoc_sent["light_threshold"] = light_threshold;
      } else if (p->name() == "sbn") {
        jdoc_sent["sbn"] = sbn;
      } else if (p->name() == "shn") {
        jdoc_sent["shn"] = shn;
      } else if (p->name() == "sbj") {
        jdoc_sent["sbj"] = sbj;
      } else if (p->name() == "shj") {
        jdoc_sent["shj"] = shj;
      }  else if (p->name() == "ssid") {
        jdoc_sent["ssid"] = ssid;
      } else if (p->name() == "mac") {
        jdoc_sent["mac"] = esp_mac;
      } else if (p->name() == "ip_esp") {
        jdoc_sent["ip_esp"] = esp_ip;
      } else if (p->name() == "uptime") {
        jdoc_sent["uptime"] = timer;
      } else if (p->name() == "where") {
        jdoc_sent["where"] = "45";
      } else {
        request->send(404);
      }
    }
    char payload[256];
    serializeJson(jdoc_sent, payload);
    request->send(200, "application/json", payload);
  });

  // If request doesn't match any route, returns 404.
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });

  // Start server
  server.begin();
}

void setVariables() {
  timer++;
  tempValue = atoi(get_temperature(TempSensor).c_str());
  light = get_light(LightPin);
  if (light_threshold < 250) {
    if (tempValue >= shn) {
      cooler = true;
      heater = false;
    } else if ( tempValue < shn && tempValue > sbn) {
      cooler = false;
      heater = false;
    } else if (tempValue <= sbn) {
      cooler = false;
      heater = true;
    }
  } else {
    if (tempValue >= shj) {
      cooler = true;
      heater = false;
    } else if ( tempValue < shj && tempValue > sbj) {
      cooler = false;
      heater = false;
    } else if (tempValue <= sbj) {
      cooler = false;
      heater = true;
    }
  }
  ssid = WiFi.SSID();
  esp_mac = WiFi.macAddress();
  esp_ip = WiFi.localIP().toString();
}

int sendDataToServer() {
  StaticJsonDocument<256> jdoc_sent;
  char payload[256];
  jdoc_sent["temperature"] = tempValue;
  jdoc_sent["light"] = light;
  jdoc_sent["cooler"] = cooler;
  jdoc_sent["heater"] = heater;
  jdoc_sent["sp"] = target_sp;
  jdoc_sent["light_threshold"] = light_threshold;
  jdoc_sent["sbn"] = sbn;
  jdoc_sent["shn"] = shn;
  jdoc_sent["sbj"] = sbj;
  jdoc_sent["shj"] = shj;
  jdoc_sent["uptime"] = timer;
  serializeJson(jdoc_sent, payload);
  http.begin("http://" + target_ip + ":" + target_port + "/");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);
  http.end();
  return httpResponseCode;
}

/*---- Arduino IDE paradigm : setup+loop -----*/
void setup() {
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

  setVariables();
}


void loop() {
  ArduinoOTA.handle();
  setVariables();
  if (target_sp == 0) {
    sendingData = false;
  } else if (sendingData && (millis() - lastTime) > (target_sp * 1000)) {
    sendDataToServer();
    lastTime = millis();
  }

  // Use this new temp for regulation updating ?
  delay(1000);

}
