#pragma once

#include "mathvm.h"

using namespace mathvm;
using namespace std;

struct CodeImpl : mathvm::Code {
    Status* execute(vector<Var*>& vars) override {
        return nullptr;
    };

    void disassemble(ostream& out = cout, FunctionFilter* filter = 0) override {

    };
};