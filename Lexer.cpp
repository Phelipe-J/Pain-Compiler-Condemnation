#include <iostream>
#include <stdexcept>

#include "Lexer.hpp"

// --- CONSTRUTOR ---
Lexer::Lexer(const std::string &filename) {
    currentLine = 1;
    currentColumn = 1;
    
    file.open(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Error: Could not open the file!");
    }
    initializeDictionary();
}

// --- DESTRUTOR ---
Lexer::~Lexer() {
    if (file.is_open()) {
        file.close();
    }
}

// --- DICIONÁRIO ---
void Lexer::initializeDictionary() {      
    dictionary["TSSSSSSS"] = TokenType::INT;
    dictionary["TSSSSTSS"] = TokenType::FIXED_POINT;
    dictionary["TSTSSSSS"] = TokenType::FLOAT;
    dictionary["TTSSSSSS"] = TokenType::CHAR_TYPE;
    dictionary["TTTSSSST"] = TokenType::STRING_TYPE;
    dictionary["TSSSSSST"] = TokenType::BOOL_TYPE;
    
    dictionary["TSTTTTSS"] = TokenType::IF;
    dictionary["TSTTTTST"] = TokenType::ELSE;
    dictionary["TTTTTTTS"] = TokenType::SWITCH;
    dictionary["TSSSTTTT"] = TokenType::TERNARY;
    dictionary["TTSTSSST"] = TokenType::CASE;
    dictionary["TSTSSTTS"] = TokenType::DEFAULT;
    
    dictionary["TTSTSTSS"] = TokenType::FOR;
    dictionary["TSTSTSTS"] = TokenType::WHILE;
    dictionary["TSTSTSTT"] = TokenType::DO;
    dictionary["TSSSSSTT"] = TokenType::GOTO;
    dictionary["TTSSTTTS"] = TokenType::BREAK;
    
    dictionary["TSTTTTTS"] = TokenType::ARRAY;
    dictionary["TTTTTTTT"] = TokenType::MULTI_ARRAY;
    dictionary["TTSTSTST"] = TokenType::DICTIONARY;
    
    dictionary["TTSSSSST"] = TokenType::FUNCTION;
    dictionary["TSTSSSST"] = TokenType::RETURN;
    
    dictionary["TSTTTSTS"] = TokenType::STRUCT;
    
    dictionary["SSTTTTST"] = TokenType::ASSIGN;
    dictionary["SSTSTSTT"] = TokenType::PLUS;
    dictionary["SSTSTTST"] = TokenType::MINUS;
    dictionary["SSTSTSTS"] = TokenType::MULTIPLY;
    dictionary["SSTSTTTT"] = TokenType::DIVIDE;
    dictionary["TTSSSSTT"] = TokenType::EQUAL;
    dictionary["TTSSSSTS"] = TokenType::NOT_EQUAL;
    dictionary["SSTTTTTS"] = TokenType::GREATER;
    dictionary["SSTTTTSS"] = TokenType::LESS;
    dictionary["TSSTTTTS"] = TokenType::GREATER_EQUAL;
    dictionary["TSSTTTSS"] = TokenType::LESS_EQUAL;
    dictionary["TSSTSTST"] = TokenType::AND;
    dictionary["TSSTSTSS"] = TokenType::OR;
    dictionary["SSTSSSST"] = TokenType::NOT;
    dictionary["TSTSSTST"] = TokenType::MODULUS;
    
    dictionary["SSTSTSSS"] = TokenType::LPAREN;
    dictionary["SSTSTSST"] = TokenType::RPAREN;
    dictionary["STTTTSTS"] = TokenType::LBRACE;
    dictionary["STTTTTST"] = TokenType::RBRACE;
    dictionary["STSTTSTS"] = TokenType::LBRACKET;
    dictionary["STSTTTST"] = TokenType::RBRACKET;
    dictionary["SSTSSSTS"] = TokenType::QUOTE_DOUBLE;
    dictionary["SSTSSTTT"] = TokenType::QUOTE_SINGLE;
    dictionary["SSTSTTSS"] = TokenType::COMMA;
    dictionary["SSTSTTTS"] = TokenType::DOT;
    dictionary["SSTTTSTS"] = TokenType::SEMICOLON;
}

// --- MOTOR DE LEITURA ---
std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    char ch;
    std::string buffer;
    buffer.resize(CHAR_SIZE);
    int readingCount = 0;

    while (file.get(ch)) {
        if (ch == '\n') {
            currentLine++;
            currentColumn = 1;
        } else {
            currentColumn++;
        }

        if (ch == 'S' || ch == 'T') {
            buffer[readingCount] = ch;
            readingCount++;

            if (readingCount >= CHAR_SIZE) {
                if(buffer[0] == 'T'){
                    if (dictionary.find(buffer) != dictionary.end()) {
                        tokens.push_back({dictionary[buffer], buffer, currentLine, currentColumn});
                    } else {
                        std::cerr << "Erro lexico na linha " << currentLine << ", coluna " << currentColumn 
                                  << ": Comando PCC desconhecido (" << buffer << ")" << std::endl;
                        exit(1);
                    }
                } else {
                    if(dictionary.find(buffer) != dictionary.end()){
                        tokens.push_back({dictionary[buffer], buffer, currentLine, currentColumn});
                    } else {
                        tokens.push_back({TokenType::IDENTIFIER, buffer, currentLine, currentColumn});
                    }
                }
                readingCount = 0;
            }
        }
    }
    
    tokens.push_back({TokenType::EOF_TOKEN, "EOF", currentLine, currentColumn});
    return tokens;
}