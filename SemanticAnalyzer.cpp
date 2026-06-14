#include <iostream>

#include "SemanticAnalyzer.hpp"

namespace {

Type tokenTypeToType(TokenType type) {
    switch (type) {
        case TokenType::INT:         return Type::INT;
        case TokenType::FIXED_POINT: return Type::FIXED_POINT;
        case TokenType::FLOAT:       return Type::FLOAT;
        case TokenType::CHAR_TYPE:   return Type::CHAR;
        case TokenType::STRING_TYPE: return Type::STRING;
        case TokenType::BOOL_TYPE:   return Type::BOOL;
        default:                     return Type::UNKNOWN;
    }
}

std::string typeToString(Type type) {
    switch (type) {
        case Type::INT:         return "int";
        case Type::FIXED_POINT: return "fixed";
        case Type::FLOAT:       return "float";
        case Type::CHAR:        return "char";
        case Type::STRING:      return "string";
        case Type::BOOL:        return "bool";
        case Type::ERROR:       return "<erro>";
        default:                return "<desconhecido>";
    }
}

bool isNumeric(Type type) {
    return type == Type::INT || type == Type::FIXED_POINT || type == Type::FLOAT;
}

Type promoteNumeric(Type left, Type right) {
    if (left == Type::FLOAT || right == Type::FLOAT)             return Type::FLOAT;
    if (left == Type::FIXED_POINT || right == Type::FIXED_POINT) return Type::FIXED_POINT;
    return Type::INT;
}

bool isAssignable(Type target, Type source) {
    if (target == Type::ERROR || source == Type::ERROR) return true;
    if (target == source) return true;
    if (isNumeric(target) && isNumeric(source)) {
        return promoteNumeric(target, source) == target;
    }
    return false;
}

} // namespace

void SemanticAnalyzer::reportError(const std::string& message) {
    errors.push_back(message);
}

Type SemanticAnalyzer::evaluateExpression(Expression* expression) {
    if (expression == nullptr) {
        return Type::UNKNOWN;
    }
    expression->accept(*this);
    return expression->resolvedType;
}

void SemanticAnalyzer::analyzeStatements(std::vector<std::unique_ptr<Statement>>& statements) {
    for (auto& statement : statements) {
        if (statement) {
            statement->accept(*this);
        }
    }
}

void SemanticAnalyzer::requireBooleanCondition(Expression* condition, const std::string& context) {
    Type conditionType = evaluateExpression(condition);
    if (conditionType != Type::BOOL && conditionType != Type::ERROR) {
        reportError("A condicao do '" + context + "' deve ser booleana, mas e do tipo '" +
                    typeToString(conditionType) + "'.");
    }
}

void SemanticAnalyzer::registerFunction(FunctionDeclarationStatement& node) {
    if (functions.find(node.functionName) != functions.end()) {
        reportError("Funcao '" + node.functionName + "' ja foi declarada.");
        return;
    }
    FunctionSignature signature;
    signature.returnType = tokenTypeToType(node.returnType);
    for (const Parameter& param : node.parameters) {
        signature.parameterTypes.push_back(tokenTypeToType(param.type));
    }
    functions[node.functionName] = signature;
}

bool SemanticAnalyzer::analyze(std::vector<std::unique_ptr<Statement>>& program) {
    symbols.enterScope();

    for (auto& statement : program) {
        if (auto* fn = dynamic_cast<FunctionDeclarationStatement*>(statement.get())) {
            registerFunction(*fn);
        }
    }

    analyzeStatements(program);
    symbols.exitScope();

    for (const std::string& label : pendingGotos) {
        if (declaredLabels.find(label) == declaredLabels.end()) {
            reportError("Comando 'goto' para um rotulo inexistente: '" + label + "'.");
        }
    }

    if (!errors.empty()) {
        std::cerr << "\n--- ERROS SEMANTICOS (" << errors.size() << ") ---" << std::endl;
        for (const std::string& error : errors) {
            std::cerr << "  - " << error << std::endl;
        }
        return false;
    }
    return true;
}

