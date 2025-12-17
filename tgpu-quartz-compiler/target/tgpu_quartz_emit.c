#include "tgpu_quartz_emit.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// EMIT BUFFER IMPLEMENTATION
// ============================================================================

#define INITIAL_CAPACITY 1024

void emit_init(EmitBuffer *buf) {
    buf->data = malloc(INITIAL_CAPACITY);
    buf->size = 0;
    buf->capacity = INITIAL_CAPACITY;
}

void emit_free(EmitBuffer *buf) {
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0;
    buf->capacity = 0;
}

void emit_reset(EmitBuffer *buf) {
    buf->size = 0;
}

static void emit_grow(EmitBuffer *buf, int needed) {
    if (buf->size + needed > buf->capacity) {
        while (buf->capacity < buf->size + needed) {
            buf->capacity *= 2;
        }
        buf->data = realloc(buf->data, buf->capacity);
    }
}

void emit_byte(EmitBuffer *buf, uint8_t b) {
    emit_grow(buf, 1);
    buf->data[buf->size++] = b;
}

void emit_u16(EmitBuffer *buf, uint16_t w) {
    emit_byte(buf, w & 0xFF);
    emit_byte(buf, (w >> 8) & 0xFF);
}

void emit_u32(EmitBuffer *buf, uint32_t w) {
    emit_byte(buf, w & 0xFF);
    emit_byte(buf, (w >> 8) & 0xFF);
    emit_byte(buf, (w >> 16) & 0xFF);
    emit_byte(buf, (w >> 24) & 0xFF);
}

void emit_u64(EmitBuffer *buf, uint64_t w) {
    emit_u32(buf, (uint32_t)(w & 0xFFFFFFFF));
    emit_u32(buf, (uint32_t)((w >> 32) & 0xFFFFFFFF));
}

void emit_i32(EmitBuffer *buf, int32_t w) {
    emit_u32(buf, (uint32_t)w);
}

void emit_f32(EmitBuffer *buf, float f) {
    union { float f; uint32_t u; } conv;
    conv.f = f;
    emit_u32(buf, conv.u);
}

// ============================================================================
// LABEL MANAGEMENT
// ============================================================================

void labels_init(LabelManager *lm) {
    memset(lm, 0, sizeof(LabelManager));
    lm->next_label = 0;
}

int label_create(LabelManager *lm) {
    int id = lm->next_label++;
    if (id < MAX_LABELS) {
        lm->labels[id].label_id = id;
        lm->labels[id].position = -1;  // Not yet defined
        lm->label_count++;
    }
    return id;
}

void label_define(LabelManager *lm, EmitBuffer *buf, int label_id) {
    if (label_id < MAX_LABELS) {
        lm->labels[label_id].position = buf->size;
    }
}

void label_add_reloc(LabelManager *lm, EmitBuffer *buf, int label_id, RelocType type) {
    if (lm->reloc_count < MAX_RELOCATIONS) {
        Relocation *r = &lm->relocs[lm->reloc_count++];
        r->offset = buf->size;
        r->label_id = label_id;
        r->type = type;
    }
}

bool labels_resolve(LabelManager *lm, EmitBuffer *buf) {
    for (int i = 0; i < lm->reloc_count; i++) {
        Relocation *r = &lm->relocs[i];

        if (r->label_id >= MAX_LABELS) {
            fprintf(stderr, "Error: invalid label id %d\n", r->label_id);
            return false;
        }

        int target = lm->labels[r->label_id].position;
        if (target < 0) {
            fprintf(stderr, "Error: undefined label %d\n", r->label_id);
            return false;
        }

        if (r->type == RELOC_BRANCH) {
            // Relative offset from instruction position
            int32_t offset = target - (r->offset + 4);  // +4 for size of offset itself
            memcpy(&buf->data[r->offset], &offset, 4);
        } else {
            // Absolute address
            uint64_t addr = target;
            memcpy(&buf->data[r->offset], &addr, 8);
        }
    }
    return true;
}

// ============================================================================
// INSTRUCTION ENCODING HELPERS
// ============================================================================

// Encode register with type: upper 4 bits = type, lower 4 bits = register
static uint8_t encode_reg(uint8_t type, uint8_t reg) {
    return TGQ_R_GEN8(type, reg);
}

// ============================================================================
// INSTRUCTION EMISSION
// ============================================================================

void emit_nop(EmitBuffer *buf) {
    emit_byte(buf, TGQ_I_NOP);
}

void emit_scalar2(EmitBuffer *buf, uint8_t op, uint8_t type, uint8_t rd, uint8_t r1) {
    emit_byte(buf, op);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, rd));
    emit_byte(buf, encode_reg(type, r1));
}

void emit_scalar3(EmitBuffer *buf, uint8_t op, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_byte(buf, op);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, rd));
    emit_byte(buf, encode_reg(type, r1));
    emit_byte(buf, encode_reg(type, r2));
}

void emit_scalar4(EmitBuffer *buf, uint8_t op, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2, uint8_t r3) {
    emit_byte(buf, op);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, rd));
    emit_byte(buf, encode_reg(type, r1));
    emit_byte(buf, encode_reg(type, r2));
    emit_byte(buf, encode_reg(type, r3));
}

