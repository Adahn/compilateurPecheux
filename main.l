%{
#include <stdlib.h>
#include "ast.h"
#include "y.tab.h"

%}

%%

"and"           {return T_AND;}
"begin"         {return T_BEGIN;}
"do"            {return T_DO;}
"end"           {return T_END;}
"else"          {return T_ELSE;}
"function"      {return T_FUNCTION;}
"if"            {return T_IF;}
"not"           {return T_NOT;}
"or"            {return T_OR;}
"program"       {return T_PROGRAM;}
"return"        {return T_RETURN;}
"skip"          {return T_SKIP;}
"then"          {return T_THEN;}
"var"           {return T_VAR;}
"while"         {return T_WHILE;}
"write"         {return T_WRITE;}

[a-zA-Z][a-zA-Z0-9_]*   {
                		yylval.s = strdup(yytext);
                		return T_VARIABLE;
            		}

[0-9]* 			{
                		yylval.f = atof(yytext);
                		return T_NUMERIC;
            		}

[0-9]+\.[0-9]+ 			{
                		yylval.f = atof(yytext);
                		return T_NUMERIC;
            		}

">="            {return T_GE;}
"<="            {return T_LE;}
"=="            {return T_EQ;}
"!="            {return T_NE;}

[ \t\n]+
.               { return *yytext; }

%%
int yywrap(void) {
    return 1;
}