void SemanticAnalyzer::visit(LiteralExpression& node) {
    if (node.literalType == TokenType::LITERAL_INT) {
        node.resolvedType = Type::INT;
    } 
    else if (node.literalType == TokenType::LITERAL_FLOAT) {
        node.resolvedType = Type::FLOAT;
    } 
    else if (node.literalType == TokenType::LITERAL_STRING) {
        node.resolvedType = Type::STRING;
    }
    else if (node.literalType == TokenType::LITERAL_TRUE || 
        node.literalType == TokenType::LITERAL_FALSE) {
        node.resolvedType = Type::BOOL;
    }
    else {
        node.resolvedType = Type::UNKNOWN;
    }
}

void SemanticAnalyzer::visit(IdentifierExpression& node) {
    SymbolInfo* symbol = symbols.lookup(node.variableName);
    if (symbol == nullptr) {
        reportError("Variavel '" + node.variableName + "' usada sem ter sido declarada.");
        node.resolvedType = Type::ERROR;
        return;
    }
    node.resolvedType = symbol->type;
}

void SemanticAnalyzer::visit(BinaryExpression& node) {
    Type left = evaluateExpression(node.leftSide.get());
    Type right = evaluateExpression(node.rightSide.get());

    if (left == Type::ERROR || right == Type::ERROR) {
        node.resolvedType = Type::ERROR;
        return;
    }

    switch (node.operatorToken) {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULUS:
            if (!isNumeric(left) || !isNumeric(right)) {
                reportError("Operacao aritmetica invalida entre '" + typeToString(left) +
                            "' e '" + typeToString(right) + "'.");
                node.resolvedType = Type::ERROR;
            } else {
                node.resolvedType = promoteNumeric(left, right);
            }
            break;

        case TokenType::GREATER:
        case TokenType::LESS:
        case TokenType::GREATER_EQUAL:
        case TokenType::LESS_EQUAL:
            if (!isNumeric(left) || !isNumeric(right)) {
                reportError("Comparacao invalida entre '" + typeToString(left) +
                            "' e '" + typeToString(right) + "'.");
                node.resolvedType = Type::ERROR;
            } else {
                node.resolvedType = Type::BOOL;
            }
            break;

        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
            if (!isAssignable(left, right) && !isAssignable(right, left)) {
                reportError("Comparacao de igualdade entre tipos incompativeis '" +
                            typeToString(left) + "' e '" + typeToString(right) + "'.");
                node.resolvedType = Type::ERROR;
            } else {
                node.resolvedType = Type::BOOL;
            }
            break;

        case TokenType::AND:
        case TokenType::OR:
            if (left != Type::BOOL || right != Type::BOOL) {
                reportError("Operador logico exige operandos booleanos, mas recebeu '" +
                            typeToString(left) + "' e '" + typeToString(right) + "'.");
                node.resolvedType = Type::ERROR;
            } else {
                node.resolvedType = Type::BOOL;
            }
            break;

        default:
            node.resolvedType = Type::ERROR;
            break;
    }
}

void SemanticAnalyzer::visit(VariableDeclarationStatement& node) {
    Type declaredType = tokenTypeToType(node.variableType);

    if (node.initialValueExpression) {
        Type initType = evaluateExpression(node.initialValueExpression.get());
        if (!isAssignable(declaredType, initType)) {
            reportError("Nao e possivel inicializar a variavel '" + node.variableName +
                        "' (tipo '" + typeToString(declaredType) + "') com um valor do tipo '" +
                        typeToString(initType) + "'.");
        }
    }

    if (!symbols.declare(node.variableName, declaredType)) {
        reportError("Variavel '" + node.variableName + "' ja foi declarada neste escopo.");
    }
}

