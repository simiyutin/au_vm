#include "mathvm.h"
#include "parser.h"

using namespace mathvm;

Status *BytecodeTranslatorImpl::translate(const string &program, Code **code) {
    Parser parser;
    Status * status = parser.parseProgram(program);
    if (status->isError()) {
        return status;
    }

    //todo your code here


    return status;
}
