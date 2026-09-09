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
   // This duplicate is deliberate
   assert(parser_check(&p, TOK_NUMBER) == true);
   assert(parser_check(&p, TOK_NUMBER) == true);
   assert(p.current.value == 1);
   assert(parser_check(&p, TOK_NUMBER) == true);

   assert(parser_match(&p, TOK_PLUS) == false);
   assert(p.current.type == TOK_NUMBER);
   assert(p.current.value == 1);

   assert(parser_match(&p, TOK_NUMBER) == true);
   assert(p.current.type == TOK_PLUS);

   assert(parser_match(&p, TOK_PLUS) == true);
   assert(p.current.type == TOK_NUMBER);
   assert(p.current.value == 2);

   assert(parser_match(&p, TOK_NUMBER) == true);
   assert(parser_check(&p, TOK_EOF) == true);

   parser_advance(&p);
   assert(parser_check(&p, TOK_EOF) == true);
   assert(parser_match(&p, TOK_NUMBER) == false);
   assert(parser_check(&p, TOK_EOF) == true);

   printf("test_parser_1 passed\n");
   return 0;
}
