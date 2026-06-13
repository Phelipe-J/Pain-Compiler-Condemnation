#include <stdexcept>

#include "Parser.hpp"

// Construtor
SyntaxParser::SyntaxParser(std::vector<Token> tokens) {
    tokenList = tokens;
    currentTokenIndex = 0;
}

// Funções de navagação
Token SyntaxParser::peekCurrentToken() {
    return tokenList[currentTokenIndex];
}

Token SyntaxParser::advanceToNextToken() {
    if (peekCurrentToken().type != TokenType::EOF_TOKEN) {
        currentTokenIndex++;
    }
    return tokenList[currentTokenIndex - 1];
}

Token SyntaxParser::consumeToken(TokenType expectedType, std::string errorMessage) {
    if (peekCurrentToken().type == expectedType) {
        return advanceToNextToken();
    }
    throw std::runtime_error(errorMessage + " na linha " + std::to_string(peekCurrentToken().line));
}

// Parser principal
std::unique_ptr<Statement> SyntaxParser::parseStatement() {
    Token current = peekCurrentToken();

    switch (current.type) {
        case TokenType::INT:
        case TokenType::FIXED_POINT:
        case TokenType::FLOAT:
        case TokenType::CHAR_TYPE:
        case TokenType::STRING_TYPE:
        case TokenType::BOOL_TYPE:
            return parseVariableDeclaration();

        case TokenType::IDENTIFIER: 
            return parseAssignment();

        case TokenType::IF:
            return parseIfStatement();

        case TokenType::WHILE:
            return parseWhileStatement();

        case TokenType::DO:
            return parseDoWhileStatement();

        case TokenType::FOR:
            return parseForStatement();

        case TokenType::BREAK: {
            advanceToNextToken(); // Consome BREAK
            consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' apos BREAK");
            return std::make_unique<BreakStatement>();
        }
        default:
            throw std::runtime_error("Erro de Sintaxe: Comando nao reconhecido na linha " + std::to_string(current.line));
    }
}

// Expressões
// -- Primários
std::unique_ptr<Expression> SyntaxParser::parsePrimary() {
    Token current = peekCurrentToken();

    if (current.type == TokenType::LITERAL_INT) {
        advanceToNextToken();
        return std::make_unique<LiteralExpression>(current.value);
    } 
    if (current.type == TokenType::IDENTIFIER) {
        advanceToNextToken();
        return std::make_unique<IdentifierExpression>(current.value);
    }

    if (current.type == TokenType::LPAREN) {
        advanceToNextToken(); // Consome o '('
        std::unique_ptr<Expression> innerExpression = parseExpression();
        consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos a expressao");
        return innerExpression;
    }

    throw std::runtime_error("Erro de Sintaxe: Expressao invalida na linha " + std::to_string(current.line));
}

// -- Multiplicação, divisão e módulo
std::unique_ptr<Expression> SyntaxParser::parseFactor() {
    std::unique_ptr<Expression> leftNode = parsePrimary();

    while (peekCurrentToken().type == TokenType::MULTIPLY || 
           peekCurrentToken().type == TokenType::DIVIDE ||
           peekCurrentToken().type == TokenType::MODULUS) {
        
        Token operatorToken = advanceToNextToken();
        std::unique_ptr<Expression> rightNode = parsePrimary();
        
        leftNode = std::make_unique<BinaryExpression>(std::move(leftNode), operatorToken.type, std::move(rightNode));
    }
    return leftNode;
}

// -- Soma e subtração
std::unique_ptr<Expression> SyntaxParser::parseTerm() {
    std::unique_ptr<Expression> leftNode = parseFactor();

    while (peekCurrentToken().type == TokenType::PLUS || 
           peekCurrentToken().type == TokenType::MINUS) {
        
        Token operatorToken = advanceToNextToken();
        std::unique_ptr<Expression> rightNode = parseFactor();
        
        leftNode = std::make_unique<BinaryExpression>(std::move(leftNode), operatorToken.type, std::move(rightNode));
    }
    return leftNode;
}