// ============================================================================
// ARITHMETIC INSTRUCTIONS
// ============================================================================

void emit_add(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_ADD, type, rd, r1, r2);
}

void emit_sub(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_SUB, type, rd, r1, r2);
}

void emit_mul(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_MUL, type, rd, r1, r2);
}

void emit_div(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_DIV, type, rd, r1, r2);
}

void emit_fma(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2, uint8_t r3) {
    emit_scalar4(buf, TGQ_I_FML, type, rd, r1, r2, r3);
}

// ============================================================================
// BITWISE INSTRUCTIONS
// ============================================================================

void emit_and(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_AND, type, rd, r1, r2);
}

void emit_or(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_OR, type, rd, r1, r2);
}

void emit_xor(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_XOR, type, rd, r1, r2);
}

void emit_not(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1) {
    emit_scalar2(buf, TGQ_I_NOT, type, rd, r1);
}

void emit_shl(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_SHL, type, rd, r1, r2);
}

void emit_shr(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2) {
    emit_scalar3(buf, TGQ_I_SHR, type, rd, r1, r2);
}

// ============================================================================
// MOVE INSTRUCTION
// ============================================================================

void emit_mov(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1) {
    emit_scalar2(buf, TGQ_I_MOV, type, rd, r1);
}

// ============================================================================
// LOAD CONSTANT INSTRUCTIONS
// ============================================================================

void emit_lconst8(EmitBuffer *buf, uint8_t rd, uint8_t value) {
    emit_byte(buf, TGQ_I_LCONST8);
    emit_byte(buf, encode_reg(TGQ_I8, rd));
    emit_byte(buf, value);
}

void emit_lconst16(EmitBuffer *buf, uint8_t rd, uint16_t value) {
    emit_byte(buf, TGQ_I_LCONST16);
    emit_byte(buf, encode_reg(TGQ_I16, rd));
    emit_u16(buf, value);
}

void emit_lconst32(EmitBuffer *buf, uint8_t rd, uint32_t value) {
    emit_byte(buf, TGQ_I_LCONST32);
    emit_byte(buf, encode_reg(TGQ_I32, rd));
    emit_u32(buf, value);
}

void emit_lconst64(EmitBuffer *buf, uint8_t rd, uint64_t value) {
    emit_byte(buf, TGQ_I_LCONST64);
    emit_byte(buf, encode_reg(TGQ_I64, rd));
    emit_u64(buf, value);
}

void emit_lconst_f32(EmitBuffer *buf, uint8_t rd, float value) {
    emit_byte(buf, TGQ_I_LCONST32);
    emit_byte(buf, encode_reg(TGQ_FP32, rd));
    emit_f32(buf, value);
}

// ============================================================================
// MEMORY INSTRUCTIONS
// ============================================================================

void emit_ld_global(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_LD_GLOBAL, type, rd, rbase, roff);
}

void emit_st_global(EmitBuffer *buf, uint8_t type, uint8_t rsrc, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_ST_GLOBAL, type, rsrc, rbase, roff);
}

void emit_ld_local(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_LD_LOCAL, type, rd, rbase, roff);
}

void emit_st_local(EmitBuffer *buf, uint8_t type, uint8_t rsrc, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_ST_LOCAL, type, rsrc, rbase, roff);
}

// ============================================================================
// CONTROL FLOW INSTRUCTIONS
// ============================================================================

void emit_bra(EmitBuffer *buf, LabelManager *lm, int label_id) {
    emit_byte(buf, TGQ_I_BRA);
    label_add_reloc(lm, buf, label_id, RELOC_BRANCH);
    emit_i32(buf, 0);  // Placeholder for offset
}

void emit_beq(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id) {
    emit_byte(buf, TGQ_I_BEQ);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, r1));
    emit_byte(buf, encode_reg(type, r2));
    label_add_reloc(lm, buf, label_id, RELOC_BRANCH);
    emit_i32(buf, 0);  // Placeholder
}

void emit_bne(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id) {
    emit_byte(buf, TGQ_I_BNE);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, r1));
    emit_byte(buf, encode_reg(type, r2));
    label_add_reloc(lm, buf, label_id, RELOC_BRANCH);
    emit_i32(buf, 0);
}

void emit_blt(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id) {
    emit_byte(buf, TGQ_I_BLT);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, r1));
    emit_byte(buf, encode_reg(type, r2));
    label_add_reloc(lm, buf, label_id, RELOC_BRANCH);
    emit_i32(buf, 0);
}

void emit_bgt(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id) {
    emit_byte(buf, TGQ_I_BGT);
    emit_byte(buf, type);
    emit_byte(buf, encode_reg(type, r1));
    emit_byte(buf, encode_reg(type, r2));
    label_add_reloc(lm, buf, label_id, RELOC_BRANCH);
    emit_i32(buf, 0);
}

void emit_call(EmitBuffer *buf, LabelManager *lm, int label_id) {
    emit_byte(buf, TGQ_I_CALL);
    label_add_reloc(lm, buf, label_id, RELOC_BRANCH);
    emit_i32(buf, 0);
}

