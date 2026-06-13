#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "Token.hpp"

#define CHAR_SIZE 8

class Lexer {
private:
    std::ifstream file;
    int currentLine;
    int currentColumn;
    std::unordered_map<std::string, TokenType> dictionary;

    // Apenas a assinatura da função, sem a lógica
    void initializeDictionary();

public:
    // Assinaturas do Construtor, Destrutor e Motor
    Lexer(const std::string &filename);
    ~Lexer();
    std::vector<Token> tokenizeAll();
};