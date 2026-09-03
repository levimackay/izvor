/* Test 2.4-1 — the parser's token helpers.
   Drive "1+2" through the helpers by hand:
   NUMBER, then check(PLUS) false, match(NUMBER) true,
   now current is PLUS, ... down to EOF */

#include <assert.h>
#include <stdio.h>
#include "../src/parser.h"

int main(void){

   Parser p;

   parser_init(&p, "1+2");
   assert(p.current.type == TOK_NUMBER);
   assert(parser_check(&p, TOK_PLUS) == false);
   // Check asks without consuming. In order for it to work, you need to call it twice.
   // Could be fixed with a more complex parser
   // This duplicate is deliberate
   assert(parser_check(&p, TOK_NUMBER) == true);
   assert(parser_check(&p, TOK_NUMBER) == true);
   

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
