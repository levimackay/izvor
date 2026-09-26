# Roadmap

The goal is a small, complete, statically typed language that compiles to
C and then to a native binary. Small, not large. Every step has to leave
the project building and passing its tests.

## Done

**Lexer.** Numbers, names and keywords, every operator, `//` comments,
and error tokens that don't stop the scan. Literals too big for an `Int`
are an error rather than a silent overflow.

**Parser and syntax tree.** Recursive descent, one function per precedence
level. Statements, blocks, functions, calls, and recovery after errors.
Nesting past 100 levels is an error instead of a stack overflow.

**Variables and statements.** `let`, `var`, assignment, `print`, and
programs that are a list of statements.

**Types.** `Int` and `Bool`, inferred from the value or written out, and
checked everywhere they meet.

**Functions and control flow.** Typed parameters, return types, calls in
any order, recursion, `if`/`else if`/`else`, and `while`.

**Semantic checks.** Undefined names, assigning to a `let`, wrong argument
counts and types, returning the wrong thing or nothing, functions that can
fall off the end, statements that do nothing, and a warning for code after
a `return`. It reports all of them, not just the first.

**Code generation.** The checked tree becomes readable C with overflow and
division checks, evaluated strictly left to right.

**Native executables.** `izvor build` pipes the C into `cc` and produces a
binary. The first milestone, a program that adds two variables and prints
`30` as a native executable, works:

```
let x = 10;
let y = 20;
print(x + y);
```

**Infrastructure.** Diagnostics with line, column and caret, golden tests
run through both back ends, a fuzzer over the whole compiler, sanitizer
builds, and CI on Linux and macOS.

## Next: arrays, structs, and a small runtime

Aggregates, and whatever runtime support they need. Strings probably come
in here too, since `print` can't say anything but numbers yet. This is
also where izvor needs its first heap allocations at run time, which means
deciding who frees them.

## After that: concurrency

`parallel { }` and `parallel for`, without the programmer ever touching
threads, joins or scheduling. Shared mutable state inside a parallel block
should be a compile error that suggests a reduction, not a silent race.
Then dependency analysis, and then parallelizing the simple cases
automatically. These are the stretch goals and they come last on purpose.

## Smaller things

- `break` and `continue`. The checker would need to know it's inside a
  loop, and `while true` would stop counting as a loop that never ends.
- Compound assignment (`+=` and friends).
- Count columns in characters instead of bytes once names can be
  non-ASCII.
