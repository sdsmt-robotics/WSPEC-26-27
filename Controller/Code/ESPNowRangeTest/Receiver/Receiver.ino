// ESP-NOW Range Test - RECEIVER
// Listens for packets from Sender.ino and prints signal stats to the Serial
// Monitor (115200 baud). Each second it prints packets received, packet loss %,
// and average RSSI. When nothing arrives for a second it prints "NO SIGNAL".
//
// RSSI guide: > -60 dBm strong, -60..-75 good, -75..-85 weak, < -85 about to drop.

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>


const uint8_t  WIFI_CHANNEL         = 1;    // must match the sender
const uint32_t SEND_INTERVAL_MS     = 100;  // must match the sender
const uint32_t REPORT_INTERVAL_MS   = 1000;
const bool     PRINT_EVERY_PACKET   = false;  // true = one line per packet
const uint32_t PACKET_MAGIC         = 0x52414E47;

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint32_t seq;
  uint32_t senderMillis;
} RangePacket;

// Shared between the ESP-NOW callback (WiFi task) and loop()
portMUX_TYPE statsMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t rxCount = 0;        // packets received this report window
volatile int32_t  rssiSum = 0;
volatile int8_t   rssiMin = 0;
volatile int8_t   rssiMax = -127;
volatile uint32_t firstSeq = 0;       // first seq seen this window
volatile uint32_t lastSeq = 0;        // most recent seq seen overall
volatile int8_t   lastRssi = 0;
volatile bool     newPacket = false;

uint32_t prevWindowLastSeq = 0;
uint32_t totalReceived = 0;
uint32_t totalExpected = 0;
uint32_t outageEstimate = 0;          // estimated lost packets during the current outage
uint32_t lastReportMs = 0;
uint32_t lastPacketMs = 0;
bool     everReceived = false;

static void handlePacket(const uint8_t *data, int len, int8_t rssi) {
  if (len != sizeof(RangePacket)) return;
  RangePacket pkt;
  memcpy(&pkt, data, sizeof(pkt));
  if (pkt.magic != PACKET_MAGIC) return;

  portENTER_CRITICAL(&statsMux);
  if (rxCount == 0) {
    firstSeq = pkt.seq;
    rssiMin = rssi;
    rssiMax = rssi;
  }
  rxCount++;
  rssiSum += rssi;
  if (rssi < rssiMin) rssiMin = rssi;
  if (rssi > rssiMax) rssiMax = rssi;
  lastSeq = pkt.seq;
  lastRssi = rssi;
  newPacket = true;
  portEXIT_CRITICAL(&statsMux);
}

// The receive callback signature changed in Arduino-ESP32 core 3.x
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  handlePacket(data, len, info->rx_ctrl->rssi);
}
#else
// Core 2.x doesn't pass RSSI to the callback, so grab it with promiscuous mode
volatile int8_t promiscRssi = 0;
void onPromisc(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return;
  const wifi_promiscuous_pkt_t *p = (const wifi_promiscuous_pkt_t *)buf;
  promiscRssi = p->rx_ctrl.rssi;
}
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  handlePacket(data, len, promiscRssi);
}
#endif

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(onReceive);

#if ESP_ARDUINO_VERSION_MAJOR < 3
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(onPromisc);
#endif

  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Waiting for range test packets...");
  Serial.println("  time(s) | rx/exp | loss% | RSSI avg (min..max) | total loss%");
}

void loop() {
  uint32_t now = millis();

  if (PRINT_EVERY_PACKET && newPacket) {
    portENTER_CRITICAL(&statsMux);
    uint32_t seq = lastSeq;
    int8_t rssi = lastRssi;
    newPacket = false;
    portEXIT_CRITICAL(&statsMux);
    Serial.printf("  pkt seq %lu  RSSI %d dBm\n", (unsigned long)seq, rssi);
  }

  if (now - lastReportMs < REPORT_INTERVAL_MS) return;
  lastReportMs = now;

  // Snapshot and reset the window stats
  portENTER_CRITICAL(&statsMux);
  uint32_t count = rxCount;
  int32_t  sum = rssiSum;
  int8_t   minR = rssiMin;
  int8_t   maxR = rssiMax;
  uint32_t first = firstSeq;
  uint32_t last = lastSeq;
  rxCount = 0;
  rssiSum = 0;
  portEXIT_CRITICAL(&statsMux);

  float seconds = now / 1000.0f;

  if (count == 0) {
    if (everReceived) {
      // Estimate the packets we should have gotten; replaced by the exact seq gap on recovery
      outageEstimate += REPORT_INTERVAL_MS / SEND_INTERVAL_MS;
      uint32_t estExpected = totalExpected + outageEstimate;
      Serial.printf("%8.1f | NO SIGNAL (%.1f s since last packet) | total loss %.1f%%\n",
                    seconds, (now - lastPacketMs) / 1000.0f,
                    100.0f * (estExpected - totalReceived) / estExpected);
    } else {
      Serial.printf("%8.1f | NO SIGNAL (no packets yet)\n", seconds);
    }
    return;
  }

  lastPacketMs = now;

  // Expected = seq range covered this window, including any gap since last window
  uint32_t expected;
  if (!everReceived || last < prevWindowLastSeq) {
    // First packets, or sender rebooted (seq went backwards)
    expected = last - first + 1;
    totalReceived = 0;
    totalExpected = 0;
  } else {
    expected = last - prevWindowLastSeq;  // includes any packets missed during an outage
  }
  outageEstimate = 0;
  everReceived = true;
  prevWindowLastSeq = last;
  if (expected < count) expected = count;

  totalReceived += count;
  totalExpected += expected;

  float loss = 100.0f * (expected - count) / expected;
  float totalLoss = 100.0f * (totalExpected - totalReceived) / totalExpected;
  float avgRssi = (float)sum / count;

  const char *quality = avgRssi > -60 ? "strong" : avgRssi > -75 ? "good" : avgRssi > -85 ? "WEAK" : "VERY WEAK";

  Serial.printf("%8.1f | %3lu/%-3lu | %5.1f | %6.1f dBm (%d..%d) %-9s | %5.1f\n",
                seconds, (unsigned long)count, (unsigned long)expected, loss,
                avgRssi, minR, maxR, quality, totalLoss);
}
