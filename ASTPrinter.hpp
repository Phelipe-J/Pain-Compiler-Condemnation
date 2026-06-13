#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "AST.hpp"

class ASTPrinter {
private:
    static void indent(int level) {
        for (int i = 0; i < level; ++i) std::cout << "  | ";
    }

    // Expressões
    static void printExpression(Expression* expr, int level) {
        if (!expr) return;

        if (auto lit = dynamic_cast<LiteralExpression*>(expr)) {
            std::cout << "Literal [" << lit->literalValue << "]\n";
        } 
        else if (auto id = dynamic_cast<IdentifierExpression*>(expr)) {
            std::cout << "Variavel [" << id->variableName << "]\n";
        } 
        else if (auto bin = dynamic_cast<BinaryExpression*>(expr)) {
            std::cout << "Operacao Binaria (Token ID: " << static_cast<int>(bin->operatorToken) << ")\n";
            indent(level + 1); std::cout << "Esq: "; printExpression(bin->leftSide.get(), level + 1);
            indent(level + 1); std::cout << "Dir: "; printExpression(bin->rightSide.get(), level + 1);
        }
        else if (auto arrAcc = dynamic_cast<ArrayAccessExpression*>(expr)) {
            std::cout << "Acesso a Vetor/Dicionario [" << arrAcc->arrayName << "]\n";
            for (size_t i = 0; i < arrAcc->indices.size(); i++) {
                indent(level + 1); std::cout << "Indice " << i << ": ";
                printExpression(arrAcc->indices[i].get(), level + 1);
            }
        }
        else if (auto memAcc = dynamic_cast<MemberAccessExpression*>(expr)) {
            std::cout << "Acesso a Struct [" << memAcc->objectName << "." << memAcc->memberName << "]\n";
        }
        else if (auto funcCall = dynamic_cast<FunctionCallExpression*>(expr)) {
            std::cout << "Chamada de Funcao [" << funcCall->functionName << "()]\n";
            for (auto& arg : funcCall->arguments) {
                indent(level + 1); std::cout << "Arg: ";
                printExpression(arg.get(), level + 1);
            }
        }
    }

