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
    { "input-wav",      'w', "FILE",  0, "Path of input WAV file", 0 },
    { "input-bin",      'b', "FILE",  0, "Path of binary input file", 0 },
    { "input-record",   'r', 0,       0, "Record new audio for input", 0 },
    { "concat-lsbs",    'c', "N",     0, "LSBs of data to use in concatentation", 1 },
    { "output-size",    's', "BYTES", 0, "Minimum size of output", 2 },
    { "output",         'o', "FILE",  0, "Path of output file", 2 },
    { "entropy",        'e', 0,       0, "Compute ENT entropy at each stage", 3 },
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

        case 'c':
            at_opts.concat_lsbs = atoi(arg);
            if (at_opts.concat_lsbs < 1 || at_opts.concat_lsbs > 7) {
                argp_error(state, "Concat LSBs must be in [1, 7]");
            }
            break;

        case 'o':
            at_opts.output_filename = arg;
            break;

        case 's': {
            int64_t size = at_parse_size(arg);
            if (size < 0) {
                argp_error(state, "Invalid size literal");
            }
            at_opts.min_output_size = size;
            break;
        }

        case 'e':
            at_opts.entropy = true;
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

    size_t data_size = 1024 * 1024;
    uint8_t *data = NULL;

    switch (at_opts.input_type) {
        case AT_INPUT_UNSET:    
            break;

        case AT_INPUT_WAV_FILE: 
            data = at_load_wav(at_opts.input_filename, &data_size);
            break;

        case AT_INPUT_BIN_FILE:
            data = at_load_bin(at_opts.input_filename, &data_size);
            break;

        case AT_INPUT_RECORD:
            data = at_record_audio(data_size);
            break;
    }

    assert(data != NULL);
    if (at_opts.entropy) {
        printf("Raw data entropy: %f per byte\n", 
               at_calculate_ent_entropy(data, data_size));
    }

    if (at_opts.concat_lsbs > 0) {
        size_t concat_size;
        uint8_t *concat = at_concat_lsbs(data, data_size, 
                                         at_opts.concat_lsbs, &concat_size);
        if (at_opts.entropy) {
            printf("LSB Concat entropy: %f per byte\n",
                   at_calculate_ent_entropy(concat, concat_size));
        }

        free(data);
        data = concat;
        data_size = concat_size;
    }

    at_write_binary(at_opts.output_filename, data, data_size);

    SDL_Quit();

    free(data);

    return 0;
}
