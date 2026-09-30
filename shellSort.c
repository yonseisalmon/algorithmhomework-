#include "sortctx.h"

ShellGapKind shellGapKind = SHELL_GAPS_KNUTH;

const char *shellGapName(ShellGapKind kind) {
    switch (kind) {
    case SHELL_GAPS_SHELL: return "shell(n/2)";
    case SHELL_GAPS_KNUTH: return "knuth(3h+1)";
    case SHELL_GAPS_CIURA: return "ciura";
    default:               return "?";
    }
}

#define MAX_GAPS 64

static void reverseGaps(size_t *g, size_t k) {
    for (size_t i = 0; i < k / 2; i++) {
        size_t t = g[i];
        g[i] = g[k - 1 - i];
        g[k - 1 - i] = t;
    }
}

/* 간격을 큰 것부터 gaps에 채우고 개수를 돌려준다. 마지막 간격은 항상 1. */
static size_t makeGaps(size_t n, size_t *gaps) {
    size_t k = 0;
    switch (shellGapKind) {
    case SHELL_GAPS_SHELL:
        for (size_t g = n / 2; g > 0; g /= 2) gaps[k++] = g;
        break;
    case SHELL_GAPS_KNUTH: {
        size_t h = 1;
        do { gaps[k++] = h; h = 3 * h + 1; } while (h <= n / 3 && k < MAX_GAPS);
        reverseGaps(gaps, k);
        break;
    }
    case SHELL_GAPS_CIURA: {
        static const size_t CIURA[] = {1, 4, 10, 23, 57, 132, 301, 701, 1750};
        const size_t m = sizeof CIURA / sizeof CIURA[0];
        for (size_t i = 0; i < m && CIURA[i] < n; i++) gaps[k++] = CIURA[i];
        if (k == m) {                                   /* 1750 너머는 2.25배씩 */
            size_t h = CIURA[m - 1];
            while ((h = h * 9 / 4) < n && k < MAX_GAPS) gaps[k++] = h;
        }
        reverseGaps(gaps, k);
        break;
    }
    default: break;
    }
    return k;
}

/* 간격 gap만큼 떨어진 원소끼리 삽입 정렬을 하고, 간격을 줄여 가며 되풀이한다.
 * 마지막 간격 1은 보통의 삽입 정렬이다. 간격 건너뛰기 때문에 불안정. */
void shellSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    size_t gaps[MAX_GAPS];
    if (!sortCtxOpen(&c, base, size, cmp, stats)) return;

    if (n >= 2) {
        size_t k = makeGaps(n, gaps);
        for (size_t gi = 0; gi < k; gi++) {
            size_t gap = gaps[gi];
            for (size_t i = gap; i < n; i++) {
                if (sortCompareAt(&c, i - gap, i) <= 0) continue;   /* 이미 제자리 */
                sortLift(&c, i);
                sortMove(&c, i, i - gap);
                size_t j = i - gap;
                /* cmp(tmp, a[j-gap]) < 0  <=>  a[j-gap]가 tmp보다 크다 */
                while (j >= gap && sortCompareTmp(&c, j - gap) < 0) {
                    sortMove(&c, j, j - gap);
                    j -= gap;
                }
                sortDrop(&c, j);
            }
        }
    }
    sortCtxClose(&c);
}
