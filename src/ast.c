#include "ast.h"

arena ast_arena;

void* ast_alloc(size_t size)
{
    void* ptr = arena_alloc(&ast_arena, size);
    memset(ptr, 0, size);
    return ptr;
}

void* ast_dup(const void* src, size_t size)
{
    if(size == 0)
    {
        return NULL;
    }
    void* ptr = arena_alloc(&ast_arena, size);
    memcpy(ptr, src, size);
    return ptr;
}

typespec* typespec_new(typespec_type type)
{
    typespec* t = ast_alloc(sizeof(typespec));
    t->type = type;
    return t;
}

typespec* typespec_name(const char* name)
{
    typespec* t = typespec_new(TYPESPEC_NAME);
    t->name = name;
    return t;
}

typespec* typespec_func(typespec** args, size_t num_args, typespec** rets, size_t num_rets)
{
    typespec* t = typespec_new(TYPESPEC_FUNC);
    t->func.args = args;
    t->func.num_args = num_args;
    t->func.rets = rets;
    t->func.num_rets = num_rets;
    return t;
}

typespec* typespec_array(typespec* elem, expr* size)
{
    typespec* t = typespec_new(TYPESPEC_ARRAY);
    t->array.elem = elem;
    t->array.size = size;
    return t;
}

typespec* typespec_ptr(typespec* elem)
{
    typespec* t = typespec_new(TYPESPEC_PTR);
    t->ptr.elem = elem;
    return t;
}

decl* decl_new(decl_type type, const char* name)
{
    decl* d = ast_alloc(sizeof(decl));
    d->type = type;
    d->name = name;
    return d;
}

decl* decl_enum(const char* name, enum_item* items, size_t num_items)
{
    decl* d = decl_new(DECL_ENUM, name);
    d->enum_decl.items = items;
    d->enum_decl.num_items = num_items;
    return d;
}

decl* decl_aggregate(decl_type type, const char* name, aggregate_item* items, size_t num_items)
{
    assert(type == DECL_STRUCT || type == DECL_UNION);
    decl* d = decl_new(type, name);
    d->aggregate_decl.items = items;
    d->aggregate_decl.num_items = num_items;
    return d;
}

decl* decl_var(const char* name, typespec* type, expr* expr)
{
    decl* d = decl_new(DECL_VAR, name);
    d->var_decl.type = type;
    d->var_decl.expr = expr;
    return d;
}

decl* decl_const(const char* name, expr* expr)
{
    decl* d = decl_new(DECL_CONST, name);
    d->const_decl.expr = expr;
    return d;
}

decl* decl_func(const char* name, func_item* param_list, size_t num_params, typespec** return_type, size_t num_return, s_block block)
{
    decl* d = decl_new(DECL_FUNC, name);
    d->func_decl.param_list = param_list;
    d->func_decl.num_params = num_params;
    d->func_decl.return_type = return_type;
    d->func_decl.num_return = num_return;
    d->func_decl.block = block;
    return d;
}

expr* expr_new(expr_type type)
{
    expr* e = ast_alloc(sizeof(expr));
    e->type = type;
    return e;
}

expr* expr_int(u64 int_val)
{
    expr* e = expr_new(EXPR_INT);
    e->int_val = int_val;
    return e;
}

expr* expr_float(f64 float_val)
{
    expr* e = expr_new(EXPR_FLOAT);
    e->float_val = float_val;
    return e;
}

expr* expr_str(const char* str)
{
    expr* e = expr_new(EXPR_STR);
    e->str_val = str;
    return e;
}

expr* expr_name(const char* name)
{
    expr* e = expr_new(EXPR_NAME);
    e->name = name;
    return e;
}

expr* expr_cast(typespec* type, expr* exp)
{
    expr* e = expr_new(EXPR_CAST);
    e->cast.expr = exp;
}

expr* expr_call(expr* exp, expr** args, size_t num_args)
{
    expr* e = expr_new(EXPR_CALL);
    e->call.expr = exp;
    e->call.args = args;
    e->call.num_args = num_args;
    return e;
}

