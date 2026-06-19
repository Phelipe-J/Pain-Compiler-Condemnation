#pragma once
#include <string>

enum class TokenType {
    ADDRESS_OF,

    // Tipos Primitivos
    INT, FIXED_POINT, FLOAT, CHAR_TYPE, STRING_TYPE, BOOL_TYPE,

    // Controle de Fluxo
    IF, ELSE, SWITCH, TERNARY, CASE, DEFAULT,

    // Laços
    FOR, WHILE, DO, GOTO, BREAK,

    // Estruturas de Dados Avançadas
    ARRAY, MULTI_ARRAY, DICTIONARY,

    // Funções
    FUNCTION, RETURN, PRINTF, SCANF,

    // Definidos pelo usuário
    STRUCT, CALL_STRUCT,

    // Lógico Matemáticos
    ASSIGN,         // =
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    MODULUS,         // %
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    GREATER,        // >
    LESS,           // <
    GREATER_EQUAL,  // >=
    LESS_EQUAL,     // <=
    AND,            // &&
    OR,             // ||
    NOT,            // !

    // Delimitadores
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    QUOTE_DOUBLE,   // "
    QUOTE_SINGLE,   // '
    COMMA,          // ,
    DOT,            // .
    COLON,          // :
    SEMICOLON,      // ;

    // Gerados dinamicamente via ASCII
    IDENTIFIER,
    LITERAL_STRING,
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_CHAR,

    // true/false
    LITERAL_TRUE,
    LITERAL_FALSE,
    
    // Controle interno do compilador
    EOF_TOKEN
};

struct Token {
	TokenType type;
	std::string value;
	int line;
	int column;
};