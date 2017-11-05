#include "mathvm.h"
#include "parser.h"
#include "include/bytecode_translator_visitor.h"
#include "include/code_impl.h"

using namespace mathvm;

//code is output parameter
Status *BytecodeTranslatorImpl::translate(const string &program, Code **code) {

    Parser parser;
    Status * status = parser.parseProgram(program);
    if (status->isError()) {
        return status;
    }

    BytecodeTranslatorVisitor visitor;
    FunctionNode * node = parser.top()->node();
    node->visitChildren(&visitor);
    visitor.printBytecode();

    (*code) = new CodeImpl(visitor.getBytecode());

    return status;
}
