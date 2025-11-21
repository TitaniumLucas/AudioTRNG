#include "audio.h"
#include "concat.h"
#include "ccml.h"
#include "utils.h"
#include "options.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <argp.h>
#include <assert.h>

static char at_doc[]       = "Audio TRNG tool";
static char at_args_doc[]  = "";

static struct argp_option at_argp_options[] = {
    { "input-wav",      'w', "FILE",  0, "Path of input WAV file", 0 },
    { "input-bin",      'b', "FILE",  0, "Path of binary input file", 0 },
    { "input-record",   'r', 0,       0, "Record new audio for input", 0 },
    { "seconds",        's', "TIME",  0, "Milliseconds to record audio", 0 },
    { "concat-lsbs",    'c', "N",     0, "LSBs of data to use in concatentation", 1 },
    { "ccml",           'C', 0,       0, "Use CCML", 2 },
    { "output-size",    'S', "SIZE",  0, "Size of output", 3 },
    { "output",         'o', "FILE",  0, "Path of output file", 3 },
    { "entropy",        'E', 0,       0, "Compute ENT entropy at each stage", 4 },
    { "distribution",   'D', 0,       0, "Compute data distribution at each stage", 4 },
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

        case 's':
            at_opts.record_seconds = atof(arg);
            break;

        case 'c':
            at_opts.concat_lsbs = atoi(arg);
            if (at_opts.concat_lsbs < 1 || at_opts.concat_lsbs > 7) {
                argp_error(state, "Concat LSBs must be in [1, 7]");
            }
            break;

        case 'C':
            at_opts.ccml = true;
            break;

        case 'o':
            at_opts.output_filename = arg;
            break;

        case 'S': {
            int64_t size = at_parse_size(arg);
            if (size < 0) {
                argp_error(state, "Invalid size literal");
            }
            at_opts.output_size = size;
            break;
        }

        case 'E':
            at_opts.entropy = true;
            break;

        case 'D':
            at_opts.distribution = true;
            break;

        case ARGP_KEY_END:
            if (at_opts.input_type == AT_INPUT_UNSET) {
                argp_failure(state, 1, 0, "Missing input method");            
            }

            if (at_opts.output_filename == NULL) {
                at_opts.output_filename = "output.bin";
            }

            break;

        default:
            argp_error(state, "Unrecognized option: %c", key);
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

static size_t at_calculate_start_data_size(void) {
    size_t data_size = at_opts.output_size;
    
    if (at_opts.ccml) {
        data_size = at_ccml_get_input_size(data_size);
    }
    
    if (at_opts.concat_lsbs) {
        data_size = at_concat_lsbs_get_input_size(data_size);
    }

    return data_size;
}

static void at_post_stage_output(uint8_t *data, size_t data_size, 
                                 time_t start) {
    clock_t delta = clock() - start;
    double t = (double)delta / CLOCKS_PER_SEC;

    if (start) {
        printf("Stage finished in %f s.\n", t);
        printf("Throughput: %lu bytes / s.\n", (size_t)(data_size / t));
    }

    if (at_opts.entropy) {
        printf("Entropy: %f per byte\n",
                at_calculate_ent_entropy(data, data_size));
    }

    size_t const blocksize = 100 * 1024;
    size_t const n_blocks = data_size / blocksize;
    
    if (n_blocks > 2) {
        double H_min = 0.0F, H_max = 0.0F, H_total = 0.0F;
        double *Hs = at_xmalloc(n_blocks * sizeof(double));

        for (size_t i = 0; i < n_blocks; i++) {
            uint8_t *block = data + i * blocksize;
            double H = at_calculate_ent_entropy(block, blocksize);

            if (i == 0 || H < H_min) {
                H_min = H;
            }
            if (i == 0 || H > H_max) {
                H_max = H;
            }
            H_total += H;
            Hs[i] = H;
        }

        double H_mean = H_total / n_blocks;

        double H_var_total = 0.0F;
        for (size_t i = 0; i < n_blocks; i++) {
            double H_diff = Hs[i] - H_mean;
            H_var_total += H_diff * H_diff;
        }

        double H_var = H_var_total / n_blocks;
        double H_stddev = sqrt(H_var);

        printf("  Mean: %f, Min: %f, Max: %f, Stddev: %f\n", 
               H_total / n_blocks, H_min, H_max, H_stddev);
        free(Hs);
    }
}

int main(int argc, char *argv[]) {
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    size_t data_size = at_calculate_start_data_size();
    if (at_opts.output_size > 0) {
        printf("Generating %zu bytes of random output...\n", 
               at_opts.output_size);
        printf("Requires %zu input bytes.\n", data_size);
    }

    SDL_Init(SDL_INIT_AUDIO);

    size_t input_size = 0;
    uint8_t *data = NULL;

    switch (at_opts.input_type) {
        case AT_INPUT_UNSET:    
            break;

        case AT_INPUT_WAV_FILE: 
            data = at_load_wav(at_opts.input_filename, &input_size);
            break;

        case AT_INPUT_BIN_FILE:
            data = at_load_bin(at_opts.input_filename, &input_size);
            break;

        case AT_INPUT_RECORD:
            data = at_record_audio(data_size, &input_size);
            break;
    }

    if (input_size < data_size) {
        printf("Insufficient data from input:"
               " requires %zu bytes, but got %zu bytes.\n",
               data_size, input_size);
        return 1;
    }

    if (at_opts.output_size == 0) {
        data_size = input_size;
    }

    assert(data != NULL);
    printf("%zu bytes from input source\n", data_size);
    at_post_stage_output(data, data_size, 0);

    if (at_opts.concat_lsbs > 0) {
        clock_t start = clock();

        size_t concat_size = at_concat_lsbs_get_output_size(data_size);
        uint8_t *concat = at_concat_lsbs(data, concat_size);
        
        printf("\n== LSB Concat ==\n");
        printf("%zu bytes after LSB Concat\n", concat_size);
        at_post_stage_output(concat, concat_size, start);

        free(data);
        data = concat;
        data_size = concat_size;
    }

    if (at_opts.ccml) {
        clock_t start = clock();

        size_t ccml_size = at_ccml_get_output_size(data_size);
        uint8_t *ccml = at_ccml(data, ccml_size);

        printf("\n== CCML ==\n");
        printf("%zu bytes after CCML\n", ccml_size);
        at_post_stage_output(ccml, ccml_size, start);

        free(data);
        data = ccml;
        data_size = ccml_size;
    }

    at_write_binary(at_opts.output_filename, data, data_size);

    SDL_Quit();

    free(data);

    return 0;
}
