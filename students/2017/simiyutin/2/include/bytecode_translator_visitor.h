#pragma once

#include "mathvm.h"
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
        bytecode.dump(std::cout);
    }

    mathvm::Bytecode getBytecode() const {
        return bytecode;
    }

private:
    std::unordered_map<std::string, int> varMap;
    int globalVarCounter = 0;
    mathvm::Bytecode bytecode;
    std::vector<mathvm::VarType> stack;
};