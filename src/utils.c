#include "utils.h"
#include "ent/randtest.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

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

int64_t at_parse_size(char const *s) {
    if (*s == '\0') {
        return -1; // empty
    }

    uint64_t val = 0;

    for (; *s != '\0'; s++) {
        char c = *s;

        if (isdigit(c)) {
            val = val * 10 + c - '0';
        } else if (c == 'k') {
            val *= 1024ULL;
            s++;
            goto end;
        } else if (c == 'M') {
            val *= 1024ULL * 1024;
            s++;
            goto end;
        } else if (c == 'G') {
            val *= 1024ULL * 1024 * 1024;
            s++;
            goto end;
        } else if (c == 'b' || c == 'B') {
            goto end;
        } else {
            return -1;
        }
    }

end:
    if (*s == 'B' || *s == '\0') {
        return val;
    } else if (*s == 'b') {
        return (val + 7) / 8;
    } else {
        return -1;
    }
}

size_t at_align_up(size_t n, size_t align) {
    size_t rem = n % align;
    
    if (rem == 0) {
        return n;
    } else {
        return n + (align - rem);
    }
}
