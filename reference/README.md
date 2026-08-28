# Reference solutions — read this first

Working solutions for **Phase 0 and Phase 1 only**. They exist so a genuine
wall never ends the project — not so the exercises go faster.

**Rules of engagement:**

1. Don't open anything here until you've been truly stuck for 30+ minutes
   *after* re-reading the task's assigned links.
2. When you do open one, read the **smallest piece** that unblocks you,
   close the file, and write your own version from memory.
3. After your solution passes, comparing against these is fair game and
   genuinely useful — differences are things to understand, not fix.

These files mirror the stub filenames in `phase0/` and `src/`. They include
the same headers, so to compile one directly, add the include path, e.g.:

```sh
clang -std=c11 -Wall -Wextra -Iphase0 reference/phase0/08_dynarray.c phase0/08_dynarray_test.c -o /tmp/ref
clang -std=c11 -Wall -Wextra -Isrc reference/phase1/lexer.c tests/test_lexer_4.c -o /tmp/ref
```

(You shouldn't need to — they're for reading, and they pass the same tests
your code must pass.)
