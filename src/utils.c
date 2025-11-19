#include "utils.h"
#include "ent/randtest.h"
#include <stdlib.h>
#include <stdio.h>

void *at_xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        perror("malloc");
        abort();
    }
    return p;
}

void *at_xcalloc(size_t size) {
    void *p = calloc(size, 1);
    if (!p) {
        perror("calloc");
        abort();
    }
    return p;
}

void *at_xrealloc(void *p, size_t size) {
    void *p2 = realloc(p, size);
    if (!p2) {
        perror("realloc");
        abort();
    }
    return p2;
}

void at_write_binary(char const *fn, uint8_t *data, size_t size) {
    FILE *fp = fopen(fn, "wb");
    if (fp == NULL) {
        perror("fopen");
        abort();
    }

    fwrite(data, 1, size, fp);
    fclose(fp);
}

double at_calculate_ent_entropy(uint8_t *data, size_t size) {
    rt_init(0);
    rt_add(data, size);

    double r_ent, r_chisq, r_mean, r_montepicalc, r_scc;
    rt_end(&r_ent, &r_chisq, &r_mean, &r_montepicalc, &r_scc);

    return r_ent;
}
