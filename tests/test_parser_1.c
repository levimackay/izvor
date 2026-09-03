/* Test 2.4-1 — the parser's token helpers.
   Drive "1+2" through the helpers by hand:
   NUMBER, then check(PLUS) false, match(NUMBER) true,
   now current is PLUS, ... down to EOF */

/* ── STEP 0: includes ─────────────────────────────────────────────────
   Three #include lines go here, above main. Copy the shape from the top
   of tests/test_lexer_4.c.
     - the standard header that provides assert()
     - the standard header that provides printf()
     - the parser's own header. It is NOT in this folder, so the path has
       to climb out of tests/ and back down into src/. test_lexer_4.c
       shows you that path style already.
   Why parser.h and not lexer.h + ast.h too? Open src/parser.h and look
   at what it includes. You get those two for free.
   ──────────────────────────────────────────────────────────────────── */


int main(void){

   /* ── STEP 1: set up ───────────────────────────────────────────────
      Declare one variable of type Parser. Call it p. Do not use a
      pointer, just a plain struct sitting on the stack, the same way
      test_lexer_4.c declares `Lexer lx;`.

      Then call parser_init on it with the source string "1+2".
      Check the signature in src/parser.h: the first argument is a
      Parser*, so you have to pass the ADDRESS of p, not p itself.
      The & operator does that.

      Now assert that p.current.type is TOK_NUMBER.
      Why should it already be a number before you've advanced anything?
      Read parser_init in src/parser.c — its last line answers that.
      Note the dot, not an arrow: p is a struct, not a pointer.
      ─────────────────────────────────────────────────────────────── */


   /* ── STEP 2: check() answers correctly ────────────────────────────
      assert that parser_check(&p, TOK_PLUS) is false.
      Current is a NUMBER, so asking "are you a PLUS?" must say no.
      To assert something is false, put ! in front of it.
      ─────────────────────────────────────────────────────────────── */


   /* ── STEP 3: check() consumes nothing ─────────────────────────────
      assert parser_check(&p, TOK_NUMBER) is true.
      Then write the exact same assert line AGAIN.

      This looks like a pointless duplicate. It is not. It is the whole
      point of this step: if check() had a bug that advanced the parser,
      the second call would see a PLUS and fail. Two identical asserts
      that both pass is your proof that check() only asks.
      ─────────────────────────────────────────────────────────────── */


   /* ── STEP 4: match() on a miss changes nothing ────────────────────
      assert that parser_match(&p, TOK_PLUS) is false.
      Then assert p.current.type is STILL TOK_NUMBER.

      That second assert is the real test. A common bug is a match()
      that advances before it checks, silently eating a token. Look at
      lines 31-35 of src/parser.c and make sure you can explain why the
      early return protects you here.
      ─────────────────────────────────────────────────────────────── */


   /* ── STEP 5: match() on a hit consumes ────────────────────────────
      assert parser_match(&p, TOK_NUMBER) is true.
      Then assert p.current.type is now TOK_PLUS.

      Two asserts, because the return value and the side effect are two
      separate promises and either one could break on its own.
      ─────────────────────────────────────────────────────────────── */


   /* ── STEP 6: walk to the end ──────────────────────────────────────
      "1+2" has three tokens plus EOF. You have consumed the 1.
      Consume the + and the 2 the same way as STEP 5, asserting the
      return value each time, then assert p.current.type is TOK_EOF.

      Then try one more thing: call parser_match(&p, TOK_EOF) and think
      about what SHOULD happen before you assert anything. What does
      lexer_next return once it is past the end of the string? Look at
      src/lexer.c. If you are not sure, write the assert you expect,
      run it, and let the failure teach you.
      ─────────────────────────────────────────────────────────────── */


   /* ── STEP 7: say so ───────────────────────────────────────────────
      printf a line saying the test passed, matching the style of the
      last line of test_lexer_4.c. Then return 0.

      Remember: assert() is silent on success and kills the program on
      failure. So reaching this printf at all IS the pass condition.
      There is no framework here, just a macro from the C standard
      library. Nothing to clean up either — you never called
      parser_parse, so nothing was ever allocated on the heap.
      ─────────────────────────────────────────────────────────────── */

}
