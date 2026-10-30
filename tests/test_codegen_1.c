#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/check.h"
#include "../src/codegen.h"
#include "../src/parser.h"

static char code[32768];

static const char *emit(const char *src) {
    Parser p;
    parser_init(&p, src);
    Program prog = parser_parse_program(&p);
    bool checked = !p.had_error && check_program(src, &prog);
    assert(checked);

    FILE *f = tmpfile();
    assert(f != NULL);
    codegen_emit(f, src, &prog);
    rewind(f);
    size_t length = fread(code, 1, sizeof code - 1, f);
    assert(length < sizeof code - 1);
    code[length] = '\0';
    fclose(f);
    program_free(&prog);
    return code;
}

static int loops(const char *c) {
    int count = 0;
    for (const char *s = strstr(c, "while ("); s != NULL; s = strstr(s + 1, "while (")) {
        assert(strncmp(s, "while (true) {", 14) == 0 || strncmp(s, "while (false) {", 15) == 0);
        count++;
    }
    return count;
}

int main(void) {
    const char *c;

    c = emit("fn wait(n: Int) -> Int { var x = n; while x != 0 { x = x + 0; } return x; }\n"
             "print(1);\n"
             "print(wait(1));\n");
    assert(loops(c) == 1);
    assert(strstr(c, "if (!(iz_x != 0)) break;") != NULL);

    c = emit("fn same(n: Int) -> Int { return n; }\n"
             "var x = 1;\n"
             "while same(x) != same(0) { x = x + 0; }\n");
    assert(loops(c) == 1);
    assert(strstr(c, "long t1 = iz_same(iz_x);") != NULL);
    assert(strstr(c, "if (!(t1 != iz_same(0))) break;") != NULL);

    c = emit("var going = true;\n"
             "var i = 0;\n"
             "while going {\n"
             "    var j = 0;\n"
             "    while j < i { j = j + 1; }\n"
             "    i = i + 1;\n"
             "    going = i < 10;\n"
             "}\n");
    assert(loops(c) == 2);
    assert(strstr(c, "if (!iz_going) break;") != NULL);
    assert(strstr(c, "if (!(iz_j < iz_i)) break;") != NULL);

    c = emit("fn forever() -> Int { while true { print(1); } }\n");
    assert(loops(c) == 1);
    assert(strstr(c, "break;") == NULL);

    printf("test_codegen_1 passed\n");
    return 0;
}
