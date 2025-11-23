#include "progress.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define AT_PROG_COLS 80

static void at_progstate_redraw(at_progstate_t *prog) {
    for (size_t i = 0; i < prog->infolines + 1; i++) {
        printf(AT_ANSI_CURSOR_UP(1));
    }

    putc('[', stdout);
    for (size_t i = 0; i < prog->used; i++) {
        putc('=', stdout);
    }
    for (size_t i = 0; i < prog->length - prog->used; i++) {
        putc(' ', stdout);
    }
    putc(']', stdout);

    printf("\n");
}

void at_progstate_init(at_progstate_t *prog, double total, size_t infolines) {
    assert(total > 0.0F);

    prog->used = 0;
    prog->length = AT_PROG_COLS - 2;
    prog->total = total;
    prog->infolines = infolines;
}

void at_progstate_start(at_progstate_t *prog) {
    for (size_t i = 0; i < prog->infolines + 1; i++) {
        printf("\n");
    }
    at_progstate_redraw(prog);
    for (size_t i = 0; i < prog->infolines; i++) {
        printf("\n");
    }
}

void at_progstate_end(at_progstate_t *prog) {
    for (size_t i = 0; i < prog->infolines + 1; i++) {
        printf(AT_ANSI_CURSOR_UP(1) AT_ANSI_CLEAR_LINE());
    }
}

bool at_progstate_update(at_progstate_t *prog, double curr) {    
    if (curr < 0.0F) {
        curr = 0.0F;
    } else if (curr > prog->total) {
        curr = prog->total;
    }

    size_t used   = round(curr / prog->total * prog->length);
    if (used != prog->used) {
        prog->used = used;
        at_progstate_redraw(prog);

        return true;
    }

    return false;
}

void at_progstate_to_infoline(at_progstate_t *prog) {
    for (size_t i = 0; i < prog->infolines; i++) {
        printf(AT_ANSI_CURSOR_UP(1) AT_ANSI_CLEAR_LINE());
    }
}
