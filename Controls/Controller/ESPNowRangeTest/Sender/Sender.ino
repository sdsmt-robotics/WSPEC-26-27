// ESP-NOW Range Test - SENDER
// Broadcasts a numbered packet every SEND_INTERVAL_MS. Flash Receiver.ino on the
// other ESP32 and watch its Serial Monitor while you walk the boards apart.
// Broadcast is used so no MAC addresses need to be entered.

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

const uint8_t  WIFI_CHANNEL     = 1;    // must match the receiver
const uint32_t SEND_INTERVAL_MS = 100;  // 10 packets per second
const uint32_t PACKET_MAGIC     = 0x52414E47;  // "RANG" - lets the receiver ignore other traffic

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint32_t seq;
  uint32_t senderMillis;
} RangePacket;

RangePacket packet;
uint32_t lastSendMs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);  // max power for range testing

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, broadcastAddress, 6);
  peer.channel = WIFI_CHANNEL;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add broadcast peer");
    while (true) delay(1000);
  }

  packet.magic = PACKET_MAGIC;
  packet.seq = 0;

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Sending range test packets...");
}

void loop() {
  uint32_t now = millis();
  if (now - lastSendMs < SEND_INTERVAL_MS) return;
  lastSendMs = now;

  packet.seq++;
  packet.senderMillis = now;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&packet, sizeof(packet));

  // Print once a second so the sender's monitor isn't flooded
  if (packet.seq % (1000 / SEND_INTERVAL_MS) == 0) {
    Serial.printf("Sent seq %lu (%s)\n", (unsigned long)packet.seq,
                  result == ESP_OK ? "ok" : esp_err_to_name(result));
  }
}
