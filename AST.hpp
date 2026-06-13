#pragma once

#include <memory>
#include <string>

#include "Token.hpp"

// Classe base
class SyntaxTreeNode {
public:
    virtual ~SyntaxTreeNode() = default;
};

class Expression : public SyntaxTreeNode {};

class Statement : public SyntaxTreeNode {};

// ----------------------------------------------------

class LiteralExpression : public Expression {
public:
    std::string literalValue; 
    
    LiteralExpression(std::string value) : literalValue(value) {}
};

class IdentifierExpression : public Expression {
public:
    std::string variableName;
    
    IdentifierExpression(std::string name) : variableName(name) {}
};

class VariableDeclarationStatement : public Statement {
public:
    TokenType variableType; 
    std::string variableName; 
    std::unique_ptr<Expression> initialValueExpression; 

    VariableDeclarationStatement(TokenType type, std::string name, std::unique_ptr<Expression> initialValue)
        : variableType(type), variableName(name), initialValueExpression(std::move(initialValue)) {}
};

class AssignmentStatement : public Statement {
public:
    std::string variableName;
    std::unique_ptr<Expression> assignedValue;

    AssignmentStatement(std::string name, std::unique_ptr<Expression> value)
        : variableName(name), assignedValue(std::move(value)) {}
};

// Operações de dois lados (Matemática, Comparação e Lógica)
class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> leftSide;
    TokenType operatorToken;
    std::unique_ptr<Expression> rightSide;

    BinaryExpression(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
        : leftSide(std::move(left)), operatorToken(op), rightSide(std::move(right)) {}
};

class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    std::vector<std::unique_ptr<Statement>> elseBranch; // Fica vazio se não houver 'Else'

    IfStatement(std::unique_ptr<Expression> cond,
                std::vector<std::unique_ptr<Statement>> thenB,
                std::vector<std::unique_ptr<Statement>> elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
};

// Laços
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    WhileStatement(std::unique_ptr<Expression> cond, std::vector<std::unique_ptr<Statement>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

class DoWhileStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> body;
    std::unique_ptr<Expression> condition;

    DoWhileStatement(std::vector<std::unique_ptr<Statement>> b, std::unique_ptr<Expression> cond)
        : body(std::move(b)), condition(std::move(cond)) {}
};

class ForStatement : public Statement {
public:
    std::unique_ptr<Statement> initialization;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> increment; 
    std::vector<std::unique_ptr<Statement>> body;

    ForStatement(std::unique_ptr<Statement> init, std::unique_ptr<Expression> cond, 
                 std::unique_ptr<Statement> inc, std::vector<std::unique_ptr<Statement>> b)
        : initialization(std::move(init)), condition(std::move(cond)), 
          increment(std::move(inc)), body(std::move(b)) {}
};

class BreakStatement : public Statement {
public:
    BreakStatement() = default; // Nó vazio, serve apenas como uma "placa de pare" na árvore
};