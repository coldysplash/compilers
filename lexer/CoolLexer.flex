%{
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>

#include "Parser.h"
#include "CoolLexer.h"

#undef YY_DECL
#define YY_DECL int CoolLexer::yylex()

%}

white_space       [ \t]*

%option warn nodefault batch noyywrap c++
%option yylineno
%option yyclass="CoolLexer"

%%

(?i:class)           return TOKEN_KEYWORD_CLASS;

(?i:else)           return TOKEN_KEYWORD_ELSE;
(?i:fi)             return TOKEN_KEYWORD_FI;
(?i:if)             return TOKEN_KEYWORD_ELSE;
(?i:in)             return TOKEN_KEYWORD_IN;
(?i:inherits)       return TOKEN_KEYWORD_INHERITS;
(?i:isvoid)         return TOKEN_KEYWORD_ISVOID;
(?i:let)            return TOKEN_KEYWORD_LET;
(?i:loop)           return TOKEN_KEYWORD_LOOP;
(?i:pool)           return TOKEN_KEYWORD_POOL;
(?i:then)           return TOKEN_KEYWORD_THEN;
(?i:while)          return TOKEN_KEYWORD_WHILE;
(?i:case)           return TOKEN_KEYWORD_CASE;
(?i:esac)           return TOKEN_KEYWORD_ESAC;
(?i:new)            return TOKEN_KEYWORD_NEW;
(?i:of)             return TOKEN_KEYWORD_OF;
(?i:not)            return TOKEN_KEYWORD_NOT;
t(?i:rue)           return TOKEN_KEYWORD_TRUE;
f(?i:alse)          return TOKEN_KEYWORD_FALSE;

{white_space}        { /* skip spaces */ }
\n                   lineno++;
.                    Error("unrecognized character");

%%

void CoolLexer::Error(const char* msg) const
{
    std::cerr << "Lexer error (line " << lineno + 1 << "): " << msg << ": lexeme '" << YYText() << "'\n";
    std::exit(YY_EXIT_FAILURE);
}
