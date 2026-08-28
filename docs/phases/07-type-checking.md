# Phase 7 (stretch) — Basic static type checking

**Goal:** a checker that walks the AST *before execution* and rejects
programs like `1 + true` or `if (5) ...` with a clear compile-time error —
the difference between finding a bug when that line eventually runs (maybe
in production) and finding it the moment you hit save.

**Why:** right now, tinylang discovers type errors the *dynamic* way: it
runs the program and hits them (your Phase 4 runtime errors). **Static**
type checking discovers them without running anything, by reasoning about
what *kind* of value every expression must produce. This is the idea
behind Swift, TypeScript, Rust — the languages you actually want to work
in (and the one Apple will interview you in). Having built the dynamic
version first, you're in the rare position of implementing both sides of
the oldest argument in programming and understanding the tradeoff from
the inside: the checker rejects some fine programs (it must be
conservative — it can't run your code to find out), in exchange for a
guarantee about *every* execution path, including the ones your tests
never take. A type checker is also your fourth-or-fifth AST walk, which
by now should feel like home: same traversal, new question — not "what
value?" but "what *kind* of value, provably, without running it?"

**Spine reading:** Crafting Interpreters doesn't cover static typing (Lox
is dynamic), so the spine shifts:

- [Cornell CS 3110 textbook, "Type Checking"](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) —
  a free university-course chapter that builds a checker for a small
  language (SimPL) remarkably close to tinylang. The code is OCaml, but
  read it for the *rules* — each rule translates to one `switch` case in C.
- [Intro to type checking (Mukul Rathi)](https://mukulrathi.com/create-your-own-programming-language/intro-to-type-checking/) —
  a working programmer's walkthrough of building a checker for his own
  language; friendlier than the textbook, same ideas.
- The classic book here is Pierce's *Types and Programming Languages*
  (library, not free online) — strictly optional, mentioned so you know
  where the deep end is.

---

## Task 7.1 — The type language + annotations (~1.5h)

**Build:** a `Type` enum (`TYPE_NUM`, `TYPE_BOOL` — that's genuinely
enough), and syntax for annotating declarations: `let x: num = 5;`
(lexer: colon token; parser: optional annotation on `let` and on function
parameters/returns).

**The concept:** a **type** in this phase is a *claim about the future*:
`x: num` asserts "every value ever stored in `x` will be a number".
Notice that types now exist in **three different places** in your
codebase, wearing three different costumes: as *syntax* (the characters
`: num` in the source), as *checker data* (your `Type` enum, the
compile-time claim), and as *runtime tags* (Phase 4's `VAL_NUM` on actual
values). They're related but distinct — the annotation is what the
programmer promised, the enum is what the checker reasons with, the tag
is what actually happens — and keeping the three aligned is the daily
bread of compiler teams (a misalignment is a soundness bug).

Designing the annotation *syntax* is pure language design: mandatory or
optional? Where may annotations appear? You're using the postfix
`name: type` style (Swift, TypeScript, Rust) rather than C's prefix
`type name` — a choice with real ergonomic consequences for parsing
(the colon tells the parser exactly what's coming; C's prefix style is
why C declarations are famously unreadable). You'll never read a Swift
signature the same way.

**Why compilers need it:** the checker can't check claims that can't be
written. This task builds the vocabulary; the next two make it enforced.
Deliberately, nothing is checked yet — annotations parse into the AST and
sit there — because pipeline stages should land one at a time.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) —
  the first sections, through "A Type System for SimPL".
