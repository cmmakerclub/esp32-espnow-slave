#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <tcpip_adapter.h>
#if CONFIG_STATION_MODE
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP
#endif

uint8_t current_protocol;
esp_interface_t check_protocol();

esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;esp_now_peer_info_t slave;

uint32_t sentCnt = 0;
uint8_t peerCount = 0;
int packetRecv = 0;
uint8_t master_mac[6] = { 0x30, 0xAE, 0xA4, 0x99, 0x0C, 0xEC } ;
void setup() {
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
  if (esp_now_init() == ESP_OK) {
    Serial.printf("ESPNow Init Success\n");
  }
  else {
    Serial.printf("ESPNow Init Failed\n");
    ESP.restart();
  }

  esp_now_register_send_cb([&] (const uint8_t *mac_addr, esp_now_send_status_t status) {
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
  if (addStatus == ESP_OK) {
    // Pair success
    Serial.println("Pair success");
  } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
    Serial.println("Peer list full");
  } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("Out of memory");
  } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
    Serial.println("Peer Exists");
  } else {
    Serial.println("Not sure what happened");
  }
}


void loop() {
  uint8_t data = 1;
  esp_err_t result = esp_now_send(slave.peer_addr, &data, sizeof(data));
  if (result == ESP_OK) {
    Serial.println("Success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  }
  else {
    Serial.println("Not sure what happened");
  }
  Serial.println(result);
  delay(5 * 1000);
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
    esp_err_to_name_r(error_code,error_buf1,100);
    Serial.print("esp_wifi_get_protocol error code: ");
    Serial.println(error_buf1);
    Serial.print("Current protocol code is ");
    Serial.println(current_protocol);
    if ((current_protocol&WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
      Serial.println("Protocol is WIFI_PROTOCOL_11B");
    if ((current_protocol&WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
      Serial.println("Protocol is WIFI_PROTOCOL_11G");
    if ((current_protocol&WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
      Serial.println("Protocol is WIFI_PROTOCOL_11N");
    if ((current_protocol&WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
      Serial.println("Protocol is WIFI_PROTOCOL_LR");

    return current_esp_interface;
}
