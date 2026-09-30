#define _POSIX_C_SOURCE 200809L
#include "bench.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *shapeName(InputShape shape) {
    static const char *NAMES[] = {"무작위", "정렬됨", "역순", "거의정렬", "중복많음"};
    return shape < SHAPE_COUNT ? NAMES[shape] : "?";
}

int recordCompare(const void *a, const void *b) {
    int x = ((const Record *)a)->key, y = ((const Record *)b)->key;
    return (x > y) - (x < y);
}

/* 플랫폼마다 다른 rand() 대신 xorshift32: 어디서 돌려도 같은 입력이 나온다. */
static uint32_t rngNext(uint32_t *s) {
    uint32_t x = *s;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return *s = x;
}

Record *makeInput(InputShape shape, size_t n, unsigned seed) {
    Record *a = malloc((n ? n : 1) * sizeof *a);
    uint32_t s = seed ? seed : 1u;
    if (!a) return NULL;
    for (size_t i = 0; i < n; i++) {
        a[i].tag = (int)i;
        switch (shape) {
        case SHAPE_RANDOM:     a[i].key = (int)(rngNext(&s) % 1000000u); break;
        case SHAPE_SORTED:
        case SHAPE_NEARLY:     a[i].key = (int)i; break;
        case SHAPE_REVERSED:   a[i].key = (int)(n - i); break;
        case SHAPE_DUPLICATES: a[i].key = (int)(rngNext(&s) % 16u); break;
        default:               a[i].key = 0; break;
        }
    }
    if (shape == SHAPE_NEARLY && n > 1) {              /* 5%쯤 무작위로 뒤바꾼다 */
        for (size_t k = 0; k < n / 20 + 1; k++) {
            size_t i = rngNext(&s) % n, j = rngNext(&s) % n;
            int t = a[i].key; a[i].key = a[j].key; a[j].key = t;
        }
    }
    return a;
}

int benchIsSorted(const Record *a, size_t n) {
    for (size_t i = 0; i + 1 < n; i++)
        if (a[i].key > a[i + 1].key) return 0;
    return 1;
}

int benchIsStable(const Record *a, size_t n) {
    for (size_t i = 0; i + 1 < n; i++)
        if (a[i].key == a[i + 1].key && a[i].tag > a[i + 1].tag) return 0;
    return 1;
}

static double nowSec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps) {
    BenchResult r;
    Record *work = malloc((n ? n : 1) * sizeof *work);
    double total = 0.0;
    memset(&r, 0, sizeof r);
    if (!work) return r;
    if (reps < 1) reps = 1;

    for (int k = 0; k < reps; k++) {
        SortStats st;
        memcpy(work, input, n * sizeof *work);         /* 복사는 시간에서 뺀다 */
        double t0 = nowSec();
        algo->sort(work, n, sizeof *work, recordCompare, &st);
        total += nowSec() - t0;
        r.stats = st;
    }
    r.ms = total / reps * 1000.0;
    r.sorted = benchIsSorted(work, n);
    r.stable = benchIsStable(work, n);
    free(work);
    return r;
}
