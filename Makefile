CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -g -fsanitize=undefined $(EXTRA_CFLAGS)
BUILD  := build

SRC := src/lexer.c src/ast.c src/parser.c src/diag.c src/check.c src/interp.c src/codegen.c

TEST_SRC := $(wildcard tests/test_*.c)
TEST_BIN := $(patsubst tests/%.c,$(BUILD)/%,$(TEST_SRC))
GOLDEN   := $(wildcard tests/golden/*.iz)
ENDLESS  := $(wildcard tests/endless/*.iz)

.PHONY: all izvor lexdump test golden endless fuzz asan clean

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

test: $(TEST_BIN) golden endless
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

endless: $(BUILD)/izvor
	@failed=0; \
	for f in $(ENDLESS); do \
		name="$$(basename $${f%.iz})"; \
		if ! ./$(BUILD)/izvor build "$$f" -o "$(BUILD)/$$name.endless"; then \
			echo "endless: izvor build $$f failed"; failed=1; continue; \
		fi; \
		./$(BUILD)/izvor run "$$f" > /dev/null 2>&1 & run=$$!; \
		./$(BUILD)/$$name.endless > /dev/null 2>&1 & built=$$!; \
		sleep 1; \
		kill $$run $$built 2> /dev/null; \
		wait $$run 2> /dev/null; status=$$?; \
		if [ $$status -le 128 ] || [ "$$(kill -l $$status)" != TERM ]; then \
			echo "endless: izvor run $$f exited with $$status before it was killed"; failed=1; \
		fi; \
		wait $$built 2> /dev/null; status=$$?; \
		if [ $$status -le 128 ] || [ "$$(kill -l $$status)" != TERM ]; then \
			echo "endless: the compiled $$f exited with $$status before it was killed"; failed=1; \
		fi; \
	done; \
	if [ $$failed -ne 0 ]; then exit 1; fi; \
	echo "endless: every case ran until it was killed, interpreted and compiled"

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
