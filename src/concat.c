#include "concat.h"
#include "utils.h"
#include <assert.h>

uint8_t *at_concat_lsbs(uint8_t *input, size_t input_size, 
                        size_t lsb_n, size_t *output_size) {
    assert(lsb_n > 0 && lsb_n < 8);
    
    size_t output_size_bits = input_size * lsb_n;
    *output_size = (output_size_bits + 7) / 8;

    uint8_t *output = at_xcalloc(*output_size);

    size_t at_bit = 0, at_byte = 0;
    uint8_t byte = 0;
    for (size_t i = 0; i < input_size; i++) {
        uint8_t bits = input[i] & ((1U << lsb_n) - 1U);

        byte |= bits << at_bit;
        at_bit += lsb_n;

        if (at_bit >= 8) {
            output[at_byte] = byte;
            at_bit -= 8;
            at_byte++;

            if (at_bit > 0) {
                byte = bits >> (lsb_n - at_bit);
            } else {
                byte = 0;
            }
        }
    }

    if (at_bit > 0) { // Commit partial byte.
        output[at_byte] = byte;
    }

    return output;
}
