#include "lexer.h"

typedef struct
{
    const char* start;
    const char* current;
    i32 line;
} lexer;

lexer lex;
token tok;

static bool is_alpha(char c)
{
    return  (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
}

u8 char_to_digit[256] =
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

void warning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("%s(%d): warning: ", tok.name, lex.line);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("%s(%d): error: ", tok.name, lex.line);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

#define fatal_error(...) (error(__VA_ARGS__), exit(1))

void scan_int()
{
    tok.start = lex.current;
    i32 base = 10;
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
        i32 digit = char_to_digit[(unsigned char)*lex.current];
        if(digit == 0 && *lex.current != '0')
        {
            break;
        }
        if(digit >= base)
        {
            error("Digit '%c' too high for base %d", *lex.current, base);
            digit = 0;
        }
        if(val > (ULLONG_MAX - digit)/base)
        {
            error("overflow");
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
        error("Expected base %d digit, got '%c'", base, *lex.current);
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
    f64 val = strtod(start, NULL);
    if(val == HUGE_VAL)
    {
        error("Float literal overflow");
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
    i32 val = 0;
    if(*lex.current == '\'')
    {
        error("Char literal cannot be empty");
        lex.current++;
    }
    else if(*lex.current == '\n')
    {
        error("Char literal cannot be a new line");
    }
    else if(*lex.current == '\\')
    {
        lex.current++;
        val = escape_to_char[(unsigned char)*lex.current];
        if(val == 0 && *lex.current != '0')
        {
            error("Invalid char literal escape '\\%c'", *lex.current);
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
        error("Expected closing char quote, got '%c'", *lex.current);
    }
    else
    {
        lex.current++;
    }
    tok.type = TOKEN_INT;
    tok.int_val = val;
    tok.mod = MOD_CHAR;
}

//TODO: handle escape characters
void scan_string()
{
    assert(*lex.current == '"');
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
    if(*lex.current)
    {
        lex.current++;
    }
    else
    {
        fatal_error("Unexpected end of file within string literal");
    }
    buf_push(str, 0);
    tok.type = TOKEN_STR;
    tok.str_val = str;
    tok.line = lex.line;
}

#define KEYWORD(name) name##_keyword = str_intern(#name); buf_push(keywords, name##_keyword)

void init_keywords()
{
    static bool inited;
    if(inited)
    {
        return;
    }
    KEYWORD(struct);
    KEYWORD(enum);
    KEYWORD(union);
    KEYWORD(let);
    KEYWORD(fn);
    KEYWORD(const);
    KEYWORD(if);
    KEYWORD(else);
    KEYWORD(for);
    KEYWORD(while);
    KEYWORD(switch);
    KEYWORD(break);
	KEYWORD(continue);
    KEYWORD(err);
    KEYWORD(import);
    KEYWORD(extern);
    KEYWORD(in);
    KEYWORD(return);

    first_keyword = struct_keyword;
    last_keyword = return_keyword;

    inited = true;
}

#undef KEYWORD

bool is_keyword_name(const char* name)
{
    return first_keyword <= name && name <= last_keyword;
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
        while(isalnum(*lex.current) || *lex.current == '_')
        {
            lex.current++;
        }
        tok.name = str_intern_range(tok.start, lex.current);
        tok.type = is_keyword_name(tok.name) ? TOKEN_KEYWORD : TOKEN_NAME;
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
		CASE1('_', 	TOKEN_UNDERSCORE);
        CASE2('!',  TOKEN_NOT,      '=',    TOKEN_NOTEQ);
        CASE2(':',  TOKEN_COLON,    '=',    TOKEN_COLON_ASSIGN);
        CASE2('*',  TOKEN_MUL,      '=',    TOKEN_MUL_ASSIGN);
        CASE2('%',  TOKEN_MOD,      '=',    TOKEN_MOD_ASSIGN);
        CASE2('&',  TOKEN_AND,      '&',    TOKEN_AND_AND);
        CASE2('|',  TOKEN_OR,       '|',    TOKEN_OR_OR);
        CASE3('+',  TOKEN_ADD,      '=',    TOKEN_ADD_ASSIGN, '+', TOKEN_INC);
        CASE3('-',  TOKEN_SUB,      '=',    TOKEN_SUB_ASSIGN, '-', TOKEN_DEC);
        CASE3('=',  TOKEN_ASSIGN,   '=',    TOKEN_EQ,         '>', TOKEN_ARROW);

    default:
        error("No token of found");
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

const char* token_info()
{
    if(tok.type == TOKEN_NAME || tok.type == TOKEN_KEYWORD)
	{
		return tok.name;
	}
	return token_type_name(tok.type);
}

bool is_token(token_type type)
{
    return tok.type == type;
}

bool is_token_eof()
{
    return tok.type == TOKEN_EOF;
}

bool is_keyword(const char* name)
{
    return is_token(TOKEN_KEYWORD) && tok.name == name;
}

bool is_name(const char* name)
{
    return is_token(TOKEN_NAME) && tok.name == name;
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

bool match_keyword(const char* name)
{
    if(is_keyword(name))
	{
		next_token();
		return true;
	}
	return false;
}

bool expect_token(token_type type)
{
    if(is_token(type))
	{
		next_token();
		return true;
	}
	else
	{
		error("expected token %s, got %s", token_type_name(type), token_info());
	}
}
