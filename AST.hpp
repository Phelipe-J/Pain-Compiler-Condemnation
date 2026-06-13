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

// Goto e Labels
class LabelStatement : public Statement {
public:
    std::string name;
    LabelStatement(std::string n) : name(n) {}
};

class GotoStatement : public Statement {
public:
    std::string labelName;
    GotoStatement(std::string n) : labelName(n) {}
};

// Switch e Case

class CaseStatement : public Statement {
public:
    std::unique_ptr<Expression> matchValue; //nullptr = DEFAULT
    std::vector<std::unique_ptr<Statement>> body;

    CaseStatement(std::unique_ptr<Expression> val, std::vector<std::unique_ptr<Statement>> b)
        : matchValue(std::move(val)), body(std::move(b)) {}
};

class SwitchStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<CaseStatement>> cases;

    SwitchStatement(std::unique_ptr<Expression> cond, std::vector<std::unique_ptr<CaseStatement>> c)
        : condition(std::move(cond)), cases(std::move(c)) {}
};

class BreakStatement : public Statement {
public:
    BreakStatement() = default; // Nó vazio, serve como sinal de parada
};

// Vetores e Matrizes

// Ler valor de matriz
class ArrayAccessExpression : public Expression {
public:
    std::string arrayName;
    std::vector<std::unique_ptr<Expression>> indices; // Guarda a lista de [x][y][z]

    ArrayAccessExpression(std::string name, std::vector<std::unique_ptr<Expression>> idxs)
        : arrayName(name), indices(std::move(idxs)) {}
};

class ArrayDeclarationStatement : public Statement {
public:
    TokenType structureType;
    TokenType primitiveType;
    std::string arrayName;
    std::vector<std::unique_ptr<Expression>> dimensions;

    ArrayDeclarationStatement(TokenType structType, TokenType primType, std::string name, 
                              std::vector<std::unique_ptr<Expression>> dims)
        : structureType(structType), primitiveType(primType), arrayName(name), dimensions(std::move(dims)) {}
};

class ArrayAssignmentStatement : public Statement {
public:
    std::string arrayName;
    std::vector<std::unique_ptr<Expression>> indices;
    std::unique_ptr<Expression> assignedValue;

    ArrayAssignmentStatement(std::string name, std::vector<std::unique_ptr<Expression>> idxs, 
                             std::unique_ptr<Expression> value)
        : arrayName(name), indices(std::move(idxs)), assignedValue(std::move(value)) {}
};

// Tipos definidos pelo usário (Struct)

class StructDeclarationStatement : public Statement {
public:
    std::string structName;
    std::vector<std::unique_ptr<Statement>> body; // Guarda as variáveis (e futuramente funções) internas

    StructDeclarationStatement(std::string name, std::vector<std::unique_ptr<Statement>> b)
        : structName(name), body(std::move(b)) {}
};

class MemberAccessExpression : public Expression {
public:
    std::string objectName;
    std::string memberName;

    MemberAccessExpression(std::string obj, std::string mem)
        : objectName(obj), memberName(mem) {}
};

class MemberAssignmentStatement : public Statement {
public:
    std::string objectName;
    std::string memberName;
    std::unique_ptr<Expression> assignedValue;

    MemberAssignmentStatement(std::string obj, std::string mem, std::unique_ptr<Expression> val)
        : objectName(obj), memberName(mem), assignedValue(std::move(val)) {}
};

// Dictionary
class DictionaryDeclarationStatement : public Statement {
public:
    TokenType keyType;
    TokenType valueType;
    std::string dictName;

    DictionaryDeclarationStatement(TokenType kType, TokenType vType, std::string name)
        : keyType(kType), valueType(vType), dictName(name) {}
};

// Funções
struct Parameter {
    TokenType type;
    std::string name;
};

class FunctionDeclarationStatement : public Statement {
public:
    TokenType returnType;
    std::string functionName;
    std::vector<Parameter> parameters;
    std::vector<std::unique_ptr<Statement>> body;

    FunctionDeclarationStatement(TokenType retType, std::string name, 
                                 std::vector<Parameter> params, std::vector<std::unique_ptr<Statement>> b)
        : returnType(retType), functionName(name), parameters(std::move(params)), body(std::move(b)) {}
};

class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> returnValue; // Pode ser nullptr se for um retorno void

    ReturnStatement(std::unique_ptr<Expression> val) : returnValue(std::move(val)) {}
};

class FunctionCallExpression : public Expression {
public:
    std::string functionName;
    std::vector<std::unique_ptr<Expression>> arguments;

    FunctionCallExpression(std::string name, std::vector<std::unique_ptr<Expression>> args)
        : functionName(name), arguments(std::move(args)) {}
};

// Envoltório para quando a chamada de função é uma linha isolada
class FunctionCallStatement : public Statement {
public:
    std::unique_ptr<FunctionCallExpression> callExpression;

    FunctionCallStatement(std::unique_ptr<FunctionCallExpression> call) 
        : callExpression(std::move(call)) {}
};