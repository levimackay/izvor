# izvor Makefile — plain clang, no dependencies.
#
#   make            build the compiler and the token dumper
#   make test       build and run every test, then the golden error tests
#   make fuzz       throw pseudo-random input at the lexer and parser
#   make asan       rebuild under Address and LeakSanitizer and run it all
#   make clean      remove build artifacts
#
# Everything builds with UndefinedBehaviorSanitizer: out-of-bounds reads,
# signed overflow and other undefined behavior abort with a report naming
# the exact line instead of failing quietly.
# AddressSanitizer, which is what catches leaks and use-after-free, is not
# in that default: its runtime deadlocks on startup under Apple clang 17 on
# macOS 26.5 — verified again Sep 2026. `make asan` turns it on for
# toolchains where it works, and CI runs that target on Linux.
# (macOS `leaks` is not a substitute. It reports zero on a deliberately
# leaked block here, because the process is not debuggable under the
# current security policy, so a target built on it would be a green light
# wired to nothing.)

CC     := clang
# EXTRA_CFLAGS is how CI turns warnings into errors without this line
# and that one drifting apart.
CFLAGS := -std=c11 -Wall -Wextra -g -fsanitize=undefined $(EXTRA_CFLAGS)
BUILD  := build

# Every compiler source except the drivers, so tests link against the
# real thing rather than a copy that can drift out of step.
SRC := src/lexer.c src/ast.c src/parser.c src/diag.c

TEST_SRC := $(wildcard tests/test_*.c)
TEST_BIN := $(patsubst tests/%.c,$(BUILD)/%,$(TEST_SRC))
GOLDEN   := $(wildcard tests/golden/*.iz)

.PHONY: all izvor lexdump test golden fuzz asan clean

all: $(BUILD)/izvor $(BUILD)/lexdump

# Short aliases, so `make lexdump` means what it looks like it means.
izvor:   $(BUILD)/izvor
lexdump: $(BUILD)/lexdump

$(BUILD)/izvor: $(SRC) src/main.c | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/lexdump: src/lexer.c src/lexer_main.c | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/test_%: tests/test_%.c $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/fuzz: tests/fuzz.c $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

# Run every test binary, keep going after a failure, and fail the whole
# target at the end. Stopping at the first failure hides how much broke.
test: $(TEST_BIN) golden
	@failed=0; \
	for t in $(TEST_BIN); do \
		if ! ./$$t; then failed=1; fi; \
	done; \
	if [ $$failed -ne 0 ]; then echo "FAILED"; exit 1; fi; \
	echo "all unit tests passed"

# Golden tests pin the exact text of compiler errors. A diagnostic is a
# user interface, so changing one should be a deliberate act that shows
# up in a diff, not a side effect noticed by nobody.
golden: $(BUILD)/izvor
	@failed=0; \
	for f in $(GOLDEN); do \
		expected="$${f%.iz}.expected"; \
		actual="$(BUILD)/$$(basename $${f%.iz}).actual"; \
		./$(BUILD)/izvor "$$f" > "$$actual" 2>&1 || true; \
		if ! diff -u "$$expected" "$$actual"; then \
			echo "golden: $$f does not match $$expected"; failed=1; \
		fi; \
	done; \
	if [ $$failed -ne 0 ]; then exit 1; fi; \
	echo "golden: $(words $(GOLDEN)) cases match"

fuzz: $(BUILD)/fuzz
	./$(BUILD)/fuzz

# The front end allocates one Node per tree node and nothing else, so
# anything LeakSanitizer reports is a missing ast_free. Recursing into
# make keeps this a one-word command rather than a flag to remember.
asan:
	$(MAKE) clean
	$(MAKE) EXTRA_CFLAGS="-fsanitize=address -fno-omit-frame-pointer" test fuzz
	$(MAKE) clean

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
