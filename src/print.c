void print_expr(expr* exp);
void print_stmt(stmt* stmt);
void print_decl(decl* decl);

i32 indent;

void print_typespec(typespec* type)
{
	typespec* t = type;
	switch(t->type)
	{
	case TYPESPEC_NAME:
		printf("%s", t->name);
		break;
	case TYPESPEC_FUNC:
		printf("(fn (");
		for each(it, t->func.args)
		{
			printf(" ");
			print_typespec(it);
		}
		printf(" ) ");
		for each(it, t->func.rets)
		{
			printf(" ");
			print_typespec(it);
		}
		printf(")");
		break;
	case TYPESPEC_ARRAY:
		printf("(array ");
		print_typespec(t->array.elem);
		printf(" ");
		print_expr(t->array.size);
		printf(")");
	case TYPESPEC_PTR:
		printf("(ptr ");
		print_typespec(t->ptr.elem);
		printf(")");
		break;
	default:
		assert(0);
		break;
	}
}

void print_expr(expr* exp)
{
	expr* e = exp;
	switch(e->type)
	{
	case EXPR_INT:
		printf("%d", e->int_val);
		break;
	case EXPR_FLOAT:
		printf("%f", e->float_val);
		break;
	case EXPR_STR:
		printf("\"%s\"", e->str_val);
		break;
	case EXPR_NAME:
		printf("%s", e->name);
		break;
	case EXPR_CAST:
		printf("(case ");
		print_typespec(e->cast.type);
		printf(" ");
		print_expr(e->cast.expr);
		printf(")");
		break;
	case EXPR_CALL:
		printf("(");
		print_expr(e->call.expr);
		for each(it, e->call.args)
		{
			printf(" ");
			print_expr(it);
		}
		printf(")");
		break;
	case EXPR_INDEX:
		printf("(index ");
		print_expr(e->index.expr);
		printf(" ");
		print_expr(e->index.index);
		printf(")");
		break;
	case EXPR_FIELD:
		printf("(field ");
		print_expr(e->field.expr);
		printf(" %s)", e->field.name);
		break;
	case EXPR_COMPOUND:
		printf("(compound ");
		if(e->compound.type)
		{
			print_typespec(e->compound.type);
		}
		else
		{
			printf("nil");
		}
		for each(arg, e->compound.args)
		{
			printf(" ");
			print_expr(arg);
		}
		printf(")");
		break;
	case EXPR_UNARY:
		printf("(%s ", token_type_name(e->unary.op));
		print_expr(e->unary.expr);
		printf(")");
		break;
	case EXPR_BINARY:
		printf("(%s ", token_type_name(e->binary.op));
        print_expr(e->binary.left);
        printf(" ");
        print_expr(e->binary.right);
        printf(")");
        break;
    case EXPR_TERNARY:
        printf("(? ");
        print_expr(e->ternary.cond);
        printf(" ");
        print_expr(e->ternary.then_expr);
        printf(" ");
        print_expr(e->ternary.else_expr);
        printf(")");
        break;
    default:
        assert(0);
        break;
	}
}

void print_stmt_block(s_block block)
{
	printf("(block");
	indent++;
	for (stmt **it = block.stmt; it != block.stmt + block.num_stmts; it++)
	{
        printf("\n");
        print_stmt(*it);
    }
	indent--;
	printf(")");
}

