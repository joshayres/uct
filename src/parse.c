decl* parse_decl_opt();
decl* parse_decl();
typespec* parse_type();
stmt* parse_stmt();
expr* parse_expr();

typespec* parse_type_function()
{
	typespec** args = NULL;
	expect_token(TOKEN_LPAREN);
	if(!is_token(TOKEN_RPAREN))
	{
		buf_push(args, parse_type());
		while(match_token(TOKEN_COMMA))
		{
			buf_push(args, parse_type());
		}
	}
	expect_token(TOKEN_RPAREN);
	typespec** rets = NULL;
	if(match_token(TOKEN_COLON))
	{
		buf_push(rets, parse_type());
		while(match_token(TOKEN_COMMA))
		{
			buf_push(rets, parse_type());
		}
	}
	return typespec_func(ast_dup(args, buf_sizeof(args)), buf_len(args), ast_dup(rets, buf_sizeof(rets)), buf_len(rets));
}

typespec* parse_type_base()
{
	if(is_token(TOKEN_NAME))
	{
		const char* name = tok.name;
		next_token();
		return typespec_name(name);
	}
	else if(match_keyword(fn_keyword))
	{
		return parse_type_function();
	}
	else if(match_token(TOKEN_LPAREN))
	{
		typespec* type = parse_type();
		expect_token(TOKEN_RPAREN);
		return type;
	}
	fatal_syntax_error("Unexpected token %s in type", token_info());
	return NULL;
}

typespec* parse_type()
{
	typespec* type = parse_type_base();
	while(is_token(TOKEN_LBRACKET) || is_token(TOKEN_HAT))
	{
		if(match_token(TOKEN_LBRACKET))
		{
			expr* expr = NULL;
			if(!is_token(TOKEN_RBRACKET))
			{
				expr = parse_expr();
			}
			expect_token(TOKEN_RBRACKET);
			type = typespec_array(type, expr);
		}
		else
		{
			assert(is_token(TOKEN_HAT));
			next_token();
			type = typespec_ptr(type);
		}
	}
	return type;
}

expr* parse_expr_compound(typespec* type)
{
	expect_token(TOKEN_LBRACE);
	expr** args = NULL;
	if(!is_token(TOKEN_RBRACE))
	{
		buf_push(args, parse_expr());
		while(match_token(TOKEN_COMMA))
		{
			buf_push(args, parse_expr());
		}
	}
	expect_token(TOKEN_RBRACE);
	return expr_compound(type, ast_dup(args, buf_sizeof(args)), buf_len(args));
}

expr* parse_expr_operand()
{
	if(is_token(TOKEN_INT))
	{
		u64 val = tok.int_val;
		next_token();
		return expr_int(val);
	}
	else if(is_token(TOKEN_FLOAT))
	{
		f64 val = tok.float_val;
		next_token();
		return expr_float(val);
	}
	else if(is_token(TOKEN_STR))
	{
		const char* val = tok.str_val;
		next_token();
		return expr_str(val);
	}
	else if(is_token(TOKEN_NAME))
	{
		const char* name = tok.name;
		next_token();
		if(is_token(TOKEN_LBRACE))
		{
			return parse_expr_compound(typespec_name(name));
		}
		return expr_name(name);
	}
	else if(is_token(TOKEN_LBRACE))
	{
		return parse_expr_compound(NULL);
	}
	else if(is_token(TOKEN_LPAREN))
	{
		if(match_token(TOKEN_COLON))
		{
			typespec* type = parse_type();
			expect_token(TOKEN_RPAREN);
			return parse_expr_compound(type);
		}
		else
		{
			expr* expr = parse_expr();
			expect_token(TOKEN_RPAREN);
			return expr;
		}
	}
	else
	{
		fatal_syntax_error("Unexpected token %s in expression", token_info());
		return NULL;
	}
}

expr* parse_expr_base()
{
	expr* exp = parse_expr_operand();
	while(is_token(TOKEN_LPAREN) || is_token(TOKEN_LBRACKET) || is_token(TOKEN_DOT))
	{
		if(match_token(TOKEN_LPAREN))
		{
			expr** args = NULL;
			if(!is_token(TOKEN_RPAREN))
			{
				buf_push(args, parse_expr());
				while(match_token(TOKEN_COMMA))
				{
					buf_push(args, parse_expr());
				}
			}
			expect_token(TOKEN_RPAREN);
			exp = expr_call(exp, ast_dup(args, buf_sizeof(args)), buf_len(args));
		}
		if(match_token(TOKEN_LBRACKET))
		{
			 expr* index = parse_expr();
			 expect_token(TOKEN_RBRACKET);
			 exp = expr_index(exp, index);
		}
		else
		{
			next_token();
			const char* field = tok.name;
			expect_token(TOKEN_NAME);
			exp = expr_field(exp, field);
		}
	}

	return exp;
}

