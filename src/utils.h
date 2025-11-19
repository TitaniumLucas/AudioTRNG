#ifndef AT_UTILS_H
#define AT_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define AT_CEILDIV(x, y) (((x) + (y) - 1) / (y))

void *at_xmalloc(size_t size);

void *at_xcalloc(size_t size);

void *at_xrealloc(void *p, size_t size);

void at_write_binary(char const *fn, uint8_t *data, size_t size);

double at_calculate_ent_entropy(uint8_t *data, size_t size);

int64_t at_parse_size(char const *s);

size_t at_align_up(size_t n, size_t align);

#endif
