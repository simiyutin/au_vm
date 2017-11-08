#include "include/code_impl.h"
#include <cassert>
#include <functional>

using namespace mathvm;
using namespace std;

using pVar = Var *;

//TODO
// function call, native function call, local variables within loops

Status* CodeImpl::execute(vector<pVar> &vars) {
//    std::cout << "executing!" << std::endl;

    for (pVar var : vars) {
        switch (var->type()) {
            case VT_INT:
                getVarMap<int64_t>()[topMostVars[var->name()]] = var->getIntValue();
                break;
            case VT_DOUBLE:
                getVarMap<double>()[topMostVars[var->name()]] = var->getDoubleValue();
                break;
            case VT_STRING:
                stringConstants[getVarMap<std::string>()[topMostVars[var->name()]]] = var->getStringValue();
                break;
            default:
                break;
        }
    }

    std::map<Instruction, std::function<void()>> callbacks = {
            {BC_ILOAD, [this](){handleLoad<int64_t>();}},
            {BC_DLOAD, [this](){handleLoad<double>();}},
            {BC_SLOAD, [this](){handleLoad<std::string>();}},

            {BC_ILOAD1, [this](){handleLoad1<int64_t>();}},
            {BC_DLOAD1, [this](){handleLoad1<double>();}},

            {BC_ILOAD0, [this](){handleLoad0<int64_t>();}},
            {BC_DLOAD0, [this](){handleLoad0<double>();}},

            {BC_IADD, [this](){handleAdd<int64_t>();}},
            {BC_DADD, [this](){handleAdd<double>();}},

            {BC_ISUB, [this](){handleSub<int64_t>();}},
            {BC_DSUB, [this](){handleSub<double>();}},

            {BC_IMUL, [this](){handleMul<int64_t>();}},
            {BC_DMUL, [this](){handleMul<double>();}},

            {BC_IDIV, [this](){handleDiv<int64_t>();}},
            {BC_DDIV, [this](){handleDiv<double>();}},

            {BC_INEG, [this](){handleNeg<int64_t>();}},
            {BC_DNEG, [this](){handleNeg<double>();}},

            {BC_IMOD, [this](){handleMod<int64_t>();}},
            {BC_IAAND, [this](){handleAnd<int64_t>();}},
            {BC_IAOR, [this](){handleOr<int64_t>();}},
            {BC_IAXOR, [this](){handleXor<int64_t>();}},

            {BC_STOREIVAR, [this](){handleStoreVar<int64_t>();}},
            {BC_STOREDVAR, [this](){handleStoreVar<double>();}},
            {BC_STORESVAR, [this](){handleStoreVar<std::string>();}},

            {BC_LOADDVAR, [this](){handleLoadVar<double>();}},
            {BC_LOADIVAR, [this](){handleLoadVar<int64_t>();}},
            {BC_LOADSVAR, [this](){handleLoadVar<std::string>();}},

            {BC_IPRINT, [this](){handlePrint<int64_t>();}},
            {BC_DPRINT, [this](){handlePrint<double>();}},
            {BC_SPRINT, [this](){handlePrint<std::string>();}},

            {BC_IFICMPGE, [this](){handleCmpge();}},
            {BC_IFICMPLE, [this](){handleCmple();}},
            {BC_IFICMPG, [this](){handleCmpg();}},
            {BC_IFICMPL, [this](){handleCmpl();}},
            {BC_IFICMPE, [this](){handleCmpe();}},
            {BC_IFICMPNE, [this](){handleCmpne();}},
            {BC_JA, [this](){handleJa();}},

            {BC_SWAP, [this](){handleSwap();}},
            {BC_POP, [this](){handlePop();}},
    };

    while (executionPoint < bytecode.length()) {
        Instruction instruction = bytecode.getInsn(executionPoint++);
        auto it = callbacks.find(instruction);
        if (it != callbacks.end()) {
            it->second();
        } else {
            size_t length;
            const char* name = bytecodeName(instruction, &length);
            std::cout << "grust' pichal: " << name << std::endl;
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
                std::string val = stringConstants[getVarMap<std::string>()[topMostVars[var->name()]]];
                var->setStringValue(val.c_str());
                break;
            }
            default:
                break;
        }

    }

//    std::cout << "executed!" << std::endl;
    return Status::Ok();
}

void CodeImpl::disassemble(std::ostream &out, mathvm::FunctionFilter *filter) {
    bytecode.dump(out);
}