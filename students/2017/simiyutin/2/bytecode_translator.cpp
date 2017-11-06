#include "mathvm.h"
#include "parser.h"

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

    map<const AstVar*, int> varMap = visitor.getVarMap();
    map<string, int> topMostVars;

    Scope::VarIterator it(parser.top()->scope());
    while (it.hasNext()) {
        const AstVar * var = it.next();
        topMostVars[var->name()] = varMap[var];
    }

    (*code) = new CodeImpl(visitor.getBytecode(), topMostVars);

    return status;
}
