# Architecture

izvor is a compiler for a small statically typed language, written in C11
with no dependencies beyond a C compiler and `make`. This document covers
how the pieces fit together, which decisions were deliberate, and what is
known to be missing.

## The pipeline

```
source text
   |  lexer        characters   -> tokens
   v
 tokens
   |  parser       tokens       -> abstract syntax tree
   v
   AST
   |  evaluator    tree         -> a value                (today)
   |  codegen      tree         -> C source               (planned)
   v
 result
```

Everything up to the AST is finished. The evaluator is a tree walk that
exists to prove the front end is right and to act as the oracle the code
generator has to agree with once it lands. The planned backend emits C
and hands it to clang, rather than targeting machine code or LLVM.

## Modules

| File | Responsibility |
|---|---|
| `src/lexer.c` | Scans characters into a flat stream of tokens. No lookahead, no backtracking. |
| `src/ast.c` | Heap-allocated tree nodes and the recursive destructor. |
| `src/parser.c` | Recursive descent over the token stream, one function per precedence level. |
| `src/diag.c` | The single place that decides what a compiler error looks like. |
| `src/main.c` | The driver: read a file, run the pipeline, print the tree and its value. |
| `src/lexer_main.c` | `lexdump`, a token dumper used to debug the lexer in isolation. |

## Decisions worth defending

**A tagged union for AST nodes, not a struct hierarchy.** Every `Node`
carries a `NodeType` tag and a union of the payloads each kind needs. One
allocation per node, one `switch` per operation, and the compiler warns
when a new node type is added without handling it everywhere, because no
`switch` over `NodeType` has a `default` case. The alternative in C is a
base struct with derived structs cast onto it, which buys polymorphism
and loses that exhaustiveness warning.

**Tokens borrow the source, they do not own it.** A `Token` is a type, a
pointer into the source buffer, and a length. Nothing is copied and
nothing is freed. Two consequences follow. Every token is only valid
while the source buffer is alive, which is why the driver reads the whole
file into one allocation rather than streaming it. And the token text is
not NUL-terminated at the token boundary, so comparisons have to check
length before bytes, or `let` matches the prefix of `letter`.

**Recursive descent, not a parser generator.** One function per
precedence level, so the grammar rule, the C function, and the tree shape
are the same idea three times over:

```
expression -> term (("+" | "-") term)*
term       -> factor (("*" | "/") factor)*
factor     -> NUMBER | "(" expression ")" | "-" factor
```

Left associativity falls out of the loop shape: each iteration folds the
tree built so far into the left child of a new node, so `10 - 3 - 2`
groups as `(- (- 10 3) 2)` and evaluates to 5 rather than 9. There is a
test pinning exactly that.

**Errors are data the whole compiler shares.** Every stage reports
through `diag_error`, which takes the source and a byte offset and turns
them into a line, a column, and a caret under the offending character.
No stage formats its own message, so adding a semantic analysis pass
means adding one call, not inventing a second error style.

```
error: expected ')', found EOF
 --> tests/golden/unclosed-paren.iz:1:8
  |
1 | (12 + 3
  |        ^
```

**Integer division by zero is an error, not undefined behavior.** C
leaves it undefined, which means the generated program may do anything at
all. izvor decides on its behalf: report it and stop. This is the first
real language design decision in the project and it is written down
rather than inherited.

## Testing

Plain `main()` plus `assert()`, no framework, three kinds:

- **Unit tests** (`tests/test_*.c`) cover the lexer, the parser's token
  helpers, the tree shapes it builds, and the line and column math.
- **Golden tests** (`tests/golden/`) pin the exact text of every
  diagnostic. A diagnostic is a user interface, so changing one should
  show up in a diff rather than being noticed by nobody.
- **A fuzzer** (`tests/fuzz.c`) throws pseudo-random input at the lexer
  and parser and asserts only that neither crashes. The seed is fixed, so
  a failure on CI reproduces exactly on a laptop.

Everything compiles with UndefinedBehaviorSanitizer, which turns
out-of-bounds reads and signed overflow into a loud abort naming the
line. AddressSanitizer, which is what catches leaks and use-after-free,
deadlocks on startup under Apple clang 17 on macOS 26.5, so `make asan`
keeps it behind its own target and CI runs that target on Linux.

The obvious macOS alternative was tried and rejected. The system `leaks`
tool reports zero on a deliberately leaked block on this machine, because
it cannot inspect the process under the current security policy, and a
check that cannot fail is worse than no check at all.

CI builds and runs everything on Linux and macOS with warnings promoted
to errors.

## Known limitations

These are real and measured, not hypothetical.

**Parser recursion is unbounded.** Nesting is handled by the C call
stack, so a deeply nested expression overflows it. Measured on macOS with
the default 8 MB stack, `((((...1...))))` survives 20,000 levels of
nesting and aborts somewhere before 30,000. A production compiler tracks
depth and reports "expression nested too deeply" instead. Tracked on the
roadmap.

**Arithmetic is unchecked.** Evaluating a tree whose result exceeds a
`long` is signed overflow, which is undefined behavior. Under the
sanitizer build it aborts loudly, which is why the fuzzer exercises the
front end only and leaves evaluation alone. Checked arithmetic is on the
roadmap.

**Columns are counted in bytes, not characters.** An error positioned
after a multi-byte UTF-8 character reports a column further right than a
human would count, and the caret is offset by the same amount. Non-ASCII
bytes are currently rejected by the lexer anyway, so this only shows up
in the error pointing at them.

**The parser stops at the first error.** There is no recovery and no
resynchronization, so a file with three mistakes reports one. This is a
deliberate ordering choice, not an oversight: error recovery is worth
building once there are statements to resynchronize to.
