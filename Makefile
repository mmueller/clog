all:
	@echo "clog does not need to be built, just copy clog.h into your project."
	@echo ""
	@echo "To run tests, run 'make check'."

c:
	gcc -g -oclog_test -Wall -Wextra -Werror -std=c89 -pedantic $(CFLAGS) clog_test.c

cpp:
	g++ -g -oclog_test -Wall -Wextra -Werror -std=c++98 -pedantic $(CFLAGS) clog_test.c

check:
	@echo "Testing C++98 compatibility..."
	make cpp
	@echo "Testing C89 compatibility..."
	make c
	@echo "Running tests..."
	make CFLAGS="-DCLOG_SILENT" c
	./clog_test

clean:
	rm -f clog_test clog_test.out

.PHONY: clean check c cpp
