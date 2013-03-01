CC ?= gcc
CXX ?= g++
CFLAGS ?= -g -DCLOG_SILENT -Wall -Wextra -Werror -pedantic
CFLAGS += -I ..

all: clog_test

# Only tests C99 conformance because some compilers will not make vsnprintf
# available in C89 mode. :-/
clog_test_c.o: clog_test_c.c clog_test.h clog_test_cpp.h ../clog.h
	$(CC) -c -std=c99 $(CFLAGS) $<

clog_test_cpp.o: clog_test_cpp.cpp clog_test.h clog_test_cpp.h ../clog.h
	$(CXX) -c -std=c++98 $(CFLAGS) $<

clog_test: clog_test_c.o clog_test_cpp.o
	$(CXX) -o clog_test $+

check: clog_test
	@./clog_test

clean:
	rm -f clog_test clog_test.out *.o

.PHONY: clean check
