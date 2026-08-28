# Phase 3 — Tree-walking interpreter: arithmetic + variables

**Goal:** make code *run*. `eval()` walks the AST and produces a value:
`(1 + 2) * 3` → `9`. Then add statements and variables:
`let x = 4; print x * 2;` → `8`. Plus a REPL.

**Why:** this is the moment your project becomes a *language* instead of a
parser. So far you've built representations of programs; now you'll build
their **meaning**. A tree-walking interpreter is the simplest complete
strategy: the meaning of a node is defined in terms of the meaning of its
children — "the meaning of a `+` node is: evaluate both children, add the
results" — so one recursive function over the tree *is* the language's
semantics. It's a real strategy, not a toy (Ruby ran this way for its
first decade). More importantly, it forces the two questions every
language must answer: **what is a value?** and **where do variables
live?** Your answers here — a value type, an environment — survive all the
way through Phase 7.

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

**The concept:** evaluation is a definition of meaning by recursion. What
does the tree `(+ 1 (* 2 3))` *mean*? The meaning of a NUMBER node is its
number. The meaning of a BINARY node is: compute the meaning of the left
child, compute the meaning of the right child, combine them with the
operator. That's the whole interpreter — a `switch` on the node tag with
one recursive case per shape. Notice the deep alignment: the grammar rule
`term → factor (("+"|"-") factor)*`, the tree node `BINARY(+, l, r)`, and
the eval case `eval(l) + eval(r)` are *the same idea in three forms* —
syntax rule, data shape, meaning. Seeing that alignment is the insight of
this project's entire first half.

Division by zero is your first genuine **language-design decision**.
The options are all defensible: report a runtime error and stop (Python's
choice), return IEEE infinity (JavaScript's choice, free if your values
are `double`), or define it as some sentinel. There is no right answer —
only *documented* answers, and undocumented behavior is how languages
grow warts. Whatever you pick, write it down and test it. (Fair warning
from C itself: *integer* division by zero is undefined behavior in C —
your UBSan build will loudly demonstrate what that means if you evaluate
`1/0` with `long` values and no check.)

**Why compilers need it:** every language pipeline ends at "and then it
means something." You're also about to build this meaning a *second* way
(Phases 5–6 compile the same trees to bytecode), and having the
tree-walking version first gives you an oracle to test the compiler
against — the two back ends must always agree.

