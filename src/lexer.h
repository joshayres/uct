typedef enum
{
    TOKEN_EOF,

    TOKEN_COLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_AT,
    TOKEN_QUESTION,
    TOKEN_SEMICOLON,

    TOKEN_KEYWORD,

    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STR,
    TOKEN_NAME,
    TOKEN_NEG,
    TOKEN_NOT,
    TOKEN_OR,
    TOKEN_HAT,

    TOKEN_FIRST_MUL,
    TOKEN_MUL = TOKEN_FIRST_MUL,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_AND,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    TOKEN_LAST_MUL = TOKEN_RSHIFT,

    TOKEN_FIRST_ADD,
    TOKEN_ADD = TOKEN_FIRST_ADD,
    TOKEN_SUB,
    TOKEN_LAST_ADD = TOKEN_SUB,

    TOKEN_FIRST_CMP,
    TOKEN_EQ = TOKEN_FIRST_CMP,
    TOKEN_NOTEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LTEQ,
    TOKEN_GTEQ,
    TOKEN_LAST_CMP = TOKEN_GTEQ,
    TOKEN_AND_AND,
    TOKEN_OR_OR,

    TOKEN_FIRST_ASSIGN,
    TOKEN_ASSIGN = TOKEN_FIRST_ASSIGN,
    TOKEN_ADD_ASSIGN,
    TOKEN_SUB_ASSIGN,
    TOKEN_MUL_ASSIGN,
    TOKEN_DIV_ASSIGN,
    TOKEN_MOD_ASSIGN,
    TOKEN_LSHIFT_ASSIGN,
    TOKEN_RSHIFT_ASSIGN,
    TOKEN_LAST_ASSIGN = TOKEN_RSHIFT_ASSIGN,

    TOKEN_INC,
    TOKEN_DEC,
    TOKEN_COLON_ASSIGN,

    TOKEN_ARROW,
	TOKEN_UNDERSCORE,

    NUM_TOKEN_KINDS,
} token_type;

typedef enum
{
    MOD_NONE,
    MOD_HEX,
    MOD_BIN,
    MOD_OCT,
    MOD_CHAR,
} token_mod;

const char *token_type_names[] =
{
    [TOKEN_EOF] = "EOF",
    [TOKEN_COLON] = ":",
    [TOKEN_LPAREN] = "(",
    [TOKEN_RPAREN] = ")",
    [TOKEN_LBRACE] = "{",
    [TOKEN_RBRACE] = "}",
    [TOKEN_LBRACKET] = "[",
    [TOKEN_RBRACKET] = "]",
    [TOKEN_COMMA] = ",",
    [TOKEN_DOT] = ".",
    [TOKEN_AT] = "@",
    [TOKEN_QUESTION] = "?",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_KEYWORD] = "keyword",
    [TOKEN_INT] = "int",
    [TOKEN_FLOAT] = "float",
    [TOKEN_STR] = "string",
    [TOKEN_NAME] = "name",
    [TOKEN_NEG] = "~",
    [TOKEN_HAT] = "^",
    [TOKEN_NOT] = "!",
    [TOKEN_MUL] = "*",
    [TOKEN_DIV] = "/",
    [TOKEN_MOD] = "%",
    [TOKEN_AND] = "&",
    [TOKEN_LSHIFT] = "<<",
    [TOKEN_RSHIFT] = ">>",
    [TOKEN_ADD] = "+",
    [TOKEN_SUB] = "-",
    [TOKEN_OR] = "|",
    [TOKEN_EQ] = "==",
    [TOKEN_NOTEQ] = "!=",
    [TOKEN_LT] = "<",
    [TOKEN_GT] = ">",
    [TOKEN_LTEQ] = "<=",
    [TOKEN_GTEQ] = ">=",
    [TOKEN_AND_AND] = "&&",
    [TOKEN_OR_OR] = "||",
    [TOKEN_ASSIGN] = "=",
    [TOKEN_ADD_ASSIGN] = "+=",
    [TOKEN_SUB_ASSIGN] = "-=",
    [TOKEN_MUL_ASSIGN] = "*=",
    [TOKEN_DIV_ASSIGN] = "/=",
    [TOKEN_MOD_ASSIGN] = "%=",
    [TOKEN_LSHIFT_ASSIGN] = "<<=",
    [TOKEN_RSHIFT_ASSIGN] = ">>=",
    [TOKEN_INC] = "++",
    [TOKEN_DEC] = "--",
    [TOKEN_COLON_ASSIGN] = ":=",
    [TOKEN_ARROW] = "=>",
};

const char *token_type_name(token_type type)
{
    if(type < sizeof(token_type_names)/sizeof(*token_type_names))
    {
        return token_type_names[type];
    }
    else
    {
        return "<unknown>";
    }
}

const char* struct_keyword;
const char* enum_keyword;
const char* union_keyword;
const char* let_keyword;
const char* fn_keyword;
const char* const_keyword;
const char* if_keyword;
const char* else_keyword;
const char* for_keyword;
const char* while_keyword;
const char* switch_keyword;
const char* break_keyword;
const char* continue_keyword;
const char* err_keyword;
const char* import_keyword;
const char* extern_keyword;
const char* in_keyword;
const char* return_keyword;

const char* first_keyword;
const char* last_keyword;
const char** keywords;

token_type assign_token_to_binary_token[NUM_TOKEN_KINDS] =
{
    [TOKEN_ADD_ASSIGN] = TOKEN_ADD,
    [TOKEN_SUB_ASSIGN] = TOKEN_SUB,
    [TOKEN_LSHIFT_ASSIGN] = TOKEN_LSHIFT,
    [TOKEN_RSHIFT_ASSIGN] = TOKEN_RSHIFT,
    [TOKEN_MUL_ASSIGN] = TOKEN_MUL,
    [TOKEN_DIV_ASSIGN] = TOKEN_DIV,
    [TOKEN_MOD_ASSIGN] = TOKEN_MOD,
};

typedef struct
{
    token_type type;
    token_mod mod;
    const char* start;
    i32 length;
    i32 line;
    union
    {
        unsigned long long int_val;
        double float_val;
        const char* str_val;
        const char* name;
    };
} token;
