#include "options.h"
#include <stddef.h>

at_opts_t at_opts = {
    .input_filename     = NULL,
    .input_type         = AT_INPUT_UNSET,
    .record_seconds     = 0.0F,
    .concat_lsbs        = 0,
    .output_size        = 0,
    .output_filename    = NULL,
    .entropy            = false,
    .distribution       = false,
};
