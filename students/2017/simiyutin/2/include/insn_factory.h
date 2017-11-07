#pragma once

#include "mathvm.h"
#include "../../../../../include/visitors.h"

#include <sstream>
#include <map>
#include "visitors.h"

using namespace mathvm;

inline Instruction getStoreInsn(VarType type) {
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

inline Instruction getLoadInsn(VarType type) {
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

inline Instruction getAddInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DADD;
        case VT_INT:
            return BC_IADD;
        default:
            return BC_INVALID;
    }
}

inline Instruction getSubInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DSUB;
        case VT_INT:
            return BC_ISUB;
        default:
            return BC_INVALID;
    }
}

inline Instruction getMulInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DMUL;
        case VT_INT:
            return BC_IMUL;
        default:
            return BC_INVALID;
    }
}


inline Instruction getDivInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DDIV;
        case VT_INT:
            return BC_IDIV;
        default:
            return BC_INVALID;
    }
}

inline Instruction getModInsn(VarType type) {
    switch (type) {
        case VT_INT:
            return BC_IMOD;
        default:
            return BC_INVALID;
    }
}

inline Instruction getNegInsn(VarType type) {
    switch (type) {
        case VT_DOUBLE:
            return BC_DNEG;
        case VT_INT:
            return BC_INEG;
        default:
            return BC_INVALID;
    }
}

inline Instruction getOrInsn(VarType type) {
    switch (type) {
        case VT_INT:
            return BC_IAOR;
        default:
            return BC_INVALID;
    }
}

inline Instruction getAndInsn(VarType type) {
    switch (type) {
        case VT_INT:
            return BC_IAAND;
        default:
            return BC_INVALID;
    }
}

inline Instruction getXorInsn(VarType type) {
    switch (type) {
        case VT_INT:
            return BC_IAXOR;
        default:
            return BC_INVALID;
    }
}