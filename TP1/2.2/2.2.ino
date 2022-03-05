#define TRUE 1
#define FALSE 0
char receivedChar;
String receivedStr;
int sensorValue;
int jour = FALSE;
float val = 0.0;

void setup() {
  Serial.begin(9600);
  Serial.println('<Serial is read>');
}

void loop() {
  sensorValue = analogRead(A5);
  Serial.print(sensorValue , DEC);
  Serial.print('\n');

  while(Serial.available()>0) {
    receivedStr = Serial.readStringUntil('\n');
    Serial.print("\nI received : ");
    Serial.println(receivedStr);

    val = receivedStr.toFloat();
    Serial.println(val);
  }
  delay(1000);
}
