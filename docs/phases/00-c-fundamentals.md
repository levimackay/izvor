# Phase 0 — C fundamentals

**Goal:** learn exactly the C you need to build a language: variables and
types, functions, pointers, strings, structs, heap memory, multi-file
programs, and the tools (clang, make, lldb). No more, no less.

**Why C at all?** Because a language implementation is fundamentally about
memory: laying out tokens, tree nodes, and bytecode in it, and walking it
fast. Most languages you may have seen (Python, JavaScript, Java) manage
memory *for* you — which is comfortable, but it means you never see the
machine underneath. C makes memory visible instead of hiding it, so every
later phase will teach you two things at once — the compiler concept *and*
what it costs. This is also the language Crafting Interpreters Part III
uses, and the language most real runtimes (CPython, Lua, SQLite) are
written in.

**Your two main textbooks for this phase** (both free, both verified live):

- [Beej's Guide to C Programming](https://beej.us/guide/bgc/html/split/) —
  friendly, thorough, actively maintained. This is the spine of Phase 0;
  each task below links its exact chapter.
- [Modern C by Jens Gustedt](https://gustedt.gitlabpages.inria.fr/modern-c/) —
  a real, rigorous book, free from the author's page. Denser than Beej; use
  it when you want the *precise* rule rather than the intuition.
- Supplement: [learn-c.org](https://www.learn-c.org/) — free interactive
  exercises in the browser, good for 10-minute reps on a topic that hasn't
  clicked yet.
- Supplement: [Python Tutor's C visualizer](https://pythontutor.com/c.html) —
  paste in small C programs and *watch* memory, variables, and pointers
  change line by line. When a pointer bug makes no sense, this is the tool.
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

**The concept:** a computer's processor doesn't understand C. It understands
*machine code* — raw numbers that encode instructions like "add these two
registers" or "jump to this address". A **compiler** is a program that
translates the text you write into those numbers. That translation happens
in stages, and each stage has a name and an artifact you can hold in your
hands:

1. **Preprocess** — lines starting with `#` (like `#include <stdio.h>`) are
   handled first, by literal text substitution. `#include` really does just
   paste another file into yours.
2. **Compile** — the C text becomes assembly: human-readable names for
   machine instructions. That's the `.s` file you just looked at.
3. **Assemble** — the assembly becomes actual machine code, stored in an
   **object file** (`.o`). An object file is a compiled *fragment* — it may
   call functions (like `printf`) whose code isn't in it yet.
4. **Link** — the **linker** takes one or more object files plus the
   standard library, matches up every "I call function X" with "here is
   function X", and produces the final executable.

**Why it matters here:** you are about to spend this whole project
*building* a translator of your own, so you should watch a professional one
run first. And the object-file/link split is not trivia: it's what makes
multi-file programs possible (task 0.9 depends on it), it's where
"undefined symbol" errors come from, and it's the reason build systems like
make exist. Nothing in this pipeline is magic — it's programs transforming
text into other text, which is exactly what your lexer and parser will be.

**Read first:**
- [Crafting Interpreters ch. 2, "A Map of the Territory"](https://craftinginterpreters.com/a-map-of-the-territory.html) —
  the best 30-minute overview of what compilers and interpreters are, and a
  preview of everything you'll build in Phases 1–6.
- [Beej's Guide to C, "Hello, World!"](https://beej.us/guide/bgc/html/split/hello-world.html) —
  what the compiler invocation is doing, from the C side.
- [Compiler Explorer](https://godbolt.org/) (interactive) — paste your
  hello.c in and watch the assembly change *live* as you edit. Ten minutes
  here makes "compilation" concrete forever.

**Done when:** you can explain, out loud, what `hello.o` is and why the link
step exists. No test file for this task.

---

## Task 0.2 — Variables, types, and arithmetic (~1–1.5h)

**Build:** implement the functions in `phase0/02_variables.c` — small
arithmetic functions using `int`, `double`, division, and modulo.

**The concept:** in C, a variable is a *named box of bytes* of a fixed
size. The **type** tells the compiler two things: how many bytes the box
is, and how to *interpret* them. An `int` is (on your Mac) 4 bytes read as
a whole number; a `double` is 8 bytes read as a floating-point number.
That's all a type is here — there are no objects, no methods attached, no
runtime tags. The same 4 bytes could be an `int` or four `char`s; the type
is the compiler's bookkeeping, not something stored in memory.

Two arithmetic rules will bite you if you've never met them:
**integer division truncates** — `7 / 2` is `3`, not `3.5`, because when
both operands are integers C throws the remainder away — and the `%`
operator gives you that remainder (`7 % 2` is `1`). Mixing types has rules
too: `7 / 2.0` *is* `3.5`, because one `double` operand promotes the whole
expression.

**Why it matters here:** your lexer will turn the *characters* `'7'` into
the *number* 7 using exactly this arithmetic (`value * 10 + digit`), your
interpreter's number semantics will sit directly on C's, and in Phase 3
you'll make a real language-design decision about what `1 / 0` should do —
which requires knowing precisely what C does. Getting bitten by `int` vs
`double` division now, in a 5-line function, is cheap. Getting bitten by it
inside an interpreter is a lost evening.

**Read first:**
- [Beej's Guide to C — Variables and Statements](https://beej.us/guide/bgc/html/split/variables-and-statements.html) —
  the tutorial version: types, operators, arithmetic.
- [learn-c.org — Variables and Types](https://www.learn-c.org/en/Variables_and_Types) (interactive) —
  type real code in the browser and run it.
- [Modern C, Level 0–1](https://gustedt.gitlabpages.inria.fr/modern-c/) —
  the precise rules on types and arithmetic, when you want the exact truth
  rather than the intuition.

**Done when:** `make p0-02` prints `task 0.2: all tests passed`.

---

## Task 0.3 — Control flow and functions (~1–1.5h)

**Build:** `phase0/03_control.c` — `max3`, iterative `factorial`, `gcd`,
`count_digits`. Loops and ifs only; no recursion yet.

**The concept:** a program normally runs top to bottom, one statement at a
time. **Control flow** is the set of constructs that change that:
`if`/`else` chooses between paths, `while` and `for` repeat a path until a
condition turns false. A **function** is a named, reusable chunk of that
flow: it takes inputs (parameters), does work, and hands back one output
(the return value). When you call a function, execution *jumps* into it,
runs to a `return`, and jumps back to exactly where the call happened —
that jump-and-come-back mechanism is the seed of the "call stack" idea
that Phase 4 builds a whole feature on.

One C-specific habit to build now: every condition in C is just an integer
— `0` means false, anything else means true. There is no separate boolean
ceremony (C99's `bool` is a thin coat of paint over this).

**Why it matters here:** these are the load-bearing constructs of every
phase after this: the lexer is one big loop with a switch inside, the
parser is a family of functions calling each other, the VM is a `for(;;)`
around a `switch`. And in Phases 4 and 6 you will *implement* `if` and
`while` for your own language — twice, two different ways — which is much
easier when using them is reflex. `count_digits` in particular is a warm-up
for "how many characters is this number", a question your lexer asks for
real.

**Read first:**
- [Beej's Guide to C — Flow Control](https://beej.us/guide/bgc/html/split/variables-and-statements.html) —
  the second half of the Variables and Statements chapter: `if`, `while`, `for`.
- [Beej's Guide to C — Functions](https://beej.us/guide/bgc/html/split/functions.html) —
  parameters, return values, prototypes.
- [learn-c.org — For loops](https://www.learn-c.org/en/For_loops),
  [While loops](https://www.learn-c.org/en/While_loops), and
  [Functions](https://www.learn-c.org/en/Functions) (interactive reps).

**Done when:** `make p0-03` passes.

---

## Task 0.4 — Arrays and strings (~1.5–2h)

**Build:** `phase0/04_arrays.c` — sum/max over an `int` array passed as
pointer + length, then `my_strlen` and `count_char` over C strings.

**The concept:** an **array** is a row of boxes of the same type, sitting
side by side in memory with no gaps. `int a[5]` is 20 contiguous bytes;
`a[3]` means "start of the row, skip 3 int-sized steps". Crucially, a C
array does *not* know its own length — there's no `.length` attached. The
length lives in your head, or in a separate variable you pass alongside.
Get an index wrong and C will happily read or write someone else's memory
(that's what the sanitizer is here to catch).

A **C string** is just an array of `char` with a convention on top: the
text's bytes, followed by one byte of value zero — written `'\0'`, the
**null terminator** — marking the end. That terminator is the *only* way
functions know where a string stops. `"hi"` occupies three bytes: `h`,
`i`, `\0`. When you write `my_strlen`, you're implementing the convention
itself: walk forward until you meet the zero byte, counting as you go.

**Why it matters here:** *your entire language's source code will be one C
string.* The lexer is nothing but careful walking of a `const char*`,
character by character, with the null terminator telling it when the
program ends — the lexer's EOF token is literally "I found `'\0'`".
Writing `my_strlen` yourself makes the terminator real in a way no
paragraph can. Passing arrays as pointer+length is also your first taste
of the big C truth: arrays *decay to pointers* when passed to functions,
and the length has to travel separately — which is exactly how your tokens
will refer to their text in Phase 1.

**Read first:**
- [Beej's Guide to C — Arrays](https://beej.us/guide/bgc/html/split/arrays.html) —
  declaration, indexing, passing to functions.
- [Beej's Guide to C — Strings](https://beej.us/guide/bgc/html/split/strings.html) —
  the null terminator, string functions, common traps.
- [learn-c.org — Arrays](https://www.learn-c.org/en/Arrays) and
  [Strings](https://www.learn-c.org/en/Strings) (interactive).
- [Python Tutor C visualizer](https://pythontutor.com/c.html) (interactive) —
  paste your `my_strlen` in and step through it; you can *see* the walk.

**Done when:** `make p0-04` passes.

---

## Task 0.5 — Pointers I: addresses and dereferencing (~1.5–2h)

**Build:** `phase0/05_pointers.c` — `swap`, `store_sum` (write a result
through an out-parameter), `larger` (return a pointer to one of two ints).

**The concept:** every variable lives somewhere — a numbered byte in your
process's memory, like a house on a very long street. A **pointer** is a
variable whose *value is one of those addresses*. That's the entire idea.
`&x` asks "what address does `x` live at?" (read `&` as *address-of*).
If `p` holds that address, `*p` asks "what value lives at the address in
`p`?" (read `*` as *the thing pointed to*). So `*p = 5;` doesn't change
`p` — it reaches through `p` and changes `x`.

Why does C even have this? Because function arguments are *copies*. If you
pass `x` to `swap(a, b)`, the function gets photocopies and the originals
never change. Pass *addresses* instead — `swap(&x, &y)` — and the function
can reach back and modify the caller's actual variables. That's the
"out-parameter" pattern in `store_sum`, and it's how C returns more than
one thing. The mental model to build: **a pointer is a variable whose value
is an address; `&` asks where, `*` asks what's there.** Draw boxes and
arrows on paper for every exercise until you don't need to.

**Why it matters here:** pointers are the concept people fear in C, and
they are also 80% of a language implementation: the lexer holds a pointer
into the source text, every AST node points at its child nodes, every
scope points at its parent scope, and the VM's stack pointer is literally
a pointer. There is no version of this project without them. Once `swap`
makes sense, tree nodes will too — a tree is just structs pointing at
structs.

**Read first:**
- [Beej's Guide to C — Pointers](https://beej.us/guide/bgc/html/split/pointers.html) —
  the friendliest serious explanation in print (and
  [Pointers II](https://beej.us/guide/bgc/html/split/pointers2.html) later,
  when curious about pointer arithmetic).
- [learn-c.org — Pointers](https://www.learn-c.org/en/Pointers) (interactive —
  do this one even if you skip the others).
- [Python Tutor C visualizer](https://pythontutor.com/c.html) (interactive) —
  run `swap` in it and watch the arrows. This is the fastest way to make
  pointers click.
- [mycodeschool — Introduction to pointers](https://www.youtube.com/watch?v=h-HBipu_1P0) (video) —
  a patient whiteboard walkthrough of the memory model, if you want it
  narrated.

**Done when:** `make p0-05` passes.

---

## Task 0.6 — Pointers II: walking strings, and your first micro-lexer (~1.5–2h)

**Build:** `phase0/06_strings.c` — `my_strcpy`, `my_strcmp`, and
`parse_uint`: read leading decimal digits from a string and return their
numeric value (`parse_uint("123abc") == 123`).

**The concept:** two ideas meet in this task. First, characters *are*
numbers: `'0'` is just the byte 48, `'9'` is 57, and they're consecutive —
so `c >= '0' && c <= '9'` tests "is this a digit", and `c - '0'` converts
a digit character to its numeric value. Second, building a number from its
digits is repeated multiply-and-add: seeing `"123"` left to right, you
compute `0*10+1 = 1`, then `1*10+2 = 12`, then `12*10+3 = 123`. Each new
digit shifts what you have one decimal place left and drops itself in.

`my_strcmp` teaches a third idea, **lexicographic comparison**: walk two
strings in lockstep, and the first position where they differ decides
which is "less" (by byte value). Equal all the way to both terminators
means equal strings.

**Why it matters here:** `parse_uint` **is a piece of your Phase 1 lexer,
verbatim** — scanning characters, testing for digits, accumulating
`value = value*10 + (c-'0')`. And `my_strcmp` is how you'll recognize
keywords in Phase 3–4: when the lexer reads the word `print`, something
must compare it against the known keywords, and that something is exactly
this loop. After this task, Phase 1 is assembly of parts you've already
built and understand.

**Read first:**
- [Beej's Guide to C — Strings](https://beej.us/guide/bgc/html/split/strings.html) —
  especially the string-handling functions you're reimplementing.
- [man page: isalpha/isdigit/isspace](https://man7.org/linux/man-pages/man3/isalpha.3.html) —
  the standard library's character-classification functions (reference;
  you're allowed to use them, but know they're just the comparisons above).
- [Modern C, Level 1](https://gustedt.gitlabpages.inria.fr/modern-c/) —
  the pointers/arrays treatment, if Beej's version hasn't clicked.
- [freeCodeCamp — Pointers in C (full course)](https://www.youtube.com/watch?v=zuegQmMdy8M) (video) —
  the pointers-and-strings sections, if you want more guided reps before
  the test.

**Done when:** `make p0-06` passes.

---

## Task 0.7 — Structs (~1–1.5h)

**Build:** `phase0/07_structs.c` — a `Point` struct with add/equals, and a
`Rect` made of two Points with `rect_area` / `rect_contains`.

**The concept:** a **struct** is several variables that travel together
under one name — a `Point` is an `x` and a `y` glued into a single value.
The compiler lays the fields out in order in memory, and `.` reaches into
them (`p.x`). Structs nest: a `Rect` holding two `Point`s is a struct of
structs, and `r.min.x` just follows the trail. Unlike Python or Java
objects, a struct is *only* data — no methods, no hidden header, no
identity beyond its bytes.

The one behavior that surprises everyone: structs are **passed and
assigned by value** — copied wholesale. Pass a `Point` to a function and
the function gets its own copy; mutate it and the caller sees nothing.
If you *want* shared access, you pass a pointer to the struct (`Point *`),
and dereference with `->` (`p->x` is shorthand for `(*p).x`). Choosing
copy vs pointer — cheap little value vs shared mutable thing — is a daily
question in real C codebases, and you'll answer it for every structure in
this project.

**Why it matters here:** every interesting value in your language will be
a struct: a `Token` (type + text pointer + length + value), an AST node
(operator + left child + right child), the VM itself (stack + instruction
pointer). Look at `src/lexer.h` right now — the `Token` and `Lexer` types
waiting there are structs you'll fill in a week. This task is you learning
to read that header.

**Read first:**
- [Beej's Guide to C — Structs](https://beej.us/guide/bgc/html/split/structs.html) —
  declaration, `.` vs `->`, passing to functions.
- [learn-c.org — Structures](https://www.learn-c.org/en/Structures) (interactive).
- [Crafting Interpreters ch. 4, §4.2 "Lexemes and Tokens"](https://craftinginterpreters.com/scanning.html) —
  see a real Token type in the wild, and recognize it as "just a struct".

**Done when:** `make p0-07` passes.

---

## Task 0.8 — malloc/free and a growable array (~2h, the important one)

**Build:** `phase0/08_dynarray.c` — an `IntArray` struct with
`array_init`, `array_push` (doubling the capacity when full), `array_get`,
and `array_free`.

**The concept:** so far, every variable you've made lives on the **stack**
— memory that appears when a function starts and *vanishes when it
returns*. That's automatic and fast, but it has a fatal limitation: the
size of everything must be known at compile time, and nothing outlives its
function. The **heap** is the other memory: you ask for a block of any
size at runtime (`malloc(n)` returns a pointer to `n` fresh bytes), it
stays yours across function calls, and you give it back explicitly
(`free(p)`). With that power come the two iron rules of manual memory:
every `malloc` is matched by exactly one `free` (miss it → a **leak**;
do it twice → corruption), and memory you've freed is dead to you —
touching it is never OK.

The structure you're building — a pointer to a heap buffer, a `count` of
used slots, a `capacity` of allocated slots, and a `push` that doubles
the capacity with `realloc` when full — is the **dynamic array**, the
single most used data structure in systems programming. Doubling (rather
than growing by 1) is what makes pushing cheap on average: most pushes
just write a slot, and the occasional expensive copy is amortized across
all the cheap ones.

**Why it matters here:** this is the single most important task in Phase 0,
because a language implementation knows *nothing* in advance: how many
tokens will a program have? How big is its AST? How much bytecode will it
compile to? The answer every time is this exact structure. Crafting
Interpreters ch. 14 builds this very thing as the foundation of the
bytecode VM — you're building it a few weeks early, and in Phase 5 you'll
re-type it for `uint8_t` and call it a Chunk. The macOS `leaks` tool will
referee the malloc/free rule from here on.

**Read first:**
- [Beej's Guide to C — Manual Memory Allocation](https://beej.us/guide/bgc/html/split/manual-memory-allocation.html) —
  malloc, free, realloc, and the discipline around them.
- [Crafting Interpreters ch. 14, §14.3.1 "A dynamic array of instructions"](https://craftinginterpreters.com/chunks-of-bytecode.html) —
  see your exercise in the wild, as real compiler infrastructure.
- [learn-c.org — Dynamic allocation](https://www.learn-c.org/en/Dynamic_allocation) (interactive).
- [CS50x Week 4 — Memory](https://cs50.harvard.edu/x/weeks/4/) (video lecture) —
  stack vs heap, malloc, and what a segfault actually is, taught with
  Harvard production values. Watch the malloc/valgrind segments.

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

**The concept:** C compiles each `.c` file *completely independently* —
when clang compiles `09_link_test.c`, it has no idea what's in
`intarray.c`. So how can the test call `array_push`? Because the **header**
made a promise: `intarray.h` *declares* "a function with this name and
signature exists somewhere", which is enough for the compiler to generate
a call to it. The `.c` file *defines* how it actually works. The **linker**
is the stage that keeps the promise: it takes all the object files and
stitches every "I call `array_push`" to the one place `array_push` is
defined. Break the promise — declare something and define it nowhere —
and you get `undefined symbol`, the error you'll deliberately cause in
this task. Define it twice and you get `duplicate symbol`. Now both errors
are *mechanical*, not mysterious.

**make** is just recorded recipes on top of this: "*this* file is built
from *those* files with *this* command — and if the sources haven't
changed since last time, skip it." That skipping (dependency tracking) is
the entire reason build systems exist.

**Why it matters here:** your language will quickly outgrow one file —
`lexer.c`, `parser.c`, `interp.c`, `vm.c` — and headers are how they'll
share interfaces without sharing internals. From Phase 2 on you'll write
these headers yourself. Better to meet the linker's error messages in a
two-file project than a seven-file one.

**Read first:**
- [Makefile Tutorial](https://makefiletutorial.com/) — the "getting started"
  sections; skim, don't memorize.
- [GNU Make manual](https://www.gnu.org/software/make/manual/make.html) —
  the official reference; read §2 "An Introduction to Makefiles" only.
- [Beej's Guide to C — Multifile Projects](https://beej.us/guide/bgc/html/split/multifile-projects.html) —
  headers, object files, and how they fit.

**Done when:** `make p0-09` passes, and you can answer: what would break if
`intarray.h` had no include guard and got included twice?

---

## Task 0.10 — Debugging: lldb + sanitizers (~1.5–2h)

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

**The concept:** a **debugger** runs your program under a microscope: it
can pause execution at any line (a *breakpoint*), advance one line at a
time (*stepping*), show you any variable's current value, and — most
underrated — show you the **backtrace**: the chain of function calls that
led to right here. Where printf shows you one value you *guessed* would
matter, the debugger lets you look at anything once you're paused, without
recompiling. The skill to build is a loop: form a hypothesis about where
reality diverges from your intent, set a breakpoint just before it, and
check.

**Sanitizers** are the complementary tool: extra checking the compiler
weaves *into* the program. This repo builds everything with
UndefinedBehaviorSanitizer, so the categories of C bug that normally
corrupt memory silently — out-of-bounds writes, overflow, bad shifts —
instead crash immediately with a report naming the exact line. Learning to
*read* those reports (what kind of bug, which line, which values) turns
C's scariest failure mode into a routine fix.

**Why it matters here:** from Phase 2 on, you will have bugs *inside a
program that is itself running programs* — printf-debugging a recursive
parser is misery, but a backtrace shows you the entire recursion at a
glance. Ten minutes of stepping through your own factorial in lldb pays
off for months.

**Read first:**
- [LLDB official tutorial](https://lldb.llvm.org/use/tutorial.html) — the
  first half, through stepping and printing.
- [GDB→LLDB command map](https://lldb.llvm.org/use/map.html) — keep this
  open; every online answer says "in gdb do X", this translates it.
- [Clang UndefinedBehaviorSanitizer docs](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) —
  what the sanitizer this repo uses actually checks; skim the checks list.
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
