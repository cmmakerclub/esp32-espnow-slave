#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Wire.h>
#include "SSD1306.h"
#include <tcpip_adapter.h>
#if CONFIG_STATION_MODE
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
#endif

#define PG_Button 0

uint8_t current_protocol;
esp_interface_t check_protocol();

esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;
esp_now_peer_info_t slave;

uint32_t sentCnt = 0;
uint8_t peerCount = 0;
int packetRecv = 0;
uint8_t master_mac[6] = {0x30, 0xAE, 0xA4, 0x99, 0x0C, 0xEC};

SSD1306 display(0x3c, 4, 15);
char sta_mac[18];
char softap_mac[18];

void setup()
{
  pinMode(PG_Button, INPUT_PULLUP);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.flipScreenVertically();

  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");
  check_protocol();
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_STA); // must be WIFI_STA; WIFI_AP doesn't work
  esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);

  delay(10);
  memset(&slave, 0, sizeof(slave));
  check_protocol();
  // WiFi.disconnect();
  if (esp_now_init() == ESP_OK)
  {
    Serial.printf("ESPNow Init Success\n");
  }
  else
  {
    Serial.printf("ESPNow Init Failed\n");
    ESP.restart();
  }

  esp_now_register_send_cb([&](const uint8_t *mac_addr, esp_now_send_status_t status) {
    sentCnt++;
    Serial.println("SEND CB..");
  });

  esp_now_register_recv_cb([&](const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    packetRecv++;
    Serial.println("RECV CB..");
  });
  //For each Slave
  //We create a variable that will store the slave information
  //We inform the channel
  // slave.channel = CHANNEL;
  //0 not to use encryption or 1 to use
  slave.encrypt = 0;
  //Copies the array address to the structure
  memcpy(slave.peer_addr, master_mac, sizeof(master_mac));
  //Add the slave
  esp_err_t addStatus = esp_now_add_peer(&slave);
  if (addStatus == ESP_OK)
  {
    // Pair success
    Serial.println("Pair success");
  }
  else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT)
  {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  }
  else if (addStatus == ESP_ERR_ESPNOW_ARG)
  {
    Serial.println("Invalid Argument");
  }
  else if (addStatus == ESP_ERR_ESPNOW_FULL)
  {
    Serial.println("Peer list full");
  }
  else if (addStatus == ESP_ERR_ESPNOW_NO_MEM)
  {
    Serial.println("Out of memory");
  }
  else if (addStatus == ESP_ERR_ESPNOW_EXIST)
  {
    Serial.println("Peer Exists");
  }
  else
  {
    Serial.println("Not sure what happened");
  }

  strcpy(sta_mac, WiFi.macAddress().c_str());
  strcpy(softap_mac, WiFi.softAPmacAddress().c_str());
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "**ESP32-NOW**");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 24, "STA MAC :");
  display.drawString(18, 34, sta_mac);
  display.drawString(0, 44, " AP MAC :");
  display.drawString(18, 54, softap_mac);
  display.display();
  delay(2000);
  display.clear();
}

uint32_t pevMillis = 0;

void loop()
{
  uint8_t data = 1;

  if (digitalRead(PG_Button) == 0)
  {
    display.clear();
    delay(200);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "**ESP32-NOW**");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 24, "STA MAC :");
    display.drawString(18, 34, sta_mac);
    display.drawString(0, 44, " AP MAC :");
    display.drawString(18, 54, softap_mac);
    display.display();
  }

  uint32_t curMillis = millis();
  if (curMillis - pevMillis > 5000 && digitalRead(PG_Button) == 1)
  {
    pevMillis = curMillis;

    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "**ESP32-NOW**");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 24, "Status : ");
    esp_err_t result = esp_now_send(slave.peer_addr, &data, sizeof(data));
    if (result == ESP_OK)
    {
      Serial.println("Success");
      display.drawString(40, 24, "Success");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
      display.drawString(40, 24, "ESPNOW not Init.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
      Serial.println("Invalid Argument");
      display.drawString(40, 24, "Invalid Argument");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
      Serial.println("Internal Error");
      display.drawString(40, 24, "Internal Error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      display.drawString(40, 24, "ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
      Serial.println("Peer not found.");
      display.drawString(40, 24, "Peer not found.");
    }
    else
    {
      Serial.println("Not sure what happened");
      display.drawString(40, 24, "Not sure what happened");
    }

    display.display();
  }

  // const uint8_t *peer_addr = slave.peer_addr;
  // esp_err_t result = esp_now_send(peer_addr, &data, sizeof(data));
}

esp_interface_t check_protocol()
{
  char error_buf1[100];

  tcpip_adapter_get_esp_if(&current_esp_interface);
  if (current_esp_interface == ESP_IF_WIFI_STA)
    Serial.println("Interface is ESP_IF_WIFI_STA");
  else if (current_esp_interface == ESP_IF_WIFI_AP)
    Serial.println("Interface is ESP_IF_WIFI_AP");
  else
    Serial.println("Unknown interface!!");
  current_wifi_interface = current_esp_interface;
  if (current_wifi_interface == WIFI_IF_STA)
    Serial.println("Interface is WIFI_IF_STA");
  else if (current_wifi_interface == WIFI_IF_AP)
    Serial.println("Interface is WIFI_IF_AP");
  else
    Serial.println("Unknown interface!!");
  esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
  esp_err_to_name_r(error_code, error_buf1, 100);
  Serial.print("esp_wifi_get_protocol error code: ");
  Serial.println(error_buf1);
  Serial.print("Current protocol code is ");
  Serial.println(current_protocol);
  if ((current_protocol & WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
    Serial.println("Protocol is WIFI_PROTOCOL_11B");
  if ((current_protocol & WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
    Serial.println("Protocol is WIFI_PROTOCOL_11G");
  if ((current_protocol & WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
    Serial.println("Protocol is WIFI_PROTOCOL_11N");
  if ((current_protocol & WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
    Serial.println("Protocol is WIFI_PROTOCOL_LR");

  return current_esp_interface;
}