void SemanticAnalyzer::visit(AssignmentStatement& node) {
    SymbolInfo* symbol = symbols.lookup(node.variableName);
    Type valueType = evaluateExpression(node.assignedValue.get());

    if (symbol == nullptr) {
        reportError("Atribuicao para variavel '" + node.variableName +
                    "' que nao foi declarada.");
        return;
    }

    if (!isAssignable(symbol->type, valueType)) {
        reportError("Nao e possivel atribuir um valor do tipo '" + typeToString(valueType) +
                    "' a variavel '" + node.variableName + "' (tipo '" +
                    typeToString(symbol->type) + "').");
    }
}

void SemanticAnalyzer::visit(IfStatement& node) {
    requireBooleanCondition(node.condition.get(), "if");

    symbols.enterScope();
    analyzeStatements(node.thenBranch);
    symbols.exitScope();

    symbols.enterScope();
    analyzeStatements(node.elseBranch);
    symbols.exitScope();
}

void SemanticAnalyzer::visit(WhileStatement& node) {
    requireBooleanCondition(node.condition.get(), "while");

    loopDepth++;
    symbols.enterScope();
    analyzeStatements(node.body);
    symbols.exitScope();
    loopDepth--;
}

void SemanticAnalyzer::visit(DoWhileStatement& node) {
    loopDepth++;
    symbols.enterScope();
    analyzeStatements(node.body);
    symbols.exitScope();
    loopDepth--;

    requireBooleanCondition(node.condition.get(), "do-while");
}

void SemanticAnalyzer::visit(ForStatement& node) {
    symbols.enterScope();

    if (node.initialization) {
        node.initialization->accept(*this);
    }
    if (node.condition) {
        requireBooleanCondition(node.condition.get(), "for");
    }
    if (node.increment) {
        node.increment->accept(*this);
    }

    loopDepth++;
    analyzeStatements(node.body);
    loopDepth--;

    symbols.exitScope();
}

void SemanticAnalyzer::visit(BreakStatement& node) {
    (void)node;
    if (loopDepth == 0) {
        reportError("Comando 'break' usado fora de um laco.");
    }
}

void SemanticAnalyzer::visit(ArrayAccessExpression& node) {
    SymbolInfo* symbol = symbols.lookup(node.arrayName);

    std::vector<Type> indexTypes;
    for (auto& index : node.indices) {
        indexTypes.push_back(evaluateExpression(index.get()));
    }

    if (symbol == nullptr) {
        reportError("Vetor/dicionario '" + node.arrayName + "' usado sem ter sido declarado.");
        node.resolvedType = Type::ERROR;
        return;
    }

    if (symbol->kind == SymbolKind::ARRAY) {
        for (Type indexType : indexTypes) {
            if (indexType != Type::INT && indexType != Type::ERROR && indexType != Type::UNKNOWN) {
                reportError("O indice do vetor '" + node.arrayName +
                            "' deve ser inteiro, mas e do tipo '" + typeToString(indexType) + "'.");
            }
        }
        node.resolvedType = symbol->type;
    } else if (symbol->kind == SymbolKind::DICTIONARY) {
        if (indexTypes.size() != 1) {
            reportError("O acesso ao dicionario '" + node.arrayName +
                        "' deve ter exatamente uma chave.");
        } else if (!isAssignable(symbol->keyType, indexTypes[0])) {
            reportError("A chave do dicionario '" + node.arrayName + "' deve ser do tipo '" +
                        typeToString(symbol->keyType) + "', mas e '" + typeToString(indexTypes[0]) + "'.");
        }
        node.resolvedType = symbol->type;
    } else {
        reportError("A variavel '" + node.arrayName + "' nao e um vetor nem um dicionario.");
        node.resolvedType = Type::ERROR;
    }
}

void SemanticAnalyzer::visit(MemberAccessExpression& node) {
    node.resolvedType = Type::UNKNOWN;
}

