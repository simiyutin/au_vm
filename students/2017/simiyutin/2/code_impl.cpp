#include "include/code_impl.h"
#include <cassert>

using namespace mathvm;
using namespace std;

using pVar = Var *;



Status* CodeImpl::execute(vector<pVar> &vars) {
    std::cout << "executing!" << std::endl;

    for (pVar var : vars) {
        switch (var->type()) {
            case VT_INT:
                getVarMap<int64_t>()[topMostVars[var->name()]] = var->getIntValue();
                break;
            case VT_DOUBLE:
                getVarMap<double>()[topMostVars[var->name()]] = var->getDoubleValue();
                break;
            case VT_STRING:
                getVarMap<std::string>()[topMostVars[var->name()]] = var->getStringValue();
                break;
            default:
                break;
        }
    }


    while (executionPoint < bytecode.length()) {
        Instruction instruction = bytecode.getInsn(executionPoint++);
        switch (instruction) {
            case BC_ILOAD:
                std::cout << "load" << std::endl;
                handleLoad<int64_t>();
                break;


            case BC_DLOAD:
                std::cout << "load d" << std::endl;
                handleLoad<double>();
                break;


            case BC_SLOAD:
                std::cout << "load" << std::endl;
                handleLoad<std::string>();
                break;


            case BC_IADD:
                std::cout << "add" << std::endl;
                handleAdd<int64_t>();
                break;


            case BC_DADD:
                std::cout << "add d" << std::endl;
                handleAdd<double>();
                break;


            case BC_STOREIVAR:
                std::cout << "store" << std::endl;
                handleStoreVar<int64_t>();
                break;


            case BC_STOREDVAR:
                std::cout << "store d" << std::endl;
                handleStoreVar<double>();
                break;

                //todo
//            case BC_STORESVAR:
//                std::cout << "store" << std::endl;
//                handleStoreVar<std::string>();
//                break;

            case BC_LOADDVAR:
                std::cout << "loadvar d" << std::endl;
                handleLoadVar<double>();
                break;

            case BC_LOADIVAR:
                std::cout << "loadvar" << std::endl;
                handleLoadVar<int64_t>();
                break;

                //todo
//            case BC_LOADSVAR:
//                std::cout << "loadvar" << std::endl;
//                handleLoadVar<std::string>();
//                break;

            default:
                std::cout << "grust' pichal" << std::endl;
                break;
        }
    }

    for (pVar var : vars) {
        switch (var->type()) {
            case VT_INT:
                var->setIntValue(getVarMap<int64_t>()[topMostVars[var->name()]]);
                break;
            case VT_DOUBLE:
                var->setDoubleValue(getVarMap<double>()[topMostVars[var->name()]]);
                break;
            case VT_STRING: {
                std::string val = getVarMap<std::string>()[topMostVars[var->name()]];
                var->setStringValue(val.c_str());
                break;
            }
            default:
                break;
        }

    }

    std::cout << "executed!" << std::endl;
    return Status::Ok();
}

void CodeImpl::disassemble(std::ostream &out, mathvm::FunctionFilter *filter) {
    bytecode.dump(out);
}