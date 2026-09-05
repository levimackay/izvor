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
   
   // Rest of tests will go here
   
}
