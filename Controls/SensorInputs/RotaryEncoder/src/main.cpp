// Reads the YUMO E6A2-CS3E wheel encoder and prints distance over Serial.
//
// Wiring (E6A2 colour code, check against the label on your encoder):
//   Brown  -> +5 V (encoder supply is 5-12 V)
//   Blue   -> GND (shared with the ESP32-S3)
//   Black  -> Phase A -> ENCODER_PIN_A
//   White  -> Phase B -> ENCODER_PIN_B
// The outputs are open collector, so they only ever pull the line low and
// it is safe to pull them up to the ESP32's 3.3 V. Do NOT pull them up to 5 V.
//
// Serial commands: send 'r' to reset the counters.

#include <Arduino.h>

#include "Encoder.h"

static constexpr gpio_num_t ENCODER_PIN_A = GPIO_NUM_4;
static constexpr gpio_num_t ENCODER_PIN_B = GPIO_NUM_5;

static constexpr uint32_t PRINT_PERIOD_MS = 100;

// Set to false once the encoder is confirmed working to print only distance.
static constexpr bool DEBUG_OUTPUT = true;

static Encoder::Config makeEncoderConfig() {
    Encoder::Config config;
    config.pinA = ENCODER_PIN_A;
    config.pinB = ENCODER_PIN_B;
    return config;
}

static Encoder wheelEncoder(makeEncoderConfig());

// Total distance traveled in meters, refreshed every loop.
static double totalDistanceM = 0.0;

void setup() {
    Serial.begin(115200);
    delay(500);

    if (!wheelEncoder.begin()) {
        Serial.println("Encoder init failed, check the pin numbers");
        while (true) {
            delay(1000);
        }
    }

    Serial.printf("E6A2-CS3E: %u counts/rev, wheel %.4f m, %.6f m/count\n",
                  (unsigned)kEncoderCountsPerRev, kWheelDiameterM,
                  wheelEncoder.metersPerCount());
}

void loop() {
    static uint32_t lastPrintMs = 0;

    totalDistanceM = wheelEncoder.totalDistanceMeters();

    if (Serial.available() > 0 && Serial.read() == 'r') {
        wheelEncoder.reset();
        Serial.println("Encoder reset");
    }

    uint32_t now = millis();
    if (now - lastPrintMs >= PRINT_PERIOD_MS) {
        lastPrintMs = now;
        if (DEBUG_OUTPUT) {
            // A/B are the raw pin levels; they should flip between 0 and 1
            // as the shaft is turned slowly by hand.
            Serial.printf("Distance: %.3f m  A=%d B=%d  counts=%ld  errors=%lu\n",
                          totalDistanceM, gpio_get_level(ENCODER_PIN_A),
                          gpio_get_level(ENCODER_PIN_B),
                          (long)wheelEncoder.counts(),
                          (unsigned long)wheelEncoder.errorCount());
        } else {
            Serial.printf("Distance: %.3f m\n", totalDistanceM);
        }
    }
}
