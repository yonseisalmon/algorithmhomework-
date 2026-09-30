/* make run            : 사람이 읽는 표
 * ./src/main.out --csv  : 같은 측정을 CSV로
 * ./src/main.out --gaps : 셸 정렬 간격열 비교 실험 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"

typedef struct Row {
    const char *section;
    const char *shape;
    const char *algo;
    size_t n;
    double ms;
    SortStats st;
    int stable;        /* 1 안정 / 0 불안정 / -1 해당 없음(중복 없는 입력) */
} Row;

typedef struct RowSink {
    void (*begin)(void);
    void (*row)(const Row *r);
} RowSink;

static const char *stableText(int s) { return s < 0 ? "미검사" : s ? "안정" : "불안정"; }

static void tableBegin(void) {
    puts("측정값: 시간(ms)=평균 정렬 시간, 비교횟수=키 비교 횟수");
    puts("이동횟수=원소 이동(교환 1회=3회)");
    puts("안정성=중복 키의 입력 순서 보존 여부 (중복 입력에서 검사)");
}
static void tableRow(const Row *r) {
    static const char *last = NULL;
    if (last != r->section) {
        printf("\n[%s]\n%-8s %-14s %2s %9s %8s %8s %s\n", r->section, "입력",
               "알고리즘", "n", "시간(ms)", "비교횟수", "이동횟수", "안정성");
        last = r->section;
    }
    printf("%-8s %-14s %2zu %9.6f %8llu %8llu %s\n", r->shape, r->algo, r->n, r->ms,
           r->st.compares, r->st.moves, stableText(r->stable));
}

static void csvBegin(void) {
    puts("section,shape,algorithm,n,ms,compares,moves,extraBytes,maxDepth,stable");
}
static void csvRow(const Row *r) {
    printf("%s,%s,%s,%zu,%.6f,%llu,%llu,%zu,%d,%d\n", r->section, r->shape, r->algo, r->n,
           r->ms, r->st.compares, r->st.moves, r->st.extraBytes, r->st.maxDepth, r->stable);
}

/* 무엇을 잴지는 이 배열 한 곳에만 적는다. */
typedef struct Spec { const char *section; InputShape shape; size_t n; } Spec;
static const Spec SPECS[] = {
    {"정렬 알고리즘 비교 (n=10)", SHAPE_RANDOM,     10},
    {"정렬 알고리즘 비교 (n=10)", SHAPE_SORTED,     10},
    {"정렬 알고리즘 비교 (n=10)", SHAPE_REVERSED,   10},
    {"정렬 알고리즘 비교 (n=10)", SHAPE_DUPLICATES, 10},
};
#define REPS 10000
#define SEED 12345u

static int runSpecs(const RowSink *sink) {
    sink->begin();
    for (size_t s = 0; s < sizeof SPECS / sizeof SPECS[0]; s++) {
        const Spec *sp = &SPECS[s];
        Record *input = makeInput(sp->shape, sp->n, SEED);
        if (!input) return 1;
        for (size_t a = 0; a < SORT_ALGORITHM_COUNT; a++) {
            BenchResult b = benchRun(&SORT_ALGORITHMS[a], input, sp->n, REPS);
            Row r = {sp->section, shapeName(sp->shape), SORT_ALGORITHMS[a].name, sp->n,
                     b.ms, b.stats, sp->shape == SHAPE_DUPLICATES ? b.stable : -1};
            if (!b.sorted) fprintf(stderr, "!! %s: 정렬 결과가 틀림\n", r.algo);
            sink->row(&r);
        }
        free(input);
    }
    return 0;
}

static const SortAlgorithm *findAlgo(const char *name) {
    for (size_t i = 0; i < SORT_ALGORITHM_COUNT; i++)
        if (strcmp(SORT_ALGORITHMS[i].name, name) == 0) return &SORT_ALGORITHMS[i];
    return NULL;
}

/* 셸 정렬의 간격열을 바꿔 가며 비교 횟수를 잰다 (무작위 입력). */
static int runGaps(int csv) {
    static const size_t NS[] = {1000, 4000, 16000, 64000};
    const SortAlgorithm *shell = findAlgo("shellSort");
    ShellGapKind saved = shellGapKind;
    if (!shell) return 1;

    if (csv) puts("gaps,n,compares,moves,ms");
    else {
        printf("셸 정렬 간격열별 비교 횟수 (무작위)\n%-14s", "간격열");
        for (size_t i = 0; i < 4; i++) printf(" %12zu", NS[i]);
        putchar('\n');
    }
    for (int k = 0; k < SHELL_GAPS_COUNT; k++) {
        if (!csv) printf("%-14s", shellGapName((ShellGapKind)k));
        shellGapKind = (ShellGapKind)k;
        for (size_t i = 0; i < 4; i++) {
            Record *in = makeInput(SHAPE_RANDOM, NS[i], SEED);
            BenchResult b;
            if (!in) return 1;
            b = benchRun(shell, in, NS[i], 3);
            if (csv) printf("%s,%zu,%llu,%llu,%.4f\n", shellGapName((ShellGapKind)k), NS[i],
                            b.stats.compares, b.stats.moves, b.ms);
            else printf(" %12llu", b.stats.compares);
            free(in);
        }
        if (!csv) putchar('\n');
    }
    shellGapKind = saved;
    return 0;
}

int main(int argc, char **argv) {
    static const RowSink TABLE = {tableBegin, tableRow};
    static const RowSink CSV = {csvBegin, csvRow};
    if (argc == 1) return runSpecs(&TABLE);
    if (argc == 2 && strcmp(argv[1], "--csv") == 0) return runSpecs(&CSV);
    if (argc == 2 && strcmp(argv[1], "--gaps") == 0) return runGaps(0);
    if (argc == 3 && strcmp(argv[1], "--gaps") == 0 && strcmp(argv[2], "--csv") == 0)
        return runGaps(1);
    fprintf(stderr, "사용법: %s [--csv | --gaps [--csv]]\n", argv[0]);
    return 2;
}
