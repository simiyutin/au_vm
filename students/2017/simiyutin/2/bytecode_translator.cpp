#include "mathvm.h"
#include "parser.h"

#include "../../../../../include/mathvm.h"
#include "../../../../../include/ast.h"
#include "../../../../../include/visitors.h"

#include "include/bytecode_translator_visitor.h"
#include "include/code_impl.h"
#include <map>

using namespace mathvm;
using namespace std;

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

    Bytecode bytecode = visitor.getBytecode();
    map<string, int> topMostVars = visitor.getTopMostVars();
    vector<string> stringConstants = visitor.getStringConstants();
    (*code) = new CodeImpl(bytecode, topMostVars, stringConstants);

    return status;
}
