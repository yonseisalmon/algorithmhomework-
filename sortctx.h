/* 구현 전용 헤더: 정렬 구현끼리만 쓰는 도구. */
#ifndef SORTCTX_H
#define SORTCTX_H

#include "sort.h"

typedef struct SortCtx {
    char *base;        /* 배열의 첫 바이트 */
    size_t size;       /* 원소 한 개의 바이트 수 */
    SortCompare cmp;
    SortStats *stats;  /* NULL 가능 */
    char *tmp;         /* 원소 한 칸. 추가로 잡는 메모리는 이것뿐이다 */
} SortCtx;

/* 열면 stats를 0으로 비우고 extraBytes=size, maxDepth=1로 둔다. 실패하면 0. */
int  sortCtxOpen(SortCtx *c, void *base, size_t size, SortCompare cmp, SortStats *stats);
void sortCtxClose(SortCtx *c);

char *sortElemAt(const SortCtx *c, size_t i);
int   sortCompareAt(SortCtx *c, size_t i, size_t j);   /* cmp(a[i], a[j]) */
int   sortCompareTmp(SortCtx *c, size_t i);            /* cmp(tmp, a[i]) */
void  sortMove(SortCtx *c, size_t dst, size_t src);    /* a[dst] = a[src], 이동 1회 */
void  sortLift(SortCtx *c, size_t i);                  /* tmp = a[i],      이동 1회 */
void  sortDrop(SortCtx *c, size_t i);                  /* a[i] = tmp,      이동 1회 */
void  sortSwap(SortCtx *c, size_t i, size_t j);        /* 이동 3회 */

void selectionSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void shellSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

#endif
