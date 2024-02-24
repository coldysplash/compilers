#ifndef PARSER_H
#define PARSER_H

/* Token types */
enum TokenType
{
    TOKEN_KEYWORD_CLASS = 1,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_FI,
    TOKEN_KEYWORD_IN,
    TOKEN_KEYWORD_INHERITS,
    TOKEN_KEYWORD_ISVOID,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_LOOP,
    TOKEN_KEYWORD_POOL,
    TOKEN_KEYWORD_THEN,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_CASE,
    TOKEN_KEYWORD_ESAC,
    TOKEN_KEYWORD_NEW,
    TOKEN_KEYWORD_OF,
    TOKEN_KEYWORD_NOT,
    TOKEN_DOT,
    TOKEN_AT,
    TOKEN_TILDE,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_LESS_EQUAL,
    TOKEN_LESS,
    TOKEN_EQUAL,
    TOKEN_ASSIGNMENT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_OPEN_REGULAR,
    TOKEN_CLOSE_REGULAR,
    TOKEN_OPEN_SQUARE,
    TOKEN_CLOSE_SQUARE,
    TOKEN_OPEN_BLOCK,
    TOKEN_CLOSE_BLOCK,
    TOKEN_INT_DIGITS,
    TOKEN_TYPE_IDENTIFER,
    TOKEN_OBJECT_IDENTIFER,
    TOKEN_IDENTIFIER_OTHER,
    TOKEN_STRING,
    TOKEN_BAD_STRING
};

#endif