bool is_unary_op()
{
	return is_token(TOKEN_ADD) || is_token(TOKEN_SUB) || is_token(TOKEN_MUL) || is_token(TOKEN_AND);
}

expr* parse_expr_unary()
{
	if(is_unary_op())
	{
		token_type op = tok.type;
		next_token();
		return expr_unary(op, parse_expr_unary());
	}
	else
	{
		return parse_expr_base();
	}
}

bool is_mul_op()
{
	return TOKEN_FIRST_MUL <= tok.type && tok.type <= TOKEN_LAST_MUL;
}

expr* parse_expr_mul()
{
	expr* expr = parse_expr_unary();
	while(is_mul_op())
	{
		token_type op = tok.type;
		next_token();
		expr = expr_binary(op, expr, parse_expr_unary());
	}
	return expr;
}

bool is_add_op()
{
	return TOKEN_FIRST_ADD <= tok.type && tok.type <= TOKEN_LAST_ADD;
}

expr* parse_expr_add()
{
	expr* expr = parse_expr_mul();
	while(is_add_op())
	{
		token_type op = tok.type;
		next_token();
		expr = expr_binary(op, expr, parse_expr_mul());
	}
	return expr;
}

bool is_cmp_op()
{
	return TOKEN_FIRST_CMP <= tok.type && tok.type <= TOKEN_LAST_CMP;
}

expr* parse_expr_cmp()
{
	expr* expr = parse_expr_add();
	while(is_cmp_op())
	{
		token_type op = tok.type;
		next_token();
		expr = expr_binary(op, expr, parse_expr_add());
	}
	return expr;
}

expr* parse_expr_and()
{
	expr* expr = parse_expr_cmp();
	while(match_token(TOKEN_AND_AND))
	{
		expr = expr_binary(TOKEN_AND_AND, expr, parse_expr_cmp());
	}
	return expr;
}

expr* parse_expr_or()
{
	expr* expr = parse_expr_and();
	while(match_token(TOKEN_OR_OR))
	{
		expr = expr_binary(TOKEN_OR_OR, expr, parse_expr_and());
	}
	return expr;
}

expr* parse_expr_ternary()
{
	expr* exp = parse_expr_or();
	if(match_token(TOKEN_QUESTION))
	{
		expr* then_expr = parse_expr_ternary();
		expect_token(TOKEN_COLON);
		expr* else_expr = parse_expr_ternary();
		exp = expr_ternary(exp, then_expr, else_expr);
	}
	return exp;
}

expr* parse_expr()
{
	parse_expr_ternary();
}

expr* parse_paren_expr()
{
	expect_token(TOKEN_LPAREN);
	expr* expr = parse_expr();
	expect_token(TOKEN_LPAREN);
	return expr;
}

s_block parse_stmt_block()
{
	expect_token(TOKEN_LBRACE);
	stmt** stmts = NULL;
	while(!is_token_eof() && !is_token(TOKEN_RBRACE))
	{
		buf_push(stmts, parse_stmt());
	}
	expect_token(TOKEN_RBRACE);
	return (s_block){ast_dup(stmts, buf_sizeof(stmts)), buf_len(stmts)};
}

stmt* parse_stmt_if()
{
	expr* cond = parse_paren_expr();
	s_block then_block = parse_stmt_block();
	s_block else_block = {0};
	else_if* elseifs = NULL;
	while(match_keyword(else_keyword))
	{
		if(!match_keyword(if_keyword))
		{
			else_block = parse_stmt_block();
			break;
		}
		expr* elseif_cond = parse_paren_expr();
		s_block elseif_block = parse_stmt_block();
		buf_push(elseifs, (else_if){elseif_cond, elseif_block});
	}
	return stmt_if(cond, then_block, ast_dup(elseifs, buf_sizeof(elseifs)), buf_len(elseifs), else_block);
}

stmt* parse_stmt_while()
{
	expr* cond = parse_paren_expr();
	return stmt_while(cond, parse_stmt_block());
}

bool is_assign_op()
{
	return TOKEN_FIRST_ASSIGN <= tok.type && tok.type <= TOKEN_LAST_ASSIGN;
}