void SemanticAnalyzer::visit(FunctionCallExpression& node) {
    std::vector<Type> argTypes;
    for (auto& arg : node.arguments) {
        argTypes.push_back(evaluateExpression(arg.get()));
    }

    auto found = functions.find(node.functionName);
    if (found == functions.end()) {
        reportError("Funcao '" + node.functionName + "' chamada sem ter sido declarada.");
        node.resolvedType = Type::ERROR;
        return;
    }

    const FunctionSignature& signature = found->second;
    if (argTypes.size() != signature.parameterTypes.size()) {
        reportError("A funcao '" + node.functionName + "' espera " +
                    std::to_string((int)signature.parameterTypes.size()) +
                    " argumento(s), mas recebeu " + std::to_string((int)argTypes.size()) + ".");
    } else {
        for (size_t i = 0; i < argTypes.size(); i++) {
            if (!isAssignable(signature.parameterTypes[i], argTypes[i])) {
                reportError("O argumento " + std::to_string((int)(i + 1)) + " da funcao '" +
                            node.functionName + "' deve ser '" + typeToString(signature.parameterTypes[i]) +
                            "', mas e '" + typeToString(argTypes[i]) + "'.");
            }
        }
    }
    node.resolvedType = signature.returnType;
}

void SemanticAnalyzer::visit(LabelStatement& node) {
    if (!declaredLabels.insert(node.name).second) {
        reportError("Rotulo '" + node.name + "' declarado mais de uma vez.");
    }
}

void SemanticAnalyzer::visit(GotoStatement& node) {
    pendingGotos.push_back(node.labelName);
}

void SemanticAnalyzer::visit(CaseStatement& node) {
    if (node.matchValue) {
        Type caseType = evaluateExpression(node.matchValue.get());
        if (currentSwitchType != Type::UNKNOWN &&
            !isAssignable(currentSwitchType, caseType) && !isAssignable(caseType, currentSwitchType)) {
            reportError("Valor de 'case' do tipo '" + typeToString(caseType) +
                        "' incompativel com o 'switch' do tipo '" + typeToString(currentSwitchType) + "'.");
        }
    }
    analyzeStatements(node.body);
}

void SemanticAnalyzer::visit(SwitchStatement& node) {
    Type conditionType = evaluateExpression(node.condition.get());

    Type previousSwitchType = currentSwitchType;
    currentSwitchType = conditionType;

    loopDepth++;
    symbols.enterScope();

    int defaultCount = 0;
    for (auto& caseNode : node.cases) {
        if (caseNode->matchValue == nullptr) {
            if (++defaultCount > 1) {
                reportError("O 'switch' tem mais de um 'default'.");
            }
        }
        caseNode->accept(*this);
    }

    symbols.exitScope();
    loopDepth--;
    currentSwitchType = previousSwitchType;
}

void SemanticAnalyzer::visit(ArrayDeclarationStatement& node) {
    for (auto& dim : node.dimensions) {
        Type dimType = evaluateExpression(dim.get());
        if (dimType != Type::INT && dimType != Type::ERROR && dimType != Type::UNKNOWN) {
            reportError("As dimensoes do vetor '" + node.arrayName +
                        "' devem ser inteiras, mas uma e do tipo '" + typeToString(dimType) + "'.");
        }
    }

    SymbolInfo info;
    info.name = node.arrayName;
    info.type = tokenTypeToType(node.elementType);
    info.kind = SymbolKind::ARRAY;
    info.dimensions = (int)node.dimensions.size();
    if (!symbols.declare(info)) {
        reportError("Variavel '" + node.arrayName + "' ja foi declarada neste escopo.");
    }
}

