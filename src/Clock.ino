/*
 * Project Clock
 * Description:
 * Author:
 * Date:
 */

#include "application.h"
#include "LiquidCrystal/LiquidCrystal.h"

//LiquidCrystal lcd(A1, A0, D4, D5, D6, D7, D0, D1, D2, D3);
LiquidCrystal lcd(A1, A0, D4, D5, D6, D7);

// Sync every 3 hours
#define SYNC_INTERVAL (60 * 60 * 1000 * 3)

int tempPin = A2;
double temp = 0.0;
double tempF = 0.0;
int tempVoltage = 0;

#define TS_WAITING 0
#define TS_PUBLISHED 1
#define TS_RECEIVED 2
#define TS_SYNC_WAIT 3
#define TS_SYNC_COMPLETE 4

int ts_state = TS_WAITING;
int offset = -5;
String tzName = "EST";

void timezoneHandler(const char *event, const char *data) {

  char* token = strtok(strdup(data)," ");
  offset = (atoi(token) / 3600);
  token = strtok(NULL, " ");
  tzName = token;

  ts_state = TS_RECEIVED;
  Serial.print("Set offset to ");
  Serial.println(offset, DEC);

  Serial.println("Set tzName to " + tzName);
}

void syncTime() {
  String data = "";
  switch (ts_state) {
    case TS_WAITING:
      Serial.println("Sent timezone request.");
      Particle.publish("timezone", data, PRIVATE);
      ts_state = TS_PUBLISHED;
      break;
    case TS_PUBLISHED:
      // handler changes state to TS_RECEIVED, do nothing here
      Serial.println("Waiting for timezone API hook...");
      break;
    case TS_RECEIVED:
      Serial.println("Received time zone, requesting sync.");
      //Time.zone(offset);
      Particle.syncTime();
      ts_state = TS_SYNC_WAIT;
      break;
    case TS_SYNC_WAIT:
      if (Particle.syncTimePending()) {
	Serial.println("Waiting for Particle sync");
      } else {
        Serial.println("Received sync from Particle");
        ts_state = TS_SYNC_COMPLETE;
      }
      break;
    case TS_SYNC_COMPLETE:
      ts_state = TS_WAITING;
      Serial.println("Synchronized time to " + Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
      delay(250);
      break;
  }
}

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  lcd.clear();
  lcd.begin(16,2);
  lcd.print("Initializing...");
  syncTime();

  pinMode(tempPin, INPUT);
  Particle.variable("tempVoltage", &tempVoltage, INT);
  Particle.variable("temp", &temp, DOUBLE);
  Particle.variable("tempF", &tempF, DOUBLE);

  Particle.subscribe("hook-response/timezone",timezoneHandler,MY_DEVICES);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  unsigned long lastSync = Particle.timeSyncedLast();
  if(millis() - lastSync > SYNC_INTERVAL) {
    Serial.println("Last sync " + Time.format(lastSync, TIME_FORMAT_ISO8601_FULL));
    syncTime();
  }

  if(lastSync = 0) {
    lcd.setCursor(0,0);
    lcd.print("Syncing time...");
    lcd.setCursor(0,1);
    lcd.print("                ");
  }

  tempVoltage = analogRead(tempPin);
  temp = (((tempVoltage * 3.3)/4095) - 0.5) * 100;
  tempF = ((temp * 9.0) / 5.0) + 32.0;
  String strTemp = (String)((int)tempF);

  Time.zone(offset);
  int currentTime = Time.now();
  String strDOW = Time.format(currentTime,"%a");
  String strDate = Time.format(currentTime, "%Y-%m-%d");
  String strTime = Time.format(currentTime, "%H:%M:%S");

  lcd.setCursor(0,0);
  lcd.print(strDOW + " " + strDate + "  ");
  lcd.setCursor(0,1);
  lcd.print(strTime + " " + tzName + " " + strTemp + (char)223);
  delay(100);
}
