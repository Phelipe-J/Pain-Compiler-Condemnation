#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <memory>

#define CHAR_SIZE 8

enum class TokenType {
    // Tipos Primitivos
    INT, FIXED_POINT, FLOAT, CHAR_TYPE, STRING_TYPE, BOOL_TYPE,

    // Controle de Fluxo
    IF, ELSE, SWITCH, TERNARY, CASE, DEFAULT,

    // Laços
    FOR, WHILE, DO, GOTO, BREAK,

    // Estruturas de Dados Avançadas
    ARRAY, MULTI_ARRAY, DICTIONARY,

    // Funções
    FUNCTION, RETURN,

    // Definidos pelo usuário
    STRUCT,

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
    SEMICOLON,      // ;

    // Gerados dinamicamente via ASCII
    IDENTIFIER,
    LITERAL_STRING,
    LITERAL_INT,
    LITERAL_FLOAT,
    
    // Controle interno do compilador
    EOF_TOKEN
};

struct Token {
	TokenType type;
	std::string value;
	int line;
	int column;
};

class SyntaxTreeNode {
public:
    virtual ~SyntaxTreeNode() = default;
};

class Expression : public SyntaxTreeNode {};

class Statement : public SyntaxTreeNode {};

// Nó para uma expressão literal
class LiteralExpression : public Expression {
public:
	std::string literalValue;
	LiteralExpression(std::string value) : literalValue(value) {}
};

// Nó para declaração de uma nova variável
class VariableDeclarationStatement : public Statement {
public:
    TokenType variableType; 
    std::string variableName; 
    std::unique_ptr<Expression> initialValueExpression; 

    VariableDeclarationStatement(TokenType type, std::string name, std::unique_ptr<Expression> initialValue)
        : variableType(type), variableName(name), initialValueExpression(std::move(initialValue)) {}
};

// Nó para ler valor de uma variável que já existe
class IdentifierExpression : public Expression {
public:
    std::string variableName;
    IdentifierExpression(std::string name) : variableName(name) {}
};

// Nó para a reatribuição
class AssignmentStatement : public Statement {
public:
    std::string variableName;
    std::unique_ptr<Expression> assignedValue;

    AssignmentStatement(std::string name, std::unique_ptr<Expression> value)
        : variableName(name), assignedValue(std::move(value)) {}
};

class SyntaxParser {
private:
    std::vector<Token> tokenList;
    int currentTokenIndex = 0; 

    Token peekCurrentToken() {
        return tokenList[currentTokenIndex];
    }

    Token advanceToNextToken() {
        if (peekCurrentToken().type != TokenType::EOF_TOKEN) {
            currentTokenIndex++;
        }
        return tokenList[currentTokenIndex - 1];
    }

    Token consumeToken(TokenType expectedType, std::string errorMessage) {
        if (peekCurrentToken().type == expectedType) {
            return advanceToNextToken();
        }
        throw std::runtime_error(errorMessage + " na linha " + std::to_string(peekCurrentToken().line));
    }

public:
    SyntaxParser(std::vector<Token> tokens) : tokenList(tokens) {}

    std::unique_ptr<Expression> parseExpression() {
        Token current = peekCurrentToken();

        // Número solto
        if (current.type == TokenType::LITERAL_INT) {
            advanceToNextToken();
            return std::make_unique<LiteralExpression>(current.value);
        } 
        // Nome de variável
        else if (current.type == TokenType::IDENTIFIER) {
            advanceToNextToken();
            return std::make_unique<IdentifierExpression>(current.value);
        }
        
        throw std::runtime_error("Erro de Sintaxe: Expressao invalida na linha " + std::to_string(current.line));
    }

    std::unique_ptr<Statement> parseVariableDeclaration() {
        
        Token typeToken = advanceToNextToken(); 

        Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da variavel");

        consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' apos o nome da variavel");

        Token literalValueToken = consumeToken(TokenType::LITERAL_INT, "Erro de Sintaxe: Esperado valor inteiro");
        
        std::unique_ptr<Expression> initialValueExpression = parseExpression();

        consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da declaracao");

        return std::make_unique<VariableDeclarationStatement>(
            typeToken.type, 
            nameToken.value, 
            std::move(initialValueExpression)
        );
    }

    std::unique_ptr<Statement> parseAssignment() {
        Token nameToken = consumeToken(TokenType::IDENTIFIER, "Erro de Sintaxe: Esperado nome da variavel");

        // '='
        consumeToken(TokenType::ASSIGN, "Erro de Sintaxe: Esperado '=' para atribuicao");

        std::unique_ptr<Expression> valueExpression = parseExpression();

        // ';''
        consumeToken(TokenType::SEMICOLON, "Erro de Sintaxe: Esperado ';' no final da atribuicao");

        return std::make_unique<AssignmentStatement>(nameToken.value, std::move(valueExpression));
    }

