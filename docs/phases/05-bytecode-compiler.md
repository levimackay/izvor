# Phase 5 — Compile the AST to bytecode

**Goal:** a second back end. Instead of walking the tree to *evaluate*,
walk it to **emit instructions**: `1 + 2 * 3` becomes

```
CONST 1
CONST 2
CONST 3
MUL
ADD
```

— a flat byte array your Phase 6 VM will execute. This phase produces the
compiler + a disassembler; the VM comes next.

**Why:** tree-walking is slow, and the reason is physical. Your AST nodes
were malloc'd one at a time and live scattered across the heap; evaluating
`+` means chasing pointers to wherever its children landed, and modern
CPUs hate that — they're fastest streaming through *adjacent* memory, and
every pointer chase risks a cache miss costing hundreds of cycles. Real
languages (Python, Lua, Java, JS) therefore translate the tree once into
**bytecode**: dense, linear instructions for an imaginary machine, laid
out consecutively in a byte array. Same meaning, cache-friendly shape.
This phase is also the conceptual bridge from "interpreter" to
"compiler": same AST in, but now the output is *a program*, not a value —
you're translating, not executing. When you finish Phase 6 you'll
benchmark fib on both back ends and see the difference as a number you
measured.

**A heads-up about the book:** Crafting Interpreters Part III (clox)
compiles *straight from tokens* to bytecode in a single pass, skipping the
AST, using Pratt parsing. **You already have an AST — keep it.** Your
compiler is a tree walk that emits instead of evaluates (which is also how
CPython works: source → AST → bytecode). The book's chapters remain the
right reading: everything about chunks, constants, stack effects, and
disassembly transfers directly; just mentally replace "parse function" with
"AST node case".

