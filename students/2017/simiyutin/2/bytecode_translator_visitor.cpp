#include "include/bytecode_translator_visitor.h"
#include "include/insn_factory.h"
#include <string>

using namespace mathvm;


void BytecodeTranslatorVisitor::visitBinaryOpNode(BinaryOpNode *node) {
    //std::cout << "start BinaryOpNode" << std::endl;
    node->right()->visit(this);
    node->left()->visit(this);

    switch (node->kind()) {
        case tADD: {
            VarType type = stack.back();
            bytecode.addInsn(getAddInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tSUB: {
            VarType type = stack.back();
            bytecode.addInsn(getSubInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tMUL: {
            VarType type = stack.back();
            bytecode.addInsn(getMulInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tDIV: {
            VarType type = stack.back();
            bytecode.addInsn(getDivInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tMOD: {
            VarType type = stack.back();
            bytecode.addInsn(getModInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tAAND: {
            VarType type = stack.back();
            bytecode.addInsn(getAndInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tAOR: {
            VarType type = stack.back();
            bytecode.addInsn(getOrInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tAXOR: {
            VarType type = stack.back();
            bytecode.addInsn(getXorInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tLT: {
            bytecode.addInsn(BC_IFICMPGE);
            bytecode.addUInt16(-1);
            break;
        }
        default:
            std::cout << "unhandled binary token:" << tokenStr(node->kind()) << std::endl;
            exit(42);
            break;
    }
    //std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitUnaryOpNode(UnaryOpNode *node) {
    //std::cout << "start UnaryOpNode" << std::endl;
    node->visitChildren(this);
    switch (node->kind()) {
        case tSUB: {
            VarType type = stack.back();
            bytecode.addInsn(getNegInsn(type));
            break;
        }
        default:
            std::cout << "unhandled unary token:" << tokenStr(node->kind()) << std::endl;
            exit(42);
            break;
    }
    //std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitStringLiteralNode(StringLiteralNode *node) {
    //std::cout << "string literal:" << node->literal() << std::endl;
    bytecode.addInsn(BC_SLOAD);
    stringConstants.push_back(node->literal());
    bytecode.addUInt16(stringConstants.size() - 1);
    stack.push_back(VT_STRING);
}

void BytecodeTranslatorVisitor::visitDoubleLiteralNode(DoubleLiteralNode *node) {
    //std::cout << "double literal:" << node->literal() << std::endl;
    bytecode.addInsn(BC_DLOAD);
    bytecode.addDouble(node->literal());
    stack.push_back(VT_DOUBLE);
}

void BytecodeTranslatorVisitor::visitIntLiteralNode(IntLiteralNode *node) {
    //std::cout << "int literal:" << node->literal() << std::endl;
    bytecode.addInsn(BC_ILOAD);
    bytecode.addInt64(node->literal());
    stack.push_back(VT_INT);
}

void BytecodeTranslatorVisitor::visitLoadNode(LoadNode *node) {
    //std::cout << "loadNode, var name = " << node->var()->name() << std::endl;
    switch (node->var()->type()) {
        case VT_INT:
            bytecode.addInsn(BC_LOADIVAR);
            bytecode.addUInt16(varMap[node->var()]);
            stack.push_back(VT_INT);
            break;
        case VT_DOUBLE:
            bytecode.addInsn(BC_LOADDVAR);
            bytecode.addUInt16(varMap[node->var()]);
            stack.push_back(VT_DOUBLE);
            break;
        case VT_STRING:
            bytecode.addInsn(BC_LOADSVAR);
            bytecode.addUInt16(varMap[node->var()]);
            stack.push_back(VT_STRING);
            break;
        default:
            break;
    }
}


void BytecodeTranslatorVisitor::visitStoreNode(StoreNode *node) {
    //std::cout << "start StoreNode" << std::endl;

    //calculated value is now on TOS
    node->value()->visit(this);

    if (node->var()->type() != stack.back()) {
        //std::cout << "type mismatch" << std::endl;
    }

    VarType type = stack.back();
    switch (node->op()) {
        case tASSIGN:
            //store tos to variable
            bytecode.addInsn(getStoreInsn(type));
            bytecode.addUInt16(varMap[node->var()]);
            break;
        case tINCRSET:
            //load variable on tos
            bytecode.addInsn(getLoadInsn(type));
            bytecode.addUInt16(varMap[node->var()]);
            //load added val on tos
            bytecode.addInsn(getAddInsn(type));
            //store tos to variable
            bytecode.addInsn(getStoreInsn(type));
            bytecode.addUInt16(varMap[node->var()]);
            break;
        case tDECRSET:
            //load variable on tos
            bytecode.addInsn(getLoadInsn(type));
            bytecode.addUInt16(varMap[node->var()]);
            //load added val on tos
            bytecode.addInsn(getSubInsn(type));
            //store tos to variable
            bytecode.addInsn(getStoreInsn(type));
            bytecode.addUInt16(varMap[node->var()]);
            break;
        default:
            break;
    }

    stack.pop_back();
    //std::cout << "varnamE: " << node->var()->name() << std::endl;
    //std::cout << "end StoreNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitForNode(ForNode *node) {
    //std::cout << "start forNode" << std::endl;
    //std::cout << "expression: " << std::endl;
    node->inExpr()->visit(this);
    //std::cout << "body: " << std::endl;
    node->body()->visit(this);
    //std::cout << "end forNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitWhileNode(WhileNode *node) {//signed offset of jump destination
    //std::cout << "start whileNode" << std::endl;
    int16_t startPosition = bytecode.length();
    node->whileExpr()->visit(this);
    int64_t frontShiftPos = bytecode.length() - 2;
    assert(bytecode.getInt16(frontShiftPos) == -1);

    node->loopBlock()->visit(this);
    bytecode.addInsn(BC_JA);
    int16_t currentPosition = bytecode.length();
    int16_t backShift = startPosition - currentPosition;
//    std::cout << "start position: " << startPosition << std::endl;
//    std::cout << "back position: " << currentPosition << std::endl;
    bytecode.addInt16(backShift);

    int16_t skipOffset = bytecode.length() - frontShiftPos;
//    std::cout << "skipOffset: " << skipOffset << std::endl;

    union {
        int16_t val;
        uint8_t bits[sizeof(int16_t)];
    } u;
    u.val = skipOffset;
    for (size_t i = 0; i < sizeof(u.bits); ++i) {
        bytecode.put(frontShiftPos + i, u.bits[i]);
    }
    //std::cout << "end whileNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitIfNode(IfNode *node) {
    //std::cout << "start ifNode" << std::endl;
    //std::cout << "expression:" << std::endl;
    node->ifExpr()->visit(this);
    //std::cout << "then body:" << std::endl;
    node->thenBlock()->visit(this);
    //std::cout << "else body:" << std::endl;
    node->elseBlock()->visit(this);
    //std::cout << "end ifNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitBlockNode(BlockNode *node) {
    //std::cout << "start blockNode" << std::endl;

    Scope::VarIterator it(node->scope());

    while (it.hasNext()) {
        const AstVar * var = it.next();
        //std::cout << "var: " << var->name() << std::endl;
        varMap[var] = globalVarCounter++;
    }
    if (topMostVariablesNum == -1) {
        topMostVariablesNum = globalVarCounter;
    }

    Scope::FunctionIterator fit(node->scope());
    while (fit.hasNext()) {
        const AstFunction * func = fit.next();
        //std::cout << "fun:" << std::endl;
        func->node()->visit(this);
    }

    node->visitChildren(this);

    //std::cout << "end blockNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitFunctionNode(FunctionNode *node) {
    //std::cout << "start functionNode" << std::endl;
    node->visitChildren(this);
    //std::cout << "end functionNode" << std::endl;

//    NativeCallNode * native = check_native(node);
//    if (native) {
//        native->visit(this);
//    } else {
//        ss_ << "{";
//        newline();
//        tab();
//        node->visitChildren(this);
//        untab();
//        indent("}");
//        newline();
//    }
}

void BytecodeTranslatorVisitor::visitReturnNode(ReturnNode *node) {
    //std::cout << "start returnNode" << std::endl;
    //std::cout << "end returnNode" << std::endl;
//    indent("return");
//    if (node->returnExpr()) {
//        ss_ << ' ';
//        node->returnExpr()->visit(this);
//    }
//    ss_ << ";";
//    newline();
}

void BytecodeTranslatorVisitor::visitCallNode(CallNode *node) {
    //std::cout << "start callNode" << std::endl;
    //std::cout << "end callNode" << std::endl;

//    if (need_indent_) {
//        indent("");
//        ss_ << node->name();
//        print_parameters(node);
//        ss_ << ';';
//        newline();
//    } else {
//        ss_ << node->name();
//        print_parameters(node);
//    }

}

void BytecodeTranslatorVisitor::visitNativeCallNode(NativeCallNode *node) {
    //std::cout << "start nativeCallNode" << std::endl;
    //std::cout << "end nativeCallNode" << std::endl;
//    ss_ << "native '" << node->nativeName()<< "';";
//    newline();
}

void BytecodeTranslatorVisitor::visitPrintNode(PrintNode *node) {
    //std::cout << "start printNode" << std::endl;
    for (int i = 0; i < (int) node->operands(); ++i) {
        node->operandAt(i)->visit(this); //now operand is on TOS
        switch (stack.back()) {
            case VT_INT:
                bytecode.addInsn(BC_IPRINT);
                break;
            case VT_DOUBLE:
                bytecode.addInsn(BC_DPRINT);
                break;
            case VT_STRING:
                bytecode.addInsn(BC_SPRINT);
                break;
            default:
                break;
        }
    }
    //std::cout << "end printNode" << std::endl;

//    indent("print(");
//    for (int i = 0; i < (int) node->operands(); ++i) {
//        node->operandAt(i)->visit(this);
//        if (i < int (node->operands()) - 1) {
//            ss_ << ", ";
//        }
//    }
//    ss_ << ");";
//    newline();
}