    // A função principal agora vive dentro da classe e enxerga tudo
    std::vector<std::unique_ptr<Statement>> parseProgram() {
        std::vector<std::unique_ptr<Statement>> programAST;

        while (peekCurrentToken().type != TokenType::EOF_TOKEN) {
            
            Token current = peekCurrentToken();

            switch (current.type) {
                case TokenType::IDENTIFIER:
                    programAST.push_back(parseAssignment());
                    break;

                case TokenType::INT:
                case TokenType::FIXED_POINT:
                case TokenType::FLOAT:
                case TokenType::CHAR_TYPE:
                case TokenType::STRING_TYPE:
                case TokenType::BOOL_TYPE:
                    programAST.push_back(parseVariableDeclaration());
                    break;

                case TokenType::IF:
                    // programAST.push_back(parseIfStatement());
                    break;
                    
                case TokenType::FOR:
                case TokenType::WHILE:
                    // programAST.push_back(parseLoopStatement());
                    break;

                default:
                    throw std::runtime_error("Erro de Sintaxe: Comando nao reconhecido na linha " + std::to_string(current.line));
            }
        }

        return programAST; 
    }
};

class Lexer {
private:
    std::ifstream file;
    int currentLine = 1;
    int currentColumn = 1;
    std::unordered_map<std::string, TokenType> dictionary;

    void initializeDictionary() {      
        // Dados primitivos:
		dictionary["TSSSSSSS"] = TokenType::INT;
		dictionary["TSSSSTSS"] = TokenType::FIXED_POINT;
		dictionary["TSTSSSSS"] = TokenType::FLOAT;
		dictionary["TTSSSSSS"] = TokenType::CHAR_TYPE;
		dictionary["TTTSSSST"] = TokenType::STRING_TYPE;
		dictionary["TSSSSSST"] = TokenType::BOOL_TYPE;
		
		// Estruturas de Controle de Fluxo:
		// Condicionais:
		dictionary["TSTTTTSS"] = TokenType::IF;
		dictionary["TSTTTTST"] = TokenType::ELSE;
		dictionary["TTTTTTTS"] = TokenType::SWITCH;
		dictionary["TSSSTTTT"] = TokenType::TERNARY;
		dictionary["TTSTSSST"] = TokenType::CASE;
		dictionary["TSTSSTTS"] = TokenType::DEFAULT;
		
		// Laços:
		dictionary["TTSTSTSS"] = TokenType::FOR;
		dictionary["TSTSTSTS"] = TokenType::WHILE;
		dictionary["TSTSTSTT"] = TokenType::DO;
		dictionary["TSSSSSTT"] = TokenType::GOTO;
		dictionary["TTSSTTTS"] = TokenType::BREAK;
		
		// Dados Avançadas:
		dictionary["TSTTTTTS"] = TokenType::ARRAY;
		dictionary["TTTTTTTT"] = TokenType::MULTI_ARRAY;
		dictionary["TTSTSTST"] = TokenType::DICTIONARY;
		
		// Funções:
		dictionary["TTSSSSST"] = TokenType::FUNCTION;
		dictionary["TSTSSSST"] = TokenType::RETURN;
		
		// Definidos pelo usuário:
		dictionary["TSTTTSTS"] = TokenType::STRUCT;
		
		// Símbolos:
		// Lógico Matemáticos:
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
		dictionary["TSTTTTSS"] = TokenType::LESS_EQUAL;
		dictionary["TSSTSTST"] = TokenType::AND;
		dictionary["TSSTSTSS"] = TokenType::OR;
		dictionary["SSTSSSST"] = TokenType::NOT;
		dictionary["SSTSSTST"] = TokenType::MODULUS;
		
		// Delimitadores:
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

public:
    Lexer(const std::string &filename) {
        file.open(filename);
        if (!file.is_open()) {
            throw std::invalid_argument("Error: Could not open the file!");
        }
        initializeDictionary();
    }

    // Destrutor
    ~Lexer() {
        if (file.is_open()) {
            file.close();
        }
    }

    std::vector<Token> tokenizeAll() {
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

            if (ch == 'S' || ch == 'T') {		// Modificar no futuro para o space/tab
                buffer[readingCount] = ch;
                readingCount++;

                if (readingCount >= CHAR_SIZE) {
                    
                    if(buffer[0] == 'T'){		// Comando da linguagem
                        if (dictionary.find(buffer) != dictionary.end()) {
                            tokens.push_back({dictionary[buffer], buffer, currentLine, currentColumn});
                        }
                        else{
                            std::cerr << "Erro lexico na linha " << currentLine << ", coluna " << currentColumn 
                                      << ": Comando PCC desconhecido (" << buffer << ")" << std::endl;
                            exit(1);
                        }
                    }
                    else{		// Tabela ASCII
                        if(dictionary.find(buffer) != dictionary.end()){	// Operador salvo na linguagem
                            tokens.push_back({dictionary[buffer], buffer, currentLine, currentColumn});
                        }
                        else{		// Letra/número comum
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
};

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
        Lexer lexer(argv[1]);
        std::vector<Token> tokens = lexer.tokenizeAll();

        std::cout << "--- LEITURA CONCLUIDA ---" << std::endl;
        std::cout << "Total de tokens encontrados: " << tokens.size() << "\n\n";
        
        std::cout << "--- LISTA DE TOKENS ---" << std::endl;
        for (const Token& token : tokens) {
            std::cout << "[Linha " << token.line << ", Col " << token.column << "] "
                      << "Tipo: " << tokenTypeToString(token.type) 
                      << " | Valor: " << token.value << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "\nERRO FATAL: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
