#include "ccml.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#if NDEBUG
#define eprintf(fmt, ...)
#else
#define eprintf(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#endif

#define AT_CCML_MAX_SYSTEM_SIZE     8
#define AT_CCML_COUPLING_CONST      0.05
#define AT_CCML_CONTR_PARAM         1.99999

static void at_assert_little_endian() {
    int i = 1;
    assert(*((char *)&i) == 1);
}

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

static double at_ccml_tent_map(double x) {
    assert(x >= 0 && x <= 1);

    if (x < 0.5) {
        return AT_CCML_CONTR_PARAM * x;
    } else {
        return AT_CCML_CONTR_PARAM * (1 - x);
    }
}

/* `x` = x^i_t, 
   `x_left` = x^{i-1 mod L}_t, 
   `x_right` = x^{i+1 mod L}_t 
   
   returns x^i_{t+1} */
static double at_ccml_iteration(double x, double x_left, double x_right) {
    double f        = at_ccml_tent_map(x);
    double f_left   = at_ccml_tent_map(x_left);
    double f_right  = at_ccml_tent_map(x_right);
    
    double self_part  = (1 - AT_CCML_COUPLING_CONST) * f;
    double cross_part = (AT_CCML_COUPLING_CONST / 2) * (f_left + f_right);

    return  self_part + cross_part;
}

static inline uint64_t at_bitwise_double_to_u64(double d) {
    uint64_t u64;
    memcpy(&u64, &d, sizeof(uint64_t));
    return u64;
}

static inline uint64_t at_bitwise_swap(uint64_t u64) {
    return ((u64 & 0xFFFFFFFFULL) << 32) | ((u64 >> 32) & 0xFFFFFFFFULL);
}

void at_coupled_chaotic_map_lattice() {
    /* The double-to-u64 cast only works if the system is little endian. */
    at_assert_little_endian();

    size_t output_size_bits = 1024 * 1024 * 8; /* = N */
    size_t output_size = output_size_bits / 8;

    /* Assert output size in bytes is multiple of 256, for convenience. */
    assert(output_size_bits % 32 == 0);

    size_t system_size = 8; /* = L */    
    assert(system_size % 2 == 0);

    size_t n_iterations = system_size / 2; /* = lambda */
    size_t sample_size = output_size_bits / 32; /* = n */

    size_t block_output_size = system_size / 2 * sizeof(uint64_t);

    uint8_t *samples = malloc(sample_size);
    assert(samples != NULL);
    at_init_samples(samples, sample_size);

    uint8_t *output = malloc(output_size);
    assert(output != NULL);

    for (size_t i = 0; i < sample_size; i++) {
        samples[i] &= 0x07;
    }

    double state[8];
    at_init_ccml_state(state);

    for (size_t block = 0; block < output_size / block_output_size; block++) {
        uint8_t *block_samples = samples + block * system_size;
        at_ccml_perturb_state(state, block_samples, system_size);

        for (size_t i = 0; i < n_iterations; i++) {
            double next_state[AT_CCML_MAX_SYSTEM_SIZE];
            for (size_t j = 0; j < system_size; j++) {
                double x       = state[j];
                double x_left  = state[(j - 1 + system_size) % system_size];
                double x_right = state[(j + 1) % system_size];

                next_state[j] = at_ccml_iteration(x, x_left, x_right);
            }

            memcpy(state, next_state, system_size * sizeof(double));
        }

        uint64_t local_output[AT_CCML_MAX_SYSTEM_SIZE];
        for (size_t i = 0; i < system_size; i++) {
            local_output[i] = at_bitwise_double_to_u64(state[i]);
        }

        size_t output_index = block * block_output_size;
        uint64_t *block_output = (uint64_t *)(output + output_index);
        for (size_t i = 0; i < system_size / 2; i++) {
            uint64_t other   = local_output[i + system_size / 2];
            uint64_t swapped = at_bitwise_swap(other);

            block_output[i] = local_output[i] ^ swapped;
        }
    }

    // for (size_t i = 0; i < output_size / 8; i++) {
    //     eprintf("0x%016lX", ((uint64_t *)output)[i]);
    // }

    // Temporary write to output file for quick testing
    FILE *fp = fopen("output.bin", "wb");
    assert(fp != NULL);
    fwrite(output, 1, output_size, fp);
    fclose(fp);

    free(samples);
    free(output);
}
