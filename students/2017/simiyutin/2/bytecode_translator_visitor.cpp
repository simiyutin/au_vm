#include "include/bytecode_translator_visitor.h"

using namespace mathvm;


void BytecodeTranslatorVisitor::visitBinaryOpNode(BinaryOpNode *node) {
    std::cout << "start BinaryOpNode" << std::endl;
    node->left()->visit(this);
    node->right()->visit(this);
    std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitUnaryOpNode(UnaryOpNode *node) {
    std::cout << "start UnaryOpNode" << std::endl;
    node->visitChildren(this);
    std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitStringLiteralNode(StringLiteralNode *node) {
    std::cout << "string literal:" << node->literal() << std::endl;
}

void BytecodeTranslatorVisitor::visitDoubleLiteralNode(DoubleLiteralNode *node) {
    std::cout << "double literal:" << node->literal() << std::endl;
}

void BytecodeTranslatorVisitor::visitIntLiteralNode(IntLiteralNode *node) {
    std::cout << "int literal:" << node->literal() << std::endl;
}

void BytecodeTranslatorVisitor::visitLoadNode(LoadNode *node) {
    std::cout << "loadNode, var name = " << node->var()->name() << std::endl;
}

void BytecodeTranslatorVisitor::visitStoreNode(StoreNode *node) {
    std::cout << "start StoreNode" << std::endl;
    node->value()->visit(this);
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