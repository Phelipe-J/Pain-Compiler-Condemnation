#include "SymbolTable.hpp"

void SymbolTable::enterScope() {
    scopes.emplace_back();
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

bool SymbolTable::declare(const std::string& name, Type type) {
    return declare(SymbolInfo{name, type, SymbolKind::SCALAR, 0, Type::UNKNOWN});
}

bool SymbolTable::declare(const SymbolInfo& info) {
    if (scopes.empty()) {
        scopes.emplace_back();
    }
    auto& currentScope = scopes.back();
    if (currentScope.find(info.name) != currentScope.end()) {
        return false;
    }
    currentScope[info.name] = info;
    return true;
}

SymbolInfo* SymbolTable::lookup(const std::string& name) {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) {
            return &found->second;
        }
    }
    return nullptr;
}
