/*
 * Interrupt-driven x4 quadrature decoder for the ESP32-S3 (plain C).
 *
 * Every rising and falling edge on channel A and channel B is counted, so a
 * 200 pulse/rev encoder produces 800 counts/rev.
 */
#ifndef QUADRATURE_DECODER_H
#define QUADRATURE_DECODER_H

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t pin_a;
    gpio_num_t pin_b;
    bool invert;

    /* Written only from the ISR (or under the lock). */
    volatile int32_t position;  /* signed net counts */
    volatile uint32_t edges;    /* every valid edge, regardless of direction */
    volatile uint32_t errors;   /* transitions where both channels changed */
    volatile uint8_t state;     /* last sampled (A << 1) | B */

    portMUX_TYPE lock;
} quad_decoder_t;

typedef struct {
    int32_t position;
    uint32_t edges;
    uint32_t errors;
} quad_snapshot_t;

/*
 * Configures both pins as inputs with any-edge interrupts and starts counting.
 * The E6A2-CS3E has open-collector outputs, so the lines need pull-ups to
 * 3.3 V. The internal pull-ups (~45 kOhm) work at low speed; use external
 * 2.2-4.7 kOhm pull-ups for clean edges and set enable_pullups to false.
 */
esp_err_t quad_decoder_init(quad_decoder_t *dec, gpio_num_t pin_a,
                            gpio_num_t pin_b, bool enable_pullups, bool invert);

/* Detaches the interrupts. Counts are left as they were. */
void quad_decoder_deinit(quad_decoder_t *dec);

/* Reads all counters together so they are consistent with each other. */
quad_snapshot_t quad_decoder_read(quad_decoder_t *dec);

/* Zeros all counters. */
void quad_decoder_reset(quad_decoder_t *dec);

#ifdef __cplusplus
}
#endif

#endif /* QUADRATURE_DECODER_H */
