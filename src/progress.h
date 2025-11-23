#ifndef AT_PROGRESS_H
#define AT_PROGRESS_H

#include <stddef.h>
#include <stdbool.h>

#define AT_ANSI_CURSOR_UP(lines) "\x1b[" #lines "A"
#define AT_ANSI_CURSOR_DOWN(lines) "\x1b[" #lines "B"
#define AT_ANSI_CLEAR_LINE() "\x1b[2K"

typedef struct {
    size_t used;
    size_t length;
    size_t infolines;
    double total;
} at_progstate_t;

void at_progstate_init(at_progstate_t *prog, double total, size_t infolines);

void at_progstate_start(at_progstate_t *prog);

void at_progstate_end(at_progstate_t *prog);

bool at_progstate_update(at_progstate_t *prog, double curr);

void at_progstate_to_infoline(at_progstate_t *prog);

#endif
