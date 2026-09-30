#include "sortctx.h"

/* 남은 구간에서 최솟값을 찾아 맨 앞과 교환한다.
 * 비교는 입력과 무관하게 n(n-1)/2 번, 교환은 많아야 n-1 번.
 * 멀리 떨어진 원소를 교환하므로 같은 값의 앞뒤가 뒤바뀔 수 있다 → 불안정. */
void selectionSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortCtxOpen(&c, base, size, cmp, stats)) return;

    for (size_t i = 0; i + 1 < n; i++) {
        size_t min = i;
        for (size_t j = i + 1; j < n; j++) {
            if (sortCompareAt(&c, j, min) < 0) min = j;   /* 엄격히 작을 때만 갱신 */
        }
        if (min != i) sortSwap(&c, i, min);
    }
    sortCtxClose(&c);
}
