#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "secrets.h"

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;

#define IP "192.168.43.73"
#define PORT "80"
#define BASE_API "http://" IP ":" PORT "/api/travels/tracking"
char currentTravel_id[25] = "61cf55263a4b45ed19e30616";

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

/*char* getEndpoint(){
    char endpoint[128];
    sprintf(endpoint, "%s/%s", BASE_API, currentTravel_id);
    return endpoint;
}*/

void sendCurrentLocationToAPI (double lat, double lng, double speed){
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    char endpoint[128];
    sprintf(endpoint, "%s/%s", BASE_API, currentTravel_id);
    
    Serial.println(endpoint);
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    
    char httpRequestData[128];
    sprintf(httpRequestData, "{\"lat\": \"%f\", \"lng\": \"%f\", \"speed\": \"%f\"}", lat, lng, speed);
    
    int httpResponseCode = http.PUT(httpRequestData);
    //String payload = http.getString();
    
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
    //Serial.print(" | Payload: ");
    //Serial.println(payload);
    
    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected");
    conectaWiFi();
  }
}

void getCurrentLocation(){
  bool recebido = false;
  while (Serial1.available()) {
     char cIn = Serial1.read();
     recebido = gps.encode(cIn);
  }
  
  if (gps.location.isUpdated() && gps.altitude.isUpdated()){
    //Serial.print("D/M/A: ");
    //Serial.print(gps.date.value());
    //Serial.print(" | alt: ");
    //Serial.print(gps.altitude.feet());
    //Serial.print(" | satellites: ");
    //Serial.println(gps.satellites.value());

    double lat = gps.location.lat();
    double lng = gps.location.lng();
    double speed = gps.speed.kmph();
    double course = gps.course.deg();
    
    Serial.print("lat: ");
    Serial.print(lat, 6);
    Serial.print(" | lng: ");
    Serial.print(lng, 6);
    Serial.print(" | speed: ");
    Serial.print(speed, 6);
    //Serial.print(" | Date: ");
    //Serial.print(gps.date.value());
    //Serial.print(" | Time: ");
    //Serial.print(gps.time.value());
    Serial.println();

    sendCurrentLocationToAPI(lat, lng, speed);
    
    delay(10000);
  }
}

void loop() {
  getCurrentLocation();
}
