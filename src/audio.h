#ifndef AT_AUDIO_H
#define AT_AUDIO_H

#include <stdint.h>
#include <stddef.h>

uint8_t *at_record_audio(size_t output_size, size_t *recorded_size);

uint8_t *at_load_wav(char const *fn, size_t *size);

uint8_t *at_load_bin(char const *fn, size_t *size);

#endif