void print_stmt(stmt *stm)
{
	stmt *s = stm;
	switch(s->type)
	{
	case STMT_DECL:
        print_decl(s->decl);
        break;
	case STMT_RETURN:
		printf("(return");
		if(s->return_stmt.expr)
		{
			for(expr **ret = s->return_stmt.expr; ret != s->return_stmt.expr + s->return_stmt.num_exprs; ret++)
			{
				printf(" ");
				print_expr(*ret);
			}
		}
		printf(")");
		break;
	case STMT_BREAK:
		printf("(break)");
		break;
	case STMT_CONTINUE:
		printf("(continue)");
		break;
	case STMT_IF:
		printf("(if ");
		print_expr(s->if_stmt.cond);
		indent++;
		printf("\n");
		print_stmt_block(s->if_stmt.then_block);
		for each(else_if, s->if_stmt.elseifs)
		{
			printf("\n");
			printf("else if ");
			print_expr(else_if.cond);
			printf("\n");
			print_stmt_block(else_if.block);
		}
		if(s->if_stmt.else_block.num_stmts != 0)
		{
			printf("\n");
			printf("else ");
			printf("\n");
			print_stmt_block(s->if_stmt.else_block);
		}
		indent--;
		printf(")");
		break;
	case STMT_WHILE:
		printf("(while ");
		print_expr(s->while_stmt.cond);
		indent++;
		printf("\n");
		print_stmt_block(s->while_stmt.block);
		indent--;
		printf(")");
		break;
	case STMT_FOR:
		printf("(for ");
        print_stmt(s->for_stmt.init);
        print_expr(s->for_stmt.cond);
        print_stmt(s->for_stmt.next);
        indent++;
        printf("\n");
        print_stmt_block(s->for_stmt.block);
        indent--;
        printf(")");
        break;
	case STMT_SWITCH:
		printf("(switch ");
		print_expr(s->switch_stmt.expr);
		indent++;
		for each(it, s->switch_stmt.cases)
		{
			printf("\n");
			printf("(case (%s", it.is_default ? " default" : "");
			for(expr** exp = it.exprs; exp != it.exprs + it.num_exprs; exp++)
			{
				printf(" ");
				print_expr(*exp);
			}
			printf(" ) ");
			indent++;
			printf("\n");
			print_stmt_block(it.block);
			indent--;
		}
		indent--;
		printf(")");
		break;
	case STMT_ASSIGN:
        printf("(%s ", token_type_names[s->assign.op]);
        print_expr(s->assign.left);
        if (s->assign.right) {
            printf(" ");
            print_expr(s->assign.right);
        }
        printf(")");
        break;
    case STMT_INIT:
        printf("(:= %s ", s->init.name);
        print_expr(s->init.expr);
        printf(")");
        break;
    case STMT_EXPR:
        print_expr(s->expr);
        break;
    default:
        assert(0);
        break;
	}
}

void print_aggregate_decl(decl* dec)
{
	decl* d = dec;
	for (aggregate_item *it = d->aggregate_decl.items; it != d->aggregate_decl.items + d->aggregate_decl.num_items; it++)
	{
		printf("\n");
		printf("(");
		print_typespec(it->type);
		printf(" %s", it->name);
		if(it->init)
		{
			print_expr(it->init);
		}
		else
		{
			printf("nil");
		}
		printf(")");
	}
}

void print_decl(decl* dec)
{
	decl* d = dec;
	switch(d->type)
	{
	case DECL_ENUM:
		printf("(enum %s", d->name);
		indent++;
		for(enum_item *item = d->enum_decl.items; item != d->enum_decl.items + d->enum_decl.num_items; item++)
		{
			printf("\n");
			printf("(%s ", item->name);
			if(item->init)
			{
				print_expr(item->init);
			}
			else
			{
				printf("nil");
			}
			printf(")");
		}
		indent--;
		printf(")");
		break;
	case DECL_STRUCT:
		printf("(struct %s", d->name);
		indent++;
		print_aggregate_decl(d);
		indent--;
		printf(")");
		break;
	case DECL_UNION:
		printf("(union %s ", d->name);
		indent++;
		print_aggregate_decl(d);
		indent--;
		printf(")");
		break;
	case DECL_VAR:
		printf("(let %s ", d->name);
		if(d->var_decl.type)
		{
			print_typespec(d->var_decl.type);
		}
		else
		{
			printf("nil");
		}
		printf(" ");
		print_expr(d->var_decl.expr);
		printf(")");
		break;
	case DECL_CONST:
        printf("(const %s ", d->name);
        print_expr(d->const_decl.expr);
        printf(")");
        break;
	case DECL_FUNC:
		printf("(fn %s ", d->name);
		printf("(");
		for(func_item* param = d->func_decl.param_list; param != d->func_decl.param_list + d->func_decl.num_params; param++)
		{
			printf(" %s ", param->name);
			print_typespec(param->type);
		}
		printf(" ) ");
		if(d->func_decl.return_type)
		{
			for(typespec** ret = d->func_decl.return_type; ret != d->func_decl.return_type + d->func_decl.num_return; ret++)
			{
				print_typespec(*ret);
				printf(" ");
			}
		}
		else
		{
			printf("nil");
		}
		indent++;
		printf("\n");
		print_stmt_block(d->func_decl.block);
		indent--;
		printf(")");
		break;
	default:
		assert(0);
		break;
	}
}
