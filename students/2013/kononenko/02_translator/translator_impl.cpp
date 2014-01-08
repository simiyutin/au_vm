#include "stdafx.h"
#include "translator_impl.h"
#include "code_impl.h"
#include "parser.h"

namespace mathvm
{

void translator_impl::visitDoubleLiteralNode(DoubleLiteralNode* node)
{
    bytecode()->addInsn(BC_DLOAD);
    bytecode()->addDouble(node->literal()); 
    tos_type_ = VT_DOUBLE;
}

void translator_impl::visitWhileNode(WhileNode* node)
{
    Label lbl_loop(bytecode()), lbl_after(bytecode());

    bytecode()->bind(lbl_loop);
    node->whileExpr()->visit(this);

    if (tos_type_ != VT_INT)
        throw error("Condition for ints only");

    bytecode()->addInsn(BC_ILOAD0);
    bytecode()->addBranch(BC_IFICMPE, lbl_after);

    node->loopBlock()->visit(this);
    bytecode()->addBranch(BC_JA, lbl_loop);    
    bytecode()->bind(lbl_after);
    tos_type_ = VT_VOID;
}

void translator_impl::visitForNode( ForNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitFunctionNode(FunctionNode* node)
{
    context_t const &context = current_context();
    const function_id_t id = context.functions.at(node->name());

    const function_id_t old_id = dst_func_id_ ;
    dst_func_id_ = id;

    Signature const *old_signature =  signature_;
    signature_ = &node->signature();
    
    node->body()->visit(this);
    signature_ = old_signature;

    dst_func_id_ = old_id;
}

void translator_impl::visitBlockNode(BlockNode* node)
{
    typedef vector<AstNode*> nodes_vector_t;

    if (signature_)
    {
        Scope *args_scope = node->scope()->parent();
        
        assert(signature_->size() >= 1);
        for (uint32_t i = 1; i < signature_->size(); ++i)
        {
            tos_type_ = signature_->at(i).first;
            
            string const &name = signature_->at(i).second;
            AstVar *var_node = args_scope->lookupVariable(name, false);
            assert(var_node);

            store_tos_var(var_node);
        }
        
        signature_ = NULL;
    }
    
    
    for (Scope::FunctionIterator it(node->scope(), false); it.hasNext(); )
    {
        AstFunction *fn = it.next();
        current_scope_ = node->scope();
        dst_code_->get_function_dst(dst_func_id_)->set_context(current_context().id);

        fn->node()->visit(this);
    }

    for (uint32_t i = 0; i < node->nodes(); ++i)
    {
        current_scope_ = node->scope();
        scope2fn_[current_scope_] = dst_func_id_;

        dst_code_->get_function_dst(dst_func_id_)->set_context(current_context().id);
        tos_type_ = VT_VOID;

        return_ = false;
        node->nodeAt(i)->visit(this);

        if (return_)
        {   
            return_ = false;
            break;
        }

        if (tos_type_ != VT_VOID)
            bytecode()->addInsn(BC_POP);
    }

    current_scope_ = node->scope();
    dst_code_->get_function_dst(dst_func_id_)->set_context(current_context().id);
    //tos_type_ = VT_VOID;
    
    assert(current_scope_ == node->scope());
}

void translator_impl::add_context(Scope *scope, context_id_t id)
{
#if (0)
    context_t context(id);

    for (Scope::VarIterator it(scope, false); it.hasNext();)
    {
        AstVar const *var = it.next();
        const bool insertion = context.vars.insert(std::make_pair(var->name(), context.vars.size())).second;
        assert(insertion);
    }

    for (Scope::FunctionIterator it(scope, false); it.hasNext();)
    {
        AstFunction const *fn = it.next();
        const function_id_t id = dst_code_->add_function();
        const bool insertion = context.functions.insert(make_pair(fn->name(), id)).second;
        assert(insertion);
    }

    contexts_.insert(make_pair(scope, context));
#endif
}


void translator_impl::visitIfNode(IfNode* node)
{
    Label lbl_else(bytecode()), lbl_after(bytecode());
    node->ifExpr()->visit(this);

    if (tos_type_ != VT_INT)
        throw error("Condition for ints only");

    bytecode()->addInsn(BC_ILOAD0);
    bytecode()->addBranch(BC_IFICMPE, lbl_else);

    node->thenBlock()->visit(this);
    bytecode()->addBranch(BC_JA, lbl_after);    

    bytecode()->bind(lbl_else);
    if (node->elseBlock()) 
        node->elseBlock()->visit(this);    

    bytecode()->bind(lbl_after);
    tos_type_ = VT_VOID;
}

void translator_impl::visitNativeCallNode( NativeCallNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitPrintNode(PrintNode* node)
{
    for (uint32_t i = 0; i < node->operands(); ++i)
    {
        node->operandAt(i)->visit(this);
        Instruction insn;
        switch(tos_type_)
        {
        case VT_INT   : insn = BC_IPRINT; break;
        case VT_DOUBLE: insn = BC_DPRINT; break;
        case VT_STRING: insn = BC_SPRINT; break;
        default: throw error("Unprintable type");
        }
        bytecode()->addInsn(insn);
    }

    tos_type_ = VT_VOID;
}

void translator_impl::visitBinaryOpNode(BinaryOpNode* node)
{
    node->left()->visit(this);
    const VarType type1 = tos_type_;
    node->right()->visit(this);
    const VarType type2 = tos_type_;

    const TokenKind op = node->kind();

    switch(op)
    {
    case tADD:
    case tSUB:
    case tMUL:
    case tDIV:
    case tMOD: op_arithm(op, type1, type2); break;
    case tEQ :
    case tNEQ:
    case tGT :
    case tGE :
    case tLT :
    case tLE : op_comp(op, type1, type2); break;
    case tAND:
    case tOR : op_andor(op, type1, type2); break;
    case tAAND:
    case tAOR :
    case tAXOR: op_bitwise(op, type1, type2); break;

    default: throw error("Unsupported binary op");
    }
}

void translator_impl::op_arithm(TokenKind op, VarType type1, VarType type2)
{
    if (type1 != type2)
        throw error("Typecasts not supported yet");

    const VarType type = type1;

    Instruction insn;
    if (type == VT_INT)
    {
        switch(op)
        {
        case tADD: insn = BC_IADD;  break;
        case tSUB: insn = BC_ISUB;  break;
        case tMUL: insn = BC_IMUL;  break;
        case tDIV: insn = BC_IDIV;  break;
        case tMOD: insn = BC_IMOD;  break;
        default: insn = BC_INVALID; break;
        }
    }
    else if (type == VT_DOUBLE)
    {
        switch(op)
        {
        case tADD: insn = BC_DADD; break;
        case tSUB: insn = BC_DSUB; break;
        case tMUL: insn = BC_DMUL; break;
        case tDIV: insn = BC_DDIV; break;
        default: insn = BC_INVALID; break;
        }
    }

    assert(insn != BC_INVALID);

    bytecode()->addInsn(insn);
    
    tos_type_ = type;
}

void translator_impl::op_comp(TokenKind op, VarType type1, VarType type2)
{
    if (type1 != VT_INT || type2 != VT_INT)
        throw error("Comparison are supported for ints only");

    Instruction insn;    

    switch (op) {
    case tEQ : insn = BC_IFICMPE ; break;
    case tNEQ: insn = BC_IFICMPNE; break;
    case tGT : insn = BC_IFICMPG ; break;
    case tGE : insn = BC_IFICMPGE; break;
    case tLT : insn = BC_IFICMPL ; break;
    case tLE : insn = BC_IFICMPLE; break;
    default  : insn = BC_INVALID ; break;
    }    

    assert(insn != BC_INVALID);

    Label lbl_true(bytecode()), lbl_after(bytecode());
    bytecode()->addBranch(insn, lbl_true);
    bytecode()->addInsn(BC_ILOAD0);
    bytecode()->addBranch(BC_JA, lbl_after);
    bytecode()->bind(lbl_true);
    bytecode()->addInsn(BC_ILOAD1);
    bytecode()->bind(lbl_after);
}

void translator_impl::op_andor(TokenKind op, VarType type1, VarType type2)
{
    if (type1 != VT_INT || type2 != VT_INT)
        throw error("and/or is for ints only");
    const VarType type = type1;
    
    switch(op)
    {
    case tOR : bytecode()->addInsn(BC_IADD ); break;
    case tAND: bytecode()->addInsn(BC_IMUL); break;
    default: assert(false);
    }

    tos_type_ = type;
}

void translator_impl::op_bitwise(TokenKind op, VarType type1, VarType type2)
{
    if (type1 != VT_INT || type2 != VT_INT)
        throw error("bitwise is for ints only");

    const VarType type = type1;
    switch(op)
    {
    case tAAND: bytecode()->addInsn(BC_IAAND); break;
    case tAOR : bytecode()->addInsn(BC_IAOR ); break;
    case tAXOR: bytecode()->addInsn(BC_IAXOR); break;
    default: assert(false);
    }

    tos_type_ = type;
}

void translator_impl::visitStringLiteralNode(StringLiteralNode* node)
{
    const uint16_t id = dst_code_->makeStringConstant(node->literal());
    
    bytecode()->addInsn(BC_SLOAD);
    bytecode()->addInt16(id); 
    tos_type_ = VT_STRING;
}

void translator_impl::visitCallNode(CallNode* node)
{
    AstFunction *fn = current_scope_->lookupFunction(node->name(), true);
    const function_id_t id = find_function(node->name());

    Signature const &signature = fn->node()->signature();
    
    assert(node->parametersNumber() == signature.size() - 1);

    for (uint32_t i = 0; i < node->parametersNumber(); ++i)
    {
        AstNode *p = node->parameterAt(node->parametersNumber() - i - 1);
        p->visit(this);
        assert(tos_type_ == signature.at(node->parametersNumber() - i).first);
    }

    bytecode()->addInsn(BC_CALL);
    bytecode()->addInt16(id);
    tos_type_ = signature.at(0).first;
}

void translator_impl::visitIntLiteralNode(IntLiteralNode* node)
{
    bytecode()->addInsn(BC_ILOAD);
    bytecode()->addInt64(node->literal()); 
    tos_type_ = VT_INT;
}

void translator_impl::visitStoreNode(StoreNode* node)
{
    const TokenKind op = node->op();

    if (op == tASSIGN)
    {
        node->value()->visit(this);
        if (tos_type_ != node->var()->type())
            throw error("Typecasts not supported yet");
    }
    else if (op == tINCRSET || op == tDECRSET)
    {
        load_tos_var(node->var()); 
        node->value()->visit(this);

        const VarType type = tos_type_;
        if (type != VT_INT && type != VT_DOUBLE)
            throw error("Unsupported type"); 
        if (type != node->var()->type())
            throw error("Typecasts not supported yet");
        
        Instruction insn;
        switch(op)
        {
        case tINCRSET: insn = type == VT_INT ? BC_IADD : BC_DADD; break;
        case tDECRSET: insn = type == VT_INT ? BC_ISUB : BC_DSUB; break;
        default: assert(false);
        }

        bytecode()->addInsn(insn);
        
    tos_type_ = type;
    }
    else
        throw error("Unsupported store operation"); 
    
    store_tos_var(node->var());

    // a = b = 3
    load_tos_var(node->var());
    tos_type_ = node->var()->type();
}

void translator_impl::visitLoadNode(LoadNode* node)
{
    load_tos_var(node->var());
    tos_type_ = node->var()->type();
}

void translator_impl::visitReturnNode(ReturnNode* node)
{
    if (node->returnExpr()) 
    {
        node->returnExpr()->visit(this);
    }
    else
        tos_type_ = VT_VOID;
    
    bytecode()->addInsn(BC_RETURN);
}

void translator_impl::visitUnaryOpNode(UnaryOpNode* node)
{
    node->operand()->visit(this);
    const VarType type = tos_type_;

    if (node->kind() == tNOT)
    {
        if (type != VT_INT)
            throw error("Bad type");

        // There must be a better way
        Label lbl_zero(bytecode()), lbl_after(bytecode());

        bytecode()->addInsn(BC_ILOAD0);
        bytecode()->addBranch(BC_IFICMPE, lbl_zero);
        bytecode()->addInsn(BC_ILOAD0);
        bytecode()->addBranch(BC_JA, lbl_after);
        bytecode()->bind(lbl_zero);
        bytecode()->addInsn(BC_ILOAD1);
        bytecode()->bind(lbl_after);
    }
    else if (node->kind() == tSUB)
    {
        switch(type)
        {
        case VT_INT   : bytecode()->addInsn(BC_INEG); break;
        case VT_DOUBLE: bytecode()->addInsn(BC_DNEG); break;
        default: throw error("Bad type");
        }
    }
    else
    {
        throw error("Unsupported unary op");
    }

    // Stays the same
    tos_type_ = type;
}

translator_impl::translator_impl()
    : dst_code_(NULL)
    , current_scope_(NULL)
{

}

std::pair<context_id_t, var_id_t> translator_impl::get_var_ids(AstVar const *var, bool store, bool *out_is_local)
{
    Scope const *scope = var->owner();
    const function_id_t fn_id = scope2fn_.at(scope);
    
    if (out_is_local)
        *out_is_local = (fn_id == dst_func_id_);

    context_t const &context = fn_contexts_.at(fn_id);
    const var_id_t var_id = context.vars.at(var->name());
    return std::make_pair(context.id, var_id);
}

void translator_impl::load_tos_var( AstVar const *var )
{
    process_var(false, var);
}

void translator_impl::store_tos_var(AstVar const *var)
{
    process_var(true, var);
}

Instruction translator_impl::get_var_insn(bool store, AstVar const *var, bool is_local)
{
    const VarType var_type = var->type();
    if (store)
    {
        if (!is_local)
        {
            switch(var_type)
            {
            case VT_INT   : return BC_STORECTXIVAR;
            case VT_DOUBLE: return BC_STORECTXDVAR;
            case VT_STRING: return BC_STORECTXSVAR;
            default: assert(false);
            }
        }
        else
        {
            switch(var_type)
            {
            case VT_INT   : return BC_STOREIVAR;
            case VT_DOUBLE: return BC_STOREDVAR;
            case VT_STRING: return BC_STORESVAR;
            default: assert(false);
            }
        }
    }
    else
    {
        if (!is_local)
        {
            switch(var_type)
            {
            case VT_INT   : return BC_LOADCTXIVAR;
            case VT_DOUBLE: return BC_LOADCTXDVAR;
            case VT_STRING: return BC_LOADCTXSVAR;
            default: assert(false);
            }
        }
        else
        {
            switch(var_type)
            {
            case VT_INT   : return BC_LOADIVAR;
            case VT_DOUBLE: return BC_LOADDVAR;
            case VT_STRING: return BC_LOADSVAR;
            default: assert(false);
            }
        }
    }
    assert(0);
    return BC_INVALID;
}

void translator_impl::process_var(bool store, AstVar const *var)
{
    bool is_local;
    const std::pair<context_id_t, var_id_t> ids = get_var_ids(var, true, /*out*/ &is_local);

    const VarType var_type = var->type();

    if (store && tos_type_ != var_type)
        throw error("Typecasts not supported yet");

    const Instruction ins = get_var_insn(store, var, is_local);

    bytecode()->addInsn(ins);
    if (!is_local)
        bytecode()->addInt16(ids.first);
    bytecode()->addInt16(ids.second);
}

function_id_t translator_impl::find_function(string const &name) const
{
    AstFunction const *ast_fn = current_scope_->lookupFunction(name, true);
    function_id_t parent_fn_id = scope2fn_.at(ast_fn->owner());
    context_t const &context = fn_contexts_.at(parent_fn_id);
    return context.functions.at(name);
}

void translator_impl::init()
{
    tos_type_ = VT_INVALID;
    dst_code_ = NULL;
    dst_func_id_ = -1;
    current_scope_ = NULL;
    signature_ = NULL;
    //contexts_.clear();
    fn_contexts_.clear();
}

void translator_impl::prepare(AstFunction *top)
{
    dst_code_ = new code_impl();
    current_scope_ = top->scope();

    dst_func_id_ = -1;
    fn_contexts_.insert(make_pair(dst_func_id_, context_t(dst_func_id_)));
    prepare_scope(top->scope(), fn_contexts_.at(dst_func_id_), dst_func_id_);
    //init_contexts(top->owner());
}

Status* translator_impl::translate(const string& program, Code **code)
{
    init();

    Parser parser;
    Status* status = parser.parseProgram(program);
    if (status && !status->isOk())
        return status;

    try
    {
        prepare(parser.top());
        parser.top()->node()->visit(this);
        *code = dst_code_;
        return new Status();
    }
    catch (error const &e)
    {
        return new Status(e.what());
    }
}

void translator_impl::init_contexts(Scope *scope, uint32_t depth)
{
    add_context(scope, depth);
    for (uint32_t i = 0; i < scope->childScopeNumber(); ++i)
    {
        init_contexts(scope->childScopeAt(i), depth + 1);
    }
}

void translator_impl::prepare_scope(Scope *scope, context_t &context, function_id_t fn_id)
{
    assert(scope2fn_.count(scope) == 0);
    scope2fn_[scope] = fn_id;

    // add vars
    for (Scope::VarIterator it(scope, false); it.hasNext();)
    {
        AstVar const *var = it.next();
        const bool insertion = context.vars.insert(std::make_pair(var->name(), context.vars.size())).second;
        assert(insertion);
    }

    // add functions
    for (Scope::FunctionIterator it(scope, false); it.hasNext();)
    {
        AstFunction const *fn = it.next();
        const function_id_t id = dst_code_->add_function();
        const bool insertion = context.functions.insert(make_pair(fn->name(), id)).second;
        assert(insertion);

        assert(fn_contexts_.count(id) == 0);
        fn_contexts_.insert(make_pair(id, context_t(id)));

        Scope *fn_scope = fn->node()->body()->scope();
        if (fn->name() != "<top>")
            fn_scope = fn_scope->parent();
        prepare_scope(fn_scope, fn_contexts_.at(id), id);
    }

    for (size_t i = 0; i < scope->childScopeNumber(); ++i)
    {
        Scope *child = scope->childScopeAt(i);
        // isn't inner function scope
        if (scope2fn_.count(child) == 0)
            prepare_scope(child, context, fn_id);
    }

    //contexts_.insert(make_pair(scope, context));

}

} // namespace mathvm