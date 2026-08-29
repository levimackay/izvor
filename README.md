# tinylang

A small programming language, implemented from scratch in C: a lexer, a
recursive-descent parser, a tree-walking interpreter, then a bytecode
compiler and stack-based VM. "tinylang" is a placeholder name — the syntax
gets redesigned once the mechanics are solid.

## Status

The lexer is done and tested. It scans integer literals, `+ - * /`, and
parentheses, and returns an error token on anything else. The parser,
interpreter, compiler, and VM don't exist yet.

```
$ make lexdump && ./build/lexdump "12 + 3 * (40 - 5)"
NUMBER   '12'   value=12
PLUS     '+'
NUMBER   '3'   value=3
STAR     '*'
LPAREN   '('
NUMBER   '40'   value=40
MINUS    '-'
NUMBER   '5'   value=5
RPAREN   ')'
EOF
```

Next up is the recursive-descent parser, turning that token stream into an AST.

## Build and run

Everything builds with `clang` via the Makefile, no dependencies.

```sh
make p1-1       # build + run lexer test 1
make p1-4       # ... test 4
make lexdump    # build the token-dump CLI
make clean      # delete build artifacts
```

All four lexer tests pass.

## Layout

- `src/` — the language implementation
- `tests/` — hand-rolled tests, plain `main()` + `assert()`, no framework

## License

MIT — see [LICENSE](LICENSE).
