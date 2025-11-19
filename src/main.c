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
    size_t const blocksize = MB(1);
    size_t const n_blocks = size / blocksize;

    double total_entropy = 0.0F;
    for (size_t block_idx = 0; block_idx < n_blocks; block_idx++) {
        char fn[128];
        snprintf(fn, 128, "out/%s%ld.bin", name, block_idx);

        uint8_t *block = data + block_idx * blocksize;
        at_write_binary(fn, block, blocksize);

        total_entropy += at_calculate_ent_entropy(block, blocksize);
    }

    printf("Average entropy for %ld %s blocks: %f\n", 
           n_blocks, name, total_entropy / n_blocks);
    printf("Total %s entropy: %f\n", 
           name, at_calculate_ent_entropy(data, size));
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
