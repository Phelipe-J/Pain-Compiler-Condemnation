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
        case TokenType::INT:          return "INT";
        case TokenType::IDENTIFIER:   return "IDENTIFIER";
        case TokenType::ASSIGN:       return "ASSIGN";
        case TokenType::LITERAL_INT:  return "LITERAL_INT";
        case TokenType::SEMICOLON:    return "SEMICOLON";
        case TokenType::EOF_TOKEN:    return "EOF";
        default:                      return "OUTRO_TIPO";
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