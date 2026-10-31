// Wheel encoder: turns quadrature counts from the YUMO E6A2-CS3E into meters.
#pragma once

#include <stdint.h>

#include "quadrature_decoder.h"

// YUMO E6A2-CS3E: 200 pulses per revolution on each channel.
constexpr uint32_t kEncoderPulsesPerRev = 200;
// x4 decoding counts both edges of both channels.
constexpr uint32_t kEncoderCountsPerRev = kEncoderPulsesPerRev * 4;

// 4 7/8 in diameter wheel.
constexpr double kInchesToMeters = 0.0254;
constexpr double kWheelDiameterIn = 4.875;
constexpr double kWheelDiameterM = kWheelDiameterIn * kInchesToMeters;

class Encoder {
public:
    struct Config {
        gpio_num_t pinA = GPIO_NUM_NC;
        gpio_num_t pinB = GPIO_NUM_NC;
        uint32_t countsPerRev = kEncoderCountsPerRev;
        double wheelDiameterM = kWheelDiameterM;
        bool invertDirection = false;
        bool internalPullups = true;
    };

    explicit Encoder(const Config &config);
    ~Encoder();

    Encoder(const Encoder &) = delete;
    Encoder &operator=(const Encoder &) = delete;

    // Starts counting. Returns false if the pins could not be configured.
    bool begin();

    // Zeros the position, total distance and error count.
    void reset();

    // Signed net counts since begin()/reset().
    int32_t counts() const;

    // Signed net wheel revolutions.
    double revolutions() const;

    // Signed net displacement in meters (forward minus backward).
    double positionMeters() const;

    // Total distance traveled in meters, counting both directions as positive.
    double totalDistanceMeters() const;

    // Transitions where an edge was missed (both channels changed at once).
    // A rising value means the wheel is spinning too fast or the signal is noisy.
    uint32_t errorCount() const;

    double metersPerCount() const { return metersPerCount_; }

private:
    Config config_;
    double metersPerCount_;
    mutable quad_decoder_t decoder_;
    bool started_ = false;
};
