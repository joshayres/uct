#include "lexer.h"

typedef struct
{
    const char* start;
    const char* current;
    int line;
} lexer;

lexer lex;
token tok;


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

void scan_int()
{
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

void scan_float()
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
    tok.type = TOKEN_FLOAT;
    tok.float_val = val;
    tok.line = lex.line;
}

char escape_to_char[256] =
{
    ['0'] = '\0',
    ['\''] = '\'',
    ['"'] = '"',
    ['\\'] = '\\',
    ['n'] = '\n',
    ['r'] = '\r',
    ['t'] = '\t',
    ['v'] = '\v',
    ['b'] = '\b',
    ['a'] = '\a',
};

void scan_char()
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
    else if(*lex.current == '\\')
    {
        lex.current++;
        val = escape_to_char[(unsigned char)*lex.current];
        if(val == 0 && lex.current != '0')
        {
            //TODO: ERROR
        }
        lex.current++;
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
    tok.type = TOKEN_INT;
    tok.int_val = val;
    tok.mod = MOD_CHAR;
}

void scan_string()
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
        buf_push(str, *lex.current); lex.current++;
    }
    buf_push(str, 0);
    tok.type = TOKEN_STR;
    tok.str_val = str;
    tok.line = lex.line;
}

#define CASE1(c1, k1) \
    case c1: \
        tok.type = k1; \
        lex.current++; \
        break;

#define CASE2(c1, k1, c2, k2) \
    case c1: \
        tok.type = k1; \
        lex.current++; \
        if(*lex.current == c2) { \
            tok.type = k2; \
            lex.current++; \
        } \
        break;

#define CASE3(c1, k1, c2, k2, c3, k3) \
    case c1: \
        tok.type = k1; \
        lex.current++; \
        if(*lex.current == c2) { \
            tok.type = k2; \
            lex.current++; \
        } \
        else if(*lex.current == c3) { \
            tok.type = c3;\
            lex.current++;\
        }\
        break;

void next_token()
{
    tok.start = lex.current;
    tok.mod = 0;

    switch(*lex.current)
    {
    case ' ': case '\n': case '\r': case '\t': case '\v':
        while(isspace(*lex.current))
        {
            if(*lex.current++ == '\n')
            {
                lex.line++;
            }
        }
        return next_token();
    case '\'':
        scan_char();
        break;
    case '"':
        scan_string();
        break;
    case '.':
        if(isdigit(lex.current[1]))
        {
            scan_float();
        }
        else
        {
            tok.type = TOKEN_DOT;
            lex.current++;
        }
        break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
    {
        while(isdigit(*lex.current))
        {
            lex.current++;
        }
        char c = *lex.current;
        lex.current = tok.start;
        if(c == '.')
        {
            scan_float();
        }
        else
        {
            scan_int();
        }
        break;
    }
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
        while(isalnum(*lex.current) || *lex.current == '_')
        {
            lex.current++;
            //TODO: keyword handling
        }
        tok.name = str_intern_range(tok.start, lex.current);
        tok.type = TOKEN_NAME;
        break;
    case '<':
        tok.type = TOKEN_LT;
        lex.current++;
        if(*lex.current == '<')
        {
            tok.type = TOKEN_LSHIFT;
            lex.current++;
            if(*lex.current == '=')
            {
                tok.type = TOKEN_LSHIFT_ASSIGN;
                lex.current++;
            }
        }
        else if(*lex.current == '=')
        {
            tok.type = TOKEN_LTEQ;
            lex.current++;
        }
        break;
    case '>':
        tok.type = TOKEN_GT;
        lex.current++;
        if(*lex.current == '<')
        {
            tok.type = TOKEN_RSHIFT;
            lex.current++;
            if(*lex.current == '=')
            {
                tok.type = TOKEN_RSHIFT_ASSIGN;
                lex.current++;
            }
        }
        else if(*lex.current == '=')
        {
            tok.type = TOKEN_GTEQ;
            lex.current++;
        }
        break;
    case '/':
        tok.type = TOKEN_DIV;
        lex.current++;
        if(*lex.current == '=')
        {
            tok.type = TOKEN_DIV_ASSIGN;
            lex.current++;
        }
        else if(*lex.current == '/')
        {
            lex.current++;
            while(*lex.current && *lex.current != '\n')
            {
                lex.current++;
            }
            return next_token();
        }
        break;

        CASE1('\0', TOKEN_EOF);
        CASE1('(',  TOKEN_LPAREN);
        CASE1(')',  TOKEN_RPAREN);
        CASE1('{',  TOKEN_LBRACE);
        CASE1('}',  TOKEN_RBRACE);
        CASE1('[',  TOKEN_LBRACKET);
        CASE1(']',  TOKEN_RBRACKET);
        CASE1(',',  TOKEN_COMMA);
        CASE1('@',  TOKEN_AT);
        CASE1('?',  TOKEN_QUESTION);
        CASE1(';',  TOKEN_SEMICOLON);
        CASE1('^',  TOKEN_HAT);
        CASE1('~',  TOKEN_NEG);
        CASE2('!',  TOKEN_NOT,      '=',    TOKEN_NOTEQ);
        CASE2(':',  TOKEN_COLON,    '=',    TOKEN_COLON_ASSIGN);
        CASE2('=',  TOKEN_ASSIGN,   '=',    TOKEN_EQ);
        CASE2('*',  TOKEN_MUL,      '=',    TOKEN_MUL_ASSIGN);
        CASE2('%',  TOKEN_MOD,      '=',    TOKEN_MOD_ASSIGN);
        CASE2('&',  TOKEN_AND,      '&',    TOKEN_AND_AND);
        CASE2('|',  TOKEN_OR,       '|',    TOKEN_OR_OR);
        CASE3('+',  TOKEN_ADD,      '=',    TOKEN_ADD_ASSIGN, '+', TOKEN_INC);
        CASE3('-',  TOKEN_SUB,      '=',    TOKEN_SUB_ASSIGN, '-', TOKEN_DEC);

    default:
        //TODO: ERROR
        lex.current++;
        next_token();
    }
}

#undef CASE1
#undef CASE2
#undef CASE3

void init_lex(const char* source)
{
    lex.start = source;
    lex.current = source;
    lex.line = 1;
    next_token();
}

bool is_token(token_type type)
{
    return tok.type == type;
}

bool match_token(token_type type)
{
    if(is_token(type))
    {
        next_token();
        return true;
    }
    else
    {
        return false;
    }
}
