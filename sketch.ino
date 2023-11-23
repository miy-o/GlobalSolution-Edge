#include <LiquidCrystal.h>
#include "ArduinoJson.h"
#include "EspMQTTClient.h"

//Setup do Wifi
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
EspMQTTClient client{
  "Wokwi-GUEST", // SSID do WiFi
  "",             // Senha do WiFi
  "mqtt.tago.io", // Endereço do servidor MQTT
  "Default",      // Usuário
  "SEU TOKEN",   // Token do dispositivo
  "esp",          // Nome do dispositivo
  1883            // Porta de comunicação
};

int q;
double bp;
double kp;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  Serial.println("Conectando WiFi");
  while (!client.isWifiConnected()) {
    Serial.print('.');
    client.loop();
    delay(1000);
  }
  Serial.println("WiFi Conectado");

  Serial.println("Conectando com Servidor MQTT");
  while (!client.isMqttConnected()) {
    Serial.print('.');
    client.loop();
    delay(1000);
  }
  Serial.println("MQTT Conectado");
}

void loop() {
  q = analogRead(A0);
  bp = (double)q / 1024;
  bp = bp * 250;

  if (bp > 60) {
    kp = bp - 40;
  } else {
    kp = bp;
  }

  lcd.setCursor(0, 0);
  lcd.print("PS: ");
  lcd.print(bp);
  lcd.print("/");
  lcd.print(kp);
  delay(1000);

  if (bp <= 90 && kp <= 60) {
    lcd.setCursor(0, 1);
    lcd.print("Baixa");
    delay(1000);
  } else if ((bp >= 90 && kp >= 60) && (bp <= 120 && kp <= 80)) {
    lcd.setCursor(0, 1);
    lcd.print("Normal");
    delay(1000);
  } else if (bp > 140 && kp > 90) {
    lcd.setCursor(0, 1);
    lcd.print("Alta");
    delay(1000);
  }

  lcd.clear();
  delay(1000);

  // Criar um documento JSON para os dados
  StaticJsonDocument<200> documentoJson;
  documentoJson["PressaoSanguinea"] = bp;
  documentoJson["Kp"] = kp;

  // Serializar o JSON para uma string
  char bufferJson[256];
  serializeJson(documentoJson, bufferJson);

  // Publicar os dados no tópico MQTT
  client.publish("topicoTDSPI", bufferJson);
  delay(5000);

  // Mantém a conexão MQTT ativa
  client.loop();
}