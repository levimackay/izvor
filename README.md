# tinylang

A small programming language, implemented from scratch in C: a lexer, a
recursive-descent parser, a tree-walking interpreter, then a bytecode
compiler and stack-based VM. "tinylang" is a placeholder name — the syntax
gets redesigned once the mechanics are solid.

## Status

Early. The lexer is implemented and tested (`src/lexer.c`); the parser,
interpreter, compiler, and VM don't exist yet.

## Build and run

Everything builds with `clang` via the Makefile, no dependencies.

```sh
make p1-1       # build + run lexer test 1
make p1-4       # ... test 4
make lexdump    # build the token-dump CLI
make clean      # delete build artifacts
```

## Layout

- `src/` — the language implementation
- `tests/` — hand-rolled tests, plain `main()` + `assert()`, no framework

## License

MIT — see [LICENSE](LICENSE).
