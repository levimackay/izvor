# Phase 1 — Lexer (arithmetic expressions)

**Goal:** turn a string like `"(1 + 23) * 4"` into a stream of **tokens**:
`LPAREN, NUMBER(1), PLUS, NUMBER(23), RPAREN, STAR, NUMBER(4), EOF`.

**Why a lexer exists at all:** to a computer, source code is just a string
— a flat row of characters, one byte each, no structure. But characters
are the wrong unit to *think* in: `23` is two characters but one number;
the spaces around `+` mean nothing; in a bigger language `>=` is two
characters but one operator, and `print` is five characters but one word.
The **lexer** (also called a *scanner* or *tokenizer*) is the stage that
groups characters into the smallest meaningful units — **tokens** — so
the parser (Phase 2) can think in words instead of letters. It's the same
move your eyes make reading this sentence: you don't see `t`,`h`,`e`, you
see "the". Every compiler ever written starts here. Yours handles only
numbers, `+ - * /`, and parens for now — but the *shape* is identical to a
production lexer; later phases just add more token types to the same
skeleton.

**Spine reading for this phase** (read before task 1.1):
- [Crafting Interpreters ch. 4, "Scanning"](https://craftinginterpreters.com/scanning.html) —
  Nystrom builds exactly this, in Java; you're porting the ideas to C. Read
  it once now for shape, then re-read sections per task.
- [Let's Build a Simple Interpreter, part 1](https://ruslanspivak.com/lsbasi-part1/) —
  a 20-minute version of this whole phase in Python; great for the aerial view.

**Scaffold:** `src/lexer.h` (the data structures and function signatures —
read every comment), `src/lexer.c` (stubs you implement), `tests/` (one
assert-based test per task). `make p1-N` runs task N's test.

---

## Task 1.1 — Understand the data structures + `token_type_name` (~1h)

**Build:** read `src/lexer.h` until every field makes sense — `TokenType`,
`Token` (why does it hold a *pointer into the source* plus a length, instead
of a copied string?), `Lexer` (source + position: the whole state). Then
implement `token_type_name()` in `src/lexer.c`: a `switch` mapping each
`TokenType` to a printable string.

**The concept:** a token has two parts. The **type** says what *kind* of
thing it is (`TOK_NUMBER`, `TOK_PLUS`...) — an enum, which in C is just a
named integer, perfect for `switch`ing on. The **lexeme** is the actual
text it came from (`"23"`, `"+"`). The design question is how to store
that text, and the header's answer is the professional one: don't copy it.
A `Token` carries a *pointer into the original source string* plus a
*length*. The characters `2` and `3` already exist in the source — copying
them into a fresh malloc'd string per token would mean thousands of
allocations for a real program (you learned in task 0.8 what allocations
cost). The price of the trick is a lifetime rule: the source string must
stay alive as long as any token cut from it — a pointer into freed memory
is exactly the "never touch freed memory" sin from task 0.8.

Notice also how small `Lexer` is: the source pointer and a position. A
lexer needs no other memory — everything it decides, it decides by looking
at the current character. That's worth internalizing: most of the machinery
you'll build in this project is smaller than people imagine.

**Why compilers need it:** the token representation constrains everything
downstream — the parser reads these structs, error messages point through
them back into the source, and in Phase 3 the keyword check compares their
text. Design-before-code is the habit: understand the data, and the
functions write themselves. `token_type_name` looks trivial but becomes
your debugging eyes for the rest of the project — every "what did the
lexer just give me?" question for months gets answered through it.

**Read first:**
- [Crafting Interpreters ch. 4, §4.2 "Lexemes and Tokens"](https://craftinginterpreters.com/scanning.html) —
  the same design conversation, in Java.
- [Beej's Guide to C — Structs](https://beej.us/guide/bgc/html/split/structs.html) —
  if the header's structs feel shaky, review before writing.
- [Wikipedia — Lexical analysis](https://en.wikipedia.org/wiki/Lexical_analysis) —
  the reference view: token, lexeme, and where the lexer sits in a
  compiler. Good for seeing the standard vocabulary once.

**Done when:** `make p1-1` passes.

---

## Task 1.2 — Lexer state + single-character tokens (~1.5h)

**Build:** `lexer_init()`, then `lexer_next()` handling exactly:
`+ - * / ( )` and end-of-string → `TOK_EOF`. Write the two tiny static
helpers suggested in the comments (`peek`, `advance`) — everything else in
this phase is built from them.

**The concept:** a lexer's engine is a rhythm of two moves. **Peek** looks
at the current character *without* consuming it — "what's next?" —
and **advance** consumes it — "I'm taking that one." Every decision is:
peek, decide what kind of token is starting, advance past the characters
that belong to it, emit the token. The separation matters because you
often need to *look* before you can *commit*: when you see a digit you
must keep peeking to know whether the number continues, and in Phase 4,
when you see `<`, you'll peek at the next character to decide between `<`
and `<=`. One character of lookahead, honestly managed, is enough to lex
almost every real language.

The other contract in this task is the **pull API**: `lexer_next` hands
out one token per call, on demand — the lexer doesn't pre-compute a big
token list (though it could; that's a design choice). And once the source
is exhausted it returns `TOK_EOF` — *every* time, forever. That
guaranteed-EOF behavior is a kindness to the parser: it can always ask for
"the next token" without checking whether one exists, and the worst that
happens at the end is it sees EOF again.

**Why compilers need it:** this peek/advance pair is the fundamental
rhythm of the whole front end. Phase 2's parser uses the *same two
helpers, one level up* — peeking and advancing over tokens instead of
characters. Learn the rhythm here, where the items are single bytes, and
recursive descent will feel familiar instead of new.

**Read first:**
- [Crafting Interpreters ch. 4, §4.4–4.5](https://craftinginterpreters.com/scanning.html) —
  the scanning loop and recognizing single-character lexemes.
- [Let's Build a Simple Interpreter, part 1](https://ruslanspivak.com/lsbasi-part1/) —
  the same engine in Python; compare his `get_next_token` to your
  `lexer_next`.
- [Rob Pike — Lexical Scanning in Go](https://www.youtube.com/watch?v=HxaD_trXwRE) (video, ~30 min) —
  a master class on how a working lexer is structured. His state-function
  design is fancier than yours needs to be; watch it for how he *thinks*
  about peek/advance/emit.

**Done when:** `make p1-2` passes.

---

## Task 1.3 — Whitespace and number literals (~1.5h)

**Build:** make `lexer_next()` (a) silently skip spaces, tabs, and newlines,
and (b) on a digit, consume the whole run of digits and return a
`TOK_NUMBER` whose `value` is the parsed integer and whose `start`/`length`
cover the lexeme.

**The concept:** this is your first *multi-character* token — the moment
the lexer stops being a character classifier and starts **grouping**. The
algorithm for a number: note where you are (that's the token's `start`),
then keep advancing while the peeked character is a digit, accumulating
`value = value*10 + (c - '0')` as you go — literally your `parse_uint`
from task 0.6 — and when the digits stop, the distance you traveled is the
token's `length`. This "consume a run of characters that satisfy a
predicate" shape is called **maximal munch**: the lexer always takes the
longest sequence it can. `23` is one token NUMBER(23), never NUMBER(2)
then NUMBER(3), because the loop doesn't stop while digits keep coming.

Whitespace-skipping is the other half, and it's a *policy decision
disguised as a chore*: by consuming spaces, tabs and newlines before every
token and emitting nothing, you're declaring that whitespace has no
meaning in tinylang. Because it happens here, no later stage of the
pipeline ever thinks about spaces again — the parser will never even know
they existed. (Languages differ here on purpose: Python's lexer emits
INDENT/DEDENT tokens because whitespace *is* meaningful there, and
JavaScript's newline rules cause its famous semicolon-insertion quirks.
You're deciding: invisible.)

**Why compilers need it:** every stage of a compiler exists to make the
next stage's life simpler. This task is the clearest example you'll ever
see — a few lines of skipping and grouping here saves *every* downstream
stage from thinking about characters and spacing forever.

**Read first:**
- [Crafting Interpreters ch. 4, §4.6 "Longer Lexemes"](https://craftinginterpreters.com/scanning.html) —
  maximal munch and number literals (§4.6.2).
- [Let's Build a Simple Interpreter, part 2](https://ruslanspivak.com/lsbasi-part2/) —
  adds exactly these two features (whitespace, multi-digit numbers) to the
  part-1 lexer; a perfect parallel read.
- [man page: isdigit/isspace](https://man7.org/linux/man-pages/man3/isalpha.3.html) —
  the standard character-classification functions (reference).

**Done when:** `make p1-3` passes.

---

## Task 1.4 — Error tokens and full-expression sweep (~1–1.5h)

**Build:** on any character you don't recognize (`@`, `$`, a letter…),
return a `TOK_ERROR` token pointing at the offending character — and keep
the lexer in a sane state so the caller can continue. Then make the full
test pass: lexing `"(1 + 23) * 4 - 5 / 2"` end to end into the exact
expected token sequence.

**The concept:** what should a program do with input it doesn't
understand? The beginner instinct is to crash or print-and-exit from deep
inside the lexer. The professional pattern is better: **bad input is not a
crash, it's data.** An unrecognized character becomes a `TOK_ERROR` token
that flows through the same pipe as every other token, carrying a pointer
to exactly the offending character. The lexer stays calm — it consumes the
bad character and is immediately ready to lex whatever follows — and the
*caller* decides what an error means (stop? report and continue? collect
several?). This separation — low layers report, upper layers decide — is
one of the most reusable design patterns in all of software, and error
tokens are its compiler-shaped instance.

The `start` pointer you preserve is not bureaucracy: since it points into
the original source, whoever handles the error can compute *where* it
happened (subtract the source start to get an offset, count newlines to
get a line number) and eventually print
`unexpected character '@' at column 7`.

**Why compilers need it:** users of your language will judge it almost
entirely by its error messages — think about how much of your day with
clang or a Python traceback is spent reading errors. Real compilers are
engineered to report *many* errors per run, not just the first, and that's
only possible because errors are values that flow, not exceptions that
abort. You're laying that foundation now.

**Read first:**
- [Crafting Interpreters ch. 4, §4.1.1 and §4.5.1](https://craftinginterpreters.com/scanning.html) —
  error handling and lexical errors.
- [Clang's "Expressive Diagnostics"](https://clang.llvm.org/diagnostics.html) —
  what a production compiler considers a good error message, with
  examples; aspirational reading, and the reason we keep that `start`
  pointer.
- [Cornell CS 3110 — Lexing and Parsing](https://cs3110.github.io/textbook/chapters/interp/parsing.html) —
  a second, more formal view of what lexers are for and where their job
  ends.

**Done when:** `make p1-4` passes. **Phase 1's core is now done.**

---

## Task 1.5 — `lexdump`: a token-printing tool (~1h)

**Build:** implement the small `main()` in `src/lexer_main.c`: take an
expression as `argv[1]`, loop `lexer_next` until EOF, and print one token
per line using `token_type_name`. Build with `make lexdump`, then:

```sh
./build/lexdump "12 + 3*(40-5)"
```

Expected shape of output (exact formatting is yours):

```
NUMBER    '12'   value=12
PLUS      '+'
NUMBER    '3'    value=3
STAR      '*'
LPAREN    '('
NUMBER    '40'   value=40
MINUS     '-'
NUMBER    '5'    value=5
RPAREN    ')'
EOF
```

**The concept:** two things are new. First, **command-line arguments**:
when you run `./lexdump "12 + 3"`, the shell hands your `main` an array of
strings — `argc` is how many, `argv` is the array, `argv[0]` is the
program's own name and `argv[1]` is your expression. That's the standard
shape of every CLI tool you'll ever write, and it's also your first
program whose input comes from *outside* the source code. Second, the
printf formatting to make columns line up — `%-10s` pads a string to a
field width — plus the `%.*s` trick: printing a pointer+length string
(which is what your tokens hold — there's no null terminator at the end
of a lexeme!) needs `printf("%.*s", tok.length, tok.start)`.

**Why compilers need it:** every serious language implementation grows
internal debugging tools alongside itself — clang can dump its tokens,
Python ships the `tokenize` module, and now so do you. In Phase 2, when
the parser misbehaves, your first question will be "did the lexer even
produce what I think?" and this tool answers it in five seconds. Building
the *observability* tool before you need it is a habit that will mark you
as someone who has debugged real systems.

**Read first:**
- [Beej's Guide to C — The Outside Environment](https://beej.us/guide/bgc/html/split/the-outside-environment.html) —
  command-line arguments: argc/argv, with examples.
- [man page: printf(3)](https://man7.org/linux/man-pages/man3/printf.3.html) —
  the format-specifier reference; look up field width and `%.*s` precision.
- [Crafting Interpreters ch. 4, §4.1](https://craftinginterpreters.com/scanning.html) —
  how Lox's main driver drives its scanner; yours is the same shape,
  smaller.

**Done when:** the output above matches (in substance) and feeding it junk
like `"1 @ 2"` prints an ERROR token instead of crashing.

---

## Phase 1 exit check

You can explain: what a token is and why parsers don't consume characters;
why `Token` stores pointer+length instead of a string copy; the peek/advance
rhythm; and why errors are tokens. Next up, [Phase 2](02-parser.md): turning
this flat token stream into a *tree* that knows `1 + 2 * 3` means
`1 + (2 * 3)`.