stmt* parse_simple_stmt()
{
	expr* exp = parse_expr();
	stmt* stmt;
	if(match_token(TOKEN_COLON_ASSIGN))
	{
		if(exp->type != EXPR_NAME)
		{
			fatal_syntax_error(":= must be preceded by a name");
			return NULL;
		}
		stmt = stmt_init(exp->name, parse_expr());
	}
	else if(is_assign_op())
	{
		token_type op = tok.type;
		next_token();
		stmt = stmt_assign(op, exp, parse_expr());
	}
	else if(is_token(TOKEN_INC) || is_token(TOKEN_DEC))
	{
		token_type op = tok.type;
		next_token();
		stmt = stmt_assign(op, exp, NULL);
	}
	else
	{
		stmt = stmt_expr(exp);
	}

	return stmt;
}

stmt* parse_stmt_for()
{
	expect_token(TOKEN_LPAREN);
	stmt* init = NULL;
	if(!is_token(TOKEN_SEMICOLON))
	{
		init = parse_simple_stmt();
	}
	expect_token(TOKEN_SEMICOLON);
	expr* cond = NULL;
	if(!is_token(TOKEN_SEMICOLON))
	{
		cond = parse_expr();
	}
	expect_token(TOKEN_SEMICOLON);
	stmt* next = NULL;
	if(!is_token(TOKEN_RPAREN))
	{
		next = parse_simple_stmt();
		if(next->type == STMT_INIT)
		{
			syntax_error("Init statements not allowed in for-statement's next clause");
		}
	}
	expect_token(TOKEN_RPAREN);
	return stmt_for(init, cond, next, parse_stmt_block());
}

switch_case parse_stmt_switch_case()
{
	expr** exprs = NULL;
	bool is_default = false;
	while(is_token(TOKEN_ARROW) || is_token(TOKEN_UNDERSCORE))
	{
		if(match_token(TOKEN_ARROW))
		{
			buf_push(exprs, parse_expr());
		}
		else
		{
			next_token();
			if(is_default)
			{
				syntax_error("Duplicate default labels in switch case");
			}
			is_default = true;
		}
	}
	stmt** stmts = NULL;
	while(!is_token_eof() && !is_token(TOKEN_RBRACE) && !is_token(TOKEN_ARROW) && !is_token(TOKEN_UNDERSCORE))
	{
		buf_push(stmts, parse_stmt());
	}
	s_block block = {ast_dup(stmts, buf_sizeof(stmts)), buf_len(stmts)};
	return (switch_case){ast_dup(exprs, buf_sizeof(exprs)), buf_len(exprs), is_default, block};
}

stmt* parse_stmt_switch()
{
	expr* expr = parse_paren_expr();
	switch_case *cases = NULL;
	expect_token(TOKEN_LBRACE);
	while(!is_token_eof() && !is_token(TOKEN_RBRACE))
	{
		buf_push(cases, parse_stmt_switch_case());
	}
	expect_token(TOKEN_RBRACE);
	return stmt_switch(expr, ast_dup(cases, buf_sizeof(cases)), buf_len(cases));
}

stmt* parse_stmt_return()
{
	expr** exprs = NULL;
	if(!match_token(TOKEN_SEMICOLON))
	{
		buf_push(exprs, parse_expr());
		while(match_token(TOKEN_COMMA))
		{
			buf_push(exprs, parse_expr());
		}
	}
	return stmt_return(ast_dup(exprs, buf_sizeof(exprs)), buf_len(exprs));
}

stmt* parse_stmt()
{
	if(match_keyword(if_keyword))
	{
		return parse_stmt_if();
	}
	else if(match_keyword(while_keyword))
	{
		return parse_stmt_while();
	}
	else if(match_keyword(for_keyword))
	{
		return parse_stmt_for();
	}
	else if(is_token(TOKEN_LBRACE))
	{
		return stmt_block(parse_stmt_block());
	}
	else if(match_keyword(break_keyword))
	{
		expect_token(TOKEN_SEMICOLON);
		return stmt_break();
	}
	else if(match_keyword(continue_keyword))
	{
		expect_token(TOKEN_SEMICOLON);
		return stmt_continue();
	}
	else if(match_keyword(return_keyword))
	{
		return parse_stmt_return();
	}
	else
	{
		decl* decl = parse_decl_opt();
		stmt* stmt;
		if(decl)
		{
			stmt = stmt_decl(decl);
		}
		else
		{
			stmt = parse_simple_stmt();
		}
		expect_token(TOKEN_SEMICOLON);
		return stmt;
	}
}

