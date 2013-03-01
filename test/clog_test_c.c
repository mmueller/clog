/* Quick and dirty tests for the logger. */

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CLOG_MAIN
#include "clog.h"

#include "clog_test.h"
#include "clog_test_cpp.h"

#define THIS_FILE "clog_test_c.c"
#define TEST_FILE "clog_test.out"

char error_text[16384];

void error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(error_text, 16384, fmt, ap);
    error_text[16383] = 0;
}

int test_double_init()
{
    CHECK_CALL(clog_init_path(0, TEST_FILE));
    if (clog_init_path(0, TEST_FILE) == 0) {
        return 1;
    }
    clog_free(0);
    return 0;
}

int test_file_write()
{
    FILE *f = NULL;
    char buf[256];

    /* Write to the log */
    CHECK_CALL(clog_init_path(0, TEST_FILE));
    CHECK_CALL(clog_set_fmt(0, "%f: %l: %m\n"));
    clog_debug(CLOG(0), "Hello, %s!", "world");
    clog_free(0);

    /* Verify what was written */
    f = fopen(TEST_FILE, "r");
    if (!f) {
        return 1;
    }

    if (fgets(buf, 256, f) == NULL) {
        fclose(f);
        return 1;
    }
    fclose(f);
    CHECK_CALL(strcmp(buf, THIS_FILE ": DEBUG: Hello, world!\n"));

    return 0;
}

int test_file_write_nonexistent()
{
    /* Expected to fail, so success is bad */
    if (clog_init_path(0, "path-doesnt-exist/log.out") == 0) {
        return 1;
    }
    if (_clog_loggers[0] != NULL) {
        return 1;
    }

    return 0;
}

int test_fd_write()
{
    char buf[1024];
    int fd[2];
    size_t bytes;

    CHECK_CALL(pipe(fd));
    CHECK_CALL(clog_init_fd(0, fd[1]));
    CHECK_CALL(clog_set_fmt(0, "%f: %l: %m\n"));
    clog_debug(CLOG(0), "Hello, %s!", "world");
    clog_free(0);
    close(fd[1]);

    bytes = read(fd[0], buf, 1024);
    if (bytes <= 0) {
        close(fd[0]);
        return 1;
    }
    buf[bytes] = 0;
    close(fd[0]);
    CHECK_CALL(strcmp(buf, THIS_FILE ": DEBUG: Hello, world!\n"));

    return 0;
}

int test_all_levels()
{
    char buf[1024];
    int fd[2];
    size_t bytes;

    CHECK_CALL(pipe(fd));
    CHECK_CALL(clog_init_fd(0, fd[1]));
    CHECK_CALL(clog_set_fmt(0, "%f: %l: %m\n"));
    clog_debug(CLOG(0), "Hello, %s!", "world");
    clog_info(CLOG(0), "Hello, %s!", "world");
    clog_warn(CLOG(0), "Hello, %s!", "world");
    clog_error(CLOG(0), "Hello, %s!", "world");
    clog_free(0);
    close(fd[1]);

    bytes = read(fd[0], buf, 1024);
    if (bytes <= 0) {
        close(fd[0]);
        return 1;
    }
    buf[bytes] = 0;
    close(fd[0]);
    CHECK_CALL(strcmp(buf,
        THIS_FILE ": DEBUG: Hello, world!\n"
        THIS_FILE ": INFO: Hello, world!\n"
        THIS_FILE ": WARN: Hello, world!\n"
        THIS_FILE ": ERROR: Hello, world!\n"
    ));

    return 0;
}

int test_level_filtering()
{
    char buf[1024];
    int fd[2];
    size_t bytes;

    /* Write to log with level == WARN */
    CHECK_CALL(pipe(fd));
    CHECK_CALL(clog_init_fd(0, fd[1]));
    CHECK_CALL(clog_set_fmt(0, "%f: %l: %m\n"));
    CHECK_CALL(clog_set_level(0, CLOG_WARN));
    clog_debug(CLOG(0), "Hello, %s!", "world");
    clog_info(CLOG(0), "Hello, %s!", "world");
    clog_warn(CLOG(0), "Hello, %s!", "world");
    clog_error(CLOG(0), "Hello, %s!", "world");
    clog_free(0);
    close(fd[1]);

    /* Should receive only messages >= WARN */
    bytes = read(fd[0], buf, 1024);
    if (bytes <= 0) {
        close(fd[0]);
        return 1;
    }
    buf[bytes] = 0;
    close(fd[0]);
    CHECK_CALL(strcmp(buf,
        THIS_FILE ": WARN: Hello, world!\n"
        THIS_FILE ": ERROR: Hello, world!\n"
    ));

    return 0;
}

