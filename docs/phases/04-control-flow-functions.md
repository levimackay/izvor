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

**The concept:** until now your language has exactly one type, so "what
kind of value is this?" never needed asking. `true` breaks that. Once two
kinds of value flow through the same `eval`, every value must *carry its
kind with it* — and that's the same tagged-union idea from task 2.1, now
applied to runtime data: a `Value` struct holding a tag (`VAL_NUM`,
`VAL_BOOL`) and a union of payloads. This is precisely what dynamically
typed languages mean when they say "values have types, variables don't":
in Python, the tag lives on the object; in your interpreter, it lives in
this struct. And the refactor's pain — every arm of `eval` must now
unwrap inputs, check tags, and wrap outputs — is itself the lesson: types
infect everything, because every operation must decide what it accepts.
`1 + true` is now a question with three possible answers (error, coerce,
define it), and *runtime error with a clear message* is the honest one at
this stage.

There's a lexer lesson riding along too: `<=` and `==` are two characters
but one operator. The technique is one-character lookahead — on seeing
`<`, peek: if `=` follows, consume it and emit `TOK_LE`; otherwise emit
`TOK_LT`. That's **maximal munch** again (task 1.3): the lexer takes the
longest token it can, which is why `==` never lexes as two `=`s.

**Why compilers need it:** every language with more than one type — that
is, every language — has this exact machinery somewhere. When Phase 7
arrives, the compile-time checker will make most of these runtime tag
checks provably unnecessary, and having *felt* the runtime version is
what makes static typing feel like relief instead of bureaucracy.

