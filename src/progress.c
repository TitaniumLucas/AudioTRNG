#include "progress.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define AT_ANSI_CURSOR_UP(lines) "\x1b[" #lines "A"
#define AT_ANSI_CURSOR_DOWN(lines) "\x1b[" #lines "B"
#define AT_ANSI_CLEAR_LINE() "\x1b[2K"

#define AT_PROG_COLS 80

static void at_progstate_redraw(at_progstate_t *prog) {
    printf(AT_ANSI_CURSOR_UP(1));

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

void at_progstate_init(at_progstate_t *prog, double total) {
    assert(total > 0.0F);

    prog->used = 0;
    prog->length = AT_PROG_COLS - 2;
    prog->total = total;
}

void at_progstate_start(at_progstate_t *prog) {
    (void)prog;
    printf("\n");
    at_progstate_redraw(prog);
}

void at_progstate_end(at_progstate_t *prog) {
    (void)prog;
    printf(AT_ANSI_CURSOR_UP(1) AT_ANSI_CLEAR_LINE());
}

void at_progstate_update(at_progstate_t *prog, double curr) {    
    if (curr < 0.0F) {
        curr = 0.0F;
    } else if (curr > prog->total) {
        curr = prog->total;
    }

    size_t used   = round(curr / prog->total * prog->length);
    if (used != prog->used) {
        prog->used = used;
        at_progstate_redraw(prog);
    }
}