expr* expr_index(expr* exp, expr* index)
{
    expr* e = expr_new(EXPR_INDEX);
    e->index.expr = exp;
    e->index.index = index;
    return e;
}

expr* expr_field(expr* exp, const char* name)
{
    expr* e = expr_new(EXPR_FIELD);
    e->field.expr = exp;
    e->field.name = name;
}

expr* expr_compound(typespec* type, expr** args, size_t num_args)
{
    expr* e = expr_new(EXPR_COMPOUND);
    e->compound.type = type;
    e->compound.args = args;
    e->compound.num_args = num_args;
    return e;
}

expr* expr_unary(token_type op, expr* exp)
{
    expr* e = expr_new(EXPR_UNARY);
    e->unary.op = op;
    e->unary.expr = exp;
    return e;
}

expr* expr_binary(token_type op, expr* left, expr* right)
{
    expr* e = expr_new(EXPR_BINARY);
    e->binary.op = op;
    e->binary.left = left;
    e->binary.right = right;
    return e;
}

expr* expr_ternary(expr* cond, expr* then_expr, expr* else_expr)
{
    expr* e = expr_new(EXPR_TERNARY);
    e->ternary.cond = cond;
    e->ternary.then_expr = then_expr;
    e->ternary.else_expr = else_expr;
    return e;
}

stmt* stmt_new(stmt_type type)
{
    stmt* s = ast_alloc(sizeof(stmt));
    s->type = type;
    return s;
}

stmt* stmt_decl(decl* decl)
{
    stmt* s = stmt_new(STMT_DECL);
    s->decl = decl;
    return s;
}

stmt* stmt_return(expr* expr)
{
    stmt* s = stmt_new(STMT_RETURN);
    s->return_stmt.expr = expr;
    return s;
}

stmt* stmt_block(s_block block)
{
    stmt* s = stmt_new(STMT_BLOCK);
    s->block = block;
    return s;
}

stmt* stmt_if(expr* cond, s_block then_block, else_if* elseifs, size_t num_elseifs, s_block else_block)
{
    stmt* s = stmt_new(STMT_IF);
    s->if_stmt.cond = cond;
    s->if_stmt.then_block = then_block;
    s->if_stmt.elseifs = elseifs;
    s->if_stmt.num_elseifs = num_elseifs;
    s->if_stmt.else_block = else_block;
    return s;
}

stmt* stmt_for(stmt* init, expr* cond, stmt* next, s_block block)
{
    stmt* s = stmt_new(STMT_FOR);
    s->for_stmt.init = init;
    s->for_stmt.cond = cond;
    s->for_stmt.next = next;
    s->for_stmt.block = block;
    return s;
}

stmt* stmt_while(expr* cond, s_block block)
{
    stmt* s = stmt_new(STMT_WHILE);
    s->while_stmt.cond = cond;
    s->while_stmt.block = block;
    return s;
}

stmt* stmt_switch(expr* expr, switch_case* cases, size_t num_cases)
{
    stmt* s = stmt_new(STMT_SWITCH);
    s->switch_stmt.expr = expr;
    s->switch_stmt.cases = cases;
    s->switch_stmt.num_cases = num_cases;
    return s;
}

stmt* stmt_break()
{
    return stmt_new(STMT_BREAK);
}

stmt* stmt_continue()
{
    return stmt_new(STMT_CONTINUE);
}

stmt* stmt_init(const char* name, expr* expr)
{
    stmt* s = stmt_new(STMT_INIT);
    s->init.name = name;
    s->init.expr = expr;
    return s;
}

stmt* stmt_assign(token_type op, expr* left, expr* right)
{
    stmt* s = stmt_new(STMT_ASSIGN);
    s->assign.op = op;
    s->assign.left = left;
    s->assign.right = right;
    return s;
}

stmt* stmt_expr(expr* exp)
{
    stmt* s = stmt_new(STMT_EXPR);
    s->expr = exp;
    return s;
}
