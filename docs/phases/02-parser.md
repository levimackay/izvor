# Phase 2 — Recursive-descent parser → AST

**Goal:** turn the token stream into an **abstract syntax tree** (AST): a
tree of heap-allocated nodes where `1 + 2 * 3` becomes `(+ 1 (* 2 3))` —
precedence and parentheses resolved once, permanently, in the shape of the
tree.

**Why a tree?** A flat token list can't answer the questions that matter:
what is the left operand of `*`? does this minus belong to the `5` or sit
between two expressions? Structure in language is *nested* — expressions
contain expressions contain expressions — and the natural data shape for
nesting is a tree. In `1 + 2 * 3`, the tree has `+` at the root with `1`
on the left and the subtree `(* 2 3)` on the right: the fact that
multiplication binds tighter is no longer a rule to remember, it's *frozen
into the geometry*. Every later phase consumes this tree — the interpreter
walks it, the bytecode compiler walks it, the type checker walks it — and
none of them ever think about precedence again, because the parser settled
it once.

Parsing is also where your language's **grammar** becomes real: you'll
write it down as rules first, then discover the magic of **recursive
descent** — each grammar rule becomes one C function, and the grammar's
recursion becomes function-call recursion. This is not a toy technique:
GCC and V8 parse with hand-written recursive descent.

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

**The concept:** an AST node is "a value that can be one of several
shapes" — a number node holds a value; a binary node holds an operator and
two children; they're different shapes but must flow through the same
pointers. Languages like Swift give you `enum` with associated values for
this; C's idiom is the **tagged union**: a struct containing an enum
*tag* saying which shape this node currently is, plus a `union` overlaying
the different payloads in the same bytes (a union is a struct whose fields
share memory — only one is valid at a time, and the tag tells you which).
Every read follows the discipline: check the tag, then touch only that
shape's fields. Nothing enforces this but you — which is exactly why it
teaches you what Swift's enums are doing under the hood.

