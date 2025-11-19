#include "audio.h"
#include "concat.h"
#include "ccml.h"
#include "utils.h"
#include "options.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <argp.h>
#include <assert.h>

static char at_doc[]       = "Audio TRNG tool";
static char at_args_doc[]  = "";

static struct argp_option at_argp_options[] = {
    { "input-wav",      'w', "FILE", OPTION_ARG_OPTIONAL, "Path of input WAV file", 0 },
    { "input-bin",      'b', "FILE", OPTION_ARG_OPTIONAL, "Path of binary input file", 0 },
    { "input-record",   'r', 0,      OPTION_ARG_OPTIONAL, "Record new audio for input", 0 },
    { "output-size",    's', "N",    OPTION_ARG_OPTIONAL, "Minimum size of output", 1 },
    { "output",         'o', "FILE", OPTION_ARG_OPTIONAL, "Path of output file", 1 },
    { 0 },
};

#define MB(x) (1ULL * (x) * 1024 * 1024)

void write_blocks(char const *name, uint8_t *data, size_t size) {
    size_t const blocksize = MB(1);
    size_t const n_blocks = size / blocksize;

    double total_entropy = 0.0F;
    for (size_t block_idx = 0; block_idx < n_blocks; block_idx++) {
        char fn[128];
        snprintf(fn, 128, "out/%s%ld.bin", name, block_idx);

        uint8_t *block = data + block_idx * blocksize;
        at_write_binary(fn, block, blocksize);

        total_entropy += at_calculate_ent_entropy(block, blocksize);
    }

    if (n_blocks > 0) {
        printf("Average entropy for %ld %s blocks: %f\n", 
               n_blocks, name, total_entropy / n_blocks);
    }
    printf("Total %s entropy: %f\n", 
           name, at_calculate_ent_entropy(data, size));
}

static error_t at_parse_opt(int key, char *arg, struct argp_state *state) {
    (void)state;
    
    switch (key) {
        case 'w':
            at_opts.input_filename = arg;
            at_opts.input_type = AT_INPUT_WAV_FILE;
            break;

        case 'b':
            at_opts.input_filename = arg;
            at_opts.input_type = AT_INPUT_BIN_FILE;
            break;

        case 'r':
            at_opts.input_filename = NULL;
            at_opts.input_type = AT_INPUT_RECORD;
            break;

        case ARGP_KEY_END:
            if (at_opts.input_type == AT_INPUT_UNSET) {
                argp_failure(state, 1, 0, "Missing input method");            
            }

            if (at_opts.output_filename == NULL) {
                at_opts.output_filename = "output.bin";
            }

            break;
    }

    return 0;
}

static struct argp argp = {
    at_argp_options,
    at_parse_opt,
    at_args_doc,
    at_doc,
    NULL,
    NULL,
    NULL,
};

int main(int argc, char *argv[]) {
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    SDL_Init(SDL_INIT_AUDIO);

    size_t input_size = 1024 * 1024;
    uint8_t *input = NULL;

    switch (at_opts.input_type) {
        case AT_INPUT_UNSET:    
            break;

        case AT_INPUT_WAV_FILE: 
            input = at_load_wav(at_opts.input_filename, &input_size);
            break;

        case AT_INPUT_BIN_FILE:
            input = at_load_bin(at_opts.input_filename, &input_size);
            break;

        case AT_INPUT_RECORD:
            input = at_record_audio(input_size);
            break;
    }

    assert(input != NULL);

    write_blocks("input", input, input_size);
    at_write_binary("out/input.bin", input, input_size);

    size_t concat_size;
    uint8_t *concat = at_concat_lsbs(input, input_size, 3, &concat_size);
    write_blocks("concat", concat, concat_size);
    at_write_binary("out/concat.bin", concat, concat_size);

    at_write_binary(at_opts.output_filename, concat, concat_size);

    SDL_Quit();

    free(input);
    free(concat);

    return 0;
}
