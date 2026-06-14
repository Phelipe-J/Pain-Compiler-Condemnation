#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <unordered_set>

#include "AST.hpp"

class Transpiler : public ASTVisitor {
private:
    std::ostringstream out;
    int indentLevel = 0;
    std::string exprResult;
    std::unordered_set<std::string> dictNames;

    void indent();
    std::string emitExpression(Expression* expression);
    void emitBlock(std::vector<std::unique_ptr<Statement>>& body);
    std::string emitForClause(Statement* statement);
    void collectDicts(std::vector<std::unique_ptr<Statement>>& statements);

public:
    std::string generate(std::vector<std::unique_ptr<Statement>>& program);

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
