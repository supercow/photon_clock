/*
 * Project Clock
 * Description:
 * Author:
 * Date:
 */

#include "application.h"
#include "LiquidCrystal/LiquidCrystal.h"
#include "rest_client.h"

//LiquidCrystal lcd(A1, A0, D4, D5, D6, D7, D0, D1, D2, D3);
LiquidCrystal lcd(A1, A0, D4, D5, D6, D7);

int lastSync = 0;
int syncInterval = 5;

int tempPin = A2;
double temp = 0.0;
double tempF = 0.0;
int tempVoltage = 0;
//RestClient timezone = RestClient("freegeoip.net");
RestClient timezone = RestClient("104.31.251.10");
//RestClient tzOffset = RestClient("",);

void csvToArray(char* csv) {
  
}

String getTimezone() {
  Serial.println("Getting timezone...");
  String resp = "";
  int status = timezone.get("/csv", &resp);
  Serial.println("REST response: " + resp);
  if (status != 200) {
    Serial.println("Failed to retrieve timezone: " + status);
    return "";
  } else {
    Serial.println("Got timezone " + resp);
    return resp;
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