void emit_ret(EmitBuffer *buf) {
    // TGQ_I_RET is 0x1000_0000 - special encoding
    emit_u32(buf, TGQ_I_RET);
}

void emit_sync(EmitBuffer *buf) {
    emit_u32(buf, TGQ_I_SYNC);
}

// ============================================================================
// ATOMIC INSTRUCTIONS
// ============================================================================

void emit_atomic_add(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_ATOMIC_ADD, type, rd, rbase, roff);
}

void emit_atomic_sub(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_ATOMIC_SUB, type, rd, rbase, roff);
}

void emit_atomic_st(EmitBuffer *buf, uint8_t type, uint8_t rsrc, uint8_t rbase, uint8_t roff) {
    emit_scalar3(buf, TGQ_I_ATOMIC_ST, type, rsrc, rbase, roff);
}

// ============================================================================
// OUTPUT
// ============================================================================

bool emit_write_file(EmitBuffer *buf, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(buf->data, 1, buf->size, f);
    fclose(f);
    return true;
}

bool emit_write_hex(EmitBuffer *buf, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return false;

    for (int i = 0; i < buf->size; i++) {
        fprintf(f, "%02X", buf->data[i]);
        if ((i + 1) % 16 == 0) fprintf(f, "\n");
        else if ((i + 1) % 4 == 0) fprintf(f, " ");
    }
    if (buf->size % 16 != 0) fprintf(f, "\n");

    fclose(f);
    return true;
}

// ============================================================================
// DISASSEMBLY
// ============================================================================

static const char *opcode_names[] = {
    [TGQ_I_NOP]       = "nop",
    [TGQ_I_ADD]       = "add",
    [TGQ_I_SUB]       = "sub",
    [TGQ_I_MUL]       = "mul",
    [TGQ_I_DIV]       = "div",
    [TGQ_I_FML]       = "fma",
    [TGQ_I_SQRT]      = "sqrt",
    [TGQ_I_MIN]       = "min",
    [TGQ_I_MAX]       = "max",
    [TGQ_I_AND]       = "and",
    [TGQ_I_OR]        = "or",
    [TGQ_I_XOR]       = "xor",
    [TGQ_I_NOT]       = "not",
    [TGQ_I_SHL]       = "shl",
    [TGQ_I_SHR]       = "shr",
    [TGQ_I_MOV]       = "mov",
    [TGQ_I_XCHG]      = "xchg",
    [TGQ_I_BRA]       = "bra",
    [TGQ_I_BEQ]       = "beq",
    [TGQ_I_BNE]       = "bne",
    [TGQ_I_BLT]       = "blt",
    [TGQ_I_BGT]       = "bgt",
    [TGQ_I_CALL]      = "call",
    [TGQ_I_LD_GLOBAL] = "ld_global",
    [TGQ_I_ST_GLOBAL] = "st_global",
    [TGQ_I_LD_LOCAL]  = "ld_local",
    [TGQ_I_ST_LOCAL]  = "st_local",
    [TGQ_I_LCONST8]   = "lconst.8",
    [TGQ_I_LCONST16]  = "lconst.16",
    [TGQ_I_LCONST32]  = "lconst.32",
    [TGQ_I_LCONST64]  = "lconst.64",
};

static const char *type_names[] = {
    [TGQ_I8]     = "i8",
    [TGQ_I16]    = "i16",
    [TGQ_I32]    = "i32",
    [TGQ_I64]    = "i64",
    [TGQ_FP16]   = "fp16",
    [TGQ_FP32]   = "fp32",
    [TGQ_BF16]   = "bf16",
    [TGQ_BF32]   = "bf32",
    [TGQ_V4I32]  = "v4i32",
    [TGQ_V4FP16] = "v4fp16",
    [TGQ_V4FP32] = "v4fp32",
    [TGQ_V4BF16] = "v4bf16",
    [TGQ_V4BF32] = "v4bf32",
};

void emit_disassemble(EmitBuffer *buf, FILE *out) {
    int i = 0;
    while (i < buf->size) {
        fprintf(out, "%04X: ", i);

        uint8_t op = buf->data[i++];

        // Check for special 32-bit opcodes
        if (i + 3 <= buf->size) {
            uint32_t word = *(uint32_t*)&buf->data[i-1];
            if (word == TGQ_I_RET) {
                fprintf(out, "ret\n");
                i += 3;
                continue;
            }
            if (word == TGQ_I_SYNC) {
                fprintf(out, "sync\n");
                i += 3;
                continue;
            }
        }

        if (op == TGQ_I_NOP) {
            fprintf(out, "nop\n");
            continue;
        }

        const char *name = (op < sizeof(opcode_names)/sizeof(opcode_names[0])) ?
                           opcode_names[op] : "???";
        fprintf(out, "%s", name ? name : "???");

        // Read type and registers based on opcode
        if (op >= TGQ_I_ADD && op <= TGQ_I_LCONST64) {
            if (i < buf->size) {
                uint8_t type = buf->data[i++];
                fprintf(out, ".%s", type_names[type]);
            }
        }

        fprintf(out, "\n");
    }
}
