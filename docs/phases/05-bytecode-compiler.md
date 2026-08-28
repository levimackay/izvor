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

**Why:** tree-walking is slow: every `+` chases pointers all over the heap,
murdering CPU caches. Real languages (Python, Lua, Java, JS) compile to
**bytecode** — dense, linear, cache-friendly instructions for an imaginary
machine — and this phase is the conceptual bridge from "interpreter" to
"compiler": same AST in, but now the output is *a program*, not a value.
When you finish Phase 6 you'll benchmark fib on both back ends and see the
difference as a number.

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

**Why:** you're designing an **instruction set** — the same considerations
(fixed-size opcodes, operands, constant pools) as JVM class files and
Python `.pyc`. The "index into a side table" trick is universal in every
binary format you'll ever meet. And your Phase 0 dynamic array turns out to
have been compiler infrastructure all along.

**Read first:**
- [Crafting Interpreters ch. 14](https://craftinginterpreters.com/chunks-of-bytecode.html), §14.1–14.5 — this task is that chapter.

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

**Why:** you're about to write two programs (compiler, VM) that communicate
through raw bytes — when they disagree, the disassembler is the only
witness. Build the debugging tool *before* the thing that needs debugging;
that ordering is the mark of someone who has debugged binary formats
before. (Same move as `lexdump` — see the pattern?)

**Read first:**
- [Crafting Interpreters ch. 14](https://craftinginterpreters.com/chunks-of-bytecode.html), §14.5 "Disassembling Chunks".

**Done when:** the hand-built chunk from 5.1 disassembles to exactly the
listing you expect (assert on the printed string, or eyeball + freeze it).

---

## Task 5.3 — Compile arithmetic expressions (~1.5–2h)

**Build:** `compiler.c`: `compile(Node*, Chunk*)` — a post-order tree walk:
compile left child, compile right child, emit the operator's opcode.
Numbers emit `OP_CONST`.

**Why:** post-order emission is *the* insight of stack machines: by the
time `ADD` executes, its two operands are the top two stack slots —
**the tree's shape becomes the instructions' order**. This is also your
third full traversal of the same AST (eval, free/print, now compile), which
is the deep lesson: an AST is one data structure with many interpreters.

**Read first:**
- [Crafting Interpreters ch. 17](https://craftinginterpreters.com/compiling-expressions.html) —
  read for the *emitting* parts (§17.3–17.5); the Pratt-parsing machinery is
  solving a problem (no AST) you don't have.

**Done when:** asserts compile `"1 + 2 * 3"` and check the chunk's exact
byte sequence (CONST 1, CONST 2, CONST 3, MUL, ADD) — the disassembler
makes failures readable.

---

## Task 5.4 — Compile statements and variables (~1.5–2h)

**Build:** opcodes and compilation for `print`, expression statements
(`OP_POP` — why must a statement's value be popped? answer in a comment),
and global variables (`OP_DEFINE_GLOBAL`, `OP_GET_GLOBAL`, `OP_SET_GLOBAL`,
names in the constant pool).

**Why:** statements introduce **stack discipline**: every statement must
leave the stack exactly as it found it, or the machine slowly corrupts —
the bytecode version of a memory leak, and the classic VM bug. Variables
force "how does a *name* survive into a nameless instruction stream?" —
answer: it doesn't; it becomes an index. That's what "compilation erases
names" means, and it's why compiled binaries need debug-info tables to give
them back.

**Read first:**
- [Crafting Interpreters ch. 21, "Global Variables"](https://craftinginterpreters.com/global-variables.html) — compiler halves of each section.

**Done when:** `let x = 2; print x + 3;` compiles to a byte sequence you
predicted on paper *before* running the test.

---

## Task 5.5 — Compile `if` and `while`: jumps and backpatching (~2h)

**Build:** `OP_JUMP`, `OP_JUMP_IF_FALSE`, `OP_LOOP` (jump backward), with
16-bit jump offsets. The trick of the task: when you emit a forward jump,
the target doesn't exist yet — emit a placeholder, remember its offset,
and **backpatch** it once you know where the jump lands.

**Why:** this is where structured programming gets unstructured: `if` and
`while` are revealed to be sugar over conditional `goto`, which is all a CPU
actually has. Backpatching is a genuine, ancient compiler technique (it's
in the original FORTRAN compiler), and byte-splitting a 16-bit offset into
two `uint8_t`s teaches bit-level encoding you'll be glad to know at Apple.

**Read first:**
- [Crafting Interpreters ch. 23, "Jumping Back and Forth"](https://craftinginterpreters.com/jumping-back-and-forth.html) — compiler halves; the VM halves are next phase.

**Done when:** disassembling a compiled `if`/`else` and a `while` shows
jumps landing on the exact offsets you traced by hand on paper. (Functions'
bytecode — call frames — can wait; add them after Phase 6 executes the
basics, using [ch. 24](https://craftinginterpreters.com/calls-and-functions.html).)
