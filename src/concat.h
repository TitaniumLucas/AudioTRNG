#ifndef AT_CONCAT_H
#define AT_CONCAT_H

#include <stddef.h>
#include <stdint.h>

size_t at_concat_lsbs_get_input_size(size_t output_size);

uint8_t *at_concat_lsbs(uint8_t *input, size_t input_size, 
                        size_t *output_size);

#endif
