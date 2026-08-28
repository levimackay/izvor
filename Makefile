# tinylang Makefile — plain clang, no dependencies.
#
#   make p0-02 .. p0-10    build + run a Phase 0 task's test
#   make p1-1  .. p1-4     build + run a Phase 1 task's test
#   make lexdump           build the token-dump tool (task 1.5)
#   make clean             remove build artifacts
#
# Every test builds with UndefinedBehaviorSanitizer: out-of-bounds array
# writes, overflow, and other undefined behavior crash loudly with a report
# naming the line, instead of corrupting things silently.
# (AddressSanitizer would normally join it, but its runtime deadlocks on
# startup with the current Apple clang 17 / macOS 26.5 combo — verified
# Aug 2026. If `clang --version` has moved on, try adding `address,` back
# before `undefined` and see if a test binary runs.)
# Task 0.9's job is to make every line below make sense.

CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -g -fsanitize=undefined
BUILD  := build

# Task 0.9 is special: it links the two-file intarray module.
p0-09: | $(BUILD)
	$(CC) $(CFLAGS) phase0/intarray.c phase0/09_link_test.c -o $(BUILD)/$@
	./$(BUILD)/$@

# Any other Phase 0 task: compile every phase0/NN_*.c file together.
# ($* is whatever matched the %, e.g. "02"; the shell expands the glob.)
p0-%: | $(BUILD)
	$(CC) $(CFLAGS) phase0/$*_*.c -o $(BUILD)/$@
	./$(BUILD)/$@

# Phase 1 task N: the lexer + that task's test.
p1-%: | $(BUILD)
	$(CC) $(CFLAGS) src/lexer.c tests/test_lexer_$*.c -o $(BUILD)/$@
	./$(BUILD)/$@

lexdump: | $(BUILD)
	$(CC) $(CFLAGS) src/lexer.c src/lexer_main.c -o $(BUILD)/lexdump

# "Order-only prerequisite": make sure build/ exists first.
$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: clean lexdump
