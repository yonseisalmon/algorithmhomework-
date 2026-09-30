#include "sortctx.h"

/* root의 값을 heap [root, end) 안에서 제자리까지 내린다 (반복문이라 재귀 없음). */
static void siftDown(SortCtx *c, size_t root, size_t end) {
    size_t i = root;
    sortLift(c, i);                                   /* 빈자리(hole)를 만든다 */
    for (;;) {
        size_t child = 2 * i + 1;
        if (child >= end) break;
        if (child + 1 < end && sortCompareAt(c, child + 1, child) > 0) child++;
        if (sortCompareTmp(c, child) >= 0) break;     /* 들고 있는 값이 더 크면 멈춤 */
        sortMove(c, i, child);
        i = child;
    }
    sortDrop(c, i);
}

/* 1) 배열을 최대 힙으로 만든다  2) 루트(최댓값)를 끝으로 보내고 힙을 줄인다.
 * 루트와 끝을 교환하므로 같은 값의 순서가 깨진다 → 불안정. */
void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortCtxOpen(&c, base, size, cmp, stats)) return;

    for (size_t i = n / 2; i-- > 0;) siftDown(&c, i, n);
    for (size_t end = n; end > 1; end--) {
        sortSwap(&c, 0, end - 1);
        siftDown(&c, 0, end - 1);
    }
    sortCtxClose(&c);
}