- [TypeScript Handbook — Everyday Types](https://www.typescriptlang.org/docs/handbook/2/everyday-types.html) —
  see the same `x: type` annotations in a mainstream gradually-typed
  language (and try them live in its playground).
- [Wikipedia — Type system](https://en.wikipedia.org/wiki/Type_system) —
  the reference map: static vs dynamic, strong vs weak, where your little
  system sits.

**Done when:** annotated programs parse (asserts on the AST carrying the
annotation); the checker doesn't exist yet.

---

## Task 7.2 — Check expressions (~1.5–2h)

**Build:** `Type check_expr(Node*, TypeEnv*)`: literals have obvious types;
arithmetic requires num+num→num; comparisons num+num→bool; `==` requires
matching sides; `!` and `and`/`or` require bools. Errors report *expected
vs found*, with enough position info to be humane.

**The concept:** type checking an expression is a **bottom-up proof over
the tree**. The leaves are axioms: `5` is a num, `true` is a bool,
period. Every interior node is an inference rule: *if* the left child
checks as num *and* the right child checks as num, *then* this `+` node
checks as num — and if the children don't satisfy the rule, that's
precisely a type error, reported with what the rule expected vs what the
children provided. Run that over the whole tree and you've *proved* a
property of the program — "this expression is a num" — without computing
a single value. That's the magic worth sitting with: `eval` had to run
`fib(30)` to learn its answer; `check` learns its *type* instantly,
for all possible inputs at once, because it only tracks kinds, not
values.

The academic notation for these rules — `env ⊢ e : t`, read "in this
environment, expression e has type t" — appears throughout the CS 3110
chapter, and each such **typing judgment** translates to one `switch`
case in your C. You are implementing a small formal system, and it will
feel eerily like `eval` with types where values used to go. That
similarity is a deep fact with a name (abstract interpretation: running
the program over a simplified domain), not a coincidence.

**Why compilers need it:** this bottom-up walk *is* type checking — in
swiftc, tsc, javac, the machinery is richer (inference, generics) but the
skeleton is this exact recursion. And the error-message discipline
(expected X, found Y, at location Z) is what makes a checker a tool
rather than an obstacle; you have opinions about compilers' error
messages, and now you're on the hook for your own.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) —
  the typing rules and the checker implementation.
- [Wikipedia — Type rule](https://en.wikipedia.org/wiki/Type_rule) —
  decode the `Γ ⊢ e : τ` notation once and academic type material stops
  being hieroglyphics.
- [Intro to type checking (Mukul Rathi)](https://mukulrathi.com/create-your-own-programming-language/intro-to-type-checking/) —
  the same expression rules in a hands-on build, with the judgments shown
  next to the code.

**Done when:** asserts show `1 + 2` checks as num, `1 + true` is rejected
with a good message, `(1 < 2) == true` checks as bool.

---

## Task 7.3 — Check statements, variables, and control flow (~1.5–2h)

**Build:** a `TypeEnv` (name→Type, chained like Phase 4's runtime
environments — same structure, compile-time citizen), checking: `let`
initializers match annotations, assignments match declared types,
`if`/`while` conditions must be bool (goodbye `if (5)` — your language just
became stricter than C), and scoping rules mirror the runtime exactly.

**The concept:** variables force the checker to carry context. `x + 1`
can't be checked in a vacuum — the checker must know what type `x` was
declared as — so it maintains a `TypeEnv`: a name→**Type** table, chained
parent-to-child *exactly* like Phase 4's runtime environments. Same
structure, same shadowing rules, same lookup walk — but where the runtime
env maps names to values-that-are, the TypeEnv maps names to
kinds-that-are-promised. The checker is, in a precise sense, running your
program's scoping rules at compile time while ignoring all the values.
Two parallel walks, one set of scope rules — and any disagreement between
them (a name the checker resolves differently than the interpreter would)
is a **soundness bug**: the checker approves a program the runtime then
chokes on. Keeping such parallel structures honest is the key structural
insight of static analysis.

"Conditions must be bool" is your first opinionated departure from C
(where `if (5)` is legal and `while (1)` is idiomatic). Swift made the
same break with C, for the same reason: `if (x = 0)` -style bugs
disappear when conditions can't be numbers. Write one sentence in this
doc defending your rule — designers document their departures.

**Why compilers need it:** with statements checked, the guarantee turns
global: a program that passes the checker cannot hit a type error at
runtime, on *any* path. Prove it to yourself the satisfying way — run the
checker before `eval`, and the interpreter's Phase 4 runtime type errors
become unreachable. Leave them in as asserts: belt and suspenders, and
evidence you understand *why* they can't fire anymore.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) —
  the let-binding and if rules.
- [Cornell CS 3110 — Environment Model](https://cs3110.github.io/textbook/chapters/interp/environment.html) —
  the runtime environment chapter; read the two side by side and see the
  same chain doing two jobs.
- [Wikipedia — Type rule](https://en.wikipedia.org/wiki/Type_rule) —
  note how the context Γ in the judgments *is* your TypeEnv.

**Done when:** well-typed programs pass; each rule has a rejection test;
running the checker before `eval` means the interpreter's runtime type
errors literally cannot fire anymore (leave them in as asserts — belt and
suspenders, and a proof you understand *why* they're now unreachable).

---

## Task 7.4 — Function types + wiring it into the pipeline (~1.5–2h)

**Build:** function signatures (`fn f(a: num, b: num): num`), checking
calls (argument types and count against parameters, result type = declared
return) and bodies (every `return` matches the declared type). Then wire
the checker in: parse → **check** → eval/compile, with `--no-check` as an
escape hatch flag.

**The concept:** until now a `Type` was one of two enum values. A
function's type is different in kind: `fn(num, num): num` is a type
*built out of other types* — your first **compound type**, and the moment
the `Type` enum must grow into a struct (a tag plus, for functions, a
parameter-type list and a return type). Follow the thread one step and
you see why real type systems are recursive data structures: what's the
type of a function *returning* a function? A type containing a type
containing types — the same tagged-tree design as your AST, because it
has the same nesting problem. Checking a call is then rule-following with
the compound: the callee must have a function type, each argument's
checked type must match the corresponding parameter type, and the whole
call expression's type is the declared return. Checking a body is the
mirror: every `return e` must have `e` check against the declared return
type.

Wiring the pipeline — parse → check → run, refuse to run what doesn't
check — completes the architecture: your language now has the same stage
list, in miniature, as swiftc (parse, semantic analysis, then codegen).
The `--no-check` flag is a design statement worth pondering: it's the
dynamic-language escape hatch, and whether to even offer it is a debate
you can now argue both sides of.

**Why compilers need it:** functions are where type systems earn their
keep — signatures are *checked documentation*, and a call site that
compiles is a call site whose shape is proven right. This is also the
task where your three type representations (syntax, checker, tags) all
meet: annotation syntax parsed in 7.1, compound checker types here,
runtime behavior unchanged underneath.

**Read first:**
- [CS 3110 type checking chapter](https://cs3110.github.io/textbook/chapters/interp/typecheck.html) —
  function/anonymous-function rules if present in your edition; otherwise
  derive the rule yourself from the call rule — at this point, you can.
- [TypeScript Handbook — Everyday Types](https://www.typescriptlang.org/docs/handbook/2/everyday-types.html) —
  the Functions section: parameter and return annotations in a language
  you can poke at in the browser.
- [Intro to type checking (Mukul Rathi)](https://mukulrathi.com/create-your-own-programming-language/intro-to-type-checking/) —
  his function-typing treatment, for a second worked example.

**Done when:** typed fib checks and runs; a call with a bool argument to a
num parameter is rejected at compile time; the whole examples/ folder still
runs green through the full pipeline.

---

## After this

You've built, by hand, in C: a lexer, a parser, an AST, a tree-walking
interpreter, a bytecode compiler, a virtual machine, and a type checker.
That's the front-to-back anatomy of every language you'll ever use.

Now do the thing the README promised: **make it yours.** Rename it. Change
the syntax you never liked. Add strings, arrays, closures, a GC (Crafting
Interpreters ch. 19–26 cover all four). And put it on your résumé in plain
words: *designed and implemented a statically-typed programming language in
C — recursive-descent parser, bytecode compiler, stack-based VM* — then
enjoy being able to answer every follow-up question they ask.
