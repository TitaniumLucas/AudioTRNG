#include "options.h"
#include <stddef.h>

at_opts_t at_opts = {
    .input_filename     = NULL,
    .input_type         = AT_INPUT_UNSET,
    .concat_lsbs        = 0,
    .min_output_size    = 0,
    .output_filename    = NULL,
    .entropy            = false,
};
