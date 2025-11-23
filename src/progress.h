#ifndef AT_PROGRESS_H
#define AT_PROGRESS_H

#include <stddef.h>

typedef struct {
    size_t used;
    size_t length;
    double total;
} at_progstate_t;

void at_progstate_init(at_progstate_t *prog, double total);

void at_progstate_start(at_progstate_t *prog);

void at_progstate_end(at_progstate_t *prog);

void at_progstate_update(at_progstate_t *prog, double curr);

#endif
