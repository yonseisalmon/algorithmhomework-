#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

static int checks = 0, failures = 0;
#define CHECK(cond, ...)                                                   \
    do {                                                                   \
        checks++;                                                          \
        if (!(cond)) {                                                     \
            failures++;                                                    \
            printf("FAIL %s:%d ", __FILE__, __LINE__);                     \
            printf(__VA_ARGS__);                                           \
            printf("\n");                                                  \
        }                                                                  \
    } while (0)

static int cmpInt(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static void checkAgainstQsort(const SortAlgorithm *a, const int *src, size_t n, const char *what) {
    int *x = malloc((n ? n : 1) * sizeof(int));
    int *y = malloc((n ? n : 1) * sizeof(int));
    memcpy(x, src, n * sizeof(int));
    memcpy(y, src, n * sizeof(int));
    qsort(y, n, sizeof(int), cmpInt);
    a->sort(x, n, sizeof(int), cmpInt, NULL);
    CHECK(memcmp(x, y, n * sizeof(int)) == 0, "%s: %s (n=%zu)", a->name, what, n);
    free(x);
    free(y);
}

static void testBasic(const SortAlgorithm *a) {
    static const int mixed[] = {5, 2, 9, 1, 5, 6, 0, 3};
    static const int sorted[] = {1, 2, 3, 4, 5, 6};
    static const int reversed[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    static const int dups[] = {3, 1, 3, 1, 2, 2, 3, 1};
    static const int same[] = {7, 7, 7, 7, 7};
    static const int one[] = {42};
    checkAgainstQsort(a, mixed, 8, "섞임");
    checkAgainstQsort(a, sorted, 6, "정렬됨");
    checkAgainstQsort(a, reversed, 9, "역순");
    checkAgainstQsort(a, dups, 8, "중복");
    checkAgainstQsort(a, same, 5, "모두 같은 값");
    checkAgainstQsort(a, one, 1, "원소 1개");
    checkAgainstQsort(a, one, 0, "빈 배열");
}

static void testRandomAndSweep(const SortAlgorithm *a) {
    int buf[500];
    srand(1);
    for (int i = 0; i < 500; i++) buf[i] = rand() % 1000 - 500;
    checkAgainstQsort(a, buf, 500, "난수 500개");
    for (size_t n = 0; n <= 200; n++) {                /* 크기 훑기 */
        for (size_t i = 0; i < n; i++) buf[i] = rand() % 50;
        checkAgainstQsort(a, buf, n, "크기 훑기");
    }
}

static void testStability(const SortAlgorithm *a) {
    Record *in = makeInput(SHAPE_DUPLICATES, 500, 7);
    a->sort(in, 500, sizeof(Record), recordCompare, NULL);
    CHECK(benchIsSorted(in, 500), "%s: 정렬 실패", a->name);
    int stable = benchIsStable(in, 500);
    if (a->stable) CHECK(stable, "%s: 안정이라 주장했지만 순서가 깨짐", a->name);
    else           CHECK(!stable, "%s: 불안정이라 적었지만 실측은 안정", a->name);
    free(in);
}

static void testStats(const SortAlgorithm *a) {
    Record *in = makeInput(SHAPE_RANDOM, 100, 3);
    SortStats st;
    a->sort(in, 100, sizeof(Record), recordCompare, &st);
    CHECK(st.compares > 0 && st.moves > 0, "%s: 카운터가 0", a->name);
    CHECK(st.extraBytes == sizeof(Record), "%s: 추가 메모리 %zu", a->name, st.extraBytes);
    CHECK(st.maxDepth == 1, "%s: 깊이 %d", a->name, st.maxDepth);
    if (strcmp(a->name, "selectionSort") == 0) {
        CHECK(st.compares == 100ULL * 99 / 2, "선택 정렬 비교는 n(n-1)/2");
        CHECK(st.moves <= 3ULL * 99, "선택 정렬 이동은 3(n-1) 이하");
    }
    free(in);
}

static void testShellGaps(void) {
    const SortAlgorithm *shell = NULL;
    ShellGapKind saved = shellGapKind;
    static const size_t NS[] = {1, 2, 3, 10, 100, 1000, 5000};
    for (size_t i = 0; i < SORT_ALGORITHM_COUNT; i++)
        if (strcmp(SORT_ALGORITHMS[i].name, "shellSort") == 0) shell = &SORT_ALGORITHMS[i];
    CHECK(shell != NULL, "shellSort가 표에 없음");
    if (!shell) return;
    for (int k = 0; k < SHELL_GAPS_COUNT; k++) {
        shellGapKind = (ShellGapKind)k;
        for (size_t i = 0; i < sizeof NS / sizeof NS[0]; i++) {
            Record *in = makeInput(SHAPE_RANDOM, NS[i], 11);
            shell->sort(in, NS[i], sizeof(Record), recordCompare, NULL);
            CHECK(benchIsSorted(in, NS[i]), "간격열 %s, n=%zu", shellGapName((ShellGapKind)k), NS[i]);
            free(in);
        }
    }
    shellGapKind = saved;
}

static void testTools(void) {
    Record *a;
    Record bad[] = {{1, 1}, {1, 0}}, good[] = {{1, 0}, {1, 1}};
    CHECK(!benchIsStable(bad, 2), "안정성 판정기가 위반을 못 잡음");
    CHECK(benchIsStable(good, 2), "안정성 판정기가 정상을 위반으로 봄");

    a = makeInput(SHAPE_SORTED, 100, 1);
    CHECK(benchIsSorted(a, 100), "정렬됨 입력이 정렬돼 있지 않음");
    free(a);
    a = makeInput(SHAPE_REVERSED, 100, 1);
    CHECK(!benchIsSorted(a, 100) && a[0].key > a[99].key, "역순 입력 모양");
    free(a);
    a = makeInput(SHAPE_DUPLICATES, 1000, 1);
    int seen[16] = {0}, distinct = 0;
    for (int i = 0; i < 1000; i++) seen[a[i].key] = 1;
    for (int i = 0; i < 16; i++) distinct += seen[i];
    CHECK(distinct <= 16, "중복많음 입력의 종류 수");
    free(a);
    Record *r1 = makeInput(SHAPE_RANDOM, 50, 5), *r2 = makeInput(SHAPE_RANDOM, 50, 5);
    CHECK(memcmp(r1, r2, 50 * sizeof(Record)) == 0, "같은 씨앗인데 입력이 다름");
    free(r1);
    free(r2);
}

int main(void) {
    for (size_t i = 0; i < SORT_ALGORITHM_COUNT; i++) {
        const SortAlgorithm *a = &SORT_ALGORITHMS[i];
        testBasic(a);
        testRandomAndSweep(a);
        testStability(a);
        testStats(a);
    }
    testShellGaps();
    testTools();
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
