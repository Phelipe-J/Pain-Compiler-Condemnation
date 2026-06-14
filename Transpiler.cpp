#include "Transpiler.hpp"

namespace {

std::string typeToCType(TokenType type) {
    switch (type) {
        case TokenType::INT:         return "int";
        case TokenType::FIXED_POINT: return "int";
        case TokenType::FLOAT:       return "float";
        case TokenType::CHAR_TYPE:   return "char";
        case TokenType::STRING_TYPE: return "char*";
        case TokenType::BOOL_TYPE:   return "int";
        default:                     return "void";
    }
}

std::string operatorToC(TokenType type) {
    switch (type) {
        case TokenType::PLUS:          return "+";
        case TokenType::MINUS:         return "-";
        case TokenType::MULTIPLY:      return "*";
        case TokenType::DIVIDE:        return "/";
        case TokenType::MODULUS:       return "%";
        case TokenType::EQUAL:         return "==";
        case TokenType::NOT_EQUAL:     return "!=";
        case TokenType::GREATER:       return ">";
        case TokenType::LESS:          return "<";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::LESS_EQUAL:    return "<=";
        case TokenType::AND:           return "&&";
        case TokenType::OR:            return "||";
        default:                       return "?";
    }
}

} // namespace

void Transpiler::indent() {
    for (int i = 0; i < indentLevel; i++) {
        out << "    ";
    }
}

std::string Transpiler::emitExpression(Expression* expression) {
    if (expression == nullptr) {
        return "";
    }
    expression->accept(*this);
    return exprResult;
}

void Transpiler::emitBlock(std::vector<std::unique_ptr<Statement>>& body) {
    out << "{\n";
    indentLevel++;
    for (auto& statement : body) {
        if (statement) {
            statement->accept(*this);
        }
    }
    indentLevel--;
    indent();
    out << "}";
}

std::string Transpiler::emitForClause(Statement* statement) {
    if (statement == nullptr) {
        return "";
    }
    if (auto* decl = dynamic_cast<VariableDeclarationStatement*>(statement)) {
        std::string clause = typeToCType(decl->variableType) + " " + decl->variableName;
        if (decl->initialValueExpression) {
            clause += " = " + emitExpression(decl->initialValueExpression.get());
        }
        return clause;
    }
    if (auto* assign = dynamic_cast<AssignmentStatement*>(statement)) {
        return assign->variableName + " = " + emitExpression(assign->assignedValue.get());
    }
    return "";
}

void Transpiler::collectDicts(std::vector<std::unique_ptr<Statement>>& statements) {
    for (auto& statement : statements) {
        Statement* node = statement.get();
        if (node == nullptr) {
            continue;
        }
        if (auto* dict = dynamic_cast<DictionaryDeclarationStatement*>(node)) {
            dictNames.insert(dict->dictName);
        } else if (auto* fn = dynamic_cast<FunctionDeclarationStatement*>(node)) {
            collectDicts(fn->body);
        } else if (auto* iff = dynamic_cast<IfStatement*>(node)) {
            collectDicts(iff->thenBranch);
            collectDicts(iff->elseBranch);
        } else if (auto* whileNode = dynamic_cast<WhileStatement*>(node)) {
            collectDicts(whileNode->body);
        } else if (auto* doWhile = dynamic_cast<DoWhileStatement*>(node)) {
            collectDicts(doWhile->body);
        } else if (auto* forNode = dynamic_cast<ForStatement*>(node)) {
            collectDicts(forNode->body);
        } else if (auto* switchNode = dynamic_cast<SwitchStatement*>(node)) {
            for (auto& caseNode : switchNode->cases) {
                collectDicts(caseNode->body);
            }
        } else if (auto* structNode = dynamic_cast<StructDeclarationStatement*>(node)) {
            collectDicts(structNode->body);
        }
    }
}

