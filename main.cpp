#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <cstdio>
#include <filesystem>

#include "Token.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "Transpiler.hpp"

#include "ASTPrinter.hpp"

// --- FUNÇÃO AUXILIAR PARA DEBUG ---
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INT:           return "INT";
        case TokenType::FIXED_POINT:   return "FIXED_POINT";
        case TokenType::FLOAT:         return "FLOAT";
        case TokenType::CHAR_TYPE:     return "CHAR_TYPE";
        case TokenType::STRING_TYPE:   return "STRING_TYPE";
        case TokenType::BOOL_TYPE:     return "BOOL_TYPE";

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
    }
}

static bool runMsvc(const std::string& cFile, const std::string& exeFile) {
    namespace fs = std::filesystem;
    fs::path cwd = fs::current_path();
    fs::path cAbs = cwd / cFile;
    fs::path exeAbs = cwd / exeFile;
    fs::path batAbs = cwd / "pcc_build.bat";

    std::ofstream bat(batAbs);
    if (!bat) {
        return false;
    }
    bat << "@echo off\n"
        << "set \"VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\"\n"
        << "if not exist \"%VSWHERE%\" exit /b 1\n"
        << "for /f \"usebackq tokens=*\" %%i in (`\"%VSWHERE%\" -latest -property installationPath`) do set \"VSPATH=%%i\"\n"
        << "if not defined VSPATH exit /b 1\n"
        << "call \"%VSPATH%\\VC\\Auxiliary\\Build\\vcvars64.bat\" >nul 2>nul\n"
        << "cl /nologo \"" << cAbs.string() << "\" /Fe:\"" << exeAbs.string() << "\"\n"
        << "exit /b %errorlevel%\n";
    bat.close();

    std::string command = "\"\"" + batAbs.string() + "\"\"";
    int result = std::system(command.c_str());

    std::error_code ec;
    fs::remove(batAbs, ec);
    fs::remove(cwd / "saida.obj", ec);
    return result == 0;
}

static bool runCompiler(const std::string& cFile, const std::string& exeFile) {
    struct Candidate { std::string name; std::string command; };
    std::vector<Candidate> candidates = {
        { "g++",   "g++ \""   + cFile + "\" -o \"" + exeFile + "\"" },
        { "gcc",   "gcc \""   + cFile + "\" -o \"" + exeFile + "\"" },
        { "clang", "clang \"" + cFile + "\" -o \"" + exeFile + "\"" }
    };

    for (const Candidate& candidate : candidates) {
        std::string probe = "where " + candidate.name + " >nul 2>nul";
        if (std::system(probe.c_str()) != 0) {
            continue;
        }
        std::cout << "Compilando o C gerado com '" << candidate.name << "'..." << std::endl;
        return std::system(candidate.command.c_str()) == 0;
    }

    std::cout << "g++/gcc/clang nao encontrados; tentando o MSVC (cl)..." << std::endl;
    if (runMsvc(cFile, exeFile)) {
        return true;
    }

    std::cerr << "Nenhum compilador C utilizavel encontrado." << std::endl;
    std::cerr << "Instale o g++ (MinGW) para gerar o executavel final." << std::endl;
    return false;
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

        SemanticAnalyzer analyzer;
        bool semanticOk = analyzer.analyze(programAST);

        if (!semanticOk) {
            std::cerr << "\nCompilacao interrompida: erros semanticos encontrados." << std::endl;
            return 1;
        }

        std::cout << "\n--- ANALISE SEMANTICA CONCLUIDA ---" << std::endl;
        std::cout << "Nenhum erro semantico detectado." << std::endl;

        Transpiler transpiler;
        std::string generatedC = transpiler.generate(programAST);

        std::ofstream outputFile("saida.c");
        outputFile << generatedC;
        outputFile.close();

        std::cout << "\n--- TRANSPILACAO CONCLUIDA (saida.c) ---" << std::endl;
        std::cout << generatedC << std::endl;

        std::cout << "--- LINKING E LOADING ---" << std::endl;
        if (runCompiler("saida.c", "saida.exe")) {
            std::cout << "Executavel gerado com sucesso: saida.exe" << std::endl;
        } else {
            std::cerr << "Falha ao gerar o executavel a partir de saida.c." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "\nERRO FATAL DE COMPILACAO: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}