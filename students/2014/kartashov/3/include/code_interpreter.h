#ifndef CODE_INTERPRETER_H__
#define CODE_INTERPRETER_H__

#include "mathvm.h"
#include "execution_exception.h"

#include <vector>
#include <stack>
#include <map>
#include "data_holder.h"
#include "context.h"

using namespace mathvm;

class CodeInterpreter {
  public:
    typedef std::vector<DataHolder> DataStack;
    typedef std::pair<DataHolder, DataHolder> HolderPair;
    typedef std::vector<int32_t> ProgramCounterStack;
    typedef std::vector<TranslatedFunction*> FunctionStack;
    typedef std::map<int16_t, ContextPtr> ContextMap;

    CodeInterpreter(Code* code): mCode(code), mCurrentContext(new Context) {
      auto topFunction = mCode->functionByName(topFunctionName);
      mFunctionStack.push_back(topFunction);
      mProgramCounterStack.push_back(0);
    }

    void execute() {
      while(!mFunctionStack.empty()) {
//        printf("%ld\n", mFunctionStack.size());
        executeInstruction();
      }
    }

    void executeInstruction() {
      Instruction instruction = currentBytecode()->getInsn(programCounter());
      next();

      switch (instruction) {
//        case BC_INVALID: throw ExecutionException("Invalid instruction"); break;
        case BC_DLOAD: dload(); break;
        case BC_ILOAD: iload(); break;
        case BC_SLOAD: sload(); break;
        case BC_DLOAD0: dload0(); break;
        case BC_ILOAD0: iload0(); break;
        case BC_SLOAD0: sload0(); break;
        case BC_ILOAD1: iload1(); break;
        case BC_DLOAD1: dload1(); break;
        case BC_ILOADM1: iloadm1(); break;
        case BC_DLOADM1: dloadm1(); break;
        case BC_IADD: iadd(); break;
        case BC_DADD: dadd(); break;
        case BC_ISUB: isub(); break;
        case BC_DSUB: dsub(); break;
        case BC_IMUL: imul(); break;
        case BC_DMUL: dmul(); break;
        case BC_IDIV: idiv(); break;
        case BC_DDIV: ddiv(); break;
        case BC_IMOD: imod(); break;
        case BC_INEG: ineg(); break;
        case BC_DNEG: dneg(); break;
        case BC_IAOR: iaor(); break;
        case BC_IAAND: iaand(); break;
        case BC_IAXOR: iaxor(); break;
        case BC_IPRINT: iprint(); break;
        case BC_DPRINT: dprint(); break;
        case BC_SPRINT: sprint(); break;
        case BC_I2D: i2d(); break;
        case BC_D2I: d2i(); break;
        case BC_SWAP: swap(); break;
        case BC_POP: pop(); break;
/*        DO(LOADDVAR0, "Load double from variable 0, push on TOS.", 1)   \
        DO(LOADDVAR1, "Load double from variable 1, push on TOS.", 1)   \
        DO(LOADDVAR2, "Load double from variable 2, push on TOS.", 1)   \
        DO(LOADDVAR3, "Load double from variable 3, push on TOS.", 1)   \
        DO(LOADIVAR0, "Load int from variable 0, push on TOS.", 1)      \
        DO(LOADIVAR1, "Load int from variable 1, push on TOS.", 1)      \
        DO(LOADIVAR2, "Load int from variable 2, push on TOS.", 1)      \
        DO(LOADIVAR3, "Load int from variable 3, push on TOS.", 1)      \
        DO(LOADSVAR0, "Load string from variable 0, push on TOS.", 1)   \
        DO(LOADSVAR1, "Load string from variable 1, push on TOS.", 1)   \
        DO(LOADSVAR2, "Load string from variable 2, push on TOS.", 1)   \
        DO(LOADSVAR3, "Load string from variable 3, push on TOS.", 1)   \
        DO(STOREDVAR0, "Pop TOS and store to double variable 0.", 1)    \
        DO(STOREDVAR1, "Pop TOS and store to double variable 1.", 1)    \
        DO(STOREDVAR2, "Pop TOS and store to double variable 0.", 1)    \
        DO(STOREDVAR3, "Pop TOS and store to double variable 3.", 1)    \
        DO(STOREIVAR0, "Pop TOS and store to int variable 0.", 1)       \
        DO(STOREIVAR1, "Pop TOS and store to int variable 1.", 1)       \
        DO(STOREIVAR2, "Pop TOS and store to int variable 0.", 1)       \
        DO(STOREIVAR3, "Pop TOS and store to int variable 3.", 1)       \
        DO(STORESVAR0, "Pop TOS and store to string variable 0.", 1)    \
        DO(STORESVAR1, "Pop TOS and store to string variable 1.", 1)    \
        DO(STORESVAR2, "Pop TOS and store to string variable 0.", 1)    \
        DO(STORESVAR3, "Pop TOS and store to string variable 3.", 1)    \*/
        case BC_LOADDVAR: loaddvar(); break;
        case BC_LOADIVAR: loadivar(); break;
        case BC_LOADSVAR: loadsvar(); break;
        case BC_STOREDVAR: storedvar(); break;
        case BC_STOREIVAR: storeivar(); break;
        case BC_STORESVAR: storesvar(); break;
        /*
        DO(LOADCTXDVAR, "Load double from variable, whose 2-byte context and 2-byte id inlined to insn stream, push on TOS.", 5) \
        DO(LOADCTXIVAR, "Load int from variable, whose 2-byte context and 2-byte id is inlined to insn stream, push on TOS.", 5) \
        DO(LOADCTXSVAR, "Load string from variable, whose 2-byte context and 2-byte id is inlined to insn stream, push on TOS.", 5) \
        DO(STORECTXDVAR, "Pop TOS and store to double variable, whose 2-byte context and 2-byte id is inlined to insn stream.", 5) \
        DO(STORECTXIVAR, "Pop TOS and store to int variable, whose 2-byte context and 2-byte id is inlined to insn stream.", 5) \
        DO(STORECTXSVAR, "Pop TOS and store to string variable, whose 2-byte context and 2-byte id is inlined to insn stream.", 5) \*/
        case BC_DCMP: dcmp(); break;
        case BC_ICMP: icmp(); break;
        case BC_JA: break;
        case BC_IFICMPNE: break;
        case BC_IFICMPE: break;
        case BC_IFICMPG: break;
        case BC_IFICMPGE: break;
        case BC_IFICMPL: break;
        case BC_IFICMPLE: break;
        case BC_DUMP: dump(); break;
        case BC_STOP: throw ExecutionException("Execution stopped");
        case BC_CALL: break;
        case BC_CALLNATIVE: throw ExecutionException("Natives are not supported"); break;
        case BC_RETURN: bc_return(); break;
        case BC_BREAK: next(); break;
        default: printf("BOMBOM\n"); break;
      }
    }

