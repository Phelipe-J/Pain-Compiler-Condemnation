#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Token.hpp"
#include "AST.hpp"

class SyntaxParser {
private:
    std::vector<Token> tokenList;
    int currentTokenIndex; 

    // Funções auxiliares de navegação
    Token peekCurrentToken();
    Token advanceToNextToken();
    Token consumeToken(TokenType expectedType, std::string errorMessage);

public:
    // Construtor
    SyntaxParser(std::vector<Token> tokens);

    // Funções de leitura da gramática
    std::unique_ptr<Expression> parseExpression();          // Ponto de entrada
    std::unique_ptr<Expression> parseLogicalOr();           // ||
    std::unique_ptr<Expression> parseLogicalAnd();          // &&
    std::unique_ptr<Expression> parseEquality();            // ==, !=
    std::unique_ptr<Expression> parseComparison();          // <, >, <=, >=
    std::unique_ptr<Expression> parseTerm();                // +, -
    std::unique_ptr<Expression> parseFactor();              // *, /, %
    std::unique_ptr<Expression> parsePrimary();             // Números, Variáveis e ()

    std::unique_ptr<Statement> parseVariableDeclaration();
    std::unique_ptr<Statement> parseAssignment();

    std::unique_ptr<Statement> parseStatement();
    std::vector<std::unique_ptr<Statement>> parseBlock();
    std::unique_ptr<Statement> parseIfStatement();

    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseDoWhileStatement();
    std::unique_ptr<Statement> parseForStatement();
    
    std::unique_ptr<Statement> parseSwitchStatement();
    std::unique_ptr<Statement> parseGotoStatement();
    
    std::unique_ptr<Statement> parseIdentifierStatement();

    std::unique_ptr<Statement> parseArrayDeclaration();

    std::unique_ptr<Statement> parseStructDeclaration();
    std::unique_ptr<Statement> parseDictionaryDeclaration();

    std::unique_ptr<Statement> parseFunctionDeclaration();
    std::unique_ptr<Statement> parseReturnStatement();

    std::unique_ptr<Statement> parsePrintfStatement();
    std::unique_ptr<Statement> parseScanfStatement();

    // Loop Principal
    std::vector<std::unique_ptr<Statement>> parseProgram();
};