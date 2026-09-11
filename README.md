# izvor

[![ci](https://github.com/levimackay/izvor/actions/workflows/ci.yml/badge.svg)](https://github.com/levimackay/izvor/actions/workflows/ci.yml)

A statically typed programming language and its compiler, written from
scratch in C11 with no dependencies. *Izvor* is Serbian and Croatian for
"source" or "spring", and source files use the `.iz` extension.

The front end is finished: a hand-written lexer, a recursive-descent
parser, and a heap-allocated syntax tree, with diagnostics that point at
the offending character. The back end will generate C and hand it to
clang. There is no parser generator, no LLVM, and no third-party library
anywhere in the build.

## Running it

```console
$ make
$ ./build/izvor -e "12 + 3 * (40 - 5)"
(+ 12 (* 3 (- 40 5)))
= 117
```

The first line is the parsed tree printed back as an S-expression, so
precedence and associativity are visible rather than assumed. The second
is the value.

It reads files too:

```console
$ ./build/izvor program.iz
```

## Errors

Getting this right early was a priority, because a compiler is mostly a
tool for telling people what is wrong with their program.

```console
$ ./build/izvor tests/golden/unclosed-paren.iz
error: expected ')', found EOF
 --> tests/golden/unclosed-paren.iz:1:8
  |
1 | (12 + 3
  |        ^
```

Every stage reports through one module, so there is a single definition
of what an izvor error looks like. Tabs are expanded when the line is
echoed so the caret stays under the right character, and an error at the
end of a file points at the last line with something on it rather than at
the blank one after it. The exact text of all eight diagnostics is pinned
by golden tests, alongside the output of a program that compiles cleanly,
so changing any of them is a deliberate act that shows up in a diff.

## Design

- **Tagged union AST.** One `NodeType` tag and a union of payloads. No
  `switch` over that tag has a `default` case, so adding a node kind
  makes the compiler list every place that has to handle it.
- **Tokens borrow the source.** A token is a type, a pointer into the
  source buffer, and a length. Nothing is copied, nothing is freed, and
  the driver reads the whole file into one allocation so that stays true.
- **One function per precedence level.** The grammar rule, the parser
  function, and the resulting tree shape are the same idea written three
  ways.
- **Division by zero is defined.** C leaves it undefined. izvor reports
  it and stops.

The reasoning behind each of these, and the list of things that are
measurably still wrong, is in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Tests

```console
$ make test      # unit tests plus golden diagnostic tests
$ make fuzz      # 20,000 pseudo-random inputs through the front end
$ make asan      # rebuild under Address and LeakSanitizer
```

Plain `main()` and `assert()`, no framework. Unit tests cover the lexer,
the parser's token helpers, the tree shapes it builds, and the line and
column math. Golden tests pin the exact output of eight broken programs
and one correct one. The fuzzer asserts
that no input, however malformed, crashes the lexer or the parser; its
seed is fixed so a CI failure reproduces exactly. Everything builds with
UndefinedBehaviorSanitizer, CI runs the lot on Linux and macOS with
warnings promoted to errors, and a Linux job repeats it under Address and
LeakSanitizer.

## Status

Working today: integer arithmetic with correct precedence and
associativity, unary minus, parenthesized grouping, whole-input parsing
that rejects trailing tokens, and a tree-walking evaluator.

Not built yet: statements, variables, types, functions, and code
generation. The plan for each, in order, is in
[docs/ROADMAP.md](docs/ROADMAP.md).

## Layout

```
src/      the compiler
tests/    unit tests, golden diagnostic tests, and the fuzzer
docs/     architecture and roadmap
```

## License

MIT, see [LICENSE](LICENSE).
