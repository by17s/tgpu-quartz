#pragma once

#include <stdint.h>

#define TARGET_ADDR_T uint64_t

enum {
    TGQ_I8,
    TGQ_I16,
    TGQ_I32,
    TGQ_I64,
    TGQ_FP16,
    TGQ_FP32,
    TGQ_BF16,
    TGQ_BF32,

    //vec
    TGQ_V4I32,
    TGQ_V4FP16,
    TGQ_V4FP32,
    TGQ_V4BF16,
    TGQ_V4BF32,
};

enum {
    TGQ_I_NOP = 0x00,
    TGQ_I_ADD,
    TGQ_I_SUB,
    TGQ_I_MUL,
    TGQ_I_DIV,
    TGQ_I_FML,
    TGQ_I_SQRT,
    TGQ_I_MIN,
    TGQ_I_MAX,
    TGQ_I_AND,
    TGQ_I_OR,
    TGQ_I_XOR,
    TGQ_I_NOT,
    TGQ_I_SHL,
    TGQ_I_SHR,
    TGQ_I_MOV,
    TGQ_I_XCHG,
    TGQ_I_BRA,
    TGQ_I_BEQ,
    TGQ_I_BNE,
    TGQ_I_BLT,
    TGQ_I_BGT,
    TGQ_I_CALL,
    TGQ_I_LD_GLOBAL,
    TGQ_I_ST_GLOBAL,
    TGQ_I_LD_LOCAL,
    TGQ_I_ST_LOCAL,
    TGQ_I_LCONST8,
    TGQ_I_LCONST16,
    TGQ_I_LCONST32,
    TGQ_I_LCONST64

    TGQ_I_RET = 0x1000_0000,
    TGQ_I_SYNC,

}

#define TGQ_R_GEN8(T, R) ((((uint8_t)T & 0xF) << 4) | ((uint8_t)R & 0xF))
#define TGQ_I_TYPED_GEN8(T, T2, I) ((uint8_t)I | (TGQ_R_GEN8(T, T2) << 8))
#define TGQ_I_GEN8(I) ((uint8_t)I)


#define TGQ_I_SCALA2(I, T, RD, R1) TGQ_I_TYPED_GEN8(T, T, I), TGQ_R_GEN8(T, RD), TGQ_R_GEN8(T, R1)
#define TGQ_I_SCALA3(I, T, RD, R1, R2) TGQ_I_TYPED_GEN8(T, T, I), TGQ_R_GEN8(T, RD), TGQ_R_GEN8(T, R1), TGQ_R_GEN8(T, R2)
#define TGQ_I_SCALA4(I, T, RD, R1, R2, R3) TGQ_I_TYPED_GEN8(T, T, I), TGQ_R_GEN8(T, RD), TGQ_R_GEN8(T, R1), TGQ_R_GEN8(T, R2), TGQ_R_GEN8(T, R3)
#define TGQ_I_MATRIX2(I, T, RD, R1) TGQ_I_TYPED_GEN8(T, T, I), TGQ_R_GEN8(T, RD), TGQ_R_GEN8(T, R1)
#define TGQ_I_MATRIX3(I, T, RD, R1, R2) TGQ_I_TYPED_GEN8(T, T, I), TGQ_R_GEN8(T, RD), TGQ_R_GEN8(T, R1), TGQ_R_GEN8(T, R2)
#define TGQ_I_MATRIX4(I, T, RD, R1, R2, R3) TGQ_I_TYPED_GEN8(T, T, I), TGQ_R_GEN8(T, RD), TGQ_R_GEN8(T, R1), TGQ_R_GEN8(T, R2), TGQ_R_GEN8(T, R3)