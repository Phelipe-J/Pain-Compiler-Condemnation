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

    dictionary["TSSSSTTT"] = TokenType::LITERAL_TRUE;
    dictionary["TSSSSTST"] = TokenType::LITERAL_FALSE;
    
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
    dictionary["SSTTTSTS"] = TokenType::COLON;
    dictionary["SSTTTSTT"] = TokenType::SEMICOLON;
}

// --- MOTOR DE LEITURA ---
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

        if (ch == 'S' || ch == 'T') {
            buffer[readingCount] = ch;
            readingCount++;

            if (readingCount >= CHAR_SIZE) {
                
                int asciiValue = 0;
                for (int i = 0; i < 8; i++) {
                    if (buffer[i] == 'T') asciiValue |= (1 << (7 - i));
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
                        } else {
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