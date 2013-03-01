all:
	@echo "clog does not need to be built, just copy clog.h into your project."
	@echo ""
	@echo "To run tests, run 'make check'."

check:
	@$(MAKE) -w -C test check

clean:
	@$(MAKE) -w -C test clean

.PHONY: clean check
