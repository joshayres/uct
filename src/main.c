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
#include "parse.c"
#include "print.c"

int main(int argc, char **argv)
{
    init_keywords();
	init_lex(read_file("test.uct"));
	decl* d = parse_decl();
	print_decl(d);
	printf("\n");

	return 0;
}
