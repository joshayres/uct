#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include "common.c"
#include "lexer.c"
#include "ast.c"

#define assert_token_int(x) assert(tok.int_val == (x) && match_token(TOKEN_INT))
#define assert_token_float(x) assert(tok.float_val == (x) && match_token(TOKEN_FLOAT))
#define assert_token_eof() assert(is_token(0))
#define assert_token_str(x) assert(strcmp(tok.str_val, (x)) == 0 && match_token(TOKEN_STR))

int main(int argc, char **argv)
{
    init_keywords();
    //INT test
    char* source = "0 2158 0xffffff 0o12 0b1010";
    init_lex(source);
    assert_token_int(0);
    assert_token_int(2158);
    assert(tok.mod = MOD_HEX);
    assert_token_int(0xffffff);
    assert(tok.mod = MOD_OCT);
    assert_token_int(012);
    assert(tok.mod = MOD_BIN);
    assert_token_int(10);
    assert_token_eof();

    //FLOAT TEST
    init_lex("3.14 .12 42.");
    assert_token_float(3.14);
    assert_token_float(.12);
    assert_token_float(42.);
    assert_token_eof();

    //CHAR TEST
    init_lex(" 'a' '\\n'");
    assert_token_int('a');
    assert_token_int('\n');
    assert_token_eof();

    //STRING TEST
    init_lex(" \"Hi\"");
    assert_token_str("Hi");
    assert_token_eof();

    //Keyword test
    init_lex("struct");
    assert(tok.type == TOKEN_KEYWORD);
    assert(tok.name == struct_keyword);

    return 0;
}
