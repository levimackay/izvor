# Phase 2 — Recursive-descent parser → AST

**Goal:** turn the token stream into an **abstract syntax tree** (AST): a
tree of heap-allocated nodes where `1 + 2 * 3` becomes `(+ 1 (* 2 3))` —
precedence and parentheses resolved once, permanently, in the shape of the
tree.

**Why:** a flat token list can't answer "what is the left operand of `*`?"
A tree can — and every later phase consumes this tree: the interpreter walks
it, the bytecode compiler walks it, the type checker walks it. Parsing is
also where your language's *grammar* becomes real: you'll write it down as
rules first, then discover the magic of recursive descent — **each grammar
rule becomes one C function, and the grammar's recursion becomes function
call recursion**. This is not a toy technique: GCC and V8 parse this way.

**Spine reading (read before task 2.1):**
- [Crafting Interpreters ch. 5, "Representing Code"](https://craftinginterpreters.com/representing-code.html) —
  grammars and syntax trees. Skip the Java code-generation machinery
  (§5.2.2, the Visitor pattern §5.3) — in C you'll use a tagged struct and a
  `switch`, which is simpler.
- [Crafting Interpreters ch. 6, "Parsing Expressions"](https://craftinginterpreters.com/parsing-expressions.html) —
  the chapter this phase implements.

**No scaffold is provided from here on — deliberately.** You've now
implemented ten stub files and written none of the headers; designing
`ast.h` yourself *is* task 2.1, and writing the assert test for each task is
part of that task (you know the pattern: a `main()` full of `assert`s,
a Makefile rule like the existing ones).

---

## Task 2.1 — Design the AST: `ast.h` + node constructors (~1.5–2h)

**Build:** an `ast.h`/`ast.c` with a *tagged union* node: an enum tag
(`NODE_NUMBER`, `NODE_BINARY`, `NODE_UNARY`, `NODE_GROUPING` if you want
it), and per-kind payloads (number value; operator + left/right child
pointers). Write constructor functions (`ast_number(5)`,
`ast_binary(TOK_PLUS, l, r)`) that malloc and fill a node, and `ast_free`
that recursively frees a whole tree.

**Why:** the tagged union is *the* C idiom for "a value that can be one of
several shapes" — you'll use it again for runtime values (Phase 3) and
bytecode operands (Phase 5). Recursive `ast_free` is your first recursive
walk of your own tree: the same traversal shape as evaluation, compilation,
and type checking. Get comfortable here where the walk does something
trivially checkable (the `leaks` tool will confirm every node dies exactly
once).

**Read first:**
- [Crafting Interpreters ch. 5](https://craftinginterpreters.com/representing-code.html), §5.1–5.2.
- Beej on unions and structs: [Beej's Guide](https://beej.us/guide/bgc/html/split/) (ch. 7 structs; ch. 20 unions).

**Done when:** your own test builds a small tree by hand, checks its shape
via the tags/fields, frees it, and `leaks --atExit` reports clean.

---

## Task 2.2 — Write the grammar down + parser skeleton (~1–1.5h)

**Build:** in a comment at the top of `parser.c`, write the grammar:

```
expression → term
term       → factor ( ("+" | "-") factor )*
factor     → unary ( ("*" | "/") unary )*
unary      → "-" unary | primary
primary    → NUMBER | "(" expression ")"
```

Then the parser's plumbing: a `Parser` struct holding the lexer + current
token, with `advance`/`check`/`match` helpers — the token-level twins of the
lexer's character-level helpers.

**Why:** the grammar *is* the design; the code is its transcription. Notice
how precedence is encoded purely by **which rule calls which**: `term` calls
`factor`, so `*` binds tighter than `+` — no precedence numbers, no tables,
just call structure. Writing the grammar first means every parsing bug
becomes "which line of the grammar did I mistranscribe?"

**Read first:**
- [Crafting Interpreters ch. 6](https://craftinginterpreters.com/parsing-expressions.html), §6.1–6.2 (ambiguity, precedence, associativity).

**Done when:** the skeleton compiles and `match`/`check` have asserts
covering them (feed a lexer "1+2" and drive the helpers by hand).

---

## Task 2.3 — Parse primaries and unary minus (~1–1.5h)

**Build:** `primary()` (numbers, parenthesized expressions) and `unary()`
(`-x`, including `--x`). Entry point `parse_expression()` returning the tree.

**Why:** `primary` handling `"(" expression ")"` is the first *self-recursive
moment* — the bottom rule calling back to the top is exactly how nested
parens to any depth work, for free. Unary minus introduces the question
"does `-` mean negate or subtract?" — answered entirely by *where you are in
the grammar*, your first taste of context sensitivity.

**Read first:**
- [Crafting Interpreters ch. 6](https://craftinginterpreters.com/parsing-expressions.html), §6.3 "Recursive Descent Parsing".

**Done when:** asserts show `parse("42")` yields a NUMBER node,
`parse("-(5)")` yields UNARY(NUMBER), nesting works.

---

## Task 2.4 — Binary operators with precedence (~1.5–2h)

**Build:** `factor()` and `term()` with their `while`-loops, completing the
grammar. This is the heart of the phase.

**Why:** the loop shape `left = ...; while (match(op)) left = binary(op,
left, parse_next_level())` is what makes operators **left-associative**
(`8-4-2` = `(8-4)-2` = 2, not `8-(4-2)` = 6 — the tests will catch you if
you get this wrong, and *most people do the first time*). After this task,
`1 + 2 * 3` parses correctly with zero special-casing — sit for a minute
with *why*.

**Read first:**
- [Crafting Interpreters ch. 6](https://craftinginterpreters.com/parsing-expressions.html), §6.3 again — the `term`/`factor` methods.
- Later, optional: [Pratt Parsers: Expression Parsing Made Easy](https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/) —
  the elegant alternative technique (used in Phase 5's book chapters); skim
  now, appreciate later.

**Done when:** asserts verify tree shapes for `1+2*3`, `(1+2)*3`, `8-4-2`
(associativity!), `2*3/4`.

---

## Task 2.5 — Syntax errors + an AST printer (~1.5h)

**Build:** graceful failure for `"1 +"`, `"(1"`, `")"`, `"1 @ 2"` — report
the error (message + where) and return NULL rather than crashing. Then an
`ast_print` that prints the tree in parenthesized (Lisp-style) form:
`(+ 1 (* 2 3))` — extend `lexdump` or add a `parsedump` target.

**Why:** parser error handling is what separates "toy" from "usable" — and
it forces you to answer: who owns partially-built nodes when parsing fails
halfway? (Memory discipline under errors is a very C lesson, and a very
real-compiler lesson.) The printer is your Phase 2 debugging tool, and
writing it proves the walk pattern: print is the third traversal you've
written after free — evaluation in Phase 3 will be the fourth, and by then
the pattern is reflex.

**Read first:**
- [Crafting Interpreters ch. 6](https://craftinginterpreters.com/parsing-expressions.html), §6.3.3 "Syntax Errors".
- [Crafting Interpreters ch. 5](https://craftinginterpreters.com/representing-code.html), §5.4 (the AST printer).

**Done when:** bad input never crashes or leaks; `parsedump "1+2*3"` prints
`(+ 1 (* 2 3))`.