    // Comandos (blocos principais)
    static void printStatement(Statement* stmt, int level) {
        if (!stmt) return;

        // Variaveis Básicas
        if (auto varDecl = dynamic_cast<VariableDeclarationStatement*>(stmt)) {
            indent(level); std::cout << "Declaracao de Variavel [" << varDecl->variableName << "]\n";
            
            if (varDecl->initialValueExpression) {
                indent(level + 1); std::cout << "Valor Inicial: ";
                printExpression(varDecl->initialValueExpression.get(), level + 1);
            }
        }
        else if (auto assign = dynamic_cast<AssignmentStatement*>(stmt)) {
            indent(level); std::cout << "Atribuicao [" << assign->variableName << "] =\n";
            indent(level + 1); std::cout << "Valor: ";
            printExpression(assign->assignedValue.get(), level + 1);
        }
        // Controle de Fluxo: IF / ELSE
        else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            indent(level); std::cout << "Comando IF\n";
            indent(level + 1); std::cout << "Condicao: "; printExpression(ifStmt->condition.get(), level + 1);
            indent(level + 1); std::cout << "Bloco Verdadeiro:\n";
            for (auto& s : ifStmt->thenBranch) printStatement(s.get(), level + 2);
            if (!ifStmt->elseBranch.empty()) {
                indent(level + 1); std::cout << "Bloco Falso (ELSE):\n";
                for (auto& s : ifStmt->elseBranch) printStatement(s.get(), level + 2);
            }
        }
        // Laços de Repetição
        else if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
            indent(level); std::cout << "Comando WHILE\n";
            indent(level + 1); std::cout << "Condicao: "; printExpression(whileStmt->condition.get(), level + 1);
            indent(level + 1); std::cout << "Bloco:\n";
            for (auto& s : whileStmt->body) printStatement(s.get(), level + 2);
        }
        else if (auto doWhileStmt = dynamic_cast<DoWhileStatement*>(stmt)) {
            indent(level); std::cout << "Comando DO-WHILE\n";
            indent(level + 1); std::cout << "Bloco:\n";
            for (auto& s : doWhileStmt->body) printStatement(s.get(), level + 2);
            indent(level + 1); std::cout << "Condicao: "; printExpression(doWhileStmt->condition.get(), level + 1);
        }
        else if (auto forStmt = dynamic_cast<ForStatement*>(stmt)) {
            indent(level); std::cout << "Comando FOR\n";
            indent(level + 1); std::cout << "Inicializacao:\n"; printStatement(forStmt->initialization.get(), level + 2);
            indent(level + 1); std::cout << "Condicao: "; printExpression(forStmt->condition.get(), level + 1);
            indent(level + 1); std::cout << "Incremento:\n"; printStatement(forStmt->increment.get(), level + 2);
            indent(level + 1); std::cout << "Bloco:\n";
            for (auto& s : forStmt->body) printStatement(s.get(), level + 2);
        }
        // Comandos de Salto
        else if (dynamic_cast<BreakStatement*>(stmt)) {
            indent(level); std::cout << "Comando BREAK\n";
        }
        else if (auto labelStmt = dynamic_cast<LabelStatement*>(stmt)) {
            indent(level); std::cout << "Rotulo GOTO [" << labelStmt->name << ":]\n";
        }
        else if (auto gotoStmt = dynamic_cast<GotoStatement*>(stmt)) {
            indent(level); std::cout << "Comando GOTO -> [" << gotoStmt->labelName << "]\n";
        }
        // Controle Avançado: Switch/Case
        else if (auto switchStmt = dynamic_cast<SwitchStatement*>(stmt)) {
            indent(level); std::cout << "Comando SWITCH\n";
            indent(level + 1); std::cout << "Alvo: "; printExpression(switchStmt->condition.get(), level + 1);
            for (auto& c : switchStmt->cases) {
                if (c->matchValue) {
                    indent(level + 1); std::cout << "CASE:\n";
                    indent(level + 2); std::cout << "Valor: "; printExpression(c->matchValue.get(), level + 2);
                } else {
                    indent(level + 1); std::cout << "DEFAULT:\n";
                }
                for (auto& s : c->body) printStatement(s.get(), level + 2);
            }
        }
        // Estruturas de Dados: Vetores
        else if (auto arrDecl = dynamic_cast<ArrayDeclarationStatement*>(stmt)) {
            indent(level); std::cout << "Declaracao de Vetor [" << arrDecl->arrayName << "]\n";
            for (size_t i = 0; i < arrDecl->dimensions.size(); i++) {
                indent(level + 1); std::cout << "Dimensao " << i << ": ";
                printExpression(arrDecl->dimensions[i].get(), level + 1);
            }
        }
        else if (auto arrAss = dynamic_cast<ArrayAssignmentStatement*>(stmt)) {
            indent(level); std::cout << "Atribuicao em Vetor [" << arrAss->arrayName << "]\n";
            for (size_t i = 0; i < arrAss->indices.size(); i++) {
                indent(level + 1); std::cout << "Indice " << i << ": ";
                printExpression(arrAss->indices[i].get(), level + 1);
            }
            indent(level + 1); std::cout << "Novo Valor: "; printExpression(arrAss->assignedValue.get(), level + 1);
        }
        // Tipos definidos pelo Usuário
        else if (auto structDecl = dynamic_cast<StructDeclarationStatement*>(stmt)) {
            indent(level); std::cout << "Declaracao de STRUCT [" << structDecl->structName << "]\n";
            for (auto& s : structDecl->body) printStatement(s.get(), level + 1);
        }
        else if (auto memAss = dynamic_cast<MemberAssignmentStatement*>(stmt)) {
            indent(level); std::cout << "Atribuicao em Struct [" << memAss->objectName << "." << memAss->memberName << "]\n";
            indent(level + 1); std::cout << "Novo Valor: "; printExpression(memAss->assignedValue.get(), level + 1);
        }
        else if (auto dictDecl = dynamic_cast<DictionaryDeclarationStatement*>(stmt)) {
            indent(level); std::cout << "Declaracao de DICIONARIO [" << dictDecl->dictName << "]\n";
        }
        // Funções
        else if (auto funcDecl = dynamic_cast<FunctionDeclarationStatement*>(stmt)) {
            indent(level); std::cout << "Declaracao de Funcao [" << funcDecl->functionName << "]\n";
            for (auto& param : funcDecl->parameters) {
                indent(level + 1); std::cout << "Parametro [" << param.name << "]\n";
            }
            indent(level + 1); std::cout << "Corpo da Funcao:\n";
            for (auto& s : funcDecl->body) printStatement(s.get(), level + 2);
        }
        else if (auto funcCallStmt = dynamic_cast<FunctionCallStatement*>(stmt)) {
            indent(level); std::cout << "Comando de Chamada de Funcao Isolada\n";
            indent(level + 1); printExpression(funcCallStmt->callExpression.get(), level + 1);
        }
        else if (auto retStmt = dynamic_cast<ReturnStatement*>(stmt)) {
            indent(level); std::cout << "Comando RETURN\n";
            if (retStmt->returnValue) {
                indent(level + 1); std::cout << "Valor: ";
                printExpression(retStmt->returnValue.get(), level + 1);
            }
        }
    }

public:
    static void printAST(const std::vector<std::unique_ptr<Statement>>& ast) {
        std::cout << "\n================ ARVORE DE SINTAXE (AST) ================\n";
        for (const auto& stmt : ast) {
            printStatement(stmt.get(), 0);
        }
        std::cout << "=========================================================\n\n";
    }
};