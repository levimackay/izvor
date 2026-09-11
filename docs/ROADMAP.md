# Roadmap

The target is a small, complete, statically typed language that compiles
to C and then to a native binary. Not a large one. Every phase has to
leave the project building and passing its tests.

The first end to end milestone, and the thing everything below is aimed
at:

```
let x = 10
let y = 20
print(x + y)
```

compiled to C, handed to clang, and run as a native executable printing
`30`.

## Done

**Phase 1, lexer.** Integer literals, `+ - * /`, parentheses, and error
tokens that do not abort the scan. Four tests.

**Phase 2, parser and AST.** Recursive descent with one function per
precedence level, building a heap-allocated tagged-union tree. Correct
precedence, left associativity, unary minus, parenthesized grouping, and
a whole-input requirement so trailing garbage is rejected. Two tests.

**Infrastructure.** Diagnostics with line, column and caret. Golden tests
pinning every error message. A fuzzer over the front end. Sanitizer
builds, leak checking, and CI on Linux and macOS.

## Phase 3, variables and statements

The step that turns a calculator into a language. A program stops being
one expression and becomes a sequence of statements, and a name has to
mean something at run time that it cannot mean at parse time.

- **3.1 Lexer: identifiers and keywords.** `TOK_IDENT`, `let`, `var`,
  `print`, `=`. Keywords are scanned as identifiers first and classified
  afterwards, so `letter` stays one token.
- **3.2 AST: statements.** A second node family for statements alongside
  the expression nodes, plus an identifier expression. A program becomes
  a list of statements rather than one tree.
- **3.3 Parser: a program is a sequence of statements.** Needs the
  statement terminator decision below settled first.
- **3.4 Environment.** Where a name maps to a value. A flat array of name
  and value pairs is the right size for now; a hash table is machinery
  with nothing yet to pay for it. `let` bindings reject reassignment,
  `var` bindings allow it.
- **3.5 Driver.** Run a whole file of statements rather than one
  expression.

## Phase 4, types and inference

Every expression gets a type before anything runs. `let age = 22` infers
`Int`, `let age: Int = 22` states it, and the two disagree loudly when
they should. Integers, booleans, and floats if they stay cheap.

## Phase 5, functions and control flow

`fn add(a: Int, b: Int) -> Int`, `if`/`else`, loops, and call expressions.
Scope becomes a stack of environments rather than one.

## Phase 6, semantic analysis and error quality

A real pass between parsing and code generation: undefined names,
assignment to a `let`, wrong argument counts, unreachable code. Error
recovery belongs here too, so one broken file reports every problem in it
instead of only the first.

## Phase 7, code generation

Walk the typed tree and emit C. The generated code should be readable and
boringly predictable, because being able to read it is the whole argument
for this backend over LLVM.

## Phase 8, native executables

Invoke clang on the generated C, manage temporary files, surface the
underlying compiler's failures as izvor errors rather than raw output,
and produce a binary. The milestone at the top of this file closes here.

## Phase 9, arrays, structs, and a small runtime

Aggregates, and whatever minimal runtime support they need.

## Phases 10 to 12, concurrency

`parallel { }` and `parallel for`, with the programmer never touching
threads, joins, or scheduling. Shared mutable state inside a parallel
block is a compile error suggesting a reduction, not a silent race. After
that, dependency analysis, and then automatic parallelization of the
simple cases. These are the stretch goals and they come last on purpose.

## Open decisions

**How does a statement end?** A semicolon, a newline, or nothing. Nothing
works today because every statement starts with a keyword, but that stops
being true in phase 5 when a bare expression can be a statement, and
adding a terminator later breaks every program written before it. Newline
sensitivity means the lexer stops treating newlines as whitespace and
starts carrying line structure, which is real complexity for a cosmetic
gain. Leaning toward a required semicolon. Has to be settled before 3.3.

**Are declarations expressions?** `let` returning a value makes the
grammar smaller and the language stranger. Currently assumed no.

## Debt worth paying down

Carried from the limitations in [ARCHITECTURE.md](ARCHITECTURE.md):

- **Depth limit in the parser.** Track nesting depth and report an error
  instead of overflowing the C stack past roughly 20,000 levels.
- **Checked arithmetic.** Overflow in evaluation is undefined behavior
  today. Decide the language's answer, implement it, and let the fuzzer
  cover evaluation once it has one.
- **Character columns.** Count UTF-8 characters rather than bytes once
  the lexer accepts non-ASCII identifiers.
