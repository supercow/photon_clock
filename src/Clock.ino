/*
 * Project Clock
 * Description:
 * Author:
 * Date:
 */

#include "application.h"
#include "LiquidCrystal/LiquidCrystal.h"
#include "rest_client.h"
#include "jsmnSpark.h"

//LiquidCrystal lcd(A1, A0, D4, D5, D6, D7, D0, D1, D2, D3);
LiquidCrystal lcd(A1, A0, D4, D5, D6, D7);

int lastSync = 0;
int syncInterval = 5;

int tempPin = A2;
double temp = 0.0;
double tempF = 0.0;
int tempVoltage = 0;
RestClient timezone = RestClient("api.timezonedb.com");

String parseResponse(String json) {
  jsmn_parser p;
  jsmn_init(&p);
  int max_tokens = 32;
  jsmntok_t tok[max_tokens];
  status = jsmn_parse(&p, json, tok, max_tokens);

  if (status != JSMN_SUCCESS) {
    Serial.println("Failed to parse json response");
    return "";
  } else {
    for(i = 0; i < max_tokens; i++) {
      if ((jsoneq(JSON_STRING, &tok[i], "status") == 0) {
    }
  }
}

String getTimezone() {
  Serial.println("Getting timezone...");
  String resp = "";
  int status = timezone.get("/v2/get-time-zone?key=JUE3CZJK6PRH&format=json&by=zone&zone=America/New_York&fields=gmtOffset,dst", &resp);
  Serial.println("REST response: " + resp);
  if (status != 200) {
    Serial.println("Failed to retrieve timezone: " + status);
    return "";
  } else {
    String parsed = parseResponse();
    return parsed;
  }
}

void syncTime() {
  getTimezone();
  Particle.syncTime();
  lastSync = Time.now();
  Serial.println("Synchronized time to " + Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
}

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  lcd.clear();
  lcd.begin(16,2);
  lcd.print("Initializing...");
  syncTime();
  Time.zone(-5);

  pinMode(tempPin, INPUT);
  Particle.variable("tempVoltage", &tempVoltage, INT);
  Particle.variable("temp", &temp, DOUBLE);
  Particle.variable("tempF", &tempF, DOUBLE);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if (Time.now() > lastSync + syncInterval) {
    syncTime();
  }

  tempVoltage = analogRead(tempPin);
  temp = (((tempVoltage * 3.3)/4095) - 0.5) * 100;
  tempF = ((temp * 9.0) / 5.0) + 32.0;
  String strTemp = (String)((int)tempF);

  int currentTime = Time.now();
  String strDOW = Time.format(currentTime,"%a");
  String strDate = Time.format(currentTime, "%Y-%m-%d");
  String strTime = Time.format(currentTime, "%H:%M:%S");

  lcd.setCursor(0,0);
  lcd.print(strDOW + " " + strDate + "  ");
  lcd.setCursor(0,1);
  lcd.print(strTime + "  " + strTemp + (char)223 + "F");
  delay(100);
}
