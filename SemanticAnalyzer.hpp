#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "AST.hpp"
#include "SymbolTable.hpp"

class SemanticAnalyzer : public ASTVisitor {
private:
    struct FunctionSignature {
        Type returnType;
        std::vector<Type> parameterTypes;
    };

    SymbolTable symbols;
    int loopDepth = 0;
    std::vector<std::string> errors;

    std::unordered_map<std::string, FunctionSignature> functions;
    std::unordered_set<std::string> structNames;
    std::unordered_set<std::string> declaredLabels;
    std::vector<std::string> pendingGotos;

    bool insideFunction = false;
    Type currentReturnType = Type::UNKNOWN;
    Type currentSwitchType = Type::UNKNOWN;

    void reportError(const std::string& message);
    void registerFunction(FunctionDeclarationStatement& node);
    Type evaluateExpression(Expression* expression);
    void analyzeStatements(std::vector<std::unique_ptr<Statement>>& statements);
    void requireBooleanCondition(Expression* condition, const std::string& context);

public:
    bool analyze(std::vector<std::unique_ptr<Statement>>& program);

    void visit(LiteralExpression& node) override;
    void visit(IdentifierExpression& node) override;
    void visit(BinaryExpression& node) override;
    void visit(ArrayAccessExpression& node) override;
    void visit(MemberAccessExpression& node) override;
    void visit(FunctionCallExpression& node) override;
    void visit(VariableDeclarationStatement& node) override;
    void visit(AssignmentStatement& node) override;
    void visit(IfStatement& node) override;
    void visit(WhileStatement& node) override;
    void visit(DoWhileStatement& node) override;
    void visit(ForStatement& node) override;
    void visit(BreakStatement& node) override;
    void visit(LabelStatement& node) override;
    void visit(GotoStatement& node) override;
    void visit(CaseStatement& node) override;
    void visit(SwitchStatement& node) override;
    void visit(ArrayDeclarationStatement& node) override;
    void visit(ArrayAssignmentStatement& node) override;
    void visit(MemberAssignmentStatement& node) override;
    void visit(DictionaryDeclarationStatement& node) override;
    void visit(StructDeclarationStatement& node) override;
    void visit(FunctionDeclarationStatement& node) override;
    void visit(FunctionCallStatement& node) override;
    void visit(ReturnStatement& node) override;
};
