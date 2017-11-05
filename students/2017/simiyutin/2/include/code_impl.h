#pragma once
#include "../../../../../include/mathvm.h"
#include "mathvm.h"
#include <vector>
#include <map>

struct CodeImpl : mathvm::Code {

    CodeImpl(const mathvm::Bytecode & bytecode) : bytecode(bytecode), executionPoint(0) {}

    mathvm::Status* execute(std::vector<mathvm::Var*>& vars) override;

    void disassemble(std::ostream& out = std::cout, mathvm::FunctionFilter* filter = 0) override;

private:
    struct Stack : mathvm::Bytecode {
        int64_t getInt64() {
            std::cout << "getting result" << std::endl;
            if (_data.size() < sizeof(int64_t)) {
                std::cout << "bad" << std::endl;
                std::cout << _data.size() << std::endl;
                exit(42);
            }

            int64_t result = mathvm::Bytecode::getInt64(_data.size() - sizeof(int64_t));
            std::cout << "result = " << result << std::endl;
            for (size_t i = 0; i < sizeof(int64_t); ++i) {
                _data.pop_back();
            }
            return result;
        }
    };
    mathvm::Bytecode bytecode;
    size_t executionPoint;
    Stack stack;
    std::map<int, int> varmap;
};