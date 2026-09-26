# Architecture

izvor is a compiler for a small statically typed language, written in C11
with no dependencies beyond a C compiler and `make`. This covers how the
pieces fit together, which decisions were deliberate, and what is known to
be missing.

## The pipeline

```
source text
   |  lexer        characters  -> tokens
   |  parser       tokens      -> syntax tree
   |  checker      tree        -> the same tree, with a type on every expression
   v
   +--> interpreter   walks the tree and runs it           (izvor run)
   +--> codegen       walks the tree and writes C           (izvor emit)
          |  cc       C -> native executable                (izvor build)
```

Both back ends read the tree the checker has already approved, so neither
one has any error handling for bad programs. If something reaches them
that the checker should have stopped, that is a checker bug.

## Modules

| File | What it does |
|---|---|
| `src/lexer.c` | Characters into tokens. One character of lookahead, no backtracking. |
| `src/ast.c` | Tree nodes, statements, functions, and freeing all of it. |
| `src/parser.c` | Recursive descent, one function per precedence level, with error recovery. |
| `src/check.c` | Names, types, returns, and everything else that is wrong without being a syntax error. |
| `src/interp.c` | The tree-walking interpreter. |
| `src/codegen.c` | Writes the program out as C. |
| `src/diag.c` | The single place that decides what an error looks like. |
| `src/main.c` | The driver: `run`, `build`, `emit`, `-e`. |
| `src/lexer_main.c` | `lexdump`, a token dumper for debugging the lexer by itself. |

## Decisions worth defending

**A tagged union for tree nodes, not a struct hierarchy.** Every `Node`
has a `NodeType` tag and a union of what each kind needs. One allocation
per node, one `switch` per operation, and no `switch` over `NodeType` or
`StmtType` has a `default`, so adding a node kind makes the compiler list
every place that has to handle it. Statements are a separate `Stmt` type
so an expression can never end up where a statement belongs.

**Tokens borrow the source.** A token is a type, a pointer into the source
buffer, and a length. Names in the tree are the same thing, so nothing is
copied and every error can point back into the original text. The cost is
that the source buffer has to outlive everything, which is why the driver
reads the whole file into one allocation.

**Semicolons.** Every statement ends with one. Leaving them out works
until a bare expression can be a statement, and then `x\n-1` has two
readings. Making newlines significant would fix that, but it moves line
structure into the lexer for a cosmetic gain.

**Recursive descent, not a parser generator.** The grammar rule, the C
function and the tree shape are the same thing written three ways:

```
expression -> and ("||" and)*
and        -> equality ("&&" equality)*
equality   -> comparison (("==" | "!=") comparison)*
comparison -> sum (("<" | "<=" | ">" | ">=") sum)*
sum        -> term (("+" | "-") term)*
term       -> factor (("*" | "/" | "%") factor)*
factor     -> ("-" | "!") factor | NUMBER | "true" | "false"
            | NAME | NAME "(" arguments ")" | "(" expression ")"
```

Every binary level is the same loop, so they all go through one function
that takes the next level down and the operators for this one. Left
associativity comes from the loop: each pass folds what has been built so
far into the left side of a new node, so `10 - 3 - 2` is 5, not 9.

**Error recovery.** When a statement fails to parse, the parser skips
ahead to the next `;` or the next keyword that starts a statement and
carries on. It counts braces while it skips so it doesn't stop inside a
block it has half read. The parser only stops at the end of the file, and
the checker never stops early at all. Instead a broken expression gets
the type `<error>`, and nothing complains about an `<error>`, so one
mistake doesn't turn into ten messages.

**Integer overflow and division by zero are errors.** C leaves both
undefined, which means the generated program could do anything. izvor
decides: the program stops and says where. The interpreter uses the
`__builtin_*_overflow` functions for this, and the generated C calls small
`rt_` helpers that do the same checks, so both back ends fail the same way
with the same message.

**Left to right, always.** C doesn't say what order a function's
arguments or an operator's operands are evaluated in, and gcc and clang
don't agree. izvor says left to right. When a statement has more than one
thing in it that can print or fail, the code generator pulls the earlier
ones into temporaries so C has no choice about the order. Most statements
don't need that and come out as one line.

**No shadowing.** A name can't be declared again while an earlier one with
that name is still visible, and a variable can't share a name with a
function. Besides catching mistakes, this means every izvor name maps
straight onto a C name (`x` becomes `iz_x`) with no renaming. In C,
`long x = x + 1;` inside a block reads the new, uninitialized `x`, and
izvor never has to worry about generating that.

**The interpreter is the oracle.** Every golden test runs twice, once
through the interpreter and once compiled, and both runs have to produce
exactly the `.expected` file. The two back ends share nothing but the
tree, so they would have to be wrong in the same way for a bug to get
through.

## Testing

- **Unit tests** (`tests/test_*.c`) for the lexer, the parser's helpers
  and tree shapes, recovery, and the line and column math.
- **Golden tests** (`tests/golden/`) are whole programs with their exact
  output: things that work, runtime errors, and every kind of compile
  error. An error message is a user interface, so changing one should show
  up in a diff.
- **The fuzzer** (`tests/fuzz.c`) glues random tokens together into
  programs and pushes them through parsing, checking and code generation.
  It only checks that nothing crashes. The seed is fixed, so a failure on
  CI reproduces on a laptop.

Everything builds with UndefinedBehaviorSanitizer. AddressSanitizer
deadlocks on startup under Apple clang 17 on macOS 26.5, so `make asan`
has its own target and CI runs it on Linux. The macOS `leaks` tool is not
a substitute. On this machine it reports zero on a deliberately leaked
block, because it can't inspect the process under the current security
policy.

## Known limitations

**Calls nest at most 10,000 deep.** Past that the program stops with
"more than 10000 nested calls", the same way in both back ends. Without a
limit the two would crash at very different depths: the interpreter uses a
few kilobytes of C stack per izvor call, and the compiled code almost
none. The interpreter runs on its own thread with a 512 MB stack so the
sanitizer builds can still reach the limit.

**Nesting is capped at 100.** Expressions, blocks and `else if` chains
deeper than that are rejected. This keeps the parser off the edge of the
C stack and keeps the generated C under clang's default bracket depth
limit of 256.

**Columns are counted in bytes.** Non-ASCII is only allowed in comments,
and a comment runs to the end of its line, so the only error that can
land after one on the same line is an unexpected end of file.

**The most negative Int can't be written as a literal.**
`-9223372036854775808` is a minus sign applied to a number that doesn't
fit. `-9223372036854775807 - 1` works.

**`build` hides the C compiler's output.** If `cc` rejects the generated
code, that's a bug in izvor, and `izvor emit` is how to look at what it
was given.