**Read first:**
- [Crafting Interpreters ch. 7](https://craftinginterpreters.com/evaluating-expressions.html) —
  all of it; it's short and it is this task.
- [Let's Build a Simple Interpreter, part 7](https://ruslanspivak.com/lsbasi-part7/) —
  the same eval-over-an-AST in Python, with pictures of the walk.
- [Clang UBSan docs](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) —
  see `integer-divide-by-zero` in the checks list: C's own answer to this
  task's design question is "the compiler may do anything", which is why
  your language should do better.

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

**The concept:** an **expression** is evaluated for its *value* —
`1 + 2` is 3. A **statement** is executed for its *effect* — `print x;`
puts characters on the screen and has no value at all. This split runs
through almost every language you'll ever use (`x = 5` vs `if` in C,
Java, Swift...), and languages that erase it — Lisp, Rust to a degree,
where everything is an expression — are notable *because* they erase it.
Your grammar now has two levels: a program is a list of statements;
statements contain expressions. And `eval` splits accordingly: expression
nodes return a value; statement nodes return nothing but *do* something.

The lexer change is conceptually meaty too: `print` is letters, and until
now letters were errors. The standard technique: lex any run of letters
as a word, *then* check the word against the keyword list (there's your
`my_strcmp`) — if it matches, it's a `TOK_PRINT`; otherwise it's an
identifier (which task 3.3 needs anyway). Keywords are just identifiers
the language reserved for itself.

**Why compilers need it:** this task is your first end-to-end **feature
thread**: one small feature touches lexer → AST → parser → evaluator, in
that order. Threading a feature through every layer is the canonical
compiler-hacking experience — real language work is this loop repeated —
and this is the smallest possible rep. Every feature in Phase 4 repeats
it, so learn the checklist now: token, node, grammar rule, parse function,
eval case, test.

**Read first:**
- [Crafting Interpreters ch. 8, §8.1–8.2](https://craftinginterpreters.com/statements-and-state.html) —
  statements and the statement/expression split.
- [Crafting Interpreters ch. 4, §4.7](https://craftinginterpreters.com/scanning.html) —
  identifiers and keywords (you skipped it in Phase 1; now you need it).
- [Wikipedia — Statement (computer science)](https://en.wikipedia.org/wiki/Statement_%28computer_science%29) —
  the statement/expression distinction across languages, for the wider
  view.

**Done when:** running a two-statement program prints two lines; tests
capture output or check side effects.

---

## Task 3.3 — Variables and the environment (~2h)

**Build:** `let x = expr;`, reading `x` in expressions, and assignment.
The **environment**: a name→value table. A linear array of (name, value)
pairs is completely fine at this scale — `ponytail:` a hash table (Crafting
Interpreters ch. 20 builds one) is the upgrade path when lookup shows up in
a profile, not before.

**The concept:** until now, every value your language computes is used
once and gone. Variables give programs **memory across time**: `let x = 3`
stores a value under a name; `x * 2` later retrieves it. The machinery is
the **environment** — a table mapping names to their current values.
`let` adds an entry; reading a variable looks its name up; assignment
overwrites an existing entry. Three operations, one table, and suddenly
your language has state.

Two design questions hide in the small print. *What if you read a name
that was never defined?* This is your first **runtime error** that isn't
about types or arithmetic: `print y;` is grammatically perfect — the
parser waves it through — and still wrong, discoverable only when
execution reaches it. (Phase 7 exists to catch this class of error
*before* running; feel the gap now.) *What does `let x = 5` mean when `x`
already exists?* Redefinition, error, or shadowing — pick one, write it
down, test it. Also mind the C detail: your token's name is a
pointer+length into the source, so the environment must either copy the
name string or the source must outlive the environment — the task 1.1
lifetime rule coming home.

**Why compilers need it:** the environment is the single most load-bearing
data structure in an interpreter. Phase 4's block scopes and functions are
"just" environments pointing at parent environments — every scoping
feature in every language reduces to operations on this table. Get its
semantics crisp while it's one flat list.

**Read first:**
- [Crafting Interpreters ch. 8, §8.3–8.4](https://craftinginterpreters.com/statements-and-state.html) —
  environments and assignment.
- [Let's Build a Simple Interpreter, part 9](https://ruslanspivak.com/lsbasi-part9/) —
  the same idea in Python under the name "symbol table": assignment
  statements storing into a dict, lookups reading out.
- [Cornell CS 3110 — Environment Model](https://cs3110.github.io/textbook/chapters/interp/environment.html) —
  the formal treatment of environments; heavier, worth skimming to see
  the same structure described precisely.

**Done when:** `let x = 3; let y = x * 2; print x + y;` prints 9;
undefined-variable use reports a clean error with the name in it.

---

## Task 3.4 — The REPL (~1–1.5h)

**Build:** `tinylang` the executable: no args → interactive
read-eval-print loop (`fgets`, feed the line through lexer→parser→eval,
print, repeat; environment persists across lines). One arg → run that file
(`fopen`/`fread` it into a string — new C skill, Beej ch. on file I/O).

**The concept:** a **REPL** — Read, Eval, Print, Loop — is exactly its
name: read a line of input, run it through your entire pipeline, print
the result, repeat forever. The idea dates to Lisp in the 1960s and is
why `python` with no arguments drops you at `>>>`. The one subtlety that
makes a REPL good: the environment *persists across lines*, so `let x = 3`
on line one and `print x * 2;` on line two work together — which falls out
naturally if you create the environment once, outside the loop.

The file-running mode teaches the other missing C skill: getting a file's
contents into memory. The classic dance is `fopen`, `fseek` to the end,
`ftell` to learn the size, rewind, `malloc(size + 1)`, `fread`, and — the
step everyone forgets — write the `'\0'` terminator yourself, because
`fread` won't. Forget it and your lexer walks off the end of the buffer
into the sanitizer's arms. After this, "source code" is no longer a
string literal in a test; it's a file someone wrote.

**Why compilers need it:** a REPL changes your development speed
permanently — every future feature gets poked at interactively within
seconds of existing, which is why virtually every serious language ships
one. And file execution is the last piece of "a real language you could
hand to a friend": they write `program.tiny`, they run it, it works.

**Read first:**
- [Crafting Interpreters ch. 7, §7.4](https://craftinginterpreters.com/evaluating-expressions.html) —
  hooking up the interpreter (Lox's REPL arrived in ch. 4's framework;
  this wires evaluation into it).
- [Beej's Guide to C — File Input/Output](https://beej.us/guide/bgc/html/split/file-inputoutput.html) —
  fopen, fgets, fread; everything the file mode needs.
- [Wikipedia — Read–eval–print loop](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) —
  the history and anatomy of the REPL idea.

**Done when:** you can sit in the REPL doing math with variables, and
`./build/tinylang examples/first.tiny` runs a file you wrote. Save that
file into the repo — it's your language's first program. Take the screenshot;
you earned it.
