#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "tgpu_quartz_defs.h"

// ============================================================================
// BASE TYPES
// ============================================================================

typedef enum {
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CHAR,

    // Vector types
    TYPE_VEC2,
    TYPE_VEC3,
    TYPE_VEC4,
    TYPE_IVEC2,
    TYPE_IVEC3,
    TYPE_IVEC4,
    TYPE_BVEC2,
    TYPE_BVEC3,
    TYPE_BVEC4,

    // Matrix types
    TYPE_MAT2,
    TYPE_MAT3,
    TYPE_MAT4,

    // Sampler types
    TYPE_SAMPLER2D,
    TYPE_SAMPLER3D,
    TYPE_SAMPLERCUBE,

    // User-defined
    TYPE_STRUCT,
    TYPE_ARRAY,
    TYPE_FUNCTION,

    TYPE_COUNT
} BaseType;

// ============================================================================
// REGISTER CLASSES
// ============================================================================

typedef enum {
    REGCLASS_NONE = 0,
    REGCLASS_SCALAR_I8,
    REGCLASS_SCALAR_I16,
    REGCLASS_SCALAR_I32,
    REGCLASS_SCALAR_I64,
    REGCLASS_SCALAR_FP16,
    REGCLASS_SCALAR_FP32,
    REGCLASS_SCALAR_BF16,
    REGCLASS_SCALAR_BF32,
    REGCLASS_VECTOR,
    REGCLASS_MATRIX
} RegisterClass;

// ============================================================================
// TYPE INFO STRUCTURE
// ============================================================================

typedef struct TypeInfo TypeInfo;
typedef struct StructInfo StructInfo;

typedef struct StructField {
    char *name;
    TypeInfo *type;
    int offset;              // Byte offset within struct
} StructField;

struct StructInfo {
    char *name;
    StructField *fields;
    int field_count;
    int total_size;
    int alignment;
};

struct TypeInfo {
    BaseType base;
    int size;                // Size in bytes
    int alignment;           // Memory alignment
    int components;          // For vectors: 2, 3, 4; for scalars: 1

    // For arrays
    TypeInfo *element_type;
    int array_length;        // -1 for unsized

    // For structs
    char *struct_name;
    StructInfo *struct_info;

    // For functions
    TypeInfo *return_type;
    TypeInfo **param_types;
    int param_count;

    // Target-specific info
    uint8_t tgq_type;        // TGQ_I32, TGQ_FP32, etc.
    RegisterClass reg_class; // Which register file to use
};

// ============================================================================
// SWIZZLE SUPPORT
// ============================================================================

typedef struct {
    int indices[4];          // Component indices (0-3)
    int count;               // Number of components (1-4)
} SwizzleInfo;

// ============================================================================
// TYPE SYSTEM API
// ============================================================================

// Initialize type system (call once at startup)
void types_init(void);
void types_cleanup(void);

// Get type info from TGQL type name
TypeInfo *type_from_name(const char *name);

// Create array type
TypeInfo *type_make_array(TypeInfo *element, int length);

// Create struct type
TypeInfo *type_make_struct(const char *name, StructField *fields, int field_count);

// Create function type
TypeInfo *type_make_function(TypeInfo *return_type, TypeInfo **params, int param_count);

// Type checking
bool types_equal(TypeInfo *a, TypeInfo *b);
bool types_compatible(TypeInfo *a, TypeInfo *b);
bool type_is_numeric(TypeInfo *t);
bool type_is_vector(TypeInfo *t);
bool type_is_matrix(TypeInfo *t);
bool type_is_scalar(TypeInfo *t);

// Get result type for binary operation
TypeInfo *type_binary_result(const char *op, TypeInfo *left, TypeInfo *right);

// Get result type for unary operation
TypeInfo *type_unary_result(const char *op, TypeInfo *operand);

// Get member type (for struct.field or vec.xy)
TypeInfo *type_get_member(TypeInfo *type, const char *member);

// Parse swizzle pattern (xyz, rgb, stp)
SwizzleInfo *swizzle_parse(const char *pattern, int source_components);
void swizzle_free(SwizzleInfo *s);

// Get TGQ type for a base type
uint8_t type_to_tgq(TypeInfo *t);

// Get register class for a type
RegisterClass type_register_class(TypeInfo *t);

uint16_t float32_to_fp16(float f);

// Predefined types (initialized in types_init)
extern TypeInfo *TYPE_VOID_INFO;
extern TypeInfo *TYPE_BOOL_INFO;
extern TypeInfo *TYPE_INT_INFO;
extern TypeInfo *TYPE_FLOAT_INFO;
extern TypeInfo *TYPE_VEC2_INFO;
extern TypeInfo *TYPE_VEC3_INFO;
extern TypeInfo *TYPE_VEC4_INFO;
extern TypeInfo *TYPE_IVEC2_INFO;
extern TypeInfo *TYPE_IVEC3_INFO;
extern TypeInfo *TYPE_IVEC4_INFO;
extern TypeInfo *TYPE_MAT2_INFO;
extern TypeInfo *TYPE_MAT3_INFO;
extern TypeInfo *TYPE_MAT4_INFO;
