typedef struct typespec typespec;
typedef struct decl decl;
typedef struct expr expr;
typedef struct stmt stmt;

//Statement block, fuck this name
typedef struct
{
    stmt **stmt;
    size_t num_stmts;
}s_block;

typedef enum
{
    TYPESPEC_NONE,
    TYPESPEC_NAME,
    TYPESPEC_FUNC,
    TYPESPEC_ARRAY,
    TYPESPEC_PTR,
    TYPESPEC_CONST,
}typespec_type;

typedef struct
{
    size_t num_args; 
    typespec** args;
    typespec** rets;
    size_t num_rets;
}func_typespec;

typedef struct
{
    typespec* elem;
}ptr_typespec;

typedef struct
{
    typespec* elem;
    expr* size;
}array_typespec;

struct typespec
{
    typespec_type type;

    union 
    {
        const char* name;
        func_typespec func;
        ptr_typespec ptr;
        array_typespec array;
    };
};

typedef enum
{
    DECL_NONE,
    DECL_ENUM,
    DECL_ERR,
    DECL_STRUCT,
    DECL_UNION,
    DECL_VAR,
    DECL_CONST,
    DECL_FUNC,
}decl_type;

typedef struct 
{
    const char* name;
    expr* init;
}enum_item;

typedef struct
{
    enum_item* items;
    size_t num_items;
}enum_decl;

typedef struct
{
    const char* name;
    typespec* type;
	expr* init;
}aggregate_item;

typedef struct
{
    aggregate_item* items;
    size_t num_items;
}aggregate_decl;

typedef struct
{
    typespec* type;
    expr* expr;
}var_decl;

typedef struct
{
    expr* expr;
}const_decl;

typedef struct
{
    const char* name;
    typespec* type;
}func_item;

typedef struct
{
    func_item* param_list;
    size_t num_params;
    typespec** return_type;
    size_t num_return;
    s_block block;
}func_decl;

struct decl
{
    decl_type type;
    const char* name;
    union
    {
        enum_decl enum_decl;
        aggregate_decl aggregate_decl;
        var_decl var_decl;
        const_decl const_decl;
        func_decl func_decl;
    };
};

typedef enum
{
    EXPR_NONE,
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_STR,
    EXPR_NAME,
    EXPR_CAST,
    EXPR_CALL,
    EXPR_INDEX,
    EXPR_FIELD,
    EXPR_COMPOUND,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_TERNARY,
}expr_type;

typedef struct
{
    typespec* type;
    expr* expr;
}cast_expr;

typedef struct
{
    expr* expr;
    expr** args;
    size_t num_args;
}call_expr;

typedef struct
{
    expr* expr;
    expr* index;
}index_expr;

typedef struct
{
    expr* expr;
    const char* name;
}field_expr;

typedef struct
{
    typespec* type;
    expr** args;
    size_t num_args;
}compound_expr;

typedef struct
{
    token_type op;
    expr* expr;
}unary_expr;

typedef struct
{
    token_type op;
    expr* left;
    expr* right;
}binary_expr;

typedef struct
{
    expr* cond;
    expr* then_expr;
    expr* else_expr;
}ternary_expr;

struct expr
{
    expr_type type;
    union
    {
        u64 int_val;
        f64 float_val;
        const char* str_val;
        const char* name;
        cast_expr cast;
        call_expr call;
        index_expr index;
        field_expr field;
        compound_expr compound;
        unary_expr unary;
        binary_expr binary;
        ternary_expr ternary;
    };
};

typedef enum
{
    STMT_NONE,
    STMT_DECL,
    STMT_RETURN,
    STMT_BLOCK,
    STMT_IF,
    STMT_FOR,
    STMT_WHILE,
    STMT_SWITCH,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_INIT,
    STMT_ASSIGN,
    STMT_EXPR, 
}stmt_type;

typedef struct
{
    expr** expr;
	size_t num_exprs;
}return_stmt;

typedef struct
{
    expr* cond;
    s_block block;
}else_if;

typedef struct
{
    expr* cond;
    s_block then_block;
    else_if* elseifs;
    size_t num_elseifs;
    s_block else_block;
}if_stmt;

//TODO: For each loops
typedef struct
{
    stmt* init;
    expr* cond;
    stmt* next;
    s_block block;
}for_stmt;

typedef struct
{
    expr* cond;
    s_block block;
}while_stmt;

//TODO: lambdas for switch statements and in general
typedef struct
{
    expr **exprs;
    size_t num_exprs;
    bool is_default;
    s_block block;
}switch_case;

typedef struct
{
    expr* expr;
    switch_case* cases;
    size_t num_cases;
}switch_stmt;

typedef struct
{
    const char* name;
    expr* expr;
}init_stmt;

typedef struct
{
    token_type op;
    expr* left;
    expr* right;
}assign_stmt;

struct stmt
{
    stmt_type type;
    union 
    {
        return_stmt return_stmt;
        if_stmt if_stmt;
        for_stmt for_stmt;
        while_stmt while_stmt;
        switch_stmt switch_stmt;
        s_block block;
        init_stmt init;
        assign_stmt assign;
        expr* expr;
        decl* decl;
    };
};
