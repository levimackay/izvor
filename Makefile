CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -g -fsanitize=undefined $(EXTRA_CFLAGS)
BUILD  := build

SRC := src/lexer.c src/ast.c src/parser.c src/diag.c src/check.c src/interp.c src/codegen.c

TEST_SRC := $(wildcard tests/test_*.c)
TEST_BIN := $(patsubst tests/%.c,$(BUILD)/%,$(TEST_SRC))
GOLDEN   := $(wildcard tests/golden/*.iz)

.PHONY: all izvor lexdump test golden fuzz asan clean

all: $(BUILD)/izvor $(BUILD)/lexdump

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

test: $(TEST_BIN) golden
	@failed=0; \
	for t in $(TEST_BIN); do \
		if ! ./$$t; then failed=1; fi; \
	done; \
	if [ $$failed -ne 0 ]; then echo "FAILED"; exit 1; fi; \
	echo "all unit tests passed"

golden: $(BUILD)/izvor
	@failed=0; \
	for f in $(GOLDEN); do \
		expected="$${f%.iz}.expected"; \
		name="$$(basename $${f%.iz})"; \
		run="$(BUILD)/$$name.run"; \
		built="$(BUILD)/$$name.built"; \
		./$(BUILD)/izvor run "$$f" > "$$run" 2>&1 || true; \
		if ! diff -u "$$expected" "$$run"; then \
			echo "golden: izvor run $$f does not match $$expected"; failed=1; \
		fi; \
		if ./$(BUILD)/izvor build "$$f" -o "$(BUILD)/$$name.bin" > "$$built" 2>&1; then \
			./$(BUILD)/$$name.bin >> "$$built" 2>&1 || true; \
		fi; \
		if ! diff -u "$$expected" "$$built"; then \
			echo "golden: the compiled $$f does not match $$expected"; failed=1; \
		fi; \
	done; \
	if [ $$failed -ne 0 ]; then exit 1; fi; \
	echo "golden: $(words $(GOLDEN)) cases match, interpreted and compiled"

fuzz: $(BUILD)/fuzz
	./$(BUILD)/fuzz

asan:
	$(MAKE) clean
	$(MAKE) EXTRA_CFLAGS="-fsanitize=address -fno-omit-frame-pointer" test fuzz
	$(MAKE) clean

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
