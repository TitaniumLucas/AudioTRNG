#include "audio.h"
#include "progress.h"
#include "utils.h"
#include "options.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    uint8_t *data;
    size_t size;
    size_t cap;
    double var;
    bool dropping;
} at_audio_buf_t;

static double at_get_variance(Uint8 *buf, int len) {
    double sum = 0, sumsq = 0;

    for (int i = 0; i < len; i++) {
        Uint8 sample = buf[i];

        sum += sample;
        sumsq += (double)sample * sample;
    }

    double mean = sum / len;
    return sumsq / len - mean * mean;
}

static void at_record_audio_callback(void *ctx, Uint8 *stream, int len) {
    at_audio_buf_t *buf = ctx;

    buf->var = at_get_variance(stream, len);

    if (buf->var < at_opts.record_variance_threshold) {
        buf->dropping = true;
        return;
    } else {
        buf->dropping = false;
    }

    if (buf->size + len >= buf->cap) {
        while (buf->size + len >= buf->cap) {
            buf->cap *= 2;
        }

        buf->data = at_xrealloc(buf->data, buf->cap);
    }

    memcpy(buf->data + buf->size, stream, len);
    buf->size += len;
}

static const char *at_select_audio_device(void) {
    int n_devices = SDL_GetNumAudioDevices(SDL_TRUE);
    if (n_devices > 1) {
        printf("Detected multiple audio devices. Select from:\n");
    } else if (n_devices == 1) {
        return SDL_GetAudioDeviceName(0, SDL_TRUE);
    } else {
        printf("No audio devices detected.\n");
        exit(1);
    }

    for (int i = 0; i < n_devices; i++) {
        printf("  %d) %s\n", i + 1, SDL_GetAudioDeviceName(i, SDL_TRUE));
    }

    int selected = 0;
    while (selected == 0 || selected > n_devices) {
        printf("> ");
        if (scanf(" %d", &selected)) {
            // scanf isnt all that safe, result should be handled here, but 
            // lets assume that input is valid. 
        }
    }

    return SDL_GetAudioDeviceName(selected - 1, SDL_TRUE);
}

uint8_t *at_record_audio(size_t output_size, size_t *recorded_size) {
    size_t sample_rate = 44100;

    size_t transient_size = 10000;
    size_t total_size = transient_size + output_size;

    if (at_opts.record_seconds * sample_rate > total_size) {
        total_size = at_opts.record_seconds * sample_rate;
    }

    at_audio_buf_t buf = {
        .cap    = 1024,
        .size   = 0,
    };
    buf.data = at_xmalloc(buf.cap);

    char const *device_name = at_select_audio_device();
    printf("Opening '%s'...\n", device_name);

    SDL_AudioSpec desired, obtained;

    SDL_zero(desired);
    desired.freq        = sample_rate;
    desired.format      = AUDIO_U8;
    desired.channels    = 1;
    desired.samples     = 4096;
    desired.callback    = at_record_audio_callback;
    desired.userdata    = &buf;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 1, &desired, &obtained, 0);
    if (!dev) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return NULL;
    }

    if (desired.freq != obtained.freq
            || desired.format != obtained.format
            || desired.channels != obtained.channels
            || desired.samples != obtained.samples) {
        printf("Could not negotatiate to desired device settings.\n");
        return NULL;
    }   

    double seconds = (double)total_size / sample_rate;
    printf("Opened successfully. Recording for %.2f seconds", seconds);
    
    if (at_opts.record_variance_threshold) {
        printf(" with variance threshold %f...\n", 
               at_opts.record_variance_threshold);
    } else {
        printf("\n");
    }

    at_progstate_t prog;
    at_progstate_init(&prog, total_size, 1);
    at_progstate_start(&prog);

    SDL_PauseAudioDevice(dev, 0);

    while (buf.size < total_size) {
        if (!at_progstate_update(&prog, buf.size)) {
            at_progstate_to_infoline(&prog);
        }

        if (buf.dropping) {
            printf("Low variance (%.2f): dropping sample blocks!\n", 
                   buf.var);
        } else {
            printf("%ld B / %ld B | Variance: %.2f... \n", 
                   buf.size, total_size, buf.var);
        }

        SDL_Delay(100);
    }

    SDL_PauseAudioDevice(dev, 1);

    at_progstate_end(&prog);

    SDL_CloseAudioDevice(dev);

    if (buf.size < transient_size + output_size) {
        printf("Fatal: Recording ended prematurely "
               "(%d / %zu bytes collected).\n", 
               (int)(buf.size - transient_size), output_size);
        return NULL;
    }

    memmove(buf.data, buf.data + transient_size, buf.size - transient_size);
    *recorded_size = buf.size - transient_size;

    return buf.data;
}

uint8_t *at_load_wav(char const *fn, size_t *size) {
    SDL_AudioSpec spec;
    Uint8 *buf;
    Uint32 len;

    if (SDL_LoadWAV(fn, &spec, &buf, &len) == NULL) {
        fprintf(stderr, "SDL_LoadWAV failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt,
            spec.format, spec.channels, spec.freq,
            AUDIO_U16LSB, 1, 44100) < 0) {
        fprintf(stderr, "SDL_BuildAudioCVT failed: %s\n", SDL_GetError());
        SDL_FreeWAV(buf);
        return NULL;
    }

    cvt.len = len;
    cvt.buf = at_xmalloc(len * cvt.len_mult);

    memcpy(cvt.buf, buf, len);
    SDL_FreeWAV(buf);

    if (SDL_ConvertAudio(&cvt) < 0) {
        fprintf(stderr, "SDL_ConvertAudio failed: %s\n", SDL_GetError());
        free(cvt.buf);
        return NULL;
    }

    uint8_t *cbuf = at_xmalloc(cvt.len_cvt / 2);

    for (int i = 0; i < cvt.len_cvt / 2; i++) {
        cbuf[i] = cvt.buf[2 * i + 1];
    }

    free(cvt.buf);
    *size = cvt.len_cvt / 2;
    return cbuf;
}

uint8_t *at_load_bin(char const *fn, size_t *size) {
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        printf("Error: could not open file");
        return NULL;
    }

    // get file size
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (*size <= 0){
        printf("Error: file is empty");
        fclose(fp);
        return NULL;
    }

    uint8_t *buf = at_xmalloc(*size);

    //  check if allocation succesfull
    if (buf == NULL) {
        printf("Error: could not allocate memory");
        fclose(fp);
        return NULL;
    }

    size_t bytes_read = fread(buf, 1, *size, fp);
    //check if read succesful
    if (bytes_read != *size) {
        printf("Error: read unsuccesful. Read %zu of %ld bytes.\n", 
               bytes_read, *size);
        free(buf);
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    return buf;
}
