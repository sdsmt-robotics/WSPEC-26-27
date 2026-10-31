#include "Encoder.h"

#include <math.h>

Encoder::Encoder(const Config &config)
    : config_(config),
      metersPerCount_(M_PI * config.wheelDiameterM / config.countsPerRev),
      decoder_() {}

Encoder::~Encoder() {
    if (started_) {
        quad_decoder_deinit(&decoder_);
    }
}

bool Encoder::begin() {
    if (started_) {
        return true;
    }
    esp_err_t err = quad_decoder_init(&decoder_, config_.pinA, config_.pinB,
                                      config_.internalPullups,
                                      config_.invertDirection);
    started_ = (err == ESP_OK);
    return started_;
}

void Encoder::reset() {
    quad_decoder_reset(&decoder_);
}

int32_t Encoder::counts() const {
    return quad_decoder_read(&decoder_).position;
}

double Encoder::revolutions() const {
    return static_cast<double>(counts()) / config_.countsPerRev;
}

double Encoder::positionMeters() const {
    return counts() * metersPerCount_;
}

double Encoder::totalDistanceMeters() const {
    return quad_decoder_read(&decoder_).edges * metersPerCount_;
}

uint32_t Encoder::errorCount() const {
    return quad_decoder_read(&decoder_).errors;
}