    void loaddvar() {
      int16_t id = currentBytecode()->getInt16(programCounter());
      auto result = currentContext()->load(id).doubleValue;
      next2();
      push(result);
    }

    void loadivar() {
      int16_t id = currentBytecode()->getInt16(programCounter());
      auto result = currentContext()->load(id).intValue;
      next2();
      push(result);
    }

    void loadsvar() {
      int16_t id = currentBytecode()->getInt16(programCounter());
      auto result = currentContext()->load(id).stringId;
      next2();
      push(result);
    }

    void storeivar() {
      auto args = tos();
      int16_t id = currentBytecode()->getInt16(programCounter());
      currentContext()->store(id, args);
      next2();
    }

    void storedvar() {
      auto args = tos();
      int16_t id = currentBytecode()->getInt16(programCounter());
      currentContext()->store(id, args);
      next2();
    }

    void storesvar() {
      auto args = tos();
      int16_t id = currentBytecode()->getInt16(programCounter());
      currentContext()->store(id, args);
      next2();
    }

    void dload() {
      push(currentBytecode()->getDouble(programCounter()));
      next8();
    }

    void iload() {
      push(currentBytecode()->getInt64(programCounter()));
      next8();
    }

    void sload() {
      int16_t id = currentBytecode()->getInt16(programCounter());
      push(id);
      next2();
    }

    void dload0() {
      push((double) 0.0);
    }

    void iload0() {
      push((int64_t) 0);
    }

    void sload0() {
      int16_t id = mCode->makeStringConstant("");
      push(id);
    }

    void iload1() {
      push((int64_t) 1);
    }

    void dload1() {
      push((double) 1.0);
    }

    void iloadm1() {
      push((int64_t) -1);
    }

    void dloadm1() {
      push((double) -1.0);
    }

    void iadd() {
      auto args = topPair();
      push(args.first.intValue + args.second.intValue);
    }

