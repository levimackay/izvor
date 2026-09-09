# izvor

A programming language, implemented from scratch in C: a lexer, a
recursive-descent parser, a tree-walking interpreter, then a bytecode
compiler and stack-based VM. *Izvor* is Serbian/Croatian for "source" or
"spring", and source files use the `.iz` extension.

## Status

The pipeline now runs end to end for arithmetic. The lexer scans integer
literals, `+ - * /`, and parentheses. The parser is classic recursive
descent, one function per precedence level (expression, term, factor),
and builds a heap-allocated AST with number, unary, and binary nodes.
A small CLI ties it together: parse one expression, print the tree in
S-expression form, evaluate it with a tree walk.

```
$ make izvor && ./build/izvor "12 + 3 * (40 - 5)"
(+ 12 (* 3 (- 40 5)))
= 117
```

Parse errors report the character offset, what was expected, and what
was actually there. The whole input has to parse, so trailing garbage is
rejected instead of ignored. Division by zero is caught at evaluation
time rather than left as undefined behavior.

Not built yet: statements (`let`, `print`), variables, types, and
everything past expressions. The parser is covered by two tests: the
token helpers, and the shape of the trees `parser_parse` builds.

## Build and run

Everything builds with `clang` via the Makefile, no dependencies.

```sh
make izvor      # build the expression CLI
make lexdump    # build the token-dump CLI
make p1-1       # build + run a lexer test (through p1-4)
make p2-1       # build + run a parser test (through p2-2)
make clean      # delete build artifacts
```

All four lexer tests and both parser tests pass.

## Layout

- `src/` holds the language implementation
- `tests/` holds hand-rolled tests, plain `main()` + `assert()`, no framework

## License

MIT, see [LICENSE](LICENSE).
