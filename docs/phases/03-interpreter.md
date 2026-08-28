# Phase 3 — Tree-walking interpreter: arithmetic + variables

**Goal:** make code *run*. `eval()` walks the AST and produces a value:
`(1 + 2) * 3` → `9`. Then add statements and variables:
`let x = 4; print x * 2;` → `8`. Plus a REPL.

**Why:** this is the moment your project becomes a *language* instead of a
parser. A tree-walking interpreter is the simplest complete strategy — no
translation step, just "the meaning of a `+` node is: evaluate both
children, add" — and it's a real one (Ruby's original MRI worked this way).
More importantly, it forces the two questions every language must answer:
**what is a value?** and **where do variables live?** Your answers here
(a value struct, an environment) survive all the way through Phase 7.

**Spine reading:**
- [Crafting Interpreters ch. 7, "Evaluating Expressions"](https://craftinginterpreters.com/evaluating-expressions.html)
- [Crafting Interpreters ch. 8, "Statements and State"](https://craftinginterpreters.com/statements-and-state.html)

(Nystrom uses Java's `Object` for values and exceptions for runtime errors —
you have neither, which is a feature: you'll build the C equivalents, a
tagged `Value` struct and explicit error returns, and understand exactly what
Java was hiding.)

---

## Task 3.1 — `eval` for expressions (~1.5h)

**Build:** `interp.c` with `double eval(Node*)` (or `long` — decide, and
write down why): a `switch` on node tag, recursing into children. Handle
division by zero *somehow* deliberate (error? trap? IEEE infinity? — decide
and test it).

**Why:** the recursive structure of `eval` mirrors the grammar mirrors the
tree — one idea, three forms; seeing that alignment is *the* insight of this
whole project's first half. The divide-by-zero decision is your first real
**language design choice**: there's no right answer, only documented
answers. (C itself makes integer division by zero undefined behavior — your
UBSan flags will show you what that means if you don't check.)

**Read first:**
- [Crafting Interpreters ch. 7](https://craftinginterpreters.com/evaluating-expressions.html) — all of it; it's short.

**Done when:** your asserts cover precedence (`1+2*3 → 7`), parens,
unary minus, associativity (`8-4-2 → 2`), and your chosen div-by-zero
behavior.

---

## Task 3.2 — Statements: `print` and expression statements (~1.5h)

**Build:** extend grammar + AST + parser + eval:
`program → statement* ; statement → "print" expression ";" | expression ";"`.
This means Phase 1's lexer learns its first *keyword* (`print`) and
identifiers-as-lexemes — the keyword-vs-identifier check is your task 0.6
`my_strcmp` put to work.

**Why:** expressions *produce values*; statements *do things* — this split
exists in almost every language (and its absence defines the exceptions,
like Lisp). Threading one small feature through **every layer** —
lexer → AST → parser → eval — is the canonical compiler-hacking experience;
you'll repeat this loop dozens of times, and this is the smallest possible
rep.

**Read first:**
- [Crafting Interpreters ch. 8](https://craftinginterpreters.com/statements-and-state.html), §8.1–8.2.
- [Crafting Interpreters ch. 4](https://craftinginterpreters.com/scanning.html), §4.7 (identifiers/keywords — you skipped it in Phase 1; now you need it).

**Done when:** running a two-statement program prints two lines; tests
capture output or check side effects.

---

## Task 3.3 — Variables and the environment (~2h)

**Build:** `let x = expr;`, reading `x` in expressions, and assignment.
The **environment**: a name→value table. A linear array of (name, value)
pairs is completely fine at this scale — `ponytail:` a hash table (Crafting
Interpreters ch. 20 builds one) is the upgrade path when lookup shows up in
a profile, not before.

**Why:** variables are the first feature that gives your language *memory
across time*, and the environment is the single most load-bearing data
structure in an interpreter — Phase 4's scopes and closures are "just"
environments pointing at parent environments. Also your first
**runtime error** distinct from syntax errors: using an undefined variable
is grammatically perfect and still wrong — a category of error the parser
cannot catch (Phase 7 makes this exact point again, statically).

**Read first:**
- [Crafting Interpreters ch. 8](https://craftinginterpreters.com/statements-and-state.html), §8.3–8.4.

**Done when:** `let x = 3; let y = x * 2; print x + y;` prints 9;
undefined-variable use reports a clean error with the name in it.

---

## Task 3.4 — The REPL (~1–1.5h)

**Build:** `tinylang` the executable: no args → interactive
read-eval-print loop (`fgets`, feed the line through lexer→parser→eval,
print, repeat; environment persists across lines). One arg → run that file
(`fopen`/`fread` it into a string — new C skill, Beej ch. 9 on file I/O).

**Why:** a REPL changes your development speed permanently — every future
feature gets poked at interactively within seconds of existing. Reading a
file into memory is also the last missing piece of "a real language you
could hand to a friend": they can write `program.tiny` and run it.

**Read first:**
- [Crafting Interpreters ch. 7](https://craftinginterpreters.com/evaluating-expressions.html), §7.4 (hooking up the interpreter/REPL).
- [Beej's Guide to C — File I/O chapter](https://beej.us/guide/bgc/html/split/) (ch. 9 "File Input/Output").

**Done when:** you can sit in the REPL doing math with variables, and
`./build/tinylang examples/first.tiny` runs a file you wrote. Save that
file into the repo — it's your language's first program. Take the screenshot;
you earned it.
