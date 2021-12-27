#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "secrets.h"

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;

void conectaWiFi(){
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.println("Conectando-se ao Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Conectado com o IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup() {
 Serial.begin(115200);
 conectaWiFi();
 Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); //gps baud
}

void atualizaLocalizacaoAtualAPI (double latitude, double longitude){
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin("http://192.168.101:80/api/viagens/61c91ba095d669011a03c25f");
    http.addHeader("Content-Type", "application/json");
    
    char httpRequestData[128];
    sprintf(httpRequestData, "{\"lat\": \"%f\", \"long\": \"%f\"}", latitude, longitude);
    
    int httpResponseCode = http.PUT(httpRequestData);
    String payload = http.getString();
    
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void obtemLocalizacaoAtual(){
  bool recebido = false;
  while (Serial1.available()) {
     char cIn = Serial1.read();
     recebido = gps.encode(cIn);
  }
  
  if (gps.location.isUpdated() && gps.altitude.isUpdated()){
    Serial.print("D/M/A: ");
    Serial.print(gps.date.value());
    Serial.print(" | alt: ");
    Serial.print(gps.altitude.feet());
    Serial.print(" | satellites: ");
    Serial.println(gps.satellites.value());

    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    Serial.println("Latitude");
    Serial.println(latitude, 6);
    Serial.println("Longitude");
    Serial.println(longitude, 6);

    atualizaLocalizacaoAtualAPI(latitude, longitude);
    
    delay(15000);
  }
}

void loop() {
  obtemLocalizacaoAtual();
 
}