    void dadd() {
      auto args = topPair();
      push(args.first.doubleValue + args.second.doubleValue);
    }

    void isub() {
      auto args = topPair();
      push(args.first.intValue - args.second.intValue);
    }

    void dsub() {
      auto args = topPair();
      push(args.first.doubleValue - args.second.doubleValue);
    }

    void imul() {
      auto args = topPair();
      push(args.first.intValue * args.second.intValue);
    }

    void dmul() {
      auto args = topPair();
      push(args.first.doubleValue * args.second.doubleValue);
    }

    void idiv() {
      auto args = topPair();
      push(args.first.intValue / args.second.intValue);
    }

    void ddiv() {
      auto args = topPair();
      push(args.first.doubleValue / args.second.doubleValue);
    }

    void imod() {
      auto args = topPair();
      push(args.first.intValue % args.second.intValue);
    }

    void ineg() {
      auto args = tos();
      push((int64_t) !args.intValue);
    }

    void dneg() {
      auto args = tos();
      push((int64_t) !args.doubleValue);
    }

    void iaor() {
      auto args = topPair();
      push(args.first.intValue | args.second.intValue);
    }

    void iaand() {
      auto args = topPair();
      push(args.first.intValue & args.second.intValue);
    }

    void iaxor() {
      auto args = topPair();
      push(args.first.intValue ^ args.second.intValue);
    }

    void iprint() {
      auto args = tos();
      printf("%ld", args.intValue);
    }

    void dprint() {
      auto args = tos();
      printf("%f", args.doubleValue);
    }

    void sprint() {
      auto args = tos();
      printf("%s", mCode->constantById(args.stringId).c_str());
    }

    void i2d() {
      auto args = topRef();
      args.doubleValue = (double) args.intValue;
    }

    void d2i() {
      auto args = topRef();
      args.intValue = (int64_t) args.doubleValue;
    }

    void bc_return() {
      mFunctionStack.pop_back();
      mProgramCounterStack.pop_back();
    }

    void swap() {
      std::iter_swap(mDataStack.rbegin(), mDataStack.rbegin() + 1);
    }

    void icmp() {
      auto args = topPair();
      if (args.first.intValue == args.second.intValue) {
        push((int64_t) 0);
      } else if (args.first.intValue < args.second.intValue) {
        push((int64_t) -1);
      } else {
        push((int64_t) 1);
      }
    }

    void dcmp() {
      auto args = topPair();
      if (args.first.doubleValue == args.second.doubleValue) {
        push((int64_t) 0);
      } else if (args.first.doubleValue < args.second.doubleValue) {
        push((int64_t) -1);
      } else {
        push((int64_t) 1);
      }
    }

    void dump() {
      auto args = top();
      printf("%d\n", args.stringId);
      printf("%ld\n", args.intValue);
      printf("%f\n", args.doubleValue);
    }

    int32_t programCounter() {return mProgramCounterStack.back();}

    void push(int16_t value) {
      DataHolder holder;
      holder.stringId = value;
      mDataStack.push_back(holder);
    }

    void push(int64_t value) {
      DataHolder holder;
      holder.intValue = value;
      mDataStack.push_back(holder);
    }

    void push(double value) {
      DataHolder holder;
      holder.doubleValue = value;
      mDataStack.push_back(holder);
    }

    void pop() {mDataStack.pop_back();}

    DataHolder top() {return mDataStack.back();}

    DataHolder& topRef() {return mDataStack.back();}

    HolderPair topPair() {
      HolderPair result;
      result.first = tos();
      result.second = tos();
      return result;
    }

    DataHolder tos() {
      auto result = top();
      pop();
      return result;
    }

    Bytecode* currentBytecode() {
      return static_cast<BytecodeFunction*>(mFunctionStack.back())->bytecode();
    }

    ContextPtr currentContext() {
      return mCurrentContext;
    }

    void next(size_t i = 1) {mProgramCounterStack.back() += i;}
    void next2() {next(2);}
    void next3() {next(3);}
    void next4() {next(4);}
    void next8() {next(8);}

    const std::string topFunctionName = "<top>";

  private:
    ProgramCounterStack mProgramCounterStack;
    FunctionStack mFunctionStack;
    Code* mCode;
    DataStack mDataStack;
    ContextMap mContextMap;
    ContextPtr mCurrentContext;
};

#endif