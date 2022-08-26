#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <Arduino_JSON.h>
#include "secrets.h"

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;

// API Settings
#define IP "192.168.0.101"
#define PORT "3001"
#define BASE_API "http://" IP ":" PORT "/api/trips"

#define BUS_ID "63081a39555fc889e645454d"

bool travelInProgress = false;
char travel_id[25] = "";

void connectToWiFi(){
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
  connectToWiFi();
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); //gps baud
}

void sendCurrentLocationToAPI (double lat, double lng, double speed){
  Serial.println("Enviando localização atual do ônibus para o servidor.");
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    char endpoint[128];
    sprintf(endpoint, "%s/%s/currentVehicleLocation", BASE_API, travel_id);
    
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    
    char httpRequestData[128];
    sprintf(httpRequestData, "{\"lat\": \"%f\", \"lng\": \"%f\", \"speed\": \"%f\"}", lat, lng, speed);

    Serial.print("Payload: ");
    Serial.println(httpRequestData);

    Serial.print("POST: ");
    Serial.println(endpoint);
    int httpResponseCode = http.POST(httpRequestData);
    
    Serial.print("Status: ");
    Serial.println(httpResponseCode);

    http.end(); // Free resources

    if(httpResponseCode > 0){
      if(httpResponseCode == 403){
        Serial.println("Viagem encerrada.");
        travelInProgress = false;
        strncpy(travel_id, "", sizeof(travel_id));
      } else if (httpResponseCode != 200){
        Serial.println("O servidor não retornou um status de sucesso para a requisição!");
      }
    } else {
      Serial.println("Falha ao enviar requisição.");
    }
  } else {
    Serial.println("Desconectado do Wi-Fi");
    connectToWiFi();
  }
  Serial.println();
}

void getCurrentLocation(){
  //Serial.println("Obtendo localização atual do ônibus.");
  while (Serial1.available()) {
     gps.encode(Serial1.read());
  }
  
  if (gps.location.isUpdated()){
    double lat = gps.location.lat();
    double lng = gps.location.lng();
    double speed = gps.speed.kmph();

    sendCurrentLocationToAPI(lat, lng, speed);
  } /*else {
    Serial.println("O ônibus não está se movimentando!");
  }*/
}

void getTravelInProgress(){
  Serial.print("Buscando viagem em andamento para o ônibus com ID: ");
  Serial.println(BUS_ID);
  
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    char endpoint[128];
    sprintf(endpoint, "%s/inProgress/%s", BASE_API, BUS_ID);
    
    http.begin(endpoint);

    Serial.print("GET: ");
    Serial.println(endpoint);
    int httpResponseCode = http.GET();
    String payload = http.getString();
    
    Serial.print("Status: ");
    Serial.println(httpResponseCode);

    http.end(); // Free resources

    if(httpResponseCode > 0){
      if(httpResponseCode == 200){
        JSONVar travel = JSON.parse(payload);
    
        if (JSON.typeof(travel) != "undefined" && travel["_id"] != null){
          travelInProgress = true;
          strncpy(travel_id, travel["_id"], sizeof(travel_id));
          Serial.print("Viagem em andamento com esse ônibus identificada. ID: ");
          Serial.println(travel_id);
        } else {
          Serial.println("Nenhuma viagem em andamento com esse ônibus foi encontrada.");
        }
      } else {
        Serial.println("O servidor não retornou um status de sucesso para a requisição!");
      }
    } else {
      Serial.println("Falha ao enviar requisição.");
    }
  } else {
    Serial.println("Desconectado do Wi-Fi");
    connectToWiFi();
  }
  Serial.println();
}

void loop() {
  if (travelInProgress){
    getCurrentLocation();
  } else {
    getTravelInProgress();
  }
  
  delay(5000);
}