std::string Transpiler::generate(std::vector<std::unique_ptr<Statement>>& program) {
    out.str("");
    dictNames.clear();
    collectDicts(program);

    out << "#include <stdio.h>\n"
           "#include <stdlib.h>\n"
           "#include <string.h>\n\n";

    if (!dictNames.empty()) {
        out << "#define PCC_DICT_CAP 256\n"
               "typedef struct {\n"
               "    int keys[PCC_DICT_CAP];\n"
               "    int values[PCC_DICT_CAP];\n"
               "    int used[PCC_DICT_CAP];\n"
               "} PccDict;\n\n"
               "void pcc_dict_put(PccDict* d, int key, int value) {\n"
               "    int i = ((unsigned)key) % PCC_DICT_CAP;\n"
               "    while (d->used[i] && d->keys[i] != key) i = (i + 1) % PCC_DICT_CAP;\n"
               "    d->keys[i] = key; d->values[i] = value; d->used[i] = 1;\n"
               "}\n\n"
               "int pcc_dict_get(PccDict* d, int key) {\n"
               "    int i = ((unsigned)key) % PCC_DICT_CAP;\n"
               "    while (d->used[i]) {\n"
               "        if (d->keys[i] == key) return d->values[i];\n"
               "        i = (i + 1) % PCC_DICT_CAP;\n"
               "    }\n"
               "    return 0;\n"
               "}\n\n";
    }

    out << "void** pcc_gc_list = NULL;\n"
           "int pcc_gc_count = 0;\n"
           "int pcc_gc_cap = 0;\n\n"
           "void pcc_gc_cleanup() {\n"
           "    for(int i = 0; i < pcc_gc_count; i++) free(pcc_gc_list[i]);\n"
           "    free(pcc_gc_list);\n"
           "}\n\n"
           "char* pcc_strdup(const char* s) {\n"
           "    char* d = (char*)malloc(strlen(s) + 1);\n"
           "    if(d) {\n"
           "        strcpy(d, s);\n"
           "        if(pcc_gc_count >= pcc_gc_cap) {\n"
           "            pcc_gc_cap = pcc_gc_cap == 0 ? 128 : pcc_gc_cap * 2;\n"
           "            pcc_gc_list = (void**)realloc(pcc_gc_list, pcc_gc_cap * sizeof(void*));\n"
           "        }\n"
           "        pcc_gc_list[pcc_gc_count++] = d;\n"
           "    }\n"
           "    return d;\n"
           "}\n\n";

    for (auto& statement : program) {
        if (dynamic_cast<StructDeclarationStatement*>(statement.get())) {
            statement->accept(*this);
        }
    }

    for (auto& statement : program) {
        if (dynamic_cast<FunctionDeclarationStatement*>(statement.get())) {
            statement->accept(*this);
        }
    }

    out << "int main() {\n";
    indentLevel++;
    indent();
    
    out << "atexit(pcc_gc_cleanup);\n\n";

    for (auto& statement : program) {
        if (statement &&
            !dynamic_cast<FunctionDeclarationStatement*>(statement.get()) &&
            !dynamic_cast<StructDeclarationStatement*>(statement.get())) {
            statement->accept(*this);
        }
    }
    indent();
    out << "return 0;\n";
    indentLevel--;
    out << "}\n";

    return out.str();
}

void Transpiler::visit(LiteralExpression& node) {
    if (node.literalType == TokenType::LITERAL_STRING) {
        exprResult = "pcc_strdup(\"" + node.literalValue + "\")";
    } 
    else if (node.literalType == TokenType::LITERAL_TRUE) {
        exprResult = "1";
    } 
    else if (node.literalType == TokenType::LITERAL_FALSE) {
        exprResult = "0";
    } 
    else {
        exprResult = node.literalValue;
    }
}

void Transpiler::visit(IdentifierExpression& node) {
    exprResult = node.variableName;
}

void Transpiler::visit(BinaryExpression& node) {
    std::string left = emitExpression(node.leftSide.get());
    std::string right = emitExpression(node.rightSide.get());
    exprResult = "(" + left + " " + operatorToC(node.operatorToken) + " " + right + ")";
}

void Transpiler::visit(ArrayAccessExpression& node) {
    if (dictNames.count(node.arrayName)) {
        std::string key = node.indices.empty() ? "0" : emitExpression(node.indices[0].get());
        exprResult = "pcc_dict_get(&" + node.arrayName + ", " + key + ")";
        return;
    }
    std::string result = node.arrayName;
    for (auto& index : node.indices) {
        result += "[" + emitExpression(index.get()) + "]";
    }
    exprResult = result;
}

void Transpiler::visit(MemberAccessExpression& node) {
    exprResult = node.objectName + "." + node.memberName;
}

void Transpiler::visit(FunctionCallExpression& node) {
    std::string result = node.functionName + "(";
    for (size_t i = 0; i < node.arguments.size(); i++) {
        if (i > 0) {
            result += ", ";
        }
        result += emitExpression(node.arguments[i].get());
    }
    result += ")";
    exprResult = result;
}

void Transpiler::visit(VariableDeclarationStatement& node) {
    indent();
    out << typeToCType(node.variableType) << " " << node.variableName;
    if (node.initialValueExpression) {
        out << " = " << emitExpression(node.initialValueExpression.get());
    }
    out << ";\n";
}

void Transpiler::visit(AssignmentStatement& node) {
    indent();
    out << node.variableName << " = " << emitExpression(node.assignedValue.get()) << ";\n";
}

