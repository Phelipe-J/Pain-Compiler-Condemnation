#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "AST.hpp"

enum class SymbolKind {
    SCALAR,
    ARRAY,
    DICTIONARY
};

struct SymbolInfo {
    std::string name;
    Type type = Type::UNKNOWN;
    SymbolKind kind = SymbolKind::SCALAR;
    int dimensions = 0;
    Type keyType = Type::UNKNOWN;
};

class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;

public:
    void enterScope();
    void exitScope();

    bool declare(const std::string& name, Type type);
    bool declare(const SymbolInfo& info);

    SymbolInfo* lookup(const std::string& name);
};