**Read first:**
- [Crafting Interpreters ch. 4, §4.5.2](https://craftinginterpreters.com/scanning.html) —
  two-character operators in the scanner.
- [Crafting Interpreters ch. 7, §7.2.4 and §7.3](https://craftinginterpreters.com/evaluating-expressions.html) —
  truthiness, and runtime errors done deliberately.
- [Crafting Interpreters ch. 18, "Types of Values"](https://craftinginterpreters.com/types-of-values.html) —
  the C version: clox builds *exactly* the tagged `Value` struct you're
  building (it's written for the bytecode VM, but the Value design reads
  standalone).
- [Wikipedia — Tagged union](https://en.wikipedia.org/wiki/Tagged_union) —
  the same structure you used for AST nodes, now for values.

**Done when:** asserts cover comparisons, `==` on bools, and `1 + true`
producing your documented error, not garbage.

---

## Task 4.2 — `if` / `else` (~1h)

**Build:** grammar, AST node, parsing, and evaluation for
`if (cond) stmt else stmt`, plus block statements `{ ... }`.

**The concept:** evaluating `if` in a tree-walker is almost anticlimactic
— evaluate the condition, check its tag is a bool (or define truthiness),
then walk *one* of the two child statements and simply never visit the
other. Un-taken code sitting unexecuted in a tree is the whole mechanism.
The interesting work is in the parser, because `if` is your first
**statement that contains statements** — the AST is now recursive at the
statement level too, and a block `{ ... }` is a statement holding a *list*
of statements (your dynamic array from 0.8, holding node pointers).

Then comes a genuine 50-year-old landmine: the **dangling else**. In
`if (a) if (b) x; else y;` — whose `else` is that? The grammar as
naively written allows *both* readings; that's called an ambiguity, and
parsers can't ship with one. The resolution every C-family language
chose: **else binds to the nearest unmatched if**. In recursive descent
you get this almost for free — the inner `if`'s parse function is the one
running when `else` appears, so it grabs it — but you should *know* you're
making the choice, test it, and be able to name it.

**Why compilers need it:** conditional execution is the first half of
Turing-completeness (choice; loops are the other half). And the dangling
else is compiler literacy: it's the standard example of grammar ambiguity,
it will appear in any compilers conversation you ever have, and after this
task you'll have resolved it yourself in working code.

**Read first:**
- [Crafting Interpreters ch. 9, §9.2](https://craftinginterpreters.com/control-flow.html) —
  conditional execution.
- [Crafting Interpreters ch. 8, §8.5.2](https://craftinginterpreters.com/statements-and-state.html) —
  block syntax and semantics.
- [Wikipedia — Dangling else](https://en.wikipedia.org/wiki/Dangling_else) —
  the ambiguity, its history, and how different languages resolve it.

**Done when:** nested ifs with elses evaluate per the nearest-if rule;
tests prove it.

---

## Task 4.3 — Logical operators and `while` (~1–1.5h)

**Build:** `and`/`or` (or `&&`/`||` — your language, your call) with
**short-circuit** evaluation, then `while`.

**The concept:** `and` and `or` look like operators but don't behave like
`+`. With `+`, both operands are always evaluated, then combined. With
`or`, if the left side is true, the right side is **never evaluated at
all** — not "evaluated and ignored", *never run*. That's short-circuit
evaluation, and it makes evaluation *order* part of your language's
observable meaning for the first time: `false or crash()` calls `crash`,
but `true or crash()` doesn't. Which is why programmers everywhere can
safely write `x != 0 and 10/x > 2` — the idiom only works because `and`
is really control flow wearing an operator costume. Implement it that
way: evaluate the left; decide whether to even visit the right child.

`while` in a tree-walker is beautifully anticlimactic: a C `while` loop
that re-evaluates the condition node and re-walks the body node until the
condition goes false. The tree just sits there being walked repeatedly.
Enjoy how easy that was — in Phase 6 you will implement the same `while`
with raw conditional jumps and a moving instruction pointer, and you'll
appreciate exactly what the tree representation was giving you for free.

**Why compilers need it:** short-circuiting is semantics that *every*
mainstream language guarantees (C, Java, Python, Swift...), programs
depend on it for correctness (the divide-by-zero guard above), and so
your two back ends must both honor it — there's a Phase 6 test hiding in
this sentence. `while` completes Turing-completeness: your language can
now express any computation.

**Read first:**
- [Crafting Interpreters ch. 9, §9.3–9.4](https://craftinginterpreters.com/control-flow.html) —
  logical operators and while.
- [Wikipedia — Short-circuit evaluation](https://en.wikipedia.org/wiki/Short-circuit_evaluation) —
  the semantics across languages, and the idioms it enables.
- [Beej's Guide to C — Variables and Statements](https://beej.us/guide/bgc/html/split/variables-and-statements.html) —
  C's own `&&`/`||`/`while` (which your implementation language is
  giving you underneath — notice the layers).

**Done when:** a while-loop countdown runs; a test proves the right side of
`or` didn't evaluate (e.g. it would have been a division by zero).

---

## Task 4.4 — Block scope done right (~1.5h)

**Build:** environments that **chain**: each `{ }` gets a fresh environment
pointing at its parent; lookups walk the chain outward; inner `let x`
shadows outer `x` and dies at the closing brace.

**The concept:** **scope** answers the question "when I write `x`, *which*
`x` do I mean?" The rule used by essentially every modern language is
**lexical scoping**: a name refers to the nearest enclosing declaration
*in the program text*. The implementation is one pointer: give each
environment a `parent`. Entering a `{` creates a fresh, empty environment
whose parent is the current one; leaving the `}` discards it. Lookup
walks outward — check this environment, then its parent, then *its*
parent — so inner names **shadow** outer ones (the walk finds the inner
one first), inner blocks can still *see* outer variables (the walk keeps
going), and a block's variables *vanish* at the closing brace (their whole
environment is discarded). Three scoping behaviors, one parent pointer.

The C puzzle underneath: who frees an environment, and when? The block's
env dies when the block ends — but the values in it may not (what if one
was assigned into an outer variable?), and the *names* in it point into
source text or heap copies with their own lifetimes. Decide the ownership
story, write it in a comment, verify with `leaks`. Getting scoping
semantics *and* memory discipline right in ~40 lines is the kind of thing
you can talk about in an interview with real authority.

**Why compilers need it:** this parent-pointer chain is the single concept
behind locals, globals, shadowing, and — if you ever add them — closures
(a closure is just a function that keeps a pointer to the environment
where it was born). Phase 7 will build a second, compile-time copy of this
exact structure for types. It is not an exaggeration to call this chain
the skeleton of language semantics.

**Read first:**
- [Crafting Interpreters ch. 8, §8.5 "Scope"](https://craftinginterpreters.com/statements-and-state.html) —
  read it slowly; it's the concept of the phase.
- [Cornell CS 3110 — Environment Model](https://cs3110.github.io/textbook/chapters/interp/environment.html) —
  the same chain, formally: environments, lexical vs dynamic scope,
  closures.
- [Wikipedia — Scope (computer science)](https://en.wikipedia.org/wiki/Scope_%28computer_science%29) —
  the panoramic reference: lexical vs dynamic, history, and how languages
  differ.

**Done when:** shadowing tests pass: inner blocks see outer variables,
assignments in inner blocks hit the right one, inner `let`s vanish on exit.

---

## Task 4.5 — Function declarations and calls (~2h)

**Build:** `fn add(a, b) { return a + b; }` and call syntax `add(1, 2)`:
a function value (name, parameter list, body AST, and the environment it
was declared in), calls that bind arguments to parameters in a fresh
environment, arity checking (`add(1)` = clean error).

**The concept:** the quietly radical idea here is that a function is
**code as data**. When the interpreter meets `fn add(a, b) { ... }`, it
does *not* run the body — it wraps the body's AST, the parameter names,
and (for correct scoping) the environment where the declaration happened
into a function *value*, and stores it in the environment like any other
value. The body just sits there, inert, until a call.

A call `add(1, 2)` is then three composed ideas you already own:
evaluate the arguments (Phase 3 eval); create a **fresh environment**
whose parent is the function's declaration environment, and bind `a→1`,
`b→2` in it (task 4.4's chain); walk the body in that environment
(Phase 3 again). Arity checking — arguments must match parameters in
count — is one `if` and your first *good* error message of the phase.

Now pause on the free gift: **recursion already works.** When `fib`'s
body calls `fib`, that inner call gets its *own* fresh environment with
its *own* `n`, layered on the C call stack as your `eval` recurses.
Nothing extra to implement. Ask yourself *why* — the answer ("each call's
locals live in that call's environment, and the environments don't
interfere") is the whole concept of the call stack, discovered rather
than taught.

**Why compilers need it:** functions are the abstraction mechanism —
the feature that lets programs be built out of named, reusable parts.
Every language you'll ever implement or be interviewed about has some
version of this machinery: an activation record (your fresh environment),
argument binding, and a return path (next task).

**Read first:**
- [Crafting Interpreters ch. 10, §10.1–10.4](https://craftinginterpreters.com/functions.html) —
  call syntax, function objects, declarations.
- [Wikipedia — Call stack](https://en.wikipedia.org/wiki/Call_stack) —
  what "stack frame", "return address", and "unwinding" mean in every
  runtime, including the CPU-level version yours is piggybacking on.
- [Cornell CS 3110 — Environment Model](https://cs3110.github.io/textbook/chapters/interp/environment.html) —
  the closures section: why the function value captures its declaration
  environment (yours does too, even before you add closures).

**Done when:** declared functions call, arity errors report cleanly,
a function calling another function works.

---

## Task 4.6 — `return`, recursion, milestone programs (~1.5h)

**Build:** `return` that immediately unwinds out of nested blocks/loops to
the caller (in C, without exceptions: a sentinel in your eval result, or
setjmp/longjmp — investigate both, pick one, write a comment defending it).
Then write, **in tinylang**: `fib(n)` (recursive) and FizzBuzz. Save them in
`examples/`.

**The concept:** `return` is **non-local control flow** — the hardest
control problem a tree-walker faces. A `return` may fire three blocks and
two loops deep, and execution must immediately abandon all of that nesting
and reappear at the call site with a value. But your evaluator is a stack
of recursive C calls (`eval` inside `eval` inside `eval`), and C won't
abandon those frames for you. Two honest mechanisms exist. **Sentinel
returns:** every statement-eval returns a status ("completed" vs
"returning, with this value"), and every construct that runs
sub-statements — blocks, `if`, `while` — checks the status and, on
"returning", stops and passes it upward. Explicit, verbose, totally
clear: the return bubbles up frame by frame. **setjmp/longjmp:** C's
primitive non-local jump — plant a marker at the call site with
`setjmp`, and `longjmp` from anywhere below teleports straight back,
discarding intervening frames. Terse, but it skips your cleanup code on
the way past (who frees those environments now?) and reads like magic.
Investigate both, pick one, defend it in a comment. This is a genuine
engineering tradeoff — it's exceptions vs error codes in miniature, the
same debate Java and Go answered differently.

**Why compilers need it:** every language with functions has to solve
"get out from deep inside" — exceptions, panics, and early returns are
all this problem. And the milestone matters more than the mechanism:
recursive `fib` working end-to-end exercises call frames, scopes,
returns, arithmetic, and conditionals — every phase so far, load-bearing
simultaneously. When `fib(15)` prints 610, you have a programming
language.

**Read first:**
- [Crafting Interpreters ch. 10, §10.5](https://craftinginterpreters.com/functions.html) —
  return statements (Nystrom uses Java exceptions — your sentinel/longjmp
  choice is the C translation of exactly that).
- [Beej's Guide to C — setjmp/longjmp](https://beej.us/guide/bgc/html/split/setjmp-longjmp.html) —
  the friendly version of option two.
- [man page: setjmp(3)](https://man7.org/linux/man-pages/man3/setjmp.3.html) —
  the reference version, including the caveats that might talk you out of
  it.

**Done when:** `fib(15)` prints 610. FizzBuzz fizzes. Commit with pride —
you have a programming language.
