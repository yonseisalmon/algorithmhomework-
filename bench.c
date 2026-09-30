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

Record *makeInput(InputShape shape, size_t n, unsigned seed) {
    static const int RANDOM_PATTERN[] = {7, 3, 9, 1, 5, 8, 2, 6, 4, 0, 11, 13, 15, 10, 12, 14};
    static const int DUPLICATE_PATTERN[] = {3, 1, 3, 1, 2, 2, 3, 1, 5, 5, 2, 1, 3, 3, 4, 0};
    static const int FIXED_SORTED[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    static const int FIXED_REVERSED[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    Record *a = malloc((n ? n : 1) * sizeof *a);
    (void)seed;
    if (!a) return NULL;
    for (size_t i = 0; i < n; i++) {
        a[i].tag = (int)i;
        switch (shape) {
        case SHAPE_RANDOM:
            a[i].key = RANDOM_PATTERN[i % (sizeof(RANDOM_PATTERN) / sizeof(RANDOM_PATTERN[0]))];
            break;
        case SHAPE_SORTED:
            if (n <= 10) a[i].key = FIXED_SORTED[i];
            else a[i].key = (int)(i + 1);
            break;
        case SHAPE_REVERSED:
            if (n <= 10) a[i].key = FIXED_REVERSED[i];
            else a[i].key = (int)(n - i);
            break;
        case SHAPE_NEARLY: {
            if (n <= 10) a[i].key = FIXED_SORTED[i];
            else a[i].key = (int)(i + 1);
            if (i == 2) a[i].key = 7;
            else if (i == 5) a[i].key = 2;
            else if (i == 8) a[i].key = 9;
            break;
        }
        case SHAPE_DUPLICATES:
            a[i].key = DUPLICATE_PATTERN[i % (sizeof(DUPLICATE_PATTERN) / sizeof(DUPLICATE_PATTERN[0]))];
            break;
        default:
            a[i].key = 0;
            break;
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
