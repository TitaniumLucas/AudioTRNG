#include "audio.h"
#include "concat.h"
#include "ccml.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define MB(x) (1ULL * (x) * 1024 * 1024)

int main() {
    SDL_Init(SDL_INIT_AUDIO);

    // size_t audio_size = 3 * 1024 * 1024;
    // uint8_t *audio = at_record_audio(audio_size);

    size_t audio_size;
    uint8_t *audio = at_load_wav("input-large.wav", &audio_size);

    at_write_binary("audio.bin", audio, audio_size);

    size_t concat_size;
    uint8_t *concat = at_concat_lsbs(audio, audio_size, 1, &concat_size);

    size_t n_blocks = concat_size / MB(1);

    for (size_t block = 0; block < n_blocks; block++) {
        char fn[128];
        snprintf(fn, 128, "concat%ld.bin", block);
        at_write_binary(fn, concat + block * MB(1), MB(1));
    }

    SDL_Quit();

    free(audio);
    free(concat);

    return 0;
}
