#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "S8";
const char *password = "Valami234";

const long utcOffsetInSeconds = 7200;


char fogadott =0;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup(){
  Serial.begin(300);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {

      if (Serial.available() > 0) 
      {
                // read the incoming byte:
                fogadott = Serial.read();
      
      if (fogadott == 'A')
        {
        timeClient.update();
        Serial.print(timeClient.getHours());
        delay(50);
        Serial.print(timeClient.getMinutes());
        delay(50);
        Serial.println(timeClient.getSeconds());
        delay(50);
        
        }
      }
}
