#include "sortctx.h"

#include <stdlib.h>
#include <string.h>

int sortCtxOpen(SortCtx *c, void *base, size_t size, SortCompare cmp, SortStats *stats) {
    c->base = base;
    c->size = size;
    c->cmp = cmp;
    c->stats = stats;
    c->tmp = malloc(size ? size : 1);
    if (!c->tmp) return 0;
    if (stats) {
        memset(stats, 0, sizeof *stats);
        stats->extraBytes = size;
        stats->maxDepth = 1;
    }
    return 1;
}

void sortCtxClose(SortCtx *c) {
    free(c->tmp);
    c->tmp = NULL;
}

char *sortElemAt(const SortCtx *c, size_t i) { return c->base + i * c->size; }

int sortCompareAt(SortCtx *c, size_t i, size_t j) {
    if (c->stats) c->stats->compares++;
    return c->cmp(sortElemAt(c, i), sortElemAt(c, j));
}

int sortCompareTmp(SortCtx *c, size_t i) {
    if (c->stats) c->stats->compares++;
    return c->cmp(c->tmp, sortElemAt(c, i));
}

void sortMove(SortCtx *c, size_t dst, size_t src) {
    if (c->stats) c->stats->moves++;
    memmove(sortElemAt(c, dst), sortElemAt(c, src), c->size);
}

void sortLift(SortCtx *c, size_t i) {
    if (c->stats) c->stats->moves++;
    memcpy(c->tmp, sortElemAt(c, i), c->size);
}

void sortDrop(SortCtx *c, size_t i) {
    if (c->stats) c->stats->moves++;
    memcpy(sortElemAt(c, i), c->tmp, c->size);
}

void sortSwap(SortCtx *c, size_t i, size_t j) {
    if (i == j) return;
    sortLift(c, i);
    sortMove(c, i, j);
    sortDrop(c, j);
}

/* 구현 표: 정렬을 하나 더 넣으려면 여기에 한 줄만 추가하면 된다. */
const SortAlgorithm SORT_ALGORITHMS[] = {
    {"selectionSort", "O(n^2)",       "O(1)", 0, selectionSort},
    {"shellSort",     "O(n^1.3~n^2)", "O(1)", 0, shellSort},
    {"heapSort",      "O(n log n)",   "O(1)", 0, heapSort},
};
const size_t SORT_ALGORITHM_COUNT = sizeof SORT_ALGORITHMS / sizeof SORT_ALGORITHMS[0];
