# Phase 0 — C fundamentals

**Goal:** learn exactly the C you need to build a language: variables and
types, functions, pointers, strings, structs, heap memory, multi-file
programs, and the tools (clang, make, lldb). No more, no less.

**Why C at all?** Because a language implementation is fundamentally about
memory: laying out tokens, tree nodes, and bytecode in it, and walking it
fast. C makes memory visible instead of hiding it, so every later phase will
teach you two things at once — the compiler concept *and* what it costs.
This is also the language Crafting Interpreters Part III uses, and the
language most real runtimes (CPython, Lua, SQLite) are written in.

**Your two main textbooks for this phase** (both free, both verified live):

- [Beej's Guide to C Programming](https://beej.us/guide/bgc/html/split/) —
  friendly, thorough, actively maintained. This is the spine of Phase 0;
  tasks below cite it by chapter topic.
- [Modern C by Jens Gustedt](https://gustedt.gitlabpages.inria.fr/modern-c/) —
  a real, rigorous book, free from the author's page. Denser than Beej; use
  it when you want the *precise* rule rather than the intuition.
- Supplement: [learn-c.org](https://www.learn-c.org/) — free interactive
  exercises in the browser, good for 10-minute reps on a topic that hasn't
  clicked yet.
- (You'll also see *Learn C the Hard Way* recommended online — it's no
  longer freely available and has aged; the resources above replace it.)

**How the exercises work:** each task has a stub file in `phase0/` with
function signatures and comments, and a matching `_test.c` file full of
`assert()` calls. `make p0-NN` compiles both and runs the test. The test
passing is the definition of done. Tests build with
UndefinedBehaviorSanitizer on, so mistakes like out-of-bounds array writes
crash with a report pointing at the guilty line — read those reports,
they're the best C teacher there is. (The Makefile header explains why
AddressSanitizer, its usual partner, is off on this machine right now.)

---

## Task 0.1 — Toolchain + how compilation actually works (~1–1.5h)

**Build:** nothing permanent. Follow `prereqs.md`, then compile a hello-world
by hand, in stages, from the terminal:

```sh
clang hello.c -o hello            # one shot: source → executable
clang -c hello.c -o hello.o       # stage 1: compile to an object file
clang hello.o -o hello            # stage 2: link object file(s) → executable
./hello
```

Also try `clang -S hello.c -o hello.s` and open `hello.s` — that's the
assembly your C became.

**Why:** you are about to spend this whole project *building* a compiler, so
you should see one run: preprocess → compile → assemble → link. The
object-file/link split is what makes multi-file programs (task 0.9) and every
real build system make sense. Nothing here is magic — it's programs
transforming text into other text.

**Read first:**
- [Crafting Interpreters ch. 2, "A Map of the Territory"](https://craftinginterpreters.com/a-map-of-the-territory.html) —
  the best 30-minute overview of what compilers and interpreters are, and a
  preview of everything you'll build in Phases 1–6.
- [Beej's Guide to C, ch. 1–2 (foreword, hello world)](https://beej.us/guide/bgc/html/split/) —
  what the compiler invocation is doing.
- Optional fun: paste your hello.c into [Compiler Explorer](https://godbolt.org/)
  and watch the assembly change as you edit.

**Done when:** you can explain, out loud, what `hello.o` is and why the link
step exists. No test file for this task.

---

## Task 0.2 — Variables, types, and arithmetic (~1–1.5h)

**Build:** implement the functions in `phase0/02_variables.c` — small
arithmetic functions using `int`, `double`, division, and modulo.

**Why:** C types are *sizes and interpretations of bytes*, not objects.
Integer division truncating (`7 / 2 == 3`) and `%` giving remainders are not
trivia — your lexer will turn the *characters* `'7'` into the *number* 7
with exactly this arithmetic, and your interpreter's number semantics will
be built on C's. Getting bitten by `int` vs `double` division now, in a
5-line function, is cheap.

**Read first:**
- [Beej's Guide to C — Variables and Statements, Functions chapters](https://beej.us/guide/bgc/html/split/) (ch. 2–5).
- [learn-c.org — Variables and Types](https://www.learn-c.org/) (interactive).

**Done when:** `make p0-02` prints `task 0.2: all tests passed`.

---

## Task 0.3 — Control flow and functions (~1–1.5h)

**Build:** `phase0/03_control.c` — `max3`, iterative `factorial`, `gcd`,
`count_digits`. Loops and ifs only; no recursion yet.

**Why:** these are the load-bearing constructs of every phase after this:
the lexer is one big loop with a switch inside, the parser is a family of
functions calling each other, the VM is a `for(;;)` around a `switch`.
`count_digits` in particular is a warm-up for "how many characters is this
number" — a real lexer question.

**Read first:**
- [Beej's Guide to C — Flow Control and Functions](https://beej.us/guide/bgc/html/split/) (ch. 4–5).
- [learn-c.org — For loops, While loops, Functions](https://www.learn-c.org/).

**Done when:** `make p0-03` passes.

---

## Task 0.4 — Arrays and strings (~1.5–2h)

**Build:** `phase0/04_arrays.c` — sum/max over an `int` array passed as
pointer + length, then `my_strlen` and `count_char` over C strings.

**Why:** a C string is just bytes ending in `'\0'` — and *your entire
language's source code will be one C string*. The lexer is nothing but
careful walking of a `const char*`. Writing `my_strlen` yourself makes the
null terminator real in a way no paragraph can. Passing arrays as
pointer+length is also your first taste of the big C truth: arrays decay to
pointers, and the length travels separately.

**Read first:**
- [Beej's Guide to C — Arrays and Strings chapters](https://beej.us/guide/bgc/html/split/) (ch. 6, 9).
- [learn-c.org — Arrays, Strings](https://www.learn-c.org/).

**Done when:** `make p0-04` passes.

---

## Task 0.5 — Pointers I: addresses and dereferencing (~1.5–2h)

**Build:** `phase0/05_pointers.c` — `swap`, `store_sum` (write a result
through an out-parameter), `larger` (return a pointer to one of two ints).

**Why:** pointers are the concept people fear in C, and they are also 80% of
a language implementation: the lexer holds a pointer into the source, AST
nodes point at child nodes, environments point at parent scopes, the VM's
stack pointer is literally a pointer. The mental model to build here:
**a pointer is a variable whose value is an address**, `&` asks "where does
this live", `*` asks "what lives there". Once `swap` makes sense, tree nodes
will too.

**Read first:**
- [Beej's Guide to C — Pointers chapters](https://beej.us/guide/bgc/html/split/) (ch. 8, and later ch. 23 "Pointers II" when curious).
- [learn-c.org — Pointers](https://www.learn-c.org/) (interactive; do this one even if you skip the others).

**Done when:** `make p0-05` passes.

---

## Task 0.6 — Pointers II: walking strings, and your first micro-lexer (~1.5–2h)

**Build:** `phase0/06_strings.c` — `my_strcpy`, `my_strcmp`, and
`parse_uint`: read leading decimal digits from a string and return their
numeric value (`parse_uint("123abc") == 123`).

**Why:** `parse_uint` **is a piece of your Phase 1 lexer**, verbatim —
scanning characters, testing `'0' <= c && c <= '9'`, accumulating
`value = value*10 + (c-'0')`. `my_strcmp` teaches lexicographic comparison,
which you'll use to recognize keywords in Phase 4. After this task, Phase 1
is assembly of parts you've already built.

**Read first:**
- [Beej's Guide to C — Strings chapter](https://beej.us/guide/bgc/html/split/) (ch. 9, esp. string functions).
- [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/) — Level 1 on
  pointers if Beej's version hasn't clicked.

**Done when:** `make p0-06` passes.

---

## Task 0.7 — Structs (~1–1.5h)

**Build:** `phase0/07_structs.c` — a `Point` struct with add/equals, and a
`Rect` made of two Points with `rect_area` / `rect_contains`.

**Why:** a struct is "several variables that travel together" — and every
interesting value in your language will be one: `Token` (type + text +
value), AST nodes (operator + left child + right child), the VM (stack +
instruction pointer). Note how structs are passed *by value* (copied) unless
you pass a pointer — deciding which, and why, is a daily question in real C
codebases.

**Read first:**
- [Beej's Guide to C — Structs chapter](https://beej.us/guide/bgc/html/split/) (ch. 7).
- [learn-c.org — Structures](https://www.learn-c.org/).

**Done when:** `make p0-07` passes.

---

## Task 0.8 — malloc/free and a growable array (~2h, the important one)

**Build:** `phase0/08_dynarray.c` — an `IntArray` struct with
`array_init`, `array_push` (doubling the capacity when full), `array_get`,
and `array_free`.

**Why:** this is the single most important task in Phase 0. The stack can't
hold data whose size you don't know in advance — and a language
implementation knows *nothing* in advance: how many tokens? how big an AST?
how much bytecode? The answer every time is this exact structure — a
malloc'd buffer, a count, a capacity, and a doubling `realloc`. Crafting
Interpreters ch. 14 builds this very thing as the foundation of the bytecode
VM; you're building it a few weeks early. You also meet the two iron rules
of manual memory: every `malloc` has exactly one `free`, and touching freed
memory is never OK. The macOS `leaks` tool will referee the first rule.

**Read first:**
- [Beej's Guide to C — Manual Memory Allocation chapter](https://beej.us/guide/bgc/html/split/) (ch. 12: malloc, free, realloc).
- [Crafting Interpreters ch. 14, "Chunks of Bytecode"](https://craftinginterpreters.com/chunks-of-bytecode.html) —
  read §14.3 "A Dynamic Array of Instructions" and see your exercise in the wild.
- [learn-c.org — Dynamic memory allocation](https://www.learn-c.org/).

**Done when:** `make p0-08` passes, and
`leaks --atExit -- ./build/p0-08` reports 0 leaks (`leaks` is macOS's
built-in leak checker — no install needed).

---

## Task 0.9 — Headers, object files, linking, and your own Makefile (~1.5–2h)

**Build:** move your task 0.8 implementation into `phase0/intarray.c` with
the interface declared in `phase0/intarray.h` (scaffold provided — read the
header-guard comment). First build it *by hand* to feel the stages:

```sh
clang -c phase0/intarray.c -o build/intarray.o
clang -c phase0/09_link_test.c -o build/09_test.o
clang build/intarray.o build/09_test.o -o build/p0-09-manual && ./build/p0-09-manual
```

Then read this repo's `Makefile` top to bottom until every line makes sense,
and write your own five-line Makefile from scratch in a scratch directory.

**Why:** your language will quickly outgrow one file — lexer.c, parser.c,
interpreter.c, vm.c — and headers are how C files share interfaces: the
`.h` declares *what exists*, the `.c` defines *how it works*, and the linker
stitches the object files together. This is also where "undefined symbol"
and "duplicate symbol" errors come from; better to meet them in a two-file
project. Make is just recipes: *this file depends on those; if they're
newer, run this command.*

**Read first:**
- [Makefile Tutorial](https://makefiletutorial.com/) — the "getting started"
  sections; skim, don't memorize.
- [GNU Make manual](https://www.gnu.org/software/make/manual/make.html) —
  reference only; read §2 "An Introduction to Makefiles".
- [Beej's Guide to C — multi-file projects chapter](https://beej.us/guide/bgc/html/split/) (ch. 11).

**Done when:** `make p0-09` passes, and you can answer: what would break if
`intarray.h` had no include guard and got included twice?

---

## Task 0.10 — Debugging: lldb + AddressSanitizer (~1.5–2h)

**Build:** `phase0/10_buggy.c` contains a short program with **two planted
bugs**: one memory bug (the sanitizer will catch it and name the line) and
one logic bug (the assert fails; find it with lldb, not printf). Fix both.

Core lldb moves to practice:

```sh
lldb ./build/p0-10
(lldb) b 10_buggy.c:20     # breakpoint at file:line
(lldb) run
(lldb) n                   # step over        s  → step into
(lldb) p somevar           # print a variable
(lldb) bt                  # backtrace — how did I get here?
(lldb) q
```

**Why:** from Phase 1 on, you will have bugs *inside a program that is
itself running programs* — printf-debugging a parser is misery. Ten minutes
of stepping through your own factorial in lldb pays off for months. And
learning to *read* a sanitizer report (what kind of bug, which line) turns
C's scariest failure mode into a routine fix.

**Read first:**
- [LLDB official tutorial](https://lldb.llvm.org/use/tutorial.html) — the
  first half, through stepping and printing.
- [GDB→LLDB command map](https://lldb.llvm.org/use/map.html) — keep this
  open; every online answer says "in gdb do X", this translates it.
- [Clang AddressSanitizer docs](https://clang.llvm.org/docs/AddressSanitizer.html) —
  skim the example report so you recognize the format when you meet ASan on
  a machine where it runs (see the Makefile header for why not this one).

**Done when:** `make p0-10` passes, with both bugs found via the tools (not
by staring), and you've set a breakpoint, stepped, and printed a variable in
lldb at least once.

---

## Phase 0 exit check

You're ready for Phase 1 when you can do these without looking anything up:
write a function taking `const char*` and walking it to `'\0'`; explain the
difference between `char c`, `char *p`, and `char p[10]`; grow a malloc'd
buffer with realloc; and split a program into `.h`/`.c` files that build via
make. That's the entire C toolkit Phases 1–6 need — everything else you'll
pick up in context.

*Optional side quest:* [Build Your Own Lisp](https://buildyourownlisp.com/)
is a free book that teaches C *by* building a language — a fun parallel
track if you want more C reps, though it uses a parsing library where we'll
write our own.
