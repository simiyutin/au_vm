#pragma once

#include "../../../../../include/visitors.h"

#include <sstream>
#include "visitors.h"

struct BytecodeTranslatorVisitor : mathvm::AstBaseVisitor {

#define VISITOR_FUNCTION(type, name) \
    virtual void visit##type(mathvm::type* node) override;

    FOR_NODES(VISITOR_FUNCTION)
#undef VISITOR_FUNCTION

    const std::string res = "hfhfhfh";
private:

};