# Phase 4 — Control flow and functions

**Goal:** `if`/`else`, `while`, comparison and logical operators, block
scoping, and user-defined functions with parameters and `return`. Exit
criterion for the phase: **fib and FizzBuzz run in tinylang.**

**Why:** control flow is what makes a language *Turing-complete* — before
this phase, every program's runtime is proportional to its length; after
`while`, programs can loop forever, and your language can compute anything
computable. Functions then add the deepest ideas in the project: **scope**
(which `x` does this name mean?) and the **call stack** (where do a
function's locals live, and how does `return` get back?). These two ideas
are most of what separates people who use languages from people who
understand them.

**Spine reading:**
- [Crafting Interpreters ch. 9, "Control Flow"](https://craftinginterpreters.com/control-flow.html)
- [Crafting Interpreters ch. 10, "Functions"](https://craftinginterpreters.com/functions.html)

---

## Task 4.1 — Booleans and comparisons (~1.5h)

**Build:** `true`/`false` literals, `< > <= >= == !=`, and `!`. This forces
the big refactor: `eval` can no longer return a bare number — you need a
**tagged `Value` struct** (`VAL_NUM` / `VAL_BOOL`). Decide: what happens on
`1 + true`? (Runtime error is the honest answer at this stage; Phase 7
upgrades it to compile-time.)

**Why:** the moment a language has two types, *type errors exist*, and every
operation must decide what it accepts. The tagged-Value refactor touching
every arm of `eval` teaches you why people say "types infect everything" —
felt once here, and Phase 7 will feel like relief instead of bureaucracy.
Two-char operators (`<=`, `==`) also make the lexer peek ahead for the
first time — a real scanning technique (maximal munch).

**Read first:**
- [Crafting Interpreters ch. 4](https://craftinginterpreters.com/scanning.html), §4.5.2 (two-character operators).
- [Crafting Interpreters ch. 7](https://craftinginterpreters.com/evaluating-expressions.html), §7.2.5 (truthiness) and §7.3 (runtime errors).

**Done when:** asserts cover comparisons, `==` on bools, and `1 + true`
producing your documented error, not garbage.

---

## Task 4.2 — `if` / `else` (~1h)

**Build:** grammar, AST node, parsing, and evaluation for
`if (cond) stmt else stmt`, plus block statements `{ ... }`.

**Why:** your first *statement that contains statements* — the AST becomes
truly recursive at the statement level too. You'll also meet the famous
**dangling else** ambiguity (`if (a) if (b) x; else y;` — whose else is
it?) and resolve it the way C and Java do: else binds to the nearest if.
Naming a 50-year-old ambiguity and picking a policy is compiler literacy.

**Read first:**
- [Crafting Interpreters ch. 9](https://craftinginterpreters.com/control-flow.html), §9.2.
- [Crafting Interpreters ch. 8](https://craftinginterpreters.com/statements-and-state.html), §8.5.2 (block syntax).

**Done when:** nested ifs with elses evaluate per the nearest-if rule;
tests prove it.

---

## Task 4.3 — Logical operators and `while` (~1–1.5h)

**Build:** `and`/`or` (or `&&`/`||` — your language, your call) with
**short-circuit** evaluation, then `while`.

**Why:** short-circuiting is the first place evaluation *order* becomes
observable semantics — `or` isn't a function of two values, it's control
flow wearing an operator costume (the right side may never run). `while` in
a tree-walker is beautifully anticlimactic — a C `while` around the body's
`eval` — enjoy that, because in Phase 6 you'll implement the same loop with
raw conditional jumps and appreciate what the tree was giving you for free.

**Read first:**
- [Crafting Interpreters ch. 9](https://craftinginterpreters.com/control-flow.html), §9.3–9.4.

**Done when:** a while-loop countdown runs; a test proves the right side of
`or` didn't evaluate (e.g. it would have been a division by zero).

---

## Task 4.4 — Block scope done right (~1.5h)

**Build:** environments that **chain**: each `{ }` gets a fresh environment
pointing at its parent; lookups walk the chain outward; inner `let x`
shadows outer `x` and dies at the closing brace.

**Why:** this parent-pointer chain *is* lexical scoping — the single concept
behind locals, globals, shadowing, and (later, if you want them) closures.
It's also a C ownership puzzle: who frees an environment, and when? Getting
scoping semantics + memory discipline right in ~40 lines is the kind of
thing you can talk about in an interview with real authority.

**Read first:**
- [Crafting Interpreters ch. 8](https://craftinginterpreters.com/statements-and-state.html), §8.5 "Scope" — read it slowly, it's the concept of the phase.

**Done when:** shadowing tests pass: inner blocks see outer variables,
assignments in inner blocks hit the right one, inner `let`s vanish on exit.

---

## Task 4.5 — Function declarations and calls (~2h)

**Build:** `fn add(a, b) { return a + b; }` and call syntax `add(1, 2)`:
a function value (name, parameter list, body AST, and the environment it
was declared in), calls that bind arguments to parameters in a fresh
environment, arity checking (`add(1)` = clean error).

**Why:** a function value is *code as data* — the body just sits there as an
AST until called. Binding arguments in a fresh child environment is scoping
(4.4) and state (3.3) composing into something powerful — recursion works
without you writing a single line of extra code for it, which is worth
pausing on: *why does it?*

**Read first:**
- [Crafting Interpreters ch. 10](https://craftinginterpreters.com/functions.html), §10.1–10.4.

**Done when:** declared functions call, arity errors report cleanly,
a function calling another function works.

---

## Task 4.6 — `return`, recursion, milestone programs (~1.5h)

**Build:** `return` that immediately unwinds out of nested blocks/loops to
the caller (in C, without exceptions: a sentinel in your eval result, or
setjmp/longjmp — investigate both, pick one, write a comment defending it).
Then write, **in tinylang**: `fib(n)` (recursive) and FizzBuzz. Save them in
`examples/`.

**Why:** `return` from deep inside nested statements is *non-local control
flow* — the hardest control problem a tree-walker faces, and your chosen
mechanism is a genuine engineering tradeoff, not a tutorial exercise.
Recursive fib working end-to-end proves the whole machine: call stack,
scopes, returns, arithmetic, conditionals — every phase so far, load-bearing
at once. That's the milestone.

**Read first:**
- [Crafting Interpreters ch. 10](https://craftinginterpreters.com/functions.html), §10.5 (return statements).

**Done when:** `fib(15)` prints 610. FizzBuzz fizzes. Commit with pride —
you have a programming language.
