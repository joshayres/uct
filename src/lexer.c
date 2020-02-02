#include "lexer.h"

typedef struct
{
    const char* start;
    const char* current;
    int line;
} lexer;

lexer lex;

void init_lex(const char* source)
{
    lex.start = source;
    lex.current = source;
    lex.line = 1;
}

static bool is_alpha(char c)
{
    return  (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
}

uint8_t char_to_digit[256] =
{
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15,
};

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_at_end()
{
    return *lex.current == '\0';
}

static char peek_next()
{
    if(is_at_end()) return '\0';
    return lex.current[1];
}

static bool match(char expected)
{
    if(is_at_end()) return false;
    if(*lex.current != expected) return false;

    lex.current++;
    return true;
}

static token make_token(token_type type)
{
    token result;
    result.type = type;
    result.start = lex.start;
    result.length - (int)(lex.current - lex.start);
    result.line = lex.line;

    return result;
}

token scan_int()
{
    token tok = {};
    tok.start = lex.current;
    int base = 10;
    const char *start_digits = lex.current;
    if(*lex.current == '0')
    {
        lex.current++;
        if(tolower(*lex.current) == 'x')
        {
            lex.current++;
            tok.mod = MOD_HEX;
            base = 16;
            start_digits = lex.current;
        }
        else if(tolower(*lex.current) == 'b')
        {
            lex.current++;
            tok.mod = MOD_BIN;
            base = 2;
            start_digits = lex.current;
        }
        else if(tolower(*lex.current) == 'o')
        {
            lex.current++;
            tok.mod = MOD_OCT;
            base = 8;
            start_digits = lex.current;
        }
    }
    unsigned long long val = 0;
    for(;;)
    {
        if(*lex.current == '_')
        {
            lex.current++;
            continue;
        }
        int digit = char_to_digit[(unsigned char)*lex.current];
        if(digit == 0 && *lex.current != '0')
        {
            break;
        }
        if(digit >= base)
        {
            //TODO: ERROR Handling plz
            digit = 0;
        }
        if(val > (ULLONG_MAX - digit)/base)
        {
            //TODO: ERROR
            while(isdigit(*lex.current))
            {
                lex.current++;
            }
            val = 0;
            break;
        }
        val = val*base + digit;
        lex.current++;
    }
    if(lex.current == start_digits)
    {
        //TODO: ERROR
    }
    tok.type = TOKEN_INT;
    tok.int_val = val;
    tok.line = lex.line;
}

token scan_float()
{
    const char *start = lex.current;
    while(isdigit(*lex.current))
    {
        lex.current++;
    }
    if(*lex.current == '.')
    {
        lex.current++;
    }
    while(isdigit(*lex.current))
    {
        lex.current++;
    }
    double val = strtod(start, NULL);
    if(val == HUGE_VAL)
    {
        //TODO: ERROR
    }
    token tok = {};
    tok.type = TOKEN_FLOAT;
    tok.float_val = val;
    tok.line = lex.line;
}

token scan_char()
{
    assert(*lex.current == '\'');
    lex.current++;
    int val = 0;
    if(*lex.current == '\'')
    {
        //TODO: ERROR
        lex.current++;
    }
    else if(*lex.current == '\n')
    {
        //TODO: ERROR
    }
    else
    {
        val = *lex.current;
        lex.current++;
    }
    if(*lex.current != '\'')
    {
        //TODO:ERROR
    }
    else
    {
        lex.current++;
    }
    token tok = {};
    tok.type = TOKEN_INT;
    tok.int_val = val;
    tok.mod = MOD_CHAR;
}

token scan_string()
{
    assert(*lex.current == "'");
    lex.current++;
    char *str = NULL;
    while(*lex.current != '"' && !is_at_end())
    {
        if(*lex.current == '\n')
        {
            lex.line++;
        }
        buf_push(str, *lex.current);
        lex.current++;
    }
    buf_push(str, 0);
    token tok = {};
    tok.type = TOKEN_STR;
    tok.str_val = str;
    tok.line = lex.line;
}