// -- Comparação: (>, <, >=, <=)
std::unique_ptr<Expression> SyntaxParser::parseComparison() {
    std::unique_ptr<Expression> leftNode = parseTerm();

    while (peekCurrentToken().type == TokenType::GREATER || 
           peekCurrentToken().type == TokenType::LESS ||
           peekCurrentToken().type == TokenType::GREATER_EQUAL ||
           peekCurrentToken().type == TokenType::LESS_EQUAL) {
        
        Token operatorToken = advanceToNextToken();
        std::unique_ptr<Expression> rightNode = parseTerm();
        
        leftNode = std::make_unique<BinaryExpression>(std::move(leftNode), operatorToken.type, std::move(rightNode));
    }
    return leftNode;
}

// -- Igualdade (==, !=)
std::unique_ptr<Expression> SyntaxParser::parseEquality() {
    std::unique_ptr<Expression> leftNode = parseComparison();

    while (peekCurrentToken().type == TokenType::EQUAL || 
           peekCurrentToken().type == TokenType::NOT_EQUAL) {
        
        Token operatorToken = advanceToNextToken();
        std::unique_ptr<Expression> rightNode = parseComparison();
        
        leftNode = std::make_unique<BinaryExpression>(std::move(leftNode), operatorToken.type, std::move(rightNode));
    }
    return leftNode;
}

// -- &&
std::unique_ptr<Expression> SyntaxParser::parseLogicalAnd() {
    std::unique_ptr<Expression> leftNode = parseEquality();

    while (peekCurrentToken().type == TokenType::AND) {
        Token operatorToken = advanceToNextToken();
        std::unique_ptr<Expression> rightNode = parseEquality();
        
        leftNode = std::make_unique<BinaryExpression>(std::move(leftNode), operatorToken.type, std::move(rightNode));
    }
    return leftNode;
}

// -- ||
std::unique_ptr<Expression> SyntaxParser::parseLogicalOr() {
    std::unique_ptr<Expression> leftNode = parseLogicalAnd();

    while (peekCurrentToken().type == TokenType::OR) {
        Token operatorToken = advanceToNextToken();
        std::unique_ptr<Expression> rightNode = parseLogicalAnd();
        
        leftNode = std::make_unique<BinaryExpression>(std::move(leftNode), operatorToken.type, std::move(rightNode));
    }
    return leftNode;
}

// -- Ponto de entrada
std::unique_ptr<Expression> SyntaxParser::parseExpression() {
    return parseLogicalOr();
}

// Declaração de variáveis
std::unique_ptr<Statement> SyntaxParser::parseVariableDeclaration() {
    Token typeToken = advanceToNextToken(); 
    Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da variavel");
    consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' apos o nome da variavel");

    std::unique_ptr<Expression> initialValueExpression = parseExpression();

    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da declaracao");

    return std::make_unique<VariableDeclarationStatement>(
        typeToken.type, 
        nameToken.value, 
        std::move(initialValueExpression)
    );
}

// Retribuição de variáveis
std::unique_ptr<Statement> SyntaxParser::parseAssignment() {
    Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da variavel");
    consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' para atribuicao");

    std::unique_ptr<Expression> valueExpression = parseExpression();

    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da atribuicao");

    return std::make_unique<AssignmentStatement>(nameToken.value, std::move(valueExpression));
}


// Leitor de Escopo (Blocos { })
std::vector<std::unique_ptr<Statement>> SyntaxParser::parseBlock() {
    consumeToken(TokenType::LBRACE, "Erro de Sintaxe: Esperado '{' no inicio do bloco");
    
    std::vector<std::unique_ptr<Statement>> statements;
    
    while (peekCurrentToken().type != TokenType::RBRACE && peekCurrentToken().type != TokenType::EOF_TOKEN) {
        statements.push_back(parseStatement());
    }
    
    consumeToken(TokenType::RBRACE, "Erro de Sintaxe: Esperado '}' no final do bloco");
    return statements;
}

