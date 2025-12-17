#pragma once

#include "tgpu_quartz_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// ============================================================================
// EMIT BUFFER
// ============================================================================

typedef struct {
    uint8_t *data;
    int size;
    int capacity;
} EmitBuffer;

// ============================================================================
// LABEL MANAGEMENT
// ============================================================================

#define MAX_LABELS 256
#define MAX_RELOCATIONS 512

typedef enum {
    RELOC_BRANCH,      // Relative branch offset
    RELOC_ABSOLUTE     // Absolute address
} RelocType;

typedef struct {
    int offset;        // Position in buffer
    int label_id;      // Target label
    RelocType type;
} Relocation;

typedef struct {
    int label_id;
    int position;      // Position in buffer (-1 if not defined)
} LabelDef;

typedef struct {
    LabelDef labels[MAX_LABELS];
    int label_count;

    Relocation relocs[MAX_RELOCATIONS];
    int reloc_count;

    int next_label;
} LabelManager;

// ============================================================================
// REGISTER ENCODING
// ============================================================================

// Scalar register indices (0-7 for each type)
typedef enum {
    REG_A = 0, REG_B = 1, REG_C = 2, REG_D = 3,
    REG_E = 4, REG_F = 5, REG_G = 6, REG_H = 7
} RegIndex;

// ============================================================================
// EMIT BUFFER API
// ============================================================================

void emit_init(EmitBuffer *buf);
void emit_free(EmitBuffer *buf);
void emit_reset(EmitBuffer *buf);

// Raw byte emission
void emit_byte(EmitBuffer *buf, uint8_t b);
void emit_u16(EmitBuffer *buf, uint16_t w);
void emit_u32(EmitBuffer *buf, uint32_t w);
void emit_u64(EmitBuffer *buf, uint64_t w);
void emit_i32(EmitBuffer *buf, int32_t w);
void emit_f32(EmitBuffer *buf, float f);

// ============================================================================
// LABEL MANAGEMENT API
// ============================================================================

void labels_init(LabelManager *lm);
int label_create(LabelManager *lm);
void label_define(LabelManager *lm, EmitBuffer *buf, int label_id);
void label_add_reloc(LabelManager *lm, EmitBuffer *buf, int label_id, RelocType type);
bool labels_resolve(LabelManager *lm, EmitBuffer *buf);

// ============================================================================
// INSTRUCTION EMISSION
// ============================================================================

// NOP
void emit_nop(EmitBuffer *buf);

// Scalar 2-operand: rd = op(r1)
void emit_scalar2(EmitBuffer *buf, uint8_t op, uint8_t type, uint8_t rd, uint8_t r1);

// Scalar 3-operand: rd = op(r1, r2)
void emit_scalar3(EmitBuffer *buf, uint8_t op, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);

// Scalar 4-operand: rd = op(r1, r2, r3) - for FMA
void emit_scalar4(EmitBuffer *buf, uint8_t op, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2, uint8_t r3);

// Arithmetic shortcuts
void emit_add(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_sub(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_mul(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_div(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_fma(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2, uint8_t r3);

// Bitwise
void emit_and(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_or(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_xor(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_not(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1);
void emit_shl(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);
void emit_shr(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1, uint8_t r2);

// Move
void emit_mov(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t r1);

// Load constant
void emit_lconst8(EmitBuffer *buf, uint8_t rd, uint8_t value);
void emit_lconst16(EmitBuffer *buf, uint8_t rd, uint16_t value);
void emit_lconst32(EmitBuffer *buf, uint8_t rd, uint32_t value);
void emit_lconst64(EmitBuffer *buf, uint8_t rd, uint64_t value);
void emit_lconst_f32(EmitBuffer *buf, uint8_t rd, float value);

// Memory access
void emit_ld_global(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff);
void emit_st_global(EmitBuffer *buf, uint8_t type, uint8_t rsrc, uint8_t rbase, uint8_t roff);
void emit_ld_local(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff);
void emit_st_local(EmitBuffer *buf, uint8_t type, uint8_t rsrc, uint8_t rbase, uint8_t roff);

// Control flow
void emit_bra(EmitBuffer *buf, LabelManager *lm, int label_id);
void emit_beq(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id);
void emit_bne(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id);
void emit_blt(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id);
void emit_bgt(EmitBuffer *buf, uint8_t type, uint8_t r1, uint8_t r2, LabelManager *lm, int label_id);
void emit_call(EmitBuffer *buf, LabelManager *lm, int label_id);
void emit_ret(EmitBuffer *buf);
void emit_sync(EmitBuffer *buf);

// Atomic
void emit_atomic_add(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff);
void emit_atomic_sub(EmitBuffer *buf, uint8_t type, uint8_t rd, uint8_t rbase, uint8_t roff);

// ============================================================================
// OUTPUT
// ============================================================================

// Write bytecode to file
bool emit_write_file(EmitBuffer *buf, const char *filename);

// Write hex dump (for debugging/simulator)
bool emit_write_hex(EmitBuffer *buf, const char *filename);

// Print disassembly
void emit_disassemble(EmitBuffer *buf, FILE *out);
