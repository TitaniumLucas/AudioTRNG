#ifndef AT_OPTIONS_H
#define AT_OPTIONS_H

#include <stddef.h>

typedef enum {
    AT_INPUT_UNSET,
    AT_INPUT_WAV_FILE,
    AT_INPUT_BIN_FILE,
    AT_INPUT_RECORD,
} at_input_type_t;

typedef struct {
    char const *input_filename;
    at_input_type_t input_type;
    size_t min_output_size;
    char const *output_filename;
} at_opts_t;

extern at_opts_t at_opts;

#endif
