/* 공개 인터페이스: 부르는 쪽(main, bench, 테스트)은 이 헤더만 본다. */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>

typedef int (*SortCompare)(const void *a, const void *b);   /* qsort와 같은 규약 */

typedef struct SortStats {
    unsigned long long compares;   /* 비교 횟수 */
    unsigned long long moves;      /* 원소 이동 횟수 (교환 1회 = 3회) */
    size_t extraBytes;             /* 입력 배열 밖에 잡은 메모리 */
    int maxDepth;                  /* 재귀 깊이 (반복문뿐이면 1) */
} SortStats;

typedef struct SortAlgorithm {
    const char *name;
    const char *timeComplexity;
    const char *spaceComplexity;
    int stable;                    /* 안정 정렬이라고 "주장"하는 값 */
    void (*sort)(void *base, size_t n, size_t size,
                 SortCompare cmp, SortStats *stats);   /* stats가 NULL이면 측정 안 함 */
} SortAlgorithm;

extern const SortAlgorithm SORT_ALGORITHMS[];   /* 구현 표 */
extern const size_t SORT_ALGORITHM_COUNT;

/* 실험용 손잡이: 셸 정렬의 간격열 */
typedef enum ShellGapKind {
    SHELL_GAPS_SHELL,   /* n/2, n/4, ... 1  (셸 원안) */
    SHELL_GAPS_KNUTH,   /* 1, 4, 13, 40, ...  (3h+1) */
    SHELL_GAPS_CIURA,   /* 1, 4, 10, 23, 57, 132, 301, 701, 1750, ... */
    SHELL_GAPS_COUNT
} ShellGapKind;

extern ShellGapKind shellGapKind;               /* 기본: KNUTH */
const char *shellGapName(ShellGapKind kind);

#endif
