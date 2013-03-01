/* Common utilities and globals for tests. */
#ifndef __CLOG_TEST_H__
#define __CLOG_TEST_H__

#define CHECK_CALL(call) do { if ((call) != 0) { return 1; } } while (0)

void error(const char *fmt, ...);

#endif /* __CLOG_TEST_H__ */