const char* parse_name()
{
	const char* name = tok.name;
	expect_token(TOKEN_NAME);
	return name;
}

enum_item parse_decl_enum_item()
{
	const char* name = parse_name();
	expr* init = NULL;
	if(match_token(TOKEN_ASSIGN))
	{
		init = parse_expr();
	}
	return (enum_item){name, init};
}

//TODO: Figure out if I want enums and errs to be different
decl* parse_decl_enum()
{
	const char* name = parse_name();
	expect_token(TOKEN_LBRACE);
	enum_item* items = NULL;
	if(!is_token(TOKEN_RBRACE))
	{
		buf_push(items, parse_decl_enum_item());
		while(match_token(TOKEN_COMMA))
		{
			buf_push(items, parse_decl_enum_item());
		}
	}
	expect_token(TOKEN_RBRACE);
	return decl_enum(name, ast_dup(items, buf_sizeof(items)), buf_len(items));
}

aggregate_item parse_decl_aggregate_item()
{
	const char* name = parse_name();
	expect_token(TOKEN_COLON);
	typespec* type = parse_type();
	expr* init = NULL;
	if(match_token(TOKEN_ASSIGN))
	{
		init = parse_expr();
	}
	expect_token(TOKEN_SEMICOLON);
	return (aggregate_item){name, type, init};
}

decl* parse_decl_aggregate(decl_type type)
{
	assert(type == DECL_STRUCT || type == DECL_UNION);
	const char* name = parse_name();
	expect_token(TOKEN_LBRACE);
	aggregate_item* items = NULL;
	while(!is_token_eof() && !is_token(TOKEN_RBRACE))
	{
		buf_push(items, parse_decl_aggregate_item());
	}
	expect_token(TOKEN_RBRACE);
	return decl_aggregate(type, name, ast_dup(items, buf_sizeof(items)), buf_len(items));
}

decl* parse_decl_var()
{
	const char* name = parse_name();
	if(match_token(TOKEN_ASSIGN))
	{
		return decl_var(name, NULL, parse_expr());
	}
	else if(match_token(TOKEN_COLON))
	{
		typespec* type = parse_type();
		expr* expr = NULL;
		if(match_token(TOKEN_ASSIGN))
		{
			expr = parse_expr();
		}
		return decl_var(name, type, expr);
	}
	else
	{
		fatal_syntax_error("Expected : or = after let, got %s", token_info());
		return NULL;
	}
}

decl* parse_decl_const()
{
	const char* name = parse_name();
	expect_token(TOKEN_ASSIGN);
	return decl_const(name, parse_expr());
}

//TODO: Maybe support initial values for functions
func_item parse_decl_func_param()
{
	const char* name = parse_name();
	expect_token(TOKEN_COLON);
	typespec* type = parse_type();
	return (func_item){name, type};
}

decl* parse_decl_func()
{
	const char* name = parse_name();
	expect_token(TOKEN_LPAREN);
	func_item* params = NULL;
	if(!is_token(TOKEN_RPAREN))
	{
		buf_push(params, parse_decl_func_param());
		while(match_token(TOKEN_RPAREN))
		{
			buf_push(params, parse_decl_func_param());
		}
	}
	expect_token(TOKEN_RPAREN);
	typespec** ret_types = NULL;
	if(match_token(TOKEN_COLON))
	{
		buf_push(ret_types, parse_type());
		while(match_token(TOKEN_COMMA))
		{
			buf_push(ret_types, parse_type());
		}
	}

	s_block block = parse_stmt_block();
	return decl_func(name, ast_dup(params, buf_sizeof(params)), buf_len(params), ast_dup(ret_types, buf_sizeof(ret_types)), buf_len(ret_types), block);
}

decl* parse_decl_opt()
{
	if(match_keyword(enum_keyword))
	{
		//TODO: err support
		return parse_decl_enum();
	}
	else if(match_keyword(struct_keyword))
	{
		return parse_decl_aggregate(DECL_STRUCT);
	}
	else if(match_keyword(union_keyword))
	{
		return parse_decl_aggregate(DECL_UNION);
	}
	else if(match_keyword(let_keyword))
	{
		return parse_decl_var();
	}
	else if(match_keyword(const_keyword))
	{
		return parse_decl_const();
	}
	else if(match_keyword(fn_keyword))
	{
		return parse_decl_func();
	}
	else
	{
		return NULL;
	}
}

decl* parse_decl()
{
	decl* decl = parse_decl_opt();
	if(!decl)
	{
		fatal_syntax_error("Expected declaration keyword, got %s", token_info());
	}
	return decl;
}
