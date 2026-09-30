#ifndef BENCH_H
#define BENCH_H

#include "sort.h"

typedef struct Record {
    int key;   /* 정렬 기준 */
    int tag;   /* 입력에서의 순서를 새겨 둔다 (안정성 측정용) */
} Record;

typedef enum InputShape {
    SHAPE_RANDOM, SHAPE_SORTED, SHAPE_REVERSED, SHAPE_NEARLY, SHAPE_DUPLICATES, SHAPE_COUNT
} InputShape;

const char *shapeName(InputShape shape);
int recordCompare(const void *a, const void *b);            /* key만 본다 */
Record *makeInput(InputShape shape, size_t n, unsigned seed); /* free()는 부르는 쪽 몫 */

int benchIsSorted(const Record *a, size_t n);               /* key 오름차순인가 */
int benchIsStable(const Record *a, size_t n);               /* 같은 key끼리 tag 오름차순인가 */

typedef struct BenchResult {
    double ms;          /* reps회 평균 (복사 시간 제외) */
    SortStats stats;    /* 마지막 실행의 카운터 */
    int sorted;
    int stable;
} BenchResult;

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps);

#endif
