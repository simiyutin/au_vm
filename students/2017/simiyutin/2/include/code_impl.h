#pragma once
#include "../../../../../include/mathvm.h"
#include "mathvm.h"
#include <vector>
#include <map>


struct CodeImpl : mathvm::Code {

    CodeImpl(const mathvm::Bytecode & bytecode, const std::map<std::string, int> & topMostVars) :
            bytecode(bytecode), executionPoint(0), topMostVars(topMostVars) {}

    mathvm::Status* execute(std::vector<mathvm::Var*>& vars) override;

    void disassemble(std::ostream& out = std::cout, mathvm::FunctionFilter* filter = 0) override;

private:
    struct Stack : mathvm::Bytecode {
        int64_t getInt64() {
            return getTyped<int64_t>();
        }

        template <typename T>
        T getTyped() {
            if (_data.size() < sizeof(T)) {
                std::cout << "bad" << std::endl;
                std::cout << _data.size() << std::endl;
                exit(42);
            }

            T result = mathvm::Bytecode::getTyped<T>(_data.size() - sizeof(T));
            for (size_t i = 0; i < sizeof(T); ++i) {
                _data.pop_back();
            }
            return result;
        }
    };

    template<typename A>
    struct identity { typedef A type; };

    template<typename T>
    void handleLoad() {
        handleLoad(identity<T>());
    }

    template <typename T>
    void handleLoad(identity<T>) {
        T val = bytecode.getTyped<T>(executionPoint);
        stack.addTyped(val);
        executionPoint += sizeof(T);
    }

    void handleLoad(identity<std::string>) {
        uint16_t stringTableIndex = bytecode.getInt16(executionPoint);
        stack.addUInt16(stringTableIndex);
        executionPoint += sizeof(uint16_t);
    }


    template <typename T>
    void handleAdd() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst + snd); //todo pop?
    }


    template <typename T>
    void handleStoreVar() {
        uint16_t varId = bytecode.getInt16(executionPoint);
        executionPoint += sizeof(uint16_t);
        T val = stack.getTyped<T>();
        getVarMap<T>()[varId] = val;
    }


    template <typename T>
    void handleLoadVar() {
        uint16_t varId = bytecode.getInt16(executionPoint);
        executionPoint += sizeof(uint16_t);
        T val = getVarMap<T>()[varId];
        std::cout << "var val: " << val << std::endl;
        stack.addTyped(val);
    }

    template <typename T>
    std::map<int, T> & getVarMap() {
        return getVarMap(identity<T>());
    };

    std::map<int, int64_t> & getVarMap(identity<int64_t>) {
        static std::map<int, int64_t> vmap;
        return vmap;
    };

    std::map<int, double> & getVarMap(identity<double >) {
        static std::map<int, double> vmap;
        return vmap;
    };

    std::map<int, std::string> & getVarMap(identity<std::string>) {
        static std::map<int, std::string> vmap;
        return vmap;
    };



    mathvm::Bytecode bytecode;
    size_t executionPoint;
    Stack stack;

    std::map<std::string, int> topMostVars;
};