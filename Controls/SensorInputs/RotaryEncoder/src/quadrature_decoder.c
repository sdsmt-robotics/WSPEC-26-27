#include "quadrature_decoder.h"

#include <string.h>

#include "esp_attr.h"
#include "hal/gpio_ll.h"
#include "soc/gpio_struct.h"

/*
 * Transition table indexed by (previous_state << 2) | current_state, where a
 * state is (A << 1) | B. Gray-code order for one direction is
 * 00 -> 01 -> 11 -> 10 -> 00. QUAD_INVALID marks a jump where both channels
 * changed, i.e. an edge was missed.
 *
 * Kept in DRAM so the ISR can run while flash cache is disabled.
 */
#define QUAD_INVALID 2

static DRAM_ATTR const int8_t k_transition[16] = {
    /* prev 00 */  0, +1, -1, QUAD_INVALID,
    /* prev 01 */ -1,  0, QUAD_INVALID, +1,
    /* prev 10 */ +1, QUAD_INVALID,  0, -1,
    /* prev 11 */ QUAD_INVALID, -1, +1,  0,
};

static inline uint8_t IRAM_ATTR sample_pins(const quad_decoder_t *dec)
{
    /* gpio_ll is inline register access, safe to call from an IRAM ISR. */
    return (uint8_t)((gpio_ll_get_level(&GPIO, dec->pin_a) << 1) |
                     gpio_ll_get_level(&GPIO, dec->pin_b));
}

static void IRAM_ATTR quad_isr(void *arg)
{
    quad_decoder_t *dec = (quad_decoder_t *)arg;

    portENTER_CRITICAL_ISR(&dec->lock);

    uint8_t current = sample_pins(dec);
    int8_t step = k_transition[(dec->state << 2) | current];

    if (step == QUAD_INVALID) {
        dec->errors++;
    } else if (step != 0) {
        dec->position += dec->invert ? -step : step;
        dec->edges++;
    }
    dec->state = current;

    portEXIT_CRITICAL_ISR(&dec->lock);
}

esp_err_t quad_decoder_init(quad_decoder_t *dec, gpio_num_t pin_a,
                            gpio_num_t pin_b, bool enable_pullups, bool invert)
{
    if (dec == NULL || pin_a == pin_b || !GPIO_IS_VALID_GPIO(pin_a) ||
        !GPIO_IS_VALID_GPIO(pin_b)) {
        return ESP_ERR_INVALID_ARG;
    }

    memset((void *)dec, 0, sizeof(*dec));
    dec->pin_a = pin_a;
    dec->pin_b = pin_b;
    dec->invert = invert;
    portMUX_INITIALIZE(&dec->lock);

    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin_a) | (1ULL << pin_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = enable_pullups ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) {
        return err;
    }

    /* Arduino's attachInterrupt() may already have installed the service. */
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    dec->state = sample_pins(dec);

    err = gpio_isr_handler_add(pin_a, quad_isr, dec);
    if (err != ESP_OK) {
        return err;
    }
    err = gpio_isr_handler_add(pin_b, quad_isr, dec);
    if (err != ESP_OK) {
        gpio_isr_handler_remove(pin_a);
        return err;
    }
    return ESP_OK;
}

void quad_decoder_deinit(quad_decoder_t *dec)
{
    gpio_isr_handler_remove(dec->pin_a);
    gpio_isr_handler_remove(dec->pin_b);
}

quad_snapshot_t quad_decoder_read(quad_decoder_t *dec)
{
    quad_snapshot_t snap;
    portENTER_CRITICAL(&dec->lock);
    snap.position = dec->position;
    snap.edges = dec->edges;
    snap.errors = dec->errors;
    portEXIT_CRITICAL(&dec->lock);
    return snap;
}

void quad_decoder_reset(quad_decoder_t *dec)
{
    portENTER_CRITICAL(&dec->lock);
    dec->position = 0;
    dec->edges = 0;
    dec->errors = 0;
    portEXIT_CRITICAL(&dec->lock);
}
