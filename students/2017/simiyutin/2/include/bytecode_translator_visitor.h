#pragma once

#include "../../../../../include/visitors.h"

#include <sstream>
#include <unordered_map>
#include "visitors.h"

struct BytecodeTranslatorVisitor : mathvm::AstBaseVisitor {

#define VISITOR_FUNCTION(type, name) \
    virtual void visit##type(mathvm::type* node) override;

    FOR_NODES(VISITOR_FUNCTION)
#undef VISITOR_FUNCTION

    void printBytecode() const {
        for (auto el: instr) {
            std::cout << el << std::endl;
        }
    }

private:
    std::unordered_map<std::string, int> varMap;
    int globalVarCounter = 0;
    std::vector<std::string> instr;
};