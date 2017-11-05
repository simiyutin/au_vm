#include "include/code_impl.h"
#include <cassert>

using namespace mathvm;
using namespace std;

using pVar = Var *;
Status* CodeImpl::execute(vector<pVar> &vars) {
    std::cout << "executing!" << std::endl;
    while (executionPoint < bytecode.length()) {
        Instruction instruction = bytecode.getInsn(executionPoint++);
        switch (instruction) {
            case BC_ILOAD: {
                std::cout << "load" << std::endl;
                std::cout << bytecode.length() << std::endl;
                std::cout << executionPoint << std::endl;
                int64_t val = bytecode.getInt64(executionPoint);
                std::cout << val << std::endl;
                stack.addInt64(val);
                std::cout << "added!" << std::endl;
                executionPoint += sizeof(int64_t);
                break;
            }

            case BC_IADD: {
                std::cout << "add" << std::endl;
                int64_t fst = stack.getInt64(); //todo make
                int64_t snd = stack.getInt64(); //todo make
                std::cout << fst << std::endl;
                std::cout << snd << std::endl;
                stack.addInt64(fst + snd);
                break;
            }

            case BC_STOREIVAR: {
                std::cout << "store" << std::endl;
                uint16_t varId = bytecode.getInt16(executionPoint);
                executionPoint += sizeof(uint16_t);
                std::cout << varId << std::endl;
                int64_t val = stack.getInt64();
                std::cout << val << std::endl;
                std::cout << "ready for storing" << std::endl;
                varmap[varId] = val; //TODO СДЕЛАТЬ НОРМАЛЬНЫЙ СТЕК СО СТЕКФРЕЙМАМИ
                std::cout << bytecode.length() << std::endl;
                std::cout << executionPoint << std::endl;
                break;
            }

            default:
                std::cout << "grust' pichal" << std::endl;
                break;
        }
    }

    std::cout << "executed!" << std::endl;
    return Status::Ok();
}

void CodeImpl::disassemble(std::ostream &out, mathvm::FunctionFilter *filter) {
    bytecode.dump(out);
}