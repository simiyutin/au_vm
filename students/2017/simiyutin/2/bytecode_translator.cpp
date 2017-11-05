#include "mathvm.h"
#include "parser.h"
#include "include/bytecode_translator_visitor.h"

using namespace mathvm;

//code is output parameter
//todo что такое inlined into instruction stream
Status *BytecodeTranslatorImpl::translate(const string &program, Code **code) {
    std::cout << "bytecode translator!" << std::endl;

    Parser parser;
    Status * status = parser.parseProgram(program);
    if (status->isError()) {
        return status;
    }

    BytecodeTranslatorVisitor visitor;
    FunctionNode * node = parser.top()->node();
    node->visitChildren(&visitor);
    visitor.printBytecode();


    return status;
    //todo your code here

    //context? как создавать переменные? Видимо, сразу при компиляции?
    //генерировать айдишки переменных и контекстов. При интерпретации создать мапу и хранить в ней значения по айдишке или на стеке
    //int x; x = 2 + 3;
    //push 2
    //push 3
    //add
    //storeval %x

    return status;
}
