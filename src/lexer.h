typedef enum
{
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STR,

    TOKEN_EOF,
} token_type;

typedef enum
{
    MOD_NONE,
    MOD_HEX,
    MOD_BIN,
    MOD_OCT,
    MOD_CHAR,
} token_mod;

typedef struct
{
    token_type type;
    token_mod mod;
    const char* start;
    int length;
    int line;
    union
    {
        unsigned long long int_val;
        double float_val;
        const char* str_val;
        const char* name;
    };
} token;