void Transpiler::visit(IfStatement& node) {
    indent();
    out << "if (" << emitExpression(node.condition.get()) << ") ";
    emitBlock(node.thenBranch);
    if (!node.elseBranch.empty()) {
        out << " else ";
        emitBlock(node.elseBranch);
    }
    out << "\n";
}

void Transpiler::visit(WhileStatement& node) {
    indent();
    out << "while (" << emitExpression(node.condition.get()) << ") ";
    emitBlock(node.body);
    out << "\n";
}

void Transpiler::visit(DoWhileStatement& node) {
    indent();
    out << "do ";
    emitBlock(node.body);
    out << " while (" << emitExpression(node.condition.get()) << ");\n";
}

void Transpiler::visit(ForStatement& node) {
    indent();
    out << "for (" << emitForClause(node.initialization.get()) << "; "
        << (node.condition ? emitExpression(node.condition.get()) : "") << "; "
        << emitForClause(node.increment.get()) << ") ";
    emitBlock(node.body);
    out << "\n";
}

void Transpiler::visit(BreakStatement& node) {
    (void)node;
    indent();
    out << "break;\n";
}

void Transpiler::visit(LabelStatement& node) {
    indent();
    out << node.name << ":\n";
}

void Transpiler::visit(GotoStatement& node) {
    indent();
    out << "goto " << node.labelName << ";\n";
}

void Transpiler::visit(CaseStatement& node) {
    indent();
    if (node.matchValue) {
        out << "case " << emitExpression(node.matchValue.get()) << ":\n";
    } else {
        out << "default:\n";
    }
    indentLevel++;
    for (auto& statement : node.body) {
        if (statement) {
            statement->accept(*this);
        }
    }
    indentLevel--;
}

void Transpiler::visit(SwitchStatement& node) {
    indent();
    out << "switch (" << emitExpression(node.condition.get()) << ") {\n";
    indentLevel++;
    for (auto& caseNode : node.cases) {
        caseNode->accept(*this);
    }
    indentLevel--;
    indent();
    out << "}\n";
}

void Transpiler::visit(FunctionDeclarationStatement& node) {
    indent();
    out << typeToCType(node.returnType) << " " << node.functionName << "(";
    for (size_t i = 0; i < node.parameters.size(); i++) {
        if (i > 0) {
            out << ", ";
        }
        out << typeToCType(node.parameters[i].type) << " " << node.parameters[i].name;
    }
    out << ") ";
    emitBlock(node.body);
    out << "\n\n";
}

void Transpiler::visit(FunctionCallStatement& node) {
    indent();
    out << emitExpression(node.callExpression.get()) << ";\n";
}

void Transpiler::visit(ReturnStatement& node) {
    indent();
    out << "return";
    if (node.returnValue) {
        out << " " << emitExpression(node.returnValue.get());
    }
    out << ";\n";
}

void Transpiler::visit(ArrayDeclarationStatement& node) {
    indent();
    out << typeToCType(node.elementType) << " " << node.arrayName;
    for (auto& dimension : node.dimensions) {
        out << "[" << emitExpression(dimension.get()) << "]";
    }
    out << ";\n";
}

void Transpiler::visit(ArrayAssignmentStatement& node) {
    indent();
    if (dictNames.count(node.arrayName)) {
        std::string key = node.indices.empty() ? "0" : emitExpression(node.indices[0].get());
        out << "pcc_dict_put(&" << node.arrayName << ", " << key << ", "
            << emitExpression(node.assignedValue.get()) << ");\n";
        return;
    }
    out << node.arrayName;
    for (auto& index : node.indices) {
        out << "[" << emitExpression(index.get()) << "]";
    }
    out << " = " << emitExpression(node.assignedValue.get()) << ";\n";
}

void Transpiler::visit(MemberAssignmentStatement& node) {
    indent();
    out << node.objectName << "." << node.memberName << " = "
        << emitExpression(node.assignedValue.get()) << ";\n";
}

void Transpiler::visit(DictionaryDeclarationStatement& node) {
    indent();
    out << "PccDict " << node.dictName << " = {0};\n";
}

void Transpiler::visit(StructDeclarationStatement& node) {
    indent();
    out << "struct " << node.structName << " {\n";
    indentLevel++;
    for (auto& member : node.body) {
        if (auto* field = dynamic_cast<VariableDeclarationStatement*>(member.get())) {
            indent();
            out << typeToCType(field->variableType) << " " << field->variableName << ";\n";
        }
    }
    indentLevel--;
    indent();
    out << "};\n\n";
}
