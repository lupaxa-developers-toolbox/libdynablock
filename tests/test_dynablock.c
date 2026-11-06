/* Quoted path so a syntax check without -I still finds the public header. */
#include "../include/dynablock.h"

#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;
static int tests_run = 0;
static int tests_failed = 0;
static const char *current_test = NULL;

static void expect(int condition, const char *file, int line, const char *expr)
{
    if (!condition) {
        printf("FAIL %s %s:%d: %s\n",
               current_test != NULL ? current_test : "?",
               file, line, expr);
        fflush(stdout);
        failures++;
    }
}

#define EXPECT(cond) expect((cond), __FILE__, __LINE__, #cond)

static int semver_major_minor(const char *version)
{
    int parts = 0;

    if (version == NULL) {
        return 0;
    }

    while (parts < 2) {
        if (*version < '0' || *version > '9') {
            return 0;
        }
        while (*version >= '0' && *version <= '9') {
            version++;
        }
        parts++;
        if (parts == 2) {
            return 1;
        }
        if (*version != '.') {
            return 0;
        }
        version++;
    }
    return 0;
}

static void test_version_string_is_semver(void)
{
    EXPECT(semver_major_minor(DYNABLOCK_VERSION_STRING));
}

static void test_empty_pool(void)
{
    dynablock *pool = NULL;

    EXPECT(dynablock_create(NULL) == DYNABLOCK_INVAL);
    EXPECT(dynablock_create(&pool) == DYNABLOCK_OK);
    EXPECT(pool != NULL);
    EXPECT(dynablock_count(pool) == 0);
    EXPECT(dynablock_count(NULL) == 0);
    EXPECT(dynablock_destroy(&pool) == DYNABLOCK_OK);
    EXPECT(pool == NULL);
    EXPECT(dynablock_destroy(&pool) == DYNABLOCK_OK);
    EXPECT(dynablock_destroy(NULL) == DYNABLOCK_INVAL);
}

static void test_get_returns_exact_zeroed_block(void)
{
    dynablock *pool = NULL;
    unsigned char *block = NULL;
    void *out = NULL;
    size_t i;
    double *number = NULL;

    EXPECT(dynablock_create(&pool) == DYNABLOCK_OK);
    EXPECT(dynablock_get(pool, 0, &out) == DYNABLOCK_INVAL);
    EXPECT(out == NULL);
    EXPECT(dynablock_get(pool, SIZE_MAX, &out) == DYNABLOCK_INVAL);
    EXPECT(out == NULL);
    EXPECT(dynablock_count(pool) == 0);
    EXPECT(dynablock_get(pool, 12, &out) == DYNABLOCK_OK);
    EXPECT(out != NULL);
    EXPECT(((uintptr_t)out % alignof(max_align_t)) == 0);
    block = out;
    for (i = 0; i < 12; i++) {
        EXPECT(block[i] == 0);
    }
    memcpy(block, "Tim Gurney", 11);
    EXPECT(strcmp((char *)block, "Tim Gurney") == 0);
    EXPECT(dynablock_count(pool) == 1);
    EXPECT(dynablock_get(pool, sizeof(*number), (void **)&number) == DYNABLOCK_OK);
    *number = 1.5;
    EXPECT(*number == 1.5);
    EXPECT(dynablock_count(pool) == 2);
    EXPECT(dynablock_destroy(&pool) == DYNABLOCK_OK);
}

static void test_release_one_block(void)
{
    dynablock *pool = NULL;
    char *first = NULL;
    char *second = NULL;

    EXPECT(dynablock_create(&pool) == DYNABLOCK_OK);
    EXPECT(dynablock_get(pool, 8, (void **)&first) == DYNABLOCK_OK);
    EXPECT(dynablock_get(pool, 8, (void **)&second) == DYNABLOCK_OK);
    memcpy(first, "one", 4);
    memcpy(second, "two", 4);
    EXPECT(dynablock_release(pool, second) == DYNABLOCK_OK);
    EXPECT(dynablock_count(pool) == 1);
    EXPECT(strcmp(first, "one") == 0);
    EXPECT(dynablock_release(pool, second) == DYNABLOCK_INVAL);
    EXPECT(dynablock_count(pool) == 1);
    EXPECT(dynablock_release(pool, first) == DYNABLOCK_OK);
    EXPECT(dynablock_count(pool) == 0);
    EXPECT(dynablock_destroy(&pool) == DYNABLOCK_OK);
}

static void test_pools_are_independent(void)
{
    dynablock *left = NULL;
    dynablock *right = NULL;
    char *block = NULL;

    EXPECT(dynablock_create(&left) == DYNABLOCK_OK);
    EXPECT(dynablock_create(&right) == DYNABLOCK_OK);
    EXPECT(dynablock_get(left, 4, (void **)&block) == DYNABLOCK_OK);
    memcpy(block, "ab", 3);
    EXPECT(dynablock_release(right, block) == DYNABLOCK_INVAL);
    EXPECT(dynablock_count(left) == 1);
    EXPECT(dynablock_count(right) == 0);
    EXPECT(strcmp(block, "ab") == 0);
    EXPECT(dynablock_release(left, block) == DYNABLOCK_OK);
    EXPECT(dynablock_destroy(&left) == DYNABLOCK_OK);
    EXPECT(dynablock_destroy(&right) == DYNABLOCK_OK);
}

static void test_invalid_arguments(void)
{
    dynablock *pool = NULL;
    void *block = NULL;

    EXPECT(dynablock_create(&pool) == DYNABLOCK_OK);
    EXPECT(dynablock_get(NULL, 4, &block) == DYNABLOCK_INVAL);
    EXPECT(dynablock_get(pool, 4, NULL) == DYNABLOCK_INVAL);
    EXPECT(dynablock_count(pool) == 0);
    EXPECT(dynablock_release(NULL, &block) == DYNABLOCK_INVAL);
    EXPECT(dynablock_release(pool, NULL) == DYNABLOCK_INVAL);
    EXPECT(dynablock_release(pool, &block) == DYNABLOCK_INVAL);
    EXPECT(dynablock_destroy(&pool) == DYNABLOCK_OK);
}

static void run_test(const char *name, void (*fn)(void))
{
    int before = failures;

    current_test = name;
    fn();
    tests_run++;
    if (failures == before) {
        printf("tests/test_dynablock.c::%s PASSED\n", name);
    } else {
        tests_failed++;
        printf("tests/test_dynablock.c::%s FAILED\n", name);
    }
    fflush(stdout);
}

int main(void)
{
    int passed;

    run_test("test_version_string_is_semver", test_version_string_is_semver);
    run_test("test_empty_pool", test_empty_pool);
    run_test("test_get_returns_exact_zeroed_block", test_get_returns_exact_zeroed_block);
    run_test("test_release_one_block", test_release_one_block);
    run_test("test_pools_are_independent", test_pools_are_independent);
    run_test("test_invalid_arguments", test_invalid_arguments);

    passed = tests_run - tests_failed;
    if (tests_failed != 0) {
        printf("%d failed, %d passed, %d assertion(s) failed\n",
               tests_failed, passed, failures);
        return 1;
    }
    printf("%d passed\n", passed);
    return 0;
}
