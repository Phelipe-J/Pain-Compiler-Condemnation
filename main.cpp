#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "Token.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

#include "ASTPrinter.hpp"

// --- FUNÇÃO AUXILIAR PARA DEBUG ---
std::string tokenTypeToString(TokenType type) {
    switch (type) {
<<<<<<< Updated upstream
        case TokenType::INT:          return "INT";
        case TokenType::IDENTIFIER:   return "IDENTIFIER";
        case TokenType::ASSIGN:       return "ASSIGN";
        case TokenType::LITERAL_INT:  return "LITERAL_INT";
        case TokenType::SEMICOLON:    return "SEMICOLON";
        case TokenType::EOF_TOKEN:    return "EOF";
        default:                      return "OUTRO_TIPO";
=======
        case TokenType::INT:           return "INT";
        case TokenType::FIXED_POINT:   return "FIXED_POINT";
        case TokenType::FLOAT:         return "FLOAT";
        case TokenType::CHAR_TYPE:     return "CHAR_TYPE";
        case TokenType::STRING_TYPE:   return "STRING_TYPE";
        case TokenType::BOOL_TYPE:     return "BOOL_TYPE";

        case TokenType::LITERAL_TRUE:   return "LITERAL_TRUE";
        case TokenType::LITERAL_FALSE:  return "LITERAL_FALSE";

        case TokenType::IF:            return "IF";
        case TokenType::ELSE:          return "ELSE";
        case TokenType::SWITCH:        return "SWITCH";
        case TokenType::TERNARY:       return "TERNARY";
        case TokenType::CASE:          return "CASE";
        case TokenType::DEFAULT:       return "DEFAULT";

        case TokenType::FOR:           return "FOR";
        case TokenType::WHILE:         return "WHILE";
        case TokenType::DO:            return "DO";
        case TokenType::GOTO:          return "GOTO";
        case TokenType::BREAK:         return "BREAK";

        case TokenType::ARRAY:         return "ARRAY";
        case TokenType::MULTI_ARRAY:   return "MULTI_ARRAY";
        case TokenType::DICTIONARY:    return "DICTIONARY";

        case TokenType::FUNCTION:      return "FUNCTION";
        case TokenType::RETURN:        return "RETURN";

        case TokenType::STRUCT:        return "STRUCT";

        case TokenType::ASSIGN:        return "ASSIGN";
        case TokenType::PLUS:          return "PLUS";
        case TokenType::MINUS:         return "MINUS";
        case TokenType::MULTIPLY:      return "MULTIPLY";
        case TokenType::DIVIDE:        return "DIVIDE";
        case TokenType::MODULUS:       return "MODULUS";
        case TokenType::EQUAL:         return "EQUAL";
        case TokenType::NOT_EQUAL:     return "NOT_EQUAL";
        case TokenType::GREATER:       return "GREATER";
        case TokenType::LESS:          return "LESS";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS_EQUAL:    return "LESS_EQUAL";
        case TokenType::AND:           return "AND";
        case TokenType::OR:            return "OR";
        case TokenType::NOT:           return "NOT";

        case TokenType::LPAREN:        return "LPAREN";
        case TokenType::RPAREN:        return "RPAREN";
        case TokenType::LBRACE:        return "LBRACE";
        case TokenType::RBRACE:        return "RBRACE";
        case TokenType::LBRACKET:      return "LBRACKET";
        case TokenType::RBRACKET:      return "RBRACKET";
        case TokenType::QUOTE_DOUBLE:  return "QUOTE_DOUBLE";
        case TokenType::QUOTE_SINGLE:  return "QUOTE_SINGLE";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::DOT:           return "DOT";
        case TokenType::COLON:         return "COLON";
        case TokenType::SEMICOLON:     return "SEMICOLON";

        case TokenType::IDENTIFIER:    return "IDENTIFIER";
        case TokenType::LITERAL_STRING:return "LITERAL_STRING";
        case TokenType::LITERAL_INT:   return "LITERAL_INT";
        case TokenType::LITERAL_FLOAT: return "LITERAL_FLOAT";

        case TokenType::EOF_TOKEN:     return "EOF";

        default:                       return "DESCONHECIDO";
>>>>>>> Stashed changes
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Error: Command usage: PCC [file_name]" << std::endl;
        return 1;
    }

    try {
        // Instancia o Lexer e gera a fita de Tokens
        Lexer lexer(argv[1]);
        std::vector<Token> tokens = lexer.tokenizeAll();

        std::cout << "--- ANALISE LEXICA CONCLUIDA ---" << std::endl;
        std::cout << "Total de tokens encontrados: " << tokens.size() << "\n\n";
        
        std::cout << "--- LISTA DE TOKENS ---" << std::endl;
        for (const Token& token : tokens) {
            std::cout << "[Linha " << token.line << ", Col " << token.column << "] "
                      << "Tipo: " << tokenTypeToString(token.type) 
                      << " | Valor: " << token.value << std::endl;
        }
        std::cout << "\n";

        // Passa os tokens para o Parser e constrói a AST
        SyntaxParser parser(tokens);
        std::vector<std::unique_ptr<Statement>> programAST = parser.parseProgram();

        std::cout << "--- ANALISE SINTATICA CONCLUIDA ---" << std::endl;

        ASTPrinter::printAST(programAST);

        std::cout << "AST construida com sucesso!" << std::endl;
        std::cout << "Nenhum erro de sintaxe detectado." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\nERRO FATAL DE COMPILACAO: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}