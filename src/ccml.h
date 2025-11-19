#ifndef AT_CCML_H
#define AT_CCML_H

#include <stddef.h>
#include <stdint.h>

size_t at_ccml_get_input_size(size_t output_size);

uint8_t *at_ccml(uint8_t *data, size_t output_size);

#endif