**Spine reading:**
- [Crafting Interpreters ch. 14, "Chunks of Bytecode"](https://craftinginterpreters.com/chunks-of-bytecode.html)
- [Crafting Interpreters ch. 17, "Compiling Expressions"](https://craftinginterpreters.com/compiling-expressions.html)

---

## Task 5.1 — The Chunk: instruction encoding + dynamic byte array (~1.5h)

**Build:** `chunk.h`/`chunk.c`: an opcode enum (`OP_CONST`, `OP_ADD`,
`OP_SUB`, `OP_MUL`, `OP_DIV`, `OP_NEG`, `OP_RETURN`…), a `Chunk` = growable
`uint8_t` array (your task 0.8 IntArray, re-typed) + a constant pool
(growable `Value` array) — because a `Value` doesn't fit in a byte, `OP_CONST`
carries a one-byte *index* into the pool.

**The concept:** you are designing an **instruction set** — a vocabulary
of tiny operations, each encoded as a number. An **opcode** is one byte
(hence "bytecode": 256 possible instructions, plenty), and an instruction
is an opcode optionally followed by **operand** bytes that parameterize
it. `OP_ADD` needs no operands — it will act on the VM's stack. `OP_CONST`
needs to say *which* constant, and here's the encoding problem: your
`Value` struct is many bytes and doesn't fit in an instruction stream
designed around single bytes. The universal solution: put the values in a
side table (the **constant pool**) and make the instruction carry a
one-byte *index* into it. `CONST 3` means "look up pool entry 3". This
index-into-a-side-table trick is everywhere in binary formats — JVM
`.class` files have a constant pool, Python `.pyc` files store
`co_consts`, executables have string tables — because it keeps the
instruction stream small and fixed-width while letting the data be any
size.

The container for all this is your task 0.8 dynamic array, re-typed for
`uint8_t` — your Phase 0 exercise turns out to have been compiler
infrastructure all along.

**Why compilers need it:** every compiled representation is instructions
plus data, encoded compactly. Designing even a seven-opcode instruction
set makes the design pressures real: fixed-size opcodes decode fast;
operands cost space; side tables trade an indirection for density. These
are the same considerations, at 1% scale, that CPU architects and
JVM/CPython maintainers argue about.

**Read first:**
- [Crafting Interpreters ch. 14, §14.3 and §14.5](https://craftinginterpreters.com/chunks-of-bytecode.html) —
  chunks of instructions, and constants/constant pools; this task is that
  chapter.
- [Python's `dis` module docs](https://docs.python.org/3/library/dis.html) —
  a real language's opcode list. Run `python3 -c "import dis; dis.dis('1+2*3')"`
  and recognize LOAD_CONST as your OP_CONST.
- [Wikipedia — Bytecode](https://en.wikipedia.org/wiki/Bytecode) —
  the panoramic view: who uses bytecode and why.

**Done when:** asserts build a chunk by hand (write bytes, add constants)
and read back exactly what went in; no leaks.

---

## Task 5.2 — The disassembler (~1–1.5h)

**Build:** `debug.c`: print a chunk human-readably, one instruction per
line with offsets:

```
0000  CONST     0        ; 1
0002  CONST     1        ; 2
0004  ADD
0005  RETURN
```

**The concept:** a **disassembler** is the decoder ring for your binary
format: it walks the byte array and prints each instruction's name and
operands, with its **offset** (position in the array) down the left
margin. Writing one forces you to internalize your own encoding — the
loop must know that after an `OP_CONST` opcode comes one operand byte,
so instructions are *variable length* and you advance by "1 + operand
count", not by 1. (Those offsets in the margin will become load-bearing
in task 5.5, when jump instructions name their targets by offset.)

**Why compilers need it:** you're about to write two programs — a
compiler and a VM — that communicate through raw bytes. When they
disagree, neither can tell you why: the compiler thinks it wrote the
right bytes, the VM crashes on them. The disassembler is the neutral
witness that shows what's *actually in the array*. Build the debugging
tool *before* the thing that needs debugging; that ordering is the mark
of someone who has debugged binary formats before. (Same move as
`lexdump` — see the pattern? Every representation you invent gets a
printer.)

**Read first:**
- [Crafting Interpreters ch. 14, §14.4 "Disassembling Chunks"](https://craftinginterpreters.com/chunks-of-bytecode.html) —
  the same tool for clox.
- [Python's `dis` module docs](https://docs.python.org/3/library/dis.html) —
  CPython's disassembler; note its output columns (offset, opname,
  operand, resolved meaning in a comment) — the format you're imitating.
- [Oracle's `javap` docs](https://docs.oracle.com/en/java/javase/21/docs/specs/man/javap.html) —
  Java's equivalent tool (`javap -c` disassembles any .class file), for
  proof this tool exists in every ecosystem.

**Done when:** the hand-built chunk from 5.1 disassembles to exactly the
listing you expect (assert on the printed string, or eyeball + freeze it).

---

## Task 5.3 — Compile arithmetic expressions (~1.5–2h)

**Build:** `compiler.c`: `compile(Node*, Chunk*)` — a post-order tree walk:
compile left child, compile right child, emit the operator's opcode.
Numbers emit `OP_CONST`.

**The concept:** your target machine (Phase 6) is a **stack machine**: no
named registers, just a stack of values, where every instruction pops its
inputs off the top and pushes its result. So the compiler's job is to
order instructions such that *by the time an operator executes, its
operands are the top of the stack*. The ordering that achieves this is
**post-order**: children first, then the parent. For `(+ 1 (* 2 3))`:
emit `CONST 1` (stack: 1), emit `CONST 2`, `CONST 3` (stack: 1,2,3), emit
`MUL` (pops 3 and 2, pushes 6 — stack: 1,6), emit `ADD` (pops 6 and 1,
pushes 7). **The tree's shape becomes the instructions' order** — that's
the entire insight of stack-based code generation, and it's why the
compiler is a ~30-line recursive walk. You may recognize the output as
postfix / Reverse Polish notation: `1 2 3 * +` — RPN calculators and
stack bytecode are the same idea.

Note what the compiler *doesn't* do: no arithmetic. `compile` never adds
anything — it writes down instructions that *will* add, later, in the VM.
Keeping "translate" and "execute" separate in your head is the point of
the whole phase.

**Why compilers need it:** this is your third full traversal of the same
AST (eval, free/print, now compile), which is the deep lesson: **an AST
is one data structure with many interpreters** — the same tree, asked
different questions. Real compilers are stacks of exactly such walks
(check, optimize, emit), and Python's own compiler is doing this exact
post-order emit over its AST every time you import a module.

**Read first:**
- [Crafting Interpreters ch. 17](https://craftinginterpreters.com/compiling-expressions.html) —
  read for the *emitting* parts (§17.3–17.5); the Pratt-parsing machinery
  is solving a problem (no AST) you don't have.
- [Wikipedia — Reverse Polish notation](https://en.wikipedia.org/wiki/Reverse_Polish_notation) —
  your compiler's output format, under its historical name; the
  stack-evaluation section is your Phase 6 spoiler.
- [CPython's compiler design doc](https://github.com/python/cpython/blob/main/InternalDocs/compiler.md) —
  a production language doing AST → bytecode, same architecture as yours;
  skim the overview to see your pipeline at industrial scale.

**Done when:** asserts compile `"1 + 2 * 3"` and check the chunk's exact
byte sequence (CONST 1, CONST 2, CONST 3, MUL, ADD) — the disassembler
makes failures readable.

---

## Task 5.4 — Compile statements and variables (~1.5–2h)

**Build:** opcodes and compilation for `print`, expression statements
(`OP_POP` — why must a statement's value be popped? answer in a comment),
and global variables (`OP_DEFINE_GLOBAL`, `OP_GET_GLOBAL`, `OP_SET_GLOBAL`,
names in the constant pool).

**The concept:** statements introduce **stack discipline**: every
statement must leave the stack exactly as it found it. An expression
statement like `1 + 2;` computes 3 onto the stack — and nothing will ever
use it, so the compiler must emit `OP_POP` to discard it. Skip that pop
and each such statement leaves one orphan value behind; run a loop and
the stack grows until something corrupts — the bytecode version of a
memory leak, and the classic VM bug. The invariant to write on your wall:
*expressions have net stack effect +1; statements have net stack effect
0.* You can audit any bytecode listing against it by hand.

Variables pose the deeper question: the instruction stream is just bytes —
**how does a *name* like `x` survive compilation?** Answer: it doesn't.
The compiler puts the string `"x"` in the constant pool and emits
`OP_GET_GLOBAL <index>`; the name became a number. That's what
"compilation erases names" means — and it's why stripped binaries are
unreadable, why debuggers need separate debug-info tables to map numbers
back to names, and why your disassembler should print the pooled name in
a comment column to stay humane.

**Why compilers need it:** stack discipline and name resolution are the
two invariants that every bytecode compiler maintains on every statement
it emits. CPython's `LOAD_NAME 0` / `STORE_NAME 0` with names in
`co_names` is byte-for-byte the same design you're building.

**Read first:**
- [Crafting Interpreters ch. 21, "Global Variables"](https://craftinginterpreters.com/global-variables.html) —
  compiler halves of each section.
- [Python's `dis` module docs](https://docs.python.org/3/library/dis.html) —
  disassemble `x = 2` + `print(x + 3)` in Python and find your exact
  opcodes (STORE_NAME, LOAD_NAME, and the POP_TOP that answers this
  task's comment question).
- [Wikipedia — Symbol table](https://en.wikipedia.org/wiki/Symbol_table) —
  the general name→information mapping compilers keep while names still
  exist.

**Done when:** `let x = 2; print x + 3;` compiles to a byte sequence you
predicted on paper *before* running the test.

---

## Task 5.5 — Compile `if` and `while`: jumps and backpatching (~2h)

**Build:** `OP_JUMP`, `OP_JUMP_IF_FALSE`, `OP_LOOP` (jump backward), with
16-bit jump offsets. The trick of the task: when you emit a forward jump,
the target doesn't exist yet — emit a placeholder, remember its offset,
and **backpatch** it once you know where the jump lands.

**The concept:** bytecode has no `if` and no `while` — a flat instruction
array can only do one control trick: *change which instruction runs
next*. A **jump** adds an offset to the instruction pointer.
`if (c) A else B` compiles to: code for `c`, then `JUMP_IF_FALSE` over
A's code, then A, then an unconditional `JUMP` over B, then B. `while`
is a condition, a conditional jump forward (the exit), the body, and a
jump *backward* to the condition. Structured programming, revealed as
sugar over conditional goto — which is all a CPU actually has either.

The implementation snag is famous: when you emit `JUMP_IF_FALSE`, the
place it should land — after a body you haven't compiled yet — *doesn't
exist*. The ancient solution (it's in the original 1950s FORTRAN
compiler) is **backpatching**: emit the jump with placeholder offset
bytes, remember where those bytes sit, compile the body, then go back and
overwrite the placeholder with the now-known distance. Your 16-bit
offsets also force real **bit-level encoding**: 16 bits into two bytes
(`offset >> 8` and `offset & 0xff`) on the way in, reassembled on the way
out — the first of many times you'll split and join bytes in systems
work.

**Why compilers need it:** every compiled `if`, `while`, `for`, `&&`,
`break`, and `continue` in every language becomes exactly these jumps —
short-circuit `and`/`or` from task 4.3 compile to a conditional jump too
(that's the "control flow wearing an operator costume" claim, now
provable in bytes). After this task, the gap between "structured code"
and "what machines do" is closed for you personally.

**Read first:**
- [Crafting Interpreters ch. 23, "Jumping Back and Forth"](https://craftinginterpreters.com/jumping-back-and-forth.html) —
  compiler halves; the VM halves are next phase.
- [GeeksforGeeks — Backpatching in compiler design](https://www.geeksforgeeks.org/compiler-design/backpatching-in-compiler-design/) —
  the technique named and worked through in the classic textbook style.
- [Compiler Explorer](https://godbolt.org/) (interactive) — compile a C
  `if`/`while` at `-O0` and find the real conditional jumps (`b.ne`,
  `cbz`...) in the assembly: your OP_JUMP_IF_FALSE, in silicon.

**Done when:** disassembling a compiled `if`/`else` and a `while` shows
jumps landing on the exact offsets you traced by hand on paper. (Functions'
bytecode — call frames — can wait; add them after Phase 6 executes the
basics, using [ch. 24](https://craftinginterpreters.com/calls-and-functions.html).)