// Controle de fluxo
std::unique_ptr<Statement> SyntaxParser::parseIfStatement() {
    advanceToNextToken();

    consumeToken(TokenType::LPAREN, "Erro de Sintaxe: Esperado '(' apos o comando 'IF'");
    std::unique_ptr<Expression> condition = parseExpression();
    consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos a condicao do 'IF'");

    // IF for verdadeiro
    std::vector<std::unique_ptr<Statement>> thenBranch = parseBlock();
    std::vector<std::unique_ptr<Statement>> elseBranch;

    // Verifica se tem Else
    if (peekCurrentToken().type == TokenType::ELSE) {
        advanceToNextToken();
        
        // Else If ou um bloco normal
        if (peekCurrentToken().type == TokenType::IF) {
            elseBranch.push_back(parseIfStatement()); // Chamada recursiva para Else If
        } else {
            elseBranch = parseBlock();
        }
    }

    return std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

// While
std::unique_ptr<Statement> SyntaxParser::parseWhileStatement() {
    advanceToNextToken();
    
    consumeToken(TokenType::LPAREN, "Erro de Sintaxe: Esperado '(' apos 'WHILE'");
    std::unique_ptr<Expression> condition = parseExpression();
    consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos a condicao do 'WHILE'");
    
    std::vector<std::unique_ptr<Statement>> body = parseBlock(); // Lê bloco { }
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

// Do While
std::unique_ptr<Statement> SyntaxParser::parseDoWhileStatement() {
    advanceToNextToken();
    
    std::vector<std::unique_ptr<Statement>> body = parseBlock(); // Lê bloco { } primeiro
    
    consumeToken(TokenType::WHILE, "Erro de Sintaxe: Esperado 'WHILE' no final do bloco 'DO'");
    consumeToken(TokenType::LPAREN, "Erro de Sintaxe: Esperado '(' apos 'WHILE'");
    std::unique_ptr<Expression> condition = parseExpression();
    consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos a condicao");
    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da instrucao DO-WHILE");
    
    return std::make_unique<DoWhileStatement>(std::move(body), std::move(condition));
}

// For
std::unique_ptr<Statement> SyntaxParser::parseForStatement() {
    advanceToNextToken();
    consumeToken(TokenType::LPAREN, "Erro de Sintaxe: Esperado '(' apos 'FOR'");

    // Inicialização
    std::unique_ptr<Statement> initialization = nullptr;
    TokenType currentType = peekCurrentToken().type;
    
    if (currentType == TokenType::INT || currentType == TokenType::FLOAT || 
        currentType == TokenType::CHAR_TYPE || currentType == TokenType::BOOL_TYPE || 
        currentType == TokenType::FIXED_POINT || currentType == TokenType::STRING_TYPE) {
        initialization = parseVariableDeclaration(); // lê o ';' no final
    } 
    else if (currentType == TokenType::IDENTIFIER) {
        initialization = parseAssignment(); // lê o ';' no final
    } 
    else {
        consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' na inicializacao do FOR");
    }

    // Condição
    std::unique_ptr<Expression> condition = nullptr;
    if (peekCurrentToken().type != TokenType::SEMICOLON) {
        condition = parseExpression();
    }
    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' apos a condicao do FOR");

    // Incremento
    std::unique_ptr<Statement> increment = nullptr;
    if (peekCurrentToken().type != TokenType::RPAREN) {
        Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro: Esperado variavel no incremento do FOR");
        consumeToken(TokenType::ASSIGN, "Erro: Esperado '=' no incremento do FOR");
        std::unique_ptr<Expression> valueExpr = parseExpression();
        // AssignmentStatement diretamente para pular o ';'
        increment = std::make_unique<AssignmentStatement>(nameToken.value, std::move(valueExpr));
    }
    consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' para fechar os parametros do FOR");

    // Bloco de Código { }
    std::vector<std::unique_ptr<Statement>> body = parseBlock();

    return std::make_unique<ForStatement>(std::move(initialization), std::move(condition), std::move(increment), std::move(body));
}

// Loop principal
std::vector<std::unique_ptr<Statement>> SyntaxParser::parseProgram() {
    std::vector<std::unique_ptr<Statement>> programAST;

    while (peekCurrentToken().type != TokenType::EOF_TOKEN) {
        programAST.push_back(parseStatement());
    }

    return programAST; 
}