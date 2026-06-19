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
    dictionary["  \t  \t\t "] = TokenType::ADDRESS_OF;

    dictionary["\t       "] = TokenType::INT;
    dictionary["\t    \t  "] = TokenType::FIXED_POINT;
    dictionary["\t \t     "] = TokenType::FLOAT;
    dictionary["\t\t      "] = TokenType::CHAR_TYPE;
    dictionary["\t\t\t    \t"] = TokenType::STRING_TYPE;
    dictionary["\t      \t"] = TokenType::BOOL_TYPE;

    dictionary["\t    \t\t\t"] = TokenType::LITERAL_TRUE;
    dictionary["\t    \t \t"] = TokenType::LITERAL_FALSE;

    dictionary["\t \t\t\t\t  "] = TokenType::IF;
    dictionary["\t \t\t\t\t \t"] = TokenType::ELSE;
    dictionary["\t\t\t\t\t\t\t "] = TokenType::SWITCH;
    dictionary["\t   \t\t\t\t"] = TokenType::TERNARY;
    dictionary["\t\t \t   \t"] = TokenType::CASE;
    dictionary["\t \t  \t\t "] = TokenType::DEFAULT;

    dictionary["\t\t \t \t  "] = TokenType::FOR;
    dictionary["\t \t \t \t "] = TokenType::WHILE;
    dictionary["\t \t \t \t\t"] = TokenType::DO;
    dictionary["\t     \t\t"] = TokenType::GOTO;
    dictionary["\t\t  \t\t\t "] = TokenType::BREAK;

    dictionary["\t \t\t\t\t\t "] = TokenType::ARRAY;
    dictionary["\t\t\t\t\t\t\t\t"] = TokenType::MULTI_ARRAY;
    dictionary["\t\t \t \t \t"] = TokenType::DICTIONARY;

    dictionary["\t\t     \t"] = TokenType::FUNCTION;
    dictionary["\t \t    \t"] = TokenType::RETURN;
    dictionary["\t \t\t\t \t "] = TokenType::STRUCT;
    dictionary["\t \t\t  \t "] = TokenType::CALL_STRUCT;
    dictionary["\t\t\t \t\t  "] = TokenType::PRINTF;
    dictionary["\t\t \t  \t "] = TokenType::SCANF;

    dictionary["  \t\t\t\t \t"] = TokenType::ASSIGN;
    dictionary["  \t \t \t\t"] = TokenType::PLUS;
    dictionary["  \t \t\t \t"] = TokenType::MINUS;
    dictionary["  \t \t \t "] = TokenType::MULTIPLY;
    dictionary["  \t \t\t\t\t"] = TokenType::DIVIDE;
    dictionary["\t\t    \t\t"] = TokenType::EQUAL;
    dictionary["\t\t    \t "] = TokenType::NOT_EQUAL;
    dictionary["  \t\t\t\t\t "] = TokenType::GREATER;
    dictionary["  \t\t\t\t  "] = TokenType::LESS;
    dictionary["\t  \t\t\t\t "] = TokenType::GREATER_EQUAL;
    dictionary["\t  \t\t\t  "] = TokenType::LESS_EQUAL;
    dictionary["\t  \t \t \t"] = TokenType::AND;
    dictionary["\t  \t \t  "] = TokenType::OR;
    dictionary["  \t    \t"] = TokenType::NOT;
    dictionary["\t \t  \t \t"] = TokenType::MODULUS;

    dictionary["  \t \t   "] = TokenType::LPAREN;
    dictionary["  \t \t  \t"] = TokenType::RPAREN;
    dictionary[" \t\t\t\t \t\t"] = TokenType::LBRACE;
    dictionary[" \t\t\t\t\t \t"] = TokenType::RBRACE;
    dictionary[" \t \t\t \t "] = TokenType::LBRACKET;
    dictionary[" \t \t\t\t \t"] = TokenType::RBRACKET;
    dictionary["  \t   \t "] = TokenType::QUOTE_DOUBLE;
    dictionary["  \t  \t\t\t"] = TokenType::QUOTE_SINGLE;
    dictionary["  \t \t\t  "] = TokenType::COMMA;
    dictionary["  \t \t\t\t "] = TokenType::DOT;
    dictionary["  \t\t\t \t "] = TokenType::COLON;
    dictionary["  \t\t\t \t\t"] = TokenType::SEMICOLON;
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    char ch;
    std::string buffer;
    buffer.resize(CHAR_SIZE);
    int readingCount = 0;

    // Memórias e Rastreadores de Posição
    std::string currentWord = "";
    int wordLine = 0, wordCol = 0;

    std::string currentNumber = "";
    int numLine = 0, numCol = 0;

    bool insideString = false;
    std::string currentString = "";
    int strLine = 0, strCol = 0;

    bool insideChar = false;
    std::string currentChar = "";
    int charLine = 0, charCol = 0;

    auto flushWord = [&]() {
        if (!currentWord.empty()) {
            tokens.push_back({TokenType::IDENTIFIER, currentWord, wordLine, wordCol});
            currentWord = "";
        }
    };

    auto flushNumber = [&]() {
        if (!currentNumber.empty()) {
            TokenType type = (currentNumber.find('.') != std::string::npos) ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;
            tokens.push_back({type, currentNumber, numLine, numCol});
            currentNumber = "";
        }
    };

    while (file.get(ch)) {
        if (ch == '\n') {
            currentLine++;
            currentColumn = 1;
        } else {
            currentColumn++;
        }

        if (ch == '\t' || ch == ' ') {
            buffer[readingCount] = ch;
            readingCount++;

            if (readingCount >= CHAR_SIZE) {
                
                int asciiValue = 0;
                for (int i = 0; i < 8; i++) {
                    if (buffer[i] == '\t') asciiValue |= (1 << (7 - i));
                }
                char decodedChar = static_cast<char>(asciiValue);

                if (insideString) {
                    if (dictionary.find(buffer) != dictionary.end() && dictionary[buffer] == TokenType::QUOTE_DOUBLE) {
                        insideString = false;
                        tokens.push_back({TokenType::LITERAL_STRING, currentString, strLine, strCol});
                        currentString = "";
                    } else {
                        currentString += decodedChar;
                    }
                } 
                else if (insideChar) {
                    if (dictionary.find(buffer) != dictionary.end() && dictionary[buffer] == TokenType::QUOTE_SINGLE) {
                        insideChar = false;
                        tokens.push_back({TokenType::LITERAL_CHAR, currentChar, charLine, charCol});
                        currentChar = "";
                    } else {
                        currentChar += decodedChar;
                    }
                }
                else if (dictionary.find(buffer) != dictionary.end()) {
                    TokenType t = dictionary[buffer];
                    
                    if (t == TokenType::DOT && !currentNumber.empty() && currentNumber.find('.') == std::string::npos) {
                        currentNumber += '.'; // Mantém o float vivo
                    } else {
                        flushWord();
                        flushNumber();
                        
                        if (t == TokenType::QUOTE_DOUBLE) {
                            insideString = true;
                            strLine = currentLine;
                            strCol = currentColumn;
                        }
                        else if (t == TokenType::QUOTE_SINGLE) {
                            insideChar = true;
                            charLine = currentLine;
                            charCol = currentColumn;
                        }
                        else {
                            tokens.push_back({t, buffer, currentLine, currentColumn});
                        }
                    }
                }
                else {
                    bool isDigit = (asciiValue >= 48 && asciiValue <= 57);
                    bool isLetter = ((asciiValue >= 65 && asciiValue <= 90) || 
                                     (asciiValue >= 97 && asciiValue <= 122) || 
                                     asciiValue == 95); // '_'

                    if (isLetter) {
                        flushNumber();
                        if (currentWord.empty()) {
                            wordLine = currentLine;
                            wordCol = currentColumn;
                        }
                        currentWord += decodedChar;
                    } 
                    else if (isDigit) {
                        if (!currentWord.empty()) {
                            currentWord += decodedChar;
                        } else {
                            if (currentNumber.empty()) {
                                numLine = currentLine;
                                numCol = currentColumn;
                            }
                            currentNumber += decodedChar;
                        }
                    } 
                    else {
                        flushWord();
                        flushNumber();
                    }
                }

                readingCount = 0;
            }
        }
    }
    
    // Garante que o arquivo não termine sem salvar as últimas memórias
    flushWord();
    flushNumber();
    
    tokens.push_back({TokenType::EOF_TOKEN, "EOF", currentLine, currentColumn});
    return tokens;
}