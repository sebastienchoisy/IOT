#include "OneWire.h"
#include "DallasTemperature.h"
#include "ArduinoJson.h"
#define TRUE 1
#define FALSE 0
char receivedChar;
String receivedStr;
String test;
int sensorValue;
int jour = FALSE;
float val = 0.0;
  
OneWire oneWire(23);
DallasTemperature tempSensor(&oneWire);
const int ledPin = 18; 
const int ledPin2 = 19; 
  
void setup(void)
{
  
  Serial.begin(9600);
  tempSensor.begin();
  tempSensor.requestTemperaturesByIndex(0);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}
  
void loop(void)
{
  StaticJsonDocument<256> jdoc_sent;
  int lightSensorValue;
  lightSensorValue = analogRead(A5);
  jdoc_sent["temperature"]= tempSensor.getTempCByIndex(0);
  jdoc_sent["intensity"]= analogRead(A5);
  char payload[256];
  serializeJson(jdoc_sent,payload);
  Serial.println(payload);
  
  if(Serial.available()) {
    StaticJsonDocument<256> jdoc_received;
    while(Serial.available()>0){
       String s = Serial.readStringUntil('}');
       deserializeJson(jdoc_received,s);
       if(jdoc_received["redLed"]== "HIGH"){
         digitalWrite(ledPin2, HIGH);
       }
       else if(jdoc_received["yellowLed"]== "HIGH"){
        digitalWrite(ledPin, HIGH);
       }
       else if(jdoc_received["redLed"]== "LOW"){
         digitalWrite(ledPin2, LOW);
       }
       else if(jdoc_received["yellowLed"]== "LOW"){
        digitalWrite(ledPin, LOW);
      }
    }
  delay(1000);
  }
}