void SemanticAnalyzer::visit(ArrayAssignmentStatement& node) {
    SymbolInfo* symbol = symbols.lookup(node.arrayName);

    std::vector<Type> indexTypes;
    for (auto& index : node.indices) {
        indexTypes.push_back(evaluateExpression(index.get()));
    }
    Type valueType = evaluateExpression(node.assignedValue.get());

    if (symbol == nullptr) {
        reportError("Atribuicao para vetor/dicionario '" + node.arrayName +
                    "' que nao foi declarado.");
        return;
    }

    if (symbol->kind == SymbolKind::ARRAY) {
        for (Type indexType : indexTypes) {
            if (indexType != Type::INT && indexType != Type::ERROR && indexType != Type::UNKNOWN) {
                reportError("O indice do vetor '" + node.arrayName +
                            "' deve ser inteiro, mas e do tipo '" + typeToString(indexType) + "'.");
            }
        }
        if (!isAssignable(symbol->type, valueType)) {
            reportError("Nao e possivel atribuir um valor do tipo '" + typeToString(valueType) +
                        "' ao elemento do vetor '" + node.arrayName + "' (tipo '" +
                        typeToString(symbol->type) + "').");
        }
    } else if (symbol->kind == SymbolKind::DICTIONARY) {
        if (indexTypes.size() == 1 && !isAssignable(symbol->keyType, indexTypes[0])) {
            reportError("A chave do dicionario '" + node.arrayName + "' deve ser do tipo '" +
                        typeToString(symbol->keyType) + "', mas e '" + typeToString(indexTypes[0]) + "'.");
        }
        if (!isAssignable(symbol->type, valueType)) {
            reportError("Nao e possivel atribuir um valor do tipo '" + typeToString(valueType) +
                        "' ao dicionario '" + node.arrayName + "' (valor do tipo '" +
                        typeToString(symbol->type) + "').");
        }
    } else {
        reportError("A variavel '" + node.arrayName + "' nao e um vetor nem um dicionario.");
    }
}

void SemanticAnalyzer::visit(MemberAssignmentStatement& node) {
    SymbolInfo* symbol = symbols.lookup(node.objectName);
    evaluateExpression(node.assignedValue.get());

    if (symbol == nullptr) {
        reportError("Atribuicao para objeto '" + node.objectName + "' que nao foi declarado.");
    }
}

void SemanticAnalyzer::visit(DictionaryDeclarationStatement& node) {
    SymbolInfo info;
    info.name = node.dictName;
    info.type = tokenTypeToType(node.valueType);
    info.kind = SymbolKind::DICTIONARY;
    info.keyType = tokenTypeToType(node.keyType);
    if (!symbols.declare(info)) {
        reportError("Variavel '" + node.dictName + "' ja foi declarada neste escopo.");
    }
}

void SemanticAnalyzer::visit(StructDeclarationStatement& node) {
    if (!structNames.insert(node.structName).second) {
        reportError("Struct '" + node.structName + "' declarada mais de uma vez.");
    }

    symbols.enterScope();
    analyzeStatements(node.body);
    symbols.exitScope();
}

void SemanticAnalyzer::visit(FunctionDeclarationStatement& node) {
    if (functions.find(node.functionName) == functions.end()) {
        registerFunction(node);
    }

    bool previousInside = insideFunction;
    Type previousReturn = currentReturnType;
    insideFunction = true;
    currentReturnType = tokenTypeToType(node.returnType);

    symbols.enterScope();
    for (const Parameter& param : node.parameters) {
        if (!symbols.declare(param.name, tokenTypeToType(param.type))) {
            reportError("Parametro '" + param.name + "' duplicado na funcao '" +
                        node.functionName + "'.");
        }
    }
    analyzeStatements(node.body);
    symbols.exitScope();

    insideFunction = previousInside;
    currentReturnType = previousReturn;
}

void SemanticAnalyzer::visit(FunctionCallStatement& node) {
    evaluateExpression(node.callExpression.get());
}

void SemanticAnalyzer::visit(ReturnStatement& node) {
    Type valueType = Type::UNKNOWN;
    bool hasValue = (node.returnValue != nullptr);
    if (hasValue) {
        valueType = evaluateExpression(node.returnValue.get());
    }

    if (!insideFunction) {
        reportError("Comando 'return' usado fora de uma funcao.");
        return;
    }

    if (hasValue && !isAssignable(currentReturnType, valueType)) {
        reportError("Tipo de retorno incompativel: a funcao espera '" +
                    typeToString(currentReturnType) + "', mas o 'return' e do tipo '" +
                    typeToString(valueType) + "'.");
    }
}
