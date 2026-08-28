# Phase 1 — Lexer (arithmetic expressions)

**Goal:** turn a string like `"(1 + 23) * 4"` into a stream of **tokens**:
`LPAREN, NUMBER(1), PLUS, NUMBER(23), RPAREN, STAR, NUMBER(4), EOF`.

**Why a lexer exists at all:** raw source is just characters, and characters
are the wrong unit to think in — `23` is two chars but one *thing*, spaces
mean nothing, `>=` is two chars but one operator. The lexer's whole job is
to convert "stream of characters" into "stream of meaningful words" so the
parser (Phase 2) never has to think about characters again. Every compiler
ever written starts here. Yours handles only numbers, `+ - * /`, and parens
for now — the *shape* is identical to a production lexer; later phases just
add more token types to the same skeleton.

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

**Why:** design-before-code. The token representation you choose constrains
everything downstream. The pointer+length trick (instead of copying each
lexeme into its own malloc'd string) is how real lexers avoid thousands of
allocations — you learned in task 0.8 why that matters. `token_type_name`
looks trivial but becomes your debugging eyes for the rest of the project.

**Read first:**
- [Crafting Interpreters ch. 4](https://craftinginterpreters.com/scanning.html), §4.2 "Lexemes and Tokens".
- Beej ch. 7 (structs) if the header's structs feel shaky: [Beej's Guide](https://beej.us/guide/bgc/html/split/).

**Done when:** `make p1-1` passes.

---

## Task 1.2 — Lexer state + single-character tokens (~1.5h)

**Build:** `lexer_init()`, then `lexer_next()` handling exactly:
`+ - * / ( )` and end-of-string → `TOK_EOF`. Write the two tiny static
helpers suggested in the comments (`peek`, `advance`) — everything else in
this phase is built from them.

**Why:** this is the lexer's engine: look at the current character, decide,
consume, emit. The helpers matter more than they look: *peek* (look without
consuming) vs *advance* (consume) is the fundamental rhythm of both lexers
and parsers — Phase 2 uses the same pair, one level up, over tokens instead
of characters. Also note the contract you're implementing: `lexer_next` is a
*pull* API — the caller asks for tokens one at a time, and asking again
after EOF keeps returning EOF (the tests check this; it saves the parser
from ever crashing off the end).

**Read first:**
- [Crafting Interpreters ch. 4](https://craftinginterpreters.com/scanning.html), §4.4–4.5 (the scanning loop, recognizing lexemes).

**Done when:** `make p1-2` passes.

---

## Task 1.3 — Whitespace and number literals (~1.5h)

**Build:** make `lexer_next()` (a) silently skip spaces, tabs, and newlines,
and (b) on a digit, consume the whole run of digits and return a
`TOK_NUMBER` whose `value` is the parsed integer and whose `start`/`length`
cover the lexeme.

**Why:** your first *multi-character* token — the moment the lexer stops
being a character-classifier and starts grouping. The digit-accumulation
loop is literally your `parse_uint` from task 0.6. Whitespace-skipping is
the lexer earning its keep: because it happens here, no later stage ever
thinks about spaces again. (This is also where languages decide things like
"do newlines end statements?" — you're deciding: no, whitespace is
invisible.)

**Read first:**
- [Crafting Interpreters ch. 4](https://craftinginterpreters.com/scanning.html), §4.6–4.7 (longer lexemes, number literals).

**Done when:** `make p1-3` passes.

---

## Task 1.4 — Error tokens and full-expression sweep (~1–1.5h)

**Build:** on any character you don't recognize (`@`, `$`, a letter…),
return a `TOK_ERROR` token pointing at the offending character — and keep
the lexer in a sane state so the caller can continue. Then make the full
test pass: lexing `"(1 + 23) * 4 - 5 / 2"` end to end into the exact
expected token sequence.

**Why:** error handling is half of real language implementation, and the
lexer sets the pattern: **bad input is not a crash, it's data** — an error
token flows through the same pipe as every other token, and the *parser*
decides what to do with it. Users of your language will judge it almost
entirely by its error messages; the `start` pointer you preserve here is
what lets you eventually print `unexpected character '@' at column 7`.

**Read first:**
- [Crafting Interpreters ch. 4](https://craftinginterpreters.com/scanning.html), §4.1 and §4.5.1 (error handling).

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

**Why:** every serious language implementation grows internal debugging
tools alongside itself — clang has `-dump-tokens`, and now so do you. In
Phase 2, when the parser misbehaves, your first question will be "did the
lexer even produce what I think?" and this answers it in five seconds. It's
also your first `argv`-driven C program: the standard shape of every CLI
tool you'll ever write.

**Read first:**
- [Beej's Guide to C — command line arguments section](https://beej.us/guide/bgc/html/split/) (ch. 11 area, "Command Line Arguments").

**Done when:** the output above matches (in substance) and feeding it junk
like `"1 @ 2"` prints an ERROR token instead of crashing.

---

## Phase 1 exit check

You can explain: what a token is and why parsers don't consume characters;
why `Token` stores pointer+length instead of a string copy; the peek/advance
rhythm; and why errors are tokens. Next up, [Phase 2](02-parser.md): turning
this flat token stream into a *tree* that knows `1 + 2 * 3` means
`1 + (2 * 3)`.