The nodes live on the **heap** (malloc'd by your constructors) because the
tree must outlive the parser function that builds it, and because its size
depends on the program being parsed — the two reasons heap memory exists
(task 0.8). And `ast_free` is your first **recursive tree walk**: to free
a binary node, first free its left subtree, then its right, then the node
itself. Notice the shape — do something with the children, then the node.
You will write this same traversal at least four more times in this
project (print, evaluate, compile, type-check); only the "do something"
changes.

**Why compilers need it:** the AST is the central data structure of the
entire project — the one representation every stage shares. Getting
comfortable with recursive walks here, where the walk does something
trivially checkable (every node dies exactly once, and `leaks` will
referee), buys you the pattern everywhere else.

**Read first:**
- [Crafting Interpreters ch. 5, §5.1–5.2](https://craftinginterpreters.com/representing-code.html) —
  why trees, and what nodes look like.
- [Beej's Guide to C — Structs II (unions)](https://beej.us/guide/bgc/html/split/structs-ii-more-fun-with-structs.html) —
  the union mechanics you're about to use.
- [Wikipedia — Tagged union](https://en.wikipedia.org/wiki/Tagged_union) —
  the concept across languages; see your C idiom and Swift's enums
  described as the same thing.
- [AST Explorer](https://astexplorer.net/) (interactive) — type real
  JavaScript/Python and watch its actual AST render live. Five minutes of
  poking makes "the tree" concrete before you design yours.

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

**The concept:** a **grammar** is a set of rules describing every legal
sentence in a language — the same way "a sentence is a subject then a
verb then an object" describes a fragment of English. Read `term → factor
( ("+" | "-") factor )*` as: "a term is a factor, followed by zero or more
repetitions of (a plus-or-minus sign and another factor)". Each rule
defines a named *shape*; rules refer to other rules; and the whole
language is whatever the top rule can generate. This notation (a variant
of **BNF**, Backus–Naur Form, invented in 1959) is how every language you
use is specified — Python, C and Swift all have a published grammar that
looks like this.

Now the elegant part: look at *which rule calls which*. `term` is built
out of `factor`s, so by the time a `+` gets attached, the `*`s inside its
operands have already been grouped — **precedence is encoded purely by
the call order of the rules**. No precedence numbers, no tables. Lower
rules bind looser; deeper rules bind tighter; `primary` at the bottom
loops back to `expression` for parentheses. The grammar *is* the design;
the C code is its transcription — which means every parsing bug becomes
"which line of the grammar did I mistranscribe?"

**Why compilers need it:** ambiguity is the enemy. "1 + 2 * 3" must mean
one thing, every time, on every machine. A grammar structured this way
provably yields exactly one tree per input — and writing it down before
coding gives you a spec to be wrong against, which is the difference
between debugging and flailing.

**Read first:**
- [Crafting Interpreters ch. 6, §6.1–6.2](https://craftinginterpreters.com/parsing-expressions.html) —
  ambiguity, precedence, associativity, and the recursive-descent idea.
- [Cornell CS 3110 — Parsing (BNF section)](https://cs3110.github.io/textbook/chapters/interp/parsing.html) —
  the formal view of grammars, brief and readable.
- [Wikipedia — Recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser) —
  includes a complete worked example: a grammar and the C parser that
  mirrors it, side by side.

**Done when:** the skeleton compiles and `match`/`check` have asserts
covering them (feed a lexer "1+2" and drive the helpers by hand).

---

## Task 2.3 — Parse primaries and unary minus (~1–1.5h)

**Build:** `primary()` (numbers, parenthesized expressions) and `unary()`
(`-x`, including `--x`). Entry point `parse_expression()` returning the tree.

**The concept:** `primary` is the grammar's floor — the things that are
complete expressions all by themselves. A number obviously is. But the
rule's second arm, `"(" expression ")"`, is the moment recursion earns its
name: the *bottom* rule calls the *top* rule. Parse `((1))` and the call
chain goes expression → term → factor → unary → primary → sees `(` →
calls expression again → ... and unwinds cleanly. Arbitrary nesting depth,
handled by zero extra code, because the C call stack *is* your nesting
tracker. When this clicks, stop and appreciate it — it's the single most
beautiful mechanism in this whole phase.

Unary minus asks a subtler question: in `5 - 3` the minus is subtraction,
in `-3` it's negation — same character, two meanings. How do you tell?
By *where you are in the grammar*: `unary` only ever runs where an
operand is expected, so a `-` seen there must be negation; `term`'s loop
only looks for operators *after* parsing an operand, so a `-` seen there
must be subtraction. No flags, no special cases — position in the grammar
is context. `--x` falls out for free because `unary` calls itself.

**Why compilers need it:** every real language is full of symbols whose
meaning depends on context (`*` in C is multiplication *and* pointer
dereference *and* part of type syntax). Grammar-position-as-context is the
clean mechanism that makes this tractable, and unary minus is its
smallest teachable instance.

**Read first:**
- [Crafting Interpreters ch. 6, §6.2 "Recursive Descent Parsing"](https://craftinginterpreters.com/parsing-expressions.html) —
  the code shape for unary and primary.
- [Let's Build a Simple Interpreter, part 6](https://ruslanspivak.com/lsbasi-part6/) —
  adds parenthesized expressions to a Python parser; watch the bottom rule
  call the top.
- [Let's Build a Simple Interpreter, part 7](https://ruslanspivak.com/lsbasi-part7/) —
  the parser starts *building AST nodes* — exactly what yours does with
  the constructors from task 2.1.

**Done when:** asserts show `parse("42")` yields a NUMBER node,
`parse("-(5)")` yields UNARY(NUMBER), nesting works.

---

## Task 2.4 — Binary operators with precedence (~1.5–2h)

**Build:** `factor()` and `term()` with their `while`-loops, completing the
grammar. This is the heart of the phase.

**The concept:** the loop shape to internalize is:

```c
left = parse_next_level();
while (match(PLUS) || match(MINUS)) {
    op = previous_token;
    right = parse_next_level();
    left = ast_binary(op, left, right);   // old tree becomes LEFT child
}
return left;
```

Each turn of the loop takes the tree built *so far* and tucks it in as the
**left** child of a new node. That's what makes these operators
**left-associative**: `8 - 4 - 2` builds `((8-4)-2)` = 2, not `(8-(4-2))`
= 6. Associativity — which way same-precedence operators group — is
invisible until subtraction or division exposes it, and *most people get
it wrong the first time* (the tests will catch you). If you had made the
rule recurse instead of loop (`term → factor "-" term`), you'd have
gotten right-associativity; the loop-vs-recurse choice *is* the
associativity choice.

And precedence never appears in the code at all — `term`'s loop calls
`factor`, so every `*` has been fully grouped before any `+` looks around.
After this task, `1 + 2 * 3` parses correctly with zero special-casing.
Sit for a minute with *why*.

**Why compilers need it:** every expression in every language you've used
went through some version of this. There's a slicker technique (Pratt
parsing — one function with a precedence table instead of one function
per level) that real compilers often prefer once operator counts grow;
skim it now so it's familiar when the book's Part III uses it, and
appreciate that it's solving the same problem you just solved by hand.

**Read first:**
- [Crafting Interpreters ch. 6, §6.2](https://craftinginterpreters.com/parsing-expressions.html) —
  the term/factor methods and their loops.
- [Let's Build a Simple Interpreter, part 5](https://ruslanspivak.com/lsbasi-part5/) —
  precedence and associativity built up from scratch, with diagrams.
- [Pratt Parsers: Expression Parsing Made Easy](https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/) —
  the elegant alternative technique; skim now, appreciate later.

**Done when:** asserts verify tree shapes for `1+2*3`, `(1+2)*3`, `8-4-2`
(associativity!), `2*3/4`.

---

## Task 2.5 — Syntax errors + an AST printer (~1.5h)

**Build:** graceful failure for `"1 +"`, `"(1"`, `")"`, `"1 @ 2"` — report
the error (message + where) and return NULL rather than crashing. Then an
`ast_print` that prints the tree in parenthesized (Lisp-style) form:
`(+ 1 (* 2 3))` — extend `lexdump` or add a `parsedump` target.

**The concept:** a syntax error is the parser expecting one thing and
seeing another — `expression` wanted a primary and got EOF (`"1 +"`), or
`primary` consumed `(` and never found the matching `)` (`"(1"`). The
graceful response has three parts: *say what you expected and what you
found*, *say where* (that's your Phase 1 `start` pointer paying off), and
*don't crash or leak*. That last one is the C-specific lesson: when
parsing dies halfway through `1 + (2 *`, some nodes are already built —
who frees them? Decide the ownership rule (e.g. "a rule that fails frees
whatever it built before returning NULL") and enforce it everywhere. Every
real C codebase has exactly this discipline under the name "cleanup on
error paths", and `leaks` will hold you to it.

The printer's output format — `(+ 1 (* 2 3))` — is an **S-expression**,
Lisp's notation, and the de-facto standard way compiler people write
trees as text: operator first, then children, parenthesized. It reads
strangely for a day and then becomes the clearest way you've ever seen
precedence. Writing the printer is also your second recursive walk (after
`ast_free`), and it's pure: print `(`, print the operator, recurse left,
recurse right, print `)`.

**Why compilers need it:** parser error handling is what separates "toy"
from "usable" — a language that segfaults on a typo is unusable no matter
how good its features are. And the printer is your Phase 2 debugging tool:
from now on, "what did the parser build?" is a one-command question, which
will matter constantly in Phase 3.

**Read first:**
- [Crafting Interpreters ch. 6, §6.3 "Syntax Errors"](https://craftinginterpreters.com/parsing-expressions.html) —
  what a parser owes its user on bad input.
- [Crafting Interpreters ch. 5, §5.4](https://craftinginterpreters.com/representing-code.html) —
  the AST printer.
- [Wikipedia — S-expression](https://en.wikipedia.org/wiki/S-expression) —
  the notation you're printing, and why Lisp people never stopped using it.

**Done when:** bad input never crashes or leaks; `parsedump "1+2*3"` prints
`(+ 1 (* 2 3))`.
