#include "include/bytecode_translator_visitor.h"
#include <string>

using namespace mathvm;


void BytecodeTranslatorVisitor::visitBinaryOpNode(BinaryOpNode *node) {
    std::cout << "start BinaryOpNode" << std::endl;
    node->left()->visit(this);
    node->right()->visit(this);
    if (stack.size() >= 2 && stack.back() == VT_INT && stack[stack.size() - 2] == VT_INT) {
        bytecode.addInsn(BC_IADD);
        stack.pop_back();
        stack.pop_back();
        stack.push_back(VT_INT);
    } else if (stack.size() >= 2 && stack.back() == VT_DOUBLE && stack[stack.size() - 2] == VT_DOUBLE) {
        bytecode.addInsn(BC_DADD);
        stack.pop_back();
        stack.pop_back();
        stack.push_back(VT_DOUBLE);
    } else {
        std::cout << "bad stack state" << std::endl;
    }
    std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitUnaryOpNode(UnaryOpNode *node) {
    std::cout << "start UnaryOpNode" << std::endl;
    node->visitChildren(this);
    std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitStringLiteralNode(StringLiteralNode *node) {
    std::cout << "string literal:" << node->literal() << std::endl;
//    bytecode.addInsn(BC_SLOAD);
//    bytecode.addInt64(node->literal());
//    stack.push_back(VT_INT);
}

void BytecodeTranslatorVisitor::visitDoubleLiteralNode(DoubleLiteralNode *node) {
    std::cout << "double literal:" << node->literal() << std::endl;
    bytecode.addInsn(BC_DLOAD);
    bytecode.addDouble(node->literal());
    stack.push_back(VT_DOUBLE);
}

void BytecodeTranslatorVisitor::visitIntLiteralNode(IntLiteralNode *node) {
    std::cout << "int literal:" << node->literal() << std::endl;
    bytecode.addInsn(BC_ILOAD);
    bytecode.addInt64(node->literal());
    stack.push_back(VT_INT);
}

void BytecodeTranslatorVisitor::visitLoadNode(LoadNode *node) {
    std::cout << "loadNode, var name = " << node->var()->name() << std::endl;
}


Instruction getStoreInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_STOREDVAR;
        case VT_INT:
            return BC_STOREIVAR;
        case VT_STRING:
            return BC_STORESVAR;
        default:
            return BC_INVALID;
    }
}

Instruction getLoadInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_LOADDVAR;
        case VT_INT:
            return BC_LOADIVAR;
        case VT_STRING:
            return BC_LOADSVAR;
        default:
            return BC_INVALID;
    }
}

Instruction getAddInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DADD;
        case VT_INT:
            return BC_IADD;
        default:
            return BC_INVALID;
    }
}

Instruction getSubInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DSUB;
        case VT_INT:
            return BC_ISUB;
        default:
            return BC_INVALID;
    }
}


void BytecodeTranslatorVisitor::visitStoreNode(StoreNode *node) {
    std::cout << "start StoreNode" << std::endl;

    //calculated value is now on TOS
    node->value()->visit(this);

    if (node->var()->type() != stack.back()) {
        std::cout << "type mismatch" << std::endl;
    }

    VarType type = stack.back();
    switch (node->op()) {
        case tASSIGN:
            //store tos to variable
            bytecode.addInsn(getStoreInsn(type));
            bytecode.addInt16(varMap[node->var()]);
            break;
        case tINCRSET:
            //load variable on tos
            bytecode.addInsn(getLoadInsn(type));
            bytecode.addInt16(varMap[node->var()]);
            //load added val on tos
            bytecode.addInsn(getAddInsn(type));
            //store tos to variable
            bytecode.addInsn(getStoreInsn(type));
            bytecode.addInt16(varMap[node->var()]);
            break;
        case tDECRSET:
            //load variable on tos
            bytecode.addInsn(getLoadInsn(type));
            bytecode.addInt16(varMap[node->var()]);
            //load added val on tos
            bytecode.addInsn(getSubInsn(type));
            //store tos to variable
            bytecode.addInsn(getStoreInsn(type));
            bytecode.addInt16(varMap[node->var()]);
            break;
        default:
            break;
    }

    stack.pop_back();
    std::cout << "varnamE: " << node->var()->name() << std::endl;
    std::cout << "end StoreNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitForNode(ForNode *node) {
    std::cout << "start forNode" << std::endl;
    std::cout << "expression: " << std::endl;
    node->inExpr()->visit(this);
    std::cout << "body: " << std::endl;
    node->body()->visit(this);
    std::cout << "end forNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitWhileNode(WhileNode *node) {
    std::cout << "start whileNode" << std::endl;
    std::cout << "expression:" << std::endl;
    node->whileExpr()->visit(this);
    std::cout << "body:" << std::endl;
    node->loopBlock()->visit(this);
    std::cout << "end whileNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitIfNode(IfNode *node) {
    std::cout << "start ifNode" << std::endl;
    std::cout << "expression:" << std::endl;
    node->ifExpr()->visit(this);
    std::cout << "then body:" << std::endl;
    node->thenBlock()->visit(this);
    std::cout << "else body:" << std::endl;
    node->elseBlock()->visit(this);
    std::cout << "end ifNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitBlockNode(BlockNode *node) {
    std::cout << "start blockNode" << std::endl;

    Scope::VarIterator it(node->scope());

    while (it.hasNext()) {
        const AstVar * var = it.next();
        std::cout << "var: " << var->name() << std::endl;
        varMap[var] = globalVarCounter++;
    }
    if (topMostVariablesNum == -1) {
        topMostVariablesNum = globalVarCounter;
    }

    Scope::FunctionIterator fit(node->scope());
    while (fit.hasNext()) {
        const AstFunction * func = fit.next();
        std::cout << "fun:" << std::endl;
        func->node()->visit(this);
    }

    node->visitChildren(this);

    std::cout << "end blockNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitFunctionNode(FunctionNode *node) {
    std::cout << "start functionNode" << std::endl;
    node->visitChildren(this);
    std::cout << "end functionNode" << std::endl;

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
//    indent("return");
//    if (node->returnExpr()) {
//        ss_ << ' ';
//        node->returnExpr()->visit(this);
//    }
//    ss_ << ";";
//    newline();
}

void BytecodeTranslatorVisitor::visitCallNode(CallNode *node) {
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
//    ss_ << "native '" << node->nativeName()<< "';";
//    newline();
}

void BytecodeTranslatorVisitor::visitPrintNode(PrintNode *node) {
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