#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
HTTPClient http;
const uint8_t fingerprint[20] = {0xBF, 0x16, 0xAE, 0x79, 0xD2, 0xAB, 0x71, 0x44, 0xBE, 0xD8, 0xE7, 0x55, 0xA2, 0xC7, 0x0B, 0x39, 0x68, 0xDB, 0xB5, 0xD2};
String apiKey = "21RMB77JNIKFPU1T";
const char* ssid = "PK";
const char* password = "111222333";
const String url = "https://notify-api.line.me/api/notify";
const String TOKEN = "HerQnLo9kojniXHIIilHAdjsQhs4nAtFvChclfRP576";
//const String TOKEN = "";

const String messageRain = "%E0%B8%9B%E0%B8%A3%E0%B8%B0%E0%B8%95%E0%B8%B9%201%20%E0%B8%9D%E0%B8%99%E0%B8%95%E0%B8%81";
const String messagePIR = "%E0%B8%A1%E0%B8%B5%E0%B8%9C%E0%B8%B9%E0%B9%89%E0%B8%9A%E0%B8%B8%E0%B8%81%E0%B8%A3%E0%B8%B8%E0%B8%81";


ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(D1, INPUT);

 

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
}

void loop() {

    
  int rainsensor = analogRead(A0);
  int pirsensor  = digitalRead(D1);
  Serial.println(rainsensor);
  Serial.println(pirsensor);

  
if ((WiFiMulti.run() == WL_CONNECTED)) {
  
    Serial.println(rainsensor);
    http.begin("http://api.thingspeak.com/update?api_key=" + apiKey + "&field1=" + String(rainsensor) 
                + "&field2=" + String(pirsensor) );
    Serial.println("Finish Connect");
    
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    Serial.println("Finish Send Data Thingspeak");
  }
  
else if(analogRead(rainsensor) < 500)
    sentNotify(messageRain);
else if(digitalRead(pirsensor) == 1 )
     sentNotify(messagePIR);
 
  delay(1000);
}

void sentNotify(String msg) {
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, url)) {  // HTTPS

      https.addHeader("Authorization", "Bearer " + TOKEN);
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");

      Serial.print("[HTTPS] POST\n");
      int httpCode = https.POST("message=" + msg);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  delay(1000);
}
