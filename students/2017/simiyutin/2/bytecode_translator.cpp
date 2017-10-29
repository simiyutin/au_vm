#include "mathvm.h"
#include "parser.h"

using namespace mathvm;

//code is output parameter
Status *BytecodeTranslatorImpl::translate(const string &program, Code **code) {
    Parser parser;
    Status * status = parser.parseProgram(program);
    if (status->isError()) {
        return status;
    }

    //todo your code here


    return status;
}
