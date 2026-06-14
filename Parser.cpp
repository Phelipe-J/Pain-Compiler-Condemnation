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
            return parseIdentifierStatement();

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
        case TokenType::SWITCH:
            return parseSwitchStatement();

        case TokenType::GOTO:
            return parseGotoStatement();

        case TokenType::ARRAY:
        case TokenType::MULTI_ARRAY:
            return parseArrayDeclaration();

        case TokenType::STRUCT:
            return parseStructDeclaration();
            
        case TokenType::DICTIONARY:
            return parseDictionaryDeclaration();

        case TokenType::FUNCTION:
            return parseFunctionDeclaration();

        case TokenType::RETURN:
            return parseReturnStatement();

        default:
            throw std::runtime_error("Erro de Sintaxe: Comando nao reconhecido na linha " + std::to_string(current.line));
    }
}

// Expressões
// -- Primários
std::unique_ptr<Expression> SyntaxParser::parsePrimary() {
    Token current = peekCurrentToken();

    if (current.type == TokenType::LITERAL_INT || 
        current.type == TokenType::LITERAL_FLOAT || 
        current.type == TokenType::LITERAL_STRING ||
        current.type == TokenType::LITERAL_TRUE ||
        current.type == TokenType::LITERAL_FALSE) {
        advanceToNextToken();

        return std::make_unique<LiteralExpression>(current.type, current.value);
    }

    if (current.type == TokenType::IDENTIFIER) {
        Token nameToken = advanceToNextToken(); // Consome o nome
        
        // Acesso a um índice de array ou dictionary
        if (peekCurrentToken().type == TokenType::LBRACKET) {
            std::vector<std::unique_ptr<Expression>> indices;
            while (peekCurrentToken().type == TokenType::LBRACKET) {
                advanceToNextToken(); // Consome '['
                indices.push_back(parseExpression());
                consumeToken(TokenType::RBRACKET, "Erro de Sintaxe: Esperado ']' apos indice do vetor");
            }
            return std::make_unique<ArrayAccessExpression>(nameToken.value, std::move(indices));
        }

        // Struct
        if (peekCurrentToken().type == TokenType::DOT) {
            advanceToNextToken(); // Consome '.'
            Token memberToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome do membro apos '.'");
            return std::make_unique<MemberAccessExpression>(nameToken.value, memberToken.value);
        }

        // Função
        if (peekCurrentToken().type == TokenType::LPAREN) {
            advanceToNextToken(); // Consome '('
            std::vector<std::unique_ptr<Expression>> arguments;

            if (peekCurrentToken().type != TokenType::RPAREN) {
                do {
                    arguments.push_back(parseExpression());
                    if (peekCurrentToken().type == TokenType::COMMA) {
                        advanceToNextToken(); // Consome ','
                    } else {
                        break;
                    }
                } while (true);
            }
            consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos os argumentos da funcao");
            return std::make_unique<FunctionCallExpression>(nameToken.value, std::move(arguments));
        }
        
        // Variável comum
        return std::make_unique<IdentifierExpression>(nameToken.value);
    }

    if (current.type == TokenType::LPAREN) {
        advanceToNextToken(); // Consome '('
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

    std::unique_ptr<Expression> initialValueExpression = nullptr;

    if (peekCurrentToken().type == TokenType::ASSIGN) {
        advanceToNextToken(); // Consome '='
        initialValueExpression = parseExpression();
    }

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

// Goto e rótulos
std::unique_ptr<Statement> SyntaxParser::parseGotoStatement() {
    advanceToNextToken(); // Consome 'GOTO'
    Token labelToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da label apos GOTO");
    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' apos GOTO");
    
    return std::make_unique<GotoStatement>(labelToken.value);
}

// O Lexer lê uma palavra, mas o Parser precisa descobrir se é uma variável ou um rótulo de GOTO
std::unique_ptr<Statement> SyntaxParser::parseIdentifierStatement() {
    Token nameToken = advanceToNextToken(); // Consome o nome da variável/rótulo

    // Rótulo GOTO
    if (peekCurrentToken().type == TokenType::COLON) {
        advanceToNextToken(); // Consome ':'
        return std::make_unique<LabelStatement>(nameToken.value);
    }
    
    // Matriz
    if (peekCurrentToken().type == TokenType::LBRACKET) {
        std::vector<std::unique_ptr<Expression>> indices;
        while (peekCurrentToken().type == TokenType::LBRACKET) {
            advanceToNextToken(); // Consome '['
            indices.push_back(parseExpression());
            consumeToken(TokenType::RBRACKET, "Erro de Sintaxe: Esperado ']' apos indice do vetor");
        }
        
        consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' apos os indices do vetor");
        std::unique_ptr<Expression> valueExpression = parseExpression();
        consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da atribuicao");

        return std::make_unique<ArrayAssignmentStatement>(nameToken.value, std::move(indices), std::move(valueExpression));
    }

    // Struct
    if (peekCurrentToken().type == TokenType::DOT) {
        advanceToNextToken(); // Consome '.'
        Token memberToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome do membro apos '.'");

        consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' para atribuicao na Struct");
        std::unique_ptr<Expression> valueExpression = parseExpression();
        consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da atribuicao");

        return std::make_unique<MemberAssignmentStatement>(nameToken.value, memberToken.value, std::move(valueExpression));
    }

    // Função sozinha na linha
    if (peekCurrentToken().type == TokenType::LPAREN) {
        advanceToNextToken(); // Consome '('
        std::vector<std::unique_ptr<Expression>> arguments;

        if (peekCurrentToken().type != TokenType::RPAREN) {
            do {
                arguments.push_back(parseExpression());
                if (peekCurrentToken().type == TokenType::COMMA) {
                    advanceToNextToken(); // Consome ','
                } else {
                    break;
                }
            } while (true);
        }
        consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos os argumentos da funcao");
        consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' apos a chamada da funcao");

        auto callExpr = std::make_unique<FunctionCallExpression>(nameToken.value, std::move(arguments));
        return std::make_unique<FunctionCallStatement>(std::move(callExpr));
    }
    
    // Variável comum
    consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' ou ':' apos identificador");
    std::unique_ptr<Expression> valueExpression = parseExpression();
    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da atribuicao");
    
    return std::make_unique<AssignmentStatement>(nameToken.value, std::move(valueExpression));
}

// Switch e Case
std::unique_ptr<Statement> SyntaxParser::parseSwitchStatement() {
    advanceToNextToken();
    
    consumeToken(TokenType::LPAREN, "Erro de Sintaxe: Esperado '(' apos SWITCH");
    std::unique_ptr<Expression> condition = parseExpression();
    consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos a condicao do SWITCH");
    
    consumeToken(TokenType::LBRACE, "Erro de Sintaxe: Esperado '{' para iniciar os blocos do SWITCH");

    std::vector<std::unique_ptr<CaseStatement>> cases;

    while (peekCurrentToken().type != TokenType::RBRACE && peekCurrentToken().type != TokenType::EOF_TOKEN) {
        std::unique_ptr<Expression> matchValue = nullptr;
        
        if (peekCurrentToken().type == TokenType::CASE) {
            advanceToNextToken();
            matchValue = parseExpression();
            consumeToken(TokenType::COLON, "Erro de Sintaxe: Esperado ':' apos valor do CASE");
        } 
        else if (peekCurrentToken().type == TokenType::DEFAULT) {
            advanceToNextToken();
            consumeToken(TokenType::COLON, "Erro de Sintaxe: Esperado ':' apos DEFAULT");
        } 
        else {
            throw std::runtime_error("Erro de Sintaxe: Esperado CASE ou DEFAULT dentro do SWITCH na linha " + std::to_string(peekCurrentToken().line));
        }

        std::vector<std::unique_ptr<Statement>> body;
        while (peekCurrentToken().type != TokenType::CASE && 
               peekCurrentToken().type != TokenType::DEFAULT && 
               peekCurrentToken().type != TokenType::RBRACE && 
               peekCurrentToken().type != TokenType::EOF_TOKEN) {
            body.push_back(parseStatement());
        }

        cases.push_back(std::make_unique<CaseStatement>(std::move(matchValue), std::move(body)));
    }

    consumeToken(TokenType::RBRACE, "Erro de Sintaxe: Esperado '}' no final do SWITCH");
    
    return std::make_unique<SwitchStatement>(std::move(condition), std::move(cases));
}

// Vetores e Matrizes
std::unique_ptr<Statement> SyntaxParser::parseArrayDeclaration() {
    Token structTypeToken = advanceToNextToken();
    
    // O próximo token obrigatoriamente é o tipo de dado
    Token primTypeToken = advanceToNextToken(); 
    
    Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome do vetor/matriz");

    std::vector<std::unique_ptr<Expression>> dimensions;

    do {
        consumeToken(TokenType::LBRACKET, "Erro de Sintaxe: Esperado '[' na declaracao do vetor");
        dimensions.push_back(parseExpression()); // O tamanho pode ser um número ou uma conta
        consumeToken(TokenType::RBRACKET, "Erro de Sintaxe: Esperado ']' apos tamanho do vetor");
    } while (peekCurrentToken().type == TokenType::LBRACKET);

    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da declaracao do vetor");

    return std::make_unique<ArrayDeclarationStatement>(
        structTypeToken.type, primTypeToken.type, nameToken.value, std::move(dimensions)
    );
}

// Dictionary
std::unique_ptr<Statement> SyntaxParser::parseDictionaryDeclaration() {
    advanceToNextToken(); // Consome 'DICTIONARY'
    
    Token keyTypeToken = advanceToNextToken(); 
    Token valueTypeToken = advanceToNextToken();
    
    Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome do Dicionario");
    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da declaracao do Dicionario");

    return std::make_unique<DictionaryDeclarationStatement>(keyTypeToken.type, valueTypeToken.type, nameToken.value);
}

// Struct
std::unique_ptr<Statement> SyntaxParser::parseStructDeclaration() {
    advanceToNextToken(); // Consome 'STRUCT'
    
    Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da Struct");
    consumeToken(TokenType::LBRACE, "Erro de Sintaxe: Esperado '{' para iniciar a Struct");

    std::vector<std::unique_ptr<Statement>> body;

    while (peekCurrentToken().type != TokenType::RBRACE && peekCurrentToken().type != TokenType::EOF_TOKEN) {
        body.push_back(parseStatement()); // Reutiliza a função que lê declarações
    }

    consumeToken(TokenType::RBRACE, "Erro de Sintaxe: Esperado '}' no final da Struct");
    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da declaracao da Struct");

    return std::make_unique<StructDeclarationStatement>(nameToken.value, std::move(body));
}

// Funções
std::unique_ptr<Statement> SyntaxParser::parseFunctionDeclaration() {
    advanceToNextToken(); // Consome 'FUNCTION'

    Token returnTypeToken = advanceToNextToken();
    Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da funcao");

    consumeToken(TokenType::LPAREN, "Erro de Sintaxe: Esperado '(' apos o nome da funcao");

    std::vector<Parameter> parameters;
    
    if (peekCurrentToken().type != TokenType::RPAREN) {
        do {
            Token paramType = advanceToNextToken();
            Token paramName = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome do parametro");
            parameters.push_back({paramType.type, paramName.value});

            // Se o próximo for uma vírgula, consome a vírgula e o laço repete
            if (peekCurrentToken().type == TokenType::COMMA) {
                advanceToNextToken();
            } else {
                break;
            }
        } while (true);
    }

    consumeToken(TokenType::RPAREN, "Erro de Sintaxe: Esperado ')' apos os parametros");
    
    // Lê o bloco de código interno da função
    std::vector<std::unique_ptr<Statement>> body = parseBlock();

    return std::make_unique<FunctionDeclarationStatement>(returnTypeToken.type, nameToken.value, std::move(parameters), std::move(body));
}

// Return
std::unique_ptr<Statement> SyntaxParser::parseReturnStatement() {
    advanceToNextToken(); // Consome 'RETURN'

    std::unique_ptr<Expression> returnValue = nullptr;

    // Se o que vier logo depois não for o ';', é uma expressão a ser retornada
    if (peekCurrentToken().type != TokenType::SEMICOLON) {
        returnValue = parseExpression();
    }

    consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' apos o comando RETURN");

    return std::make_unique<ReturnStatement>(std::move(returnValue));
}

// Loop principal
std::vector<std::unique_ptr<Statement>> SyntaxParser::parseProgram() {
    std::vector<std::unique_ptr<Statement>> programAST;

    while (peekCurrentToken().type != TokenType::EOF_TOKEN) {
        programAST.push_back(parseStatement());
    }

    return programAST; 
}