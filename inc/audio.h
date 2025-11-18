#ifndef AT_AUDIO_H
#define AT_AUDIO_H

#include <stdint.h>
#include <stddef.h>

uint8_t *at_record_audio(size_t output_size);

int at_read_audio();

#endif
