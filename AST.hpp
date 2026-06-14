#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Token.hpp"

enum class Type {
    INT,
    FIXED_POINT,
    FLOAT,
    CHAR,
    STRING,
    BOOL,
    UNKNOWN,
    ERROR
};

class LiteralExpression;
class IdentifierExpression;
class BinaryExpression;
class ArrayAccessExpression;
class MemberAccessExpression;
class FunctionCallExpression;
class VariableDeclarationStatement;
class AssignmentStatement;
class IfStatement;
class WhileStatement;
class DoWhileStatement;
class ForStatement;
class BreakStatement;
class LabelStatement;
class GotoStatement;
class CaseStatement;
class SwitchStatement;
class ArrayDeclarationStatement;
class ArrayAssignmentStatement;
class MemberAssignmentStatement;
class DictionaryDeclarationStatement;
class StructDeclarationStatement;
class FunctionDeclarationStatement;
class FunctionCallStatement;
class ReturnStatement;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(LiteralExpression& node) = 0;
    virtual void visit(IdentifierExpression& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(ArrayAccessExpression& node) = 0;
    virtual void visit(MemberAccessExpression& node) = 0;
    virtual void visit(FunctionCallExpression& node) = 0;
    virtual void visit(VariableDeclarationStatement& node) = 0;
    virtual void visit(AssignmentStatement& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(DoWhileStatement& node) = 0;
    virtual void visit(ForStatement& node) = 0;
    virtual void visit(BreakStatement& node) = 0;
    virtual void visit(LabelStatement& node) = 0;
    virtual void visit(GotoStatement& node) = 0;
    virtual void visit(CaseStatement& node) = 0;
    virtual void visit(SwitchStatement& node) = 0;
    virtual void visit(ArrayDeclarationStatement& node) = 0;
    virtual void visit(ArrayAssignmentStatement& node) = 0;
    virtual void visit(MemberAssignmentStatement& node) = 0;
    virtual void visit(DictionaryDeclarationStatement& node) = 0;
    virtual void visit(StructDeclarationStatement& node) = 0;
    virtual void visit(FunctionDeclarationStatement& node) = 0;
    virtual void visit(FunctionCallStatement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
};

class SyntaxTreeNode {
public:
    virtual ~SyntaxTreeNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

class Expression : public SyntaxTreeNode {
public:
    Type resolvedType = Type::UNKNOWN;
};

class Statement : public SyntaxTreeNode {};

class LiteralExpression : public Expression {
public:
    std::string literalValue;

    LiteralExpression(std::string value) : literalValue(value) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class IdentifierExpression : public Expression {
public:
    std::string variableName;

    IdentifierExpression(std::string name) : variableName(name) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Operações de dois lados (Matemática, Comparação e Lógica)
class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> leftSide;
    TokenType operatorToken;
    std::unique_ptr<Expression> rightSide;

    BinaryExpression(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
        : leftSide(std::move(left)), operatorToken(op), rightSide(std::move(right)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayAccessExpression : public Expression {
public:
    std::string arrayName;
    std::vector<std::unique_ptr<Expression>> indices;

    ArrayAccessExpression(std::string name, std::vector<std::unique_ptr<Expression>> idx)
        : arrayName(std::move(name)), indices(std::move(idx)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class MemberAccessExpression : public Expression {
public:
    std::string objectName;
    std::string memberName;

    MemberAccessExpression(std::string obj, std::string member)
        : objectName(std::move(obj)), memberName(std::move(member)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FunctionCallExpression : public Expression {
public:
    std::string functionName;
    std::vector<std::unique_ptr<Expression>> arguments;

    FunctionCallExpression(std::string name, std::vector<std::unique_ptr<Expression>> args)
        : functionName(std::move(name)), arguments(std::move(args)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VariableDeclarationStatement : public Statement {
public:
    TokenType variableType;
    std::string variableName;
    std::unique_ptr<Expression> initialValueExpression;

    VariableDeclarationStatement(TokenType type, std::string name, std::unique_ptr<Expression> initialValue)
        : variableType(type), variableName(name), initialValueExpression(std::move(initialValue)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class AssignmentStatement : public Statement {
public:
    std::string variableName;
    std::unique_ptr<Expression> assignedValue;

    AssignmentStatement(std::string name, std::unique_ptr<Expression> value)
        : variableName(name), assignedValue(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
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

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Laços
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    WhileStatement(std::unique_ptr<Expression> cond, std::vector<std::unique_ptr<Statement>> b)
        : condition(std::move(cond)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class DoWhileStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> body;
    std::unique_ptr<Expression> condition;

    DoWhileStatement(std::vector<std::unique_ptr<Statement>> b, std::unique_ptr<Expression> cond)
        : body(std::move(b)), condition(std::move(cond)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
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

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Goto e Labels
class LabelStatement : public Statement {
public:
    std::string name;
    LabelStatement(std::string n) : name(n) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class GotoStatement : public Statement {
public:
    std::string labelName;
    GotoStatement(std::string n) : labelName(n) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Switch e Case

class CaseStatement : public Statement {
public:
    std::unique_ptr<Expression> matchValue; //nullptr = DEFAULT
    std::vector<std::unique_ptr<Statement>> body;

    CaseStatement(std::unique_ptr<Expression> val, std::vector<std::unique_ptr<Statement>> b)
        : matchValue(std::move(val)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class SwitchStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<CaseStatement>> cases;

    SwitchStatement(std::unique_ptr<Expression> cond, std::vector<std::unique_ptr<CaseStatement>> c)
        : condition(std::move(cond)), cases(std::move(c)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class BreakStatement : public Statement {
public:
    BreakStatement() = default; // Nó vazio, serve apenas como uma "placa de pare" na árvore

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayDeclarationStatement : public Statement {
public:
    TokenType containerType;
    TokenType elementType;
    std::string arrayName;
    std::vector<std::unique_ptr<Expression>> dimensions;

    ArrayDeclarationStatement(TokenType container, TokenType element, std::string name,
                              std::vector<std::unique_ptr<Expression>> dims)
        : containerType(container), elementType(element),
          arrayName(std::move(name)), dimensions(std::move(dims)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayAssignmentStatement : public Statement {
public:
    std::string arrayName;
    std::vector<std::unique_ptr<Expression>> indices;
    std::unique_ptr<Expression> assignedValue;

    ArrayAssignmentStatement(std::string name, std::vector<std::unique_ptr<Expression>> idx,
                             std::unique_ptr<Expression> value)
        : arrayName(std::move(name)), indices(std::move(idx)), assignedValue(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class DictionaryDeclarationStatement : public Statement {
public:
    TokenType keyType;
    TokenType valueType;
    std::string dictName;

    DictionaryDeclarationStatement(TokenType key, TokenType value, std::string name)
        : keyType(key), valueType(value), dictName(std::move(name)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class StructDeclarationStatement : public Statement {
public:
    std::string structName;
    std::vector<std::unique_ptr<Statement>> body;

    StructDeclarationStatement(std::string name, std::vector<std::unique_ptr<Statement>> b)
        : structName(std::move(name)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class MemberAssignmentStatement : public Statement {
public:
    std::string objectName;
    std::string memberName;
    std::unique_ptr<Expression> assignedValue;

    MemberAssignmentStatement(std::string obj, std::string member, std::unique_ptr<Expression> value)
        : objectName(std::move(obj)), memberName(std::move(member)), assignedValue(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

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

    FunctionDeclarationStatement(TokenType ret, std::string name,
                                 std::vector<Parameter> params,
                                 std::vector<std::unique_ptr<Statement>> b)
        : returnType(ret), functionName(std::move(name)),
          parameters(std::move(params)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FunctionCallStatement : public Statement {
public:
    std::unique_ptr<Expression> callExpression;

    FunctionCallStatement(std::unique_ptr<Expression> call)
        : callExpression(std::move(call)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> returnValue;

    ReturnStatement(std::unique_ptr<Expression> value)
        : returnValue(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};