int test_multiple_loggers()
{
    char buf[1024];
    size_t bytes;
    int id;

    for (id = 0; id < CLOG_MAX_LOGGERS; id++) {
        int fd[2];
        char exp[256];
        CHECK_CALL(pipe(fd));
        CHECK_CALL(clog_init_fd(id, fd[1]));
        CHECK_CALL(clog_set_fmt(id, "%f: %l: %m\n"));
        clog_debug(CLOG(id), "Hello, %d!", id);
        close(fd[1]);
        bytes = read(fd[0], buf, 1024);
        if (bytes <= 0) {
            close(fd[0]);
            return 1;
        }
        buf[bytes] = 0;
        close(fd[0]);
        snprintf(exp, 256, "%s: DEBUG: Hello, %d!\n", THIS_FILE, id);
        CHECK_CALL(strcmp(buf, exp));
    }
    for (id = 0; id < CLOG_MAX_LOGGERS; id++) {
        clog_free(id);
    }

    return 0;
}

int test_bad_format()
{
    char too_long[300];
    memset(too_long, 'a', 299);
    too_long[299] = 0;

    CHECK_CALL(clog_init_path(0, "clog_test.out"));
    if (clog_set_fmt(0, too_long) == 0) {
        return 1;
    }
    clog_free(0);
    return 0;
}

int test_long_message()
{
    FILE *f = NULL;
    char buf[51000];
    char message[50000];
    char exp[51000];
    memset(message, 'b', 49999);
    message[49999] = 0;

    /* Write to the log */
    CHECK_CALL(clog_init_path(0, TEST_FILE));
    CHECK_CALL(clog_set_fmt(0, "%f: %l: %m\n"));
    clog_debug(CLOG(0), message);
    clog_free(0);

    /* Verify what was written */
    f = fopen(TEST_FILE, "r");
    if (!f) {
        return 1;
    }

    if (fgets(buf, 51000, f) == NULL) {
        fclose(f);
        return 1;
    }
    fclose(f);

    snprintf(exp, 51000, "%s: DEBUG: %s\n", THIS_FILE, message);
    CHECK_CALL(strcmp(buf, exp));
    return 0;
}

int test_performance()
{
    const int MICROS_PER_SEC = 1000000;
    const size_t NUM_MESSAGES = 200000;
    unsigned long start_time, end_time;
    struct timeval tv;
    size_t messages = 0;
    double run_time;
    unsigned long messages_per_second;

    /* Init */
    CHECK_CALL(clog_init_path(0, TEST_FILE));

    /* Run test */
    CHECK_CALL(gettimeofday(&tv, NULL));
    start_time = tv.tv_sec * MICROS_PER_SEC + tv.tv_usec;
    for (messages = 0; messages < NUM_MESSAGES; messages++) {
        clog_debug(CLOG(0), "Hello, %s!", "high-performing world");
    }
    CHECK_CALL(gettimeofday(&tv, NULL));
    end_time = tv.tv_sec * MICROS_PER_SEC + tv.tv_usec;
    clog_free(0);

    /* Goal: 100,000 messages per second. */
    run_time = (end_time - start_time) / (double) MICROS_PER_SEC;
    messages_per_second = messages / run_time;
    error("  Target 100000 msgs/sec, got %lu.\n", messages_per_second);
    if (messages_per_second < 100000) {
        return 1;
    }

    return 0;
}

typedef int (*test_function_t)();

typedef struct {
    const char *name;
    test_function_t function;
    int pass;
} test_case;

#define TEST_CASE(name) { #name, name, -1 }

int main(int argc, char *argv[])
{
    test_case tests[] = {
        // C tests
        TEST_CASE(test_double_init),
        TEST_CASE(test_file_write),
        TEST_CASE(test_file_write_nonexistent),
        TEST_CASE(test_fd_write),
        TEST_CASE(test_all_levels),
        TEST_CASE(test_level_filtering),
        TEST_CASE(test_multiple_loggers),
        TEST_CASE(test_bad_format),
        TEST_CASE(test_long_message),

        // C++ tests
        TEST_CASE(test_cpp_hello),

        // Performance tests
        TEST_CASE(test_performance)
    };

    const size_t num_tests = sizeof(tests) / sizeof(test_case);
    const char *test_name = NULL;
    int pass = 0, fail = 0;
    size_t i, j;

    if (argc > 1) {
        test_name = argv[1];
    }

    for (i = 0; i < num_tests; i++) {
        if (test_name && strcmp(tests[i].name, test_name) != 0) {
            continue;
        }
        if (unlink(TEST_FILE) == -1 && errno != ENOENT) {
            perror("unlink");
        }
        printf("%s: ", tests[i].name);
        if (tests[i].function() == 0) {
            tests[i].pass = 1;
            pass++;
            printf("OK\n");
        } else {
            tests[i].pass = 0;
            fail++;
            printf("FAIL\n");
        }
        if (strlen(error_text)) {
            printf("%s", error_text);
        }

        /* Restore global state in case test didn't clean up */
        for (j = 0; j < CLOG_MAX_LOGGERS; j++) {
            _clog_loggers[j] = NULL;
        }
        error_text[0] = '\0';
    }
    printf("\n");
    if (pass == 0 && fail == 0) {
        printf("No such test: %s\n", test_name);
        return 1;
    } else {
        printf("%d successes, %d failures.\n", pass, fail);
    }

    if (fail > 0) {
        printf("Failing cases:\n");
        for (i = 0; i < num_tests; ++i) {
            if (tests[i].pass == 0) {
                printf("    %s\n", tests[i].name);
            }
        }
    }

    return fail ? 1 : 0;
}
