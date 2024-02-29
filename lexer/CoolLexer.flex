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

white_space       [ \t\f\b\r]*
digits            [0-9]

%Start            STR
%x                COMMENT

%option warn nodefault batch noyywrap c++
%option yylineno
%option yyclass="CoolLexer"

%%

<INITIAL>(\")           {   yymore(); BEGIN(STR);   }
<STR>\n                 {   Error("Wrong newline in string"); 
                            BEGIN(INITIAL); 
                            lineno++; 
                            return TOKEN_BAD_STRING;
                        }
<STR><<EOF>>            {   Error("EOF in string"); 
                            BEGIN(INITIAL); 
                            return TOKEN_BAD_STRING; 
                        }
<STR>\0                 {   BEGIN(INITIAL); 
                            Error("Can't use \\0 in strings");  
                            yymore(); 
                            return TOKEN_BAD_STRING; 
                        }

<STR>[^\\\"\n]*         {yymore();}
<STR>\\[^\n]            {yymore();}
<STR>\\\n               {lineno++; yymore();}
<STR>\"                 {Escape(); BEGIN(INITIAL); return TOKEN_STRING;}

--.*                      { }
"*)"                      { Error("Unmatched comment ending"); BEGIN(INITIAL); return ERROR; }
"(*"                      { BEGIN(COMMENT); comment_level = 0; }
<COMMENT>"(*"             { comment_level++; }
<COMMENT><<EOF>>          { Error("EOF in comment"); BEGIN(INITIAL); return ERROR; }
<COMMENT>\n               { lineno++; }
<COMMENT>.                { }
<COMMENT>"*)"             {
                            if (comment_level == 0) {
                                BEGIN(INITIAL);
                            }
                            comment_level--;
                          }

(?i:class)              return TOKEN_KEYWORD_CLASS;
(?i:else)               return TOKEN_KEYWORD_ELSE;
(?i:fi)                 return TOKEN_KEYWORD_FI;
(?i:if)                 return TOKEN_KEYWORD_ELSE;
(?i:in)                 return TOKEN_KEYWORD_IN;
(?i:inherits)           return TOKEN_KEYWORD_INHERITS;
(?i:isvoid)             return TOKEN_KEYWORD_ISVOID;
(?i:let)                return TOKEN_KEYWORD_LET;
(?i:loop)               return TOKEN_KEYWORD_LOOP;
(?i:pool)               return TOKEN_KEYWORD_POOL;
(?i:then)               return TOKEN_KEYWORD_THEN;
(?i:while)              return TOKEN_KEYWORD_WHILE;
(?i:case)               return TOKEN_KEYWORD_CASE;
(?i:esac)               return TOKEN_KEYWORD_ESAC;
(?i:new)                return TOKEN_KEYWORD_NEW;
(?i:of)                 return TOKEN_KEYWORD_OF;
(?i:not)                return TOKEN_KEYWORD_NOT;
t(?i:rue)               return TOKEN_KEYWORD_TRUE;
f(?i:alse)              return TOKEN_KEYWORD_FALSE;

"@"                     return TOKEN_AT;
"~"                     return TOKEN_TILDE;
"*"                     return TOKEN_MUL;
"/"                     return TOKEN_DIV;
"+"                     return TOKEN_PLUS;
"-"                     return TOKEN_MINUS;
"<="                    return TOKEN_LESS_EQUAL;
"<"                     return TOKEN_LESS;
"="                     return TOKEN_EQUAL;
"<-"                    return TOKEN_ASSIGNMENT;
"."                     return TOKEN_DOT;
","                     return TOKEN_COMMA;
";"                     return TOKEN_SEMICOLON;
":"                     return TOKEN_COLON;
"("                     return TOKEN_OPEN_REGULAR;
")"                     return TOKEN_CLOSE_REGULAR;
"["                     return TOKEN_OPEN_SQUARE;
"]"                     return TOKEN_CLOSE_SQUARE;
"{"                     return TOKEN_OPEN_BLOCK;
"}"                     return TOKEN_CLOSE_BLOCK;

{digits}+               return TOKEN_INT_DIGITS;

[A-Z][A_Za-z0-9_]*      return TOKEN_TYPE_IDENTIFER;
[a-z][A-Za-z0-9_]*      return TOKEN_OBJECT_IDENTIFER;
_[A-Za-z0-9_]*          return TOKEN_IDENTIFIER_OTHER;

{white_space}        { /* skip spaces */ }
\n                   lineno++;
.                    Error("unrecognized character");

%%

void CoolLexer::Error(const char* msg) const
{
    std::cerr << "Lexer error (line " << lineno + 1 << "): " << msg << ": lexeme '" << YYText() << "'\n";
    std::exit(YY_EXIT_FAILURE);
}

void CoolLexer::Escape(){
    const char *input = yytext;
    char *output = yytext;
    input++; // Skip opening '\"'
    while (*(input + 1) /* Skip closing '\"' */ ) {
        if (*input == '\\') {
            input++; // Skip '\\'
            switch (*input) {
                case 'n': *output++ = '\n'; break;
                case 't': *output++ = '\t'; break;
                case 'f': *output++ = '\f'; break;
                case 'b': *output++ = '\b'; break;
                default: *output++ = *input;
            }
        } else {
            *output++ = *input;
        }
        input++;
    }
    *output = '\0'; // Null-terminate the output string
}