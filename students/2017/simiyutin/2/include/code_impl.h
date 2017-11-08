#pragma once
#include "../../../../../include/mathvm.h"
#include "mathvm.h"
#include <vector>
#include <map>


namespace detail {
    template <typename T>
    struct image;

    template <>
    struct image<int64_t> {
        using type = int64_t;
    };

    template <>
    struct image<double> {
        using type = double;
    };

    template <>
    struct image<std::string> {
        using type = uint16_t;
    };
}

struct CodeImpl : mathvm::Code {

    CodeImpl(const mathvm::Bytecode & bytecode, const std::map<std::string, int> & topMostVars, const std::vector<std::string> & stringConstants) :
            bytecode(bytecode), executionPoint(0), topMostVars(topMostVars), stringConstants(stringConstants) {}

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
                std::cout << "stack has less bytes than needed for requested type" << std::endl;
                std::cout << "cur stack size: " << _data.size() << std::endl;
                std::cout << "needed: " << sizeof(T) << std::endl;
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
    void handleLoad1() {
        T val = 1;
        stack.addTyped(val);
    }

    template <typename T>
    void handleLoad0() {
        T val = 0;
        stack.addTyped(val);
    }


    template <typename T>
    void handleAdd() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst + snd);
    }

    template <typename T>
    void handleSub() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst - snd);
    }

    template <typename T>
    void handleMul() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst * snd);
    }

    template <typename T>
    void handleDiv() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst / snd);
    }

    template <typename T>
    void handleMod() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst % snd);
    }

    template <typename T>
    void handleAnd() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst & snd);
    }

    template <typename T>
    void handleOr() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst | snd);
    }

    template <typename T>
    void handleXor() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst ^ snd);
    }

    template <typename T>
    void handleNeg() {
        T fst = stack.getTyped<T>();
        stack.addTyped(-fst);
    }

    template <typename T>
    void handleStoreVar() {
        handleStoreVar(identity<T>());
    }

    template <typename T>
    void handleStoreVar(identity<T>) {
        uint16_t varId = bytecode.getInt16(executionPoint);
        executionPoint += sizeof(uint16_t);
        T val = stack.getTyped<T>();
        getVarMap<T>()[varId] = val;
    }

    void handleStoreVar(identity<std::string>) {
        uint16_t varId = bytecode.getInt16(executionPoint);
        executionPoint += sizeof(uint16_t);
        uint16_t stringId = stack.getTyped<uint16_t>();
        getVarMap<std::string>()[varId] = stringId;
    }


    template <typename T>
    void handleLoadVar() {
        handleLoadVar(identity<T>());
    }

    template <typename T>
    void handleLoadVar(identity<T>) {
        uint16_t varId = bytecode.getInt16(executionPoint);
        executionPoint += sizeof(uint16_t);
        T val = getVarMap<T>()[varId];
        stack.addTyped(val);
    }

    void handleLoadVar(identity<std::string>) {
        uint16_t varId = bytecode.getInt16(executionPoint);
        executionPoint += sizeof(uint16_t);
        uint16_t stringId = getVarMap<std::string>()[varId];
        stack.addUInt16(stringId);
    }

    template <typename T>
    void handlePrint() {
        handlePrint(identity<T>());
    }

    template <typename T>
    void handlePrint(identity<T>) {
        T el = stack.getTyped<T>();
        std::cout << el;
    }

    void handlePrint(identity<std::string>) {
        uint16_t id = stack.getTyped<uint16_t>();
        std::cout << stringConstants[id];
    }

    void handleCmpge() {
        handleConditionalJump([](int upper, int lower){ return upper >= lower;});
    }

    void handleCmple() {
        handleConditionalJump([](int upper, int lower){ return upper <= lower;});
    }

    void handleCmpg() {
        handleConditionalJump([](int upper, int lower){ return upper > lower;});
    }

    void handleCmpl() {
        handleConditionalJump([](int upper, int lower){ return upper < lower;});
    }

    void handleCmpe() {
        handleConditionalJump([](int upper, int lower){ return upper == lower;});
    }

    void handleCmpne() {
        handleConditionalJump([](int upper, int lower){ return upper != lower;});
    }

    template <typename FUNCTOR>
    void handleConditionalJump(const FUNCTOR & f) {
        int16_t shift = bytecode.getInt16(executionPoint);
        int64_t upper = stack.getTyped<int64_t>();
        int64_t lower = stack.getTyped<int64_t>();
        stack.addTyped(lower);
        stack.addTyped(upper);
        if (f(upper, lower)) {
            executionPoint += shift;
        } else {
            executionPoint += sizeof(int16_t);
        }
    }

    void handleJa() {
        int16_t shift = bytecode.getInt16(executionPoint);
        executionPoint += shift;
    }

    //assume swapping integers
    void handleSwap() {
        int64_t upper = stack.getTyped<int64_t>();
        int64_t lower = stack.getTyped<int64_t>();
        stack.addInt64(upper);
        stack.addInt64(lower);
    }

    //assume popping integers
    void handlePop() {
        int64_t upper = stack.getTyped<int64_t>();
        (void) upper;
    }

    template <typename T>
    std::map<int, typename detail::image<T>::type> & getVarMap() {
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

    //map from variable to string id
    std::map<int, uint16_t> & getVarMap(identity<std::string>) {
        static std::map<int, uint16_t> vmap;
        return vmap;
    };



    mathvm::Bytecode bytecode;
    size_t executionPoint;
    Stack stack;
    std::map<std::string, int> topMostVars;
    std::vector<std::string> stringConstants;
};