#include "include/bytecode_translator_visitor.h"
#include "include/insn_factory.h"
#include <string>
#include <fstream>

using namespace mathvm;


void BytecodeTranslatorVisitor::visitBinaryOpNode(BinaryOpNode *node) {
    //std::cout << "start BinaryOpNode" << std::endl;
    switch (node->kind()) {
        case tADD: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getAddInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tSUB: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getSubInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tMUL: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getMulInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tDIV: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getDivInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tMOD: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getModInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tAAND: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getAndInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tAOR: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getOrInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tAXOR: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = stack.back();
            bytecode.addInsn(getXorInsn(type));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(type);
            break;
        }
        case tEQ: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPE, BC_IFICMPNE);
            break;
        }
        case tLT: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPL, BC_IFICMPGE);
            break;
        }
        case tGT: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPG, BC_IFICMPLE);
            break;
        }
        case tGE: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPGE, BC_IFICMPL);
            break;
        }
        case tLE: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPLE, BC_IFICMPG);
            break;
        }
        case tAND: {
            if (!expressionEndLabel) {
                assert(!expressionStartLabel);
                expressionEndLabel = new Label();
            }
            node->right()->visit(this);
            node->left()->visit(this);
            break;
        }
        case tOR: {
            if (!expressionStartLabel) {
                assert(!expressionEndLabel);
                expressionStartLabel = new Label();
            }
            node->left()->visit(this);
            node->right()->visit(this);
            break;
        }
        case tRANGE: {
            node->left()->visit(this);
            node->right()->visit(this);
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
    switch (node->kind()) {
        case tSUB: {
            node->visitChildren(this);
            VarType type = stack.back();
            bytecode.addInsn(getNegInsn(type));
            break;
        }
        case tNOT: {
            bool prevInverse = inverse;
            inverse = !inverse;
            node->visitChildren(this);
            inverse = prevInverse;
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

    node->inExpr()->visit(this); // на стеке сейчас находится коненое значение, а потом начальное значение
    bytecode.addInsn(BC_SWAP);

    // присваиваем начальное значение переменной
    bytecode.addInsn(getStoreInsn(VT_INT));
    bytecode.addUInt16(varMap[node->var()]);

    Label startLabel;
    uint32_t startPosition = bytecode.length();
    Label endLabel;

    //проверяем, не нужно ли прекратить цикл. Предполагается, что на вершине стека находится верхняя граница отерируемой переменной
    bytecode.addInsn(getLoadInsn(VT_INT));
    bytecode.addUInt16(varMap[node->var()]);
    bytecode.addBranch(BC_IFICMPG, endLabel);

    //выполняем вычисления
    node->body()->visit(this);

    //инкрементируем переменную
    bytecode.addInsn(getLoadInsn(VT_INT));
    bytecode.addUInt16(varMap[node->var()]);
    bytecode.addInsn(BC_ILOAD1);
    bytecode.addInsn(BC_IADD);
    bytecode.addInsn(getStoreInsn(VT_INT));
    bytecode.addUInt16(varMap[node->var()]);

    //делаем джамп к проверке
    bytecode.addInsn(BC_POP);
    bytecode.addBranch(BC_JA, startLabel);
    startLabel.bind(startPosition, &bytecode);

    //конец цикла
    endLabel.bind(bytecode.current(), &bytecode);
    
}

void BytecodeTranslatorVisitor::visitWhileNode(WhileNode *node) {
    Label startLabel;
    uint32_t startPosition = bytecode.length();
    node->whileExpr()->visit(this);
    Label * localExprStartLabel = expressionStartLabel;
    expressionStartLabel = nullptr;
    Label * localExprEndLabel = expressionEndLabel;
    expressionEndLabel = nullptr;
    //after expression visit, only one of (expressionStartLabel, expressionEndLabel) is initialized,
    //depending on type of logical operators within expression (||, &&)
    if (localExprStartLabel) { // || operators
        localExprEndLabel = new Label();
        bytecode.addBranch(BC_JA, *localExprEndLabel);
        localExprStartLabel->bind(bytecode.length(), &bytecode);
    }
    node->loopBlock()->visit(this);
    bytecode.addBranch(BC_JA, startLabel);
    startLabel.bind(startPosition, &bytecode);
    localExprEndLabel->bind(bytecode.length(), &bytecode);
    delete localExprStartLabel;
    delete localExprEndLabel;
}

void BytecodeTranslatorVisitor::visitIfNode(IfNode *node) {
    node->ifExpr()->visit(this);
    Label * localExprStartLabel = expressionStartLabel;
    expressionStartLabel = nullptr;
    Label * localExprEndLabel = expressionEndLabel;
    expressionEndLabel = nullptr;
    //after expression visit, only one of (expressionStartLabel, expressionEndLabel) is initialized,
    //depending on type of logical operators within expression (||, &&)
    if (localExprStartLabel) { // || operators
        localExprEndLabel = new Label();
        bytecode.addBranch(BC_JA, *localExprEndLabel);
        localExprStartLabel->bind(bytecode.length(), &bytecode);
    }
    node->thenBlock()->visit(this);
    Label endOfElseBlockLabel;
    bytecode.addBranch(BC_JA, endOfElseBlockLabel);
    localExprEndLabel->bind(bytecode.length(), &bytecode);
    if (node->elseBlock()) {
        node->elseBlock()->visit(this);
    }
    endOfElseBlockLabel.bind(bytecode.current(), &bytecode);
    delete localExprStartLabel;
    delete localExprEndLabel;
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