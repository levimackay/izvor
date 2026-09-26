# izvor

[![ci](https://github.com/levimackay/izvor/actions/workflows/ci.yml/badge.svg)](https://github.com/levimackay/izvor/actions/workflows/ci.yml)

A small statically typed programming language and its compiler, written
from scratch in C11 with no dependencies. *Izvor* is Serbian and Croatian
for "source" or "spring", and source files use the `.iz` extension.

izvor compiles to C and hands that to your system's C compiler, so a
program ends up as a normal native executable. It can also run a program
directly with a tree-walking interpreter, which the test suite uses to
check the compiled output against.

```
fn fib(n: Int) -> Int {
    if n < 2 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

var i = 0;
while i < 10 {
    print(fib(i));
    i = i + 1;
}
```

## Running it

```console
$ make
$ ./build/izvor build fib.iz        # writes ./fib
$ ./fib
$ ./build/izvor run fib.iz          # interpret it instead
$ ./build/izvor emit fib.iz         # print the generated C
$ ./build/izvor -e "12 + 3 * (40 - 5)"
(+ 12 (* 3 (- 40 5)))
= 117
```

`build` needs a C compiler on your PATH. It uses `cc` unless `CC` says
otherwise. `-e` evaluates a single expression and prints the parsed tree
first, which is handy for checking precedence.

## The language

- Two types, `Int` (64-bit, signed) and `Bool`.
- `let` for bindings that never change, `var` for ones that do. A type
  annotation is optional: `let age: Int = 22;` and `let age = 22;` mean the
  same thing.
- Functions with typed parameters and an optional return type. They can
  be declared anywhere at the top level and called before they appear.
- `if` / `else if` / `else`, `while`, and `return`.
- Arithmetic `+ - * / %`, comparisons, `== !=`, and `&& || !` with
  short-circuiting.
- `print(value);` prints an `Int` or a `Bool` on its own line.
- `//` comments.

Statements end with a semicolon. Top-level statements are the program and
run in order. Functions only see their own parameters and locals, not
top-level variables.

A few things C leaves undefined are defined here. Integer overflow and
division by zero stop the program with an error pointing at the operator.
Operands and arguments are always evaluated left to right.

```console
$ ./build/izvor run tests/golden/overflow.iz
1
error: integer overflow
 --> tests/golden/overflow.iz:3:11
```

## Errors

```console
$ ./build/izvor run tests/golden/wrong-arguments.iz
error: 'add' takes 2 arguments, found 3
 --> tests/golden/wrong-arguments.iz:5:7
  |
5 | print(add(1, 2, 3));
  |       ^
error: argument 'b' of 'add' must be Int, found Bool
 --> tests/golden/wrong-arguments.iz:6:14
  |
6 | print(add(1, true));
  |              ^
```

The parser recovers after a syntax error and keeps going, and the checker
reports every type error it finds, so one run shows you everything wrong
with a file instead of just the first thing.

## Tests

```console
$ make test      # unit tests, then every golden program both ways
$ make fuzz      # 20,000 random programs through the whole compiler
$ make asan      # the same under Address and LeakSanitizer
```

Each program in `tests/golden/` has an `.expected` file holding its exact
output, errors included. `make test` runs every one through the
interpreter and also compiles it to a binary and runs that, and both have
to match the file. Unit tests are plain `main()` and `assert()`. Everything
builds with UndefinedBehaviorSanitizer, and CI runs it all on Linux and
macOS.

More on how it's put together and why is in
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md). What's next is in
[docs/ROADMAP.md](docs/ROADMAP.md).

## IDE

There's a design prototype of an IDE for izvor,
[Izvor Studio](https://github.com/levimackay/izvor-studio). It was designed
with Claude Design, and it shows where the tooling could go rather than
anything that exists yet.

## Layout

```
src/      the compiler
tests/    unit tests, golden programs, and the fuzzer
docs/     architecture and roadmap
```

## License

MIT, see [LICENSE](LICENSE).
