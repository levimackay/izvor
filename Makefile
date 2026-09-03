# izvor Makefile — plain clang, no dependencies.
#
#   make p1-1 .. p1-4    build + run a lexer test
#   make p2-1            build + run an AST test
#   make lexdump         build the token-dump CLI
#   make clean           remove build artifacts
#
# Tests build with UndefinedBehaviorSanitizer: out-of-bounds access,
# overflow, and other undefined behavior crash loudly with a report naming
# the exact line, instead of failing silently.
# (AddressSanitizer would normally join it, but its runtime deadlocks on
# startup with Apple clang 17 / macOS 26.5 — verified Aug 2026. Try adding
# `address,` back before `undefined` if your toolchain has moved on.)

CC     := clang
CFLAGS := -std=c11 -Wall -Wextra -g -fsanitize=undefined
BUILD  := build

p1-%: | $(BUILD)
	$(CC) $(CFLAGS) src/lexer.c tests/test_lexer_$*.c -o $(BUILD)/$@
	./$(BUILD)/$@

p2-%: | $(BUILD)
	$(CC) $(CFLAGS) src/lexer.c src/ast.c src/parser.c tests/test_parser_$*.c -o $(BUILD)/$@
	./$(BUILD)/$@

lexdump: | $(BUILD)
	$(CC) $(CFLAGS) src/lexer.c src/lexer_main.c -o $(BUILD)/lexdump

izvor: | $(BUILD)
	$(CC) $(CFLAGS) src/lexer.c src/ast.c src/parser.c src/main.c -o $(BUILD)/izvor

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: clean lexdump
