#include "ccml.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Temporary, would be replaced by requesting an audio recording 
   containing `sample_size` samples of 1 byte each. */
static void at_init_samples(uint8_t *samples, size_t sample_size) {
    for (size_t i = 0; i < sample_size; i++) {
        samples[i] = i & 0xFF;
    }
}

static void at_init_ccml_state(double state[8]) {
    state[0] = 0.141592;
    state[1] = 0.653589;
    state[2] = 0.793238;
    state[3] = 0.462643;
    state[4] = 0.383279;
    state[5] = 0.502884;
    state[6] = 0.197169;
    state[7] = 0.399375;
}

/* This assumes that `samples` is adjusted to the samples for the 
   current generated block. */
static void at_ccml_perturb_state(double state[8], 
                                  uint8_t samples[], 
                                  size_t system_size) {
    for (size_t j = 0; j < system_size; j++) {
        state[j] = ((0.071428571 * samples[j]) + state[j]) * 0.666666667;
    }
}

/* `x` = x^i_t, 
   `x_left` = x^{i-1 mod L}_t, 
   `x_right` = x^{i+1 mod L}_t 
   
   returns x^i_{t+1} */
static double at_ccml_iteration(double x, double x_left, double x_right) {
    (void)x_left, (void)x_right;
    return x;
}

static inline uint64_t at_bitwise_double_to_u64(double d) {
    uint64_t u64;
    memcpy(&u64, &d, sizeof(uint64_t));
    return u64;
}

void at_coupled_chaotic_map_lattice() {
    size_t output_size_bits = 1024; /* = N */
    size_t system_size = 8; /* = L */    
    size_t n_iterations = system_size / 2; /* = lambda */
    size_t sample_size = output_size_bits / 32; /* = n */

    uint8_t *samples = malloc(sample_size);
    assert(samples != NULL);
    at_init_samples(samples, sample_size);

    for (size_t i = 0; i < sample_size; i++) {
        /* Extract 3 bits from each sample: modify in-place as samples are not 
           used elsewhere. */
        samples[i] &= 0x07;
    }

    double state[8];
    at_init_ccml_state(state);

    while (1) { // TODO
        at_ccml_perturb_state(state, samples + 0, system_size); // TODO: move through samples

        for (size_t i = 0; i < n_iterations; i++) {
            double next_state[8];
            for (size_t j = 0; j < system_size; j++) {
                double x       = state[j];
                double x_left  = state[(j - 1 + system_size) % system_size];
                double x_right = state[(j + 1) % system_size];

                next_state[j] = at_ccml_iteration(x, x_left, x_right);
            }

            memcpy(state, next_state, system_size * sizeof(double));
        }

        uint64_t outputs[8];

        for (size_t i = 0; i < system_size; i++) {
            outputs[i] = at_bitwise_double_to_u64(state[i]);
        }

        for (size_t i = 0; i < system_size / 2; i++) {
            // TODO: swap
            outputs[i] = outputs[i] ^ outputs[i + system_size / 2];
        }
    }
}
