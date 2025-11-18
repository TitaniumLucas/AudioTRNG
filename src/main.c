#include "audio.h"
#include "ccml.h"
#include <SDL2/SDL.h>
#include <stdio.h>

int main() {
    SDL_Init(SDL_INIT_AUDIO);

    at_record_audio(1024 * 1024);

    //at_coupled_chaotic_map_lattice();

    SDL_Quit();

    return 0;
}
