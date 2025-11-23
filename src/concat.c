#include "concat.h"
#include "progress.h"
#include "utils.h"
#include "options.h"
#include <assert.h>
#include <stdio.h>

size_t at_concat_lsbs_get_input_size(size_t output_size) {
    return AT_CEILDIV(output_size * 8, at_opts.concat_lsbs);
}

size_t at_concat_lsbs_get_output_size(size_t input_size) {
    return input_size * at_opts.concat_lsbs / 8;
}

uint8_t *at_concat_lsbs(uint8_t *input, size_t output_size) {
    size_t lsb_n = at_opts.concat_lsbs;
    
    assert(lsb_n > 0 && lsb_n < 8);

    size_t input_size = at_concat_lsbs_get_input_size(output_size);

    uint8_t *output = at_xcalloc(output_size + 1);

    size_t bitpos = 0; 

    uint32_t mask = (1u << lsb_n) - 1u;

    at_progstate_t prog;
    at_progstate_init(&prog, input_size);
    at_progstate_start(&prog);

    for (size_t i = 0; i < input_size; i++) {
        uint32_t bits = input[i] & mask;

        size_t byte_pos     = bitpos / 8;
        size_t bit_offset   = bitpos % 8;

        output[byte_pos] |= bits << bit_offset;

        if (bit_offset + lsb_n > 8) {
            output[byte_pos + 1] |= bits >> (8 - bit_offset);
        }

        bitpos += lsb_n;

        // update every ~16k items, slows down program otherwise
        if ((i & 0xFFF) == 0) {  
            at_progstate_update(&prog, i);
        }
    }

    at_progstate_end(&prog);

    return output;
}
