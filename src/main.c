#include "audio.h"
#include "concat.h"
#include "ccml.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define MB(x) (1ULL * (x) * 1024 * 1024)

void write_blocks(char const *name, uint8_t *data, size_t size) {
    size_t n_blocks = size / MB(1);

    for (size_t block = 0; block < n_blocks; block++) {
        char fn[128];
        snprintf(fn, 128, "out/%s%ld.bin", name, block);
        at_write_binary(fn, data + block * MB(1), MB(1));
    }
}

int main() {
    SDL_Init(SDL_INIT_AUDIO);

    // size_t audio_size = 1024 * 1024;
    // uint8_t *audio = at_record_audio(audio_size);

    size_t audio_size;
    uint8_t *audio = at_load_wav("input-large-2.wav", &audio_size);
    write_blocks("audio", audio, audio_size);
    at_write_binary("out/audio.bin", audio, audio_size);

    size_t concat_size;
    uint8_t *concat = at_concat_lsbs(audio, audio_size, 3, &concat_size);
    write_blocks("concat", concat, concat_size);
    at_write_binary("out/concat.bin", concat, concat_size);

    SDL_Quit();

    free(audio);
    free(concat);

    return 0;
}
