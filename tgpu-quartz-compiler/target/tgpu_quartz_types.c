#include "tgpu_quartz_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// PREDEFINED TYPE INSTANCES
// ============================================================================

TypeInfo *TYPE_VOID_INFO = NULL;
TypeInfo *TYPE_BOOL_INFO = NULL;
TypeInfo *TYPE_INT_INFO = NULL;
TypeInfo *TYPE_FLOAT_INFO = NULL;
TypeInfo *TYPE_FP16_INFO = NULL;
TypeInfo *TYPE_VEC2_INFO = NULL;
TypeInfo *TYPE_VEC3_INFO = NULL;
TypeInfo *TYPE_VEC4_INFO = NULL;
TypeInfo *TYPE_IVEC2_INFO = NULL;
TypeInfo *TYPE_IVEC3_INFO = NULL;
TypeInfo *TYPE_IVEC4_INFO = NULL;
TypeInfo *TYPE_MAT2_INFO = NULL;
TypeInfo *TYPE_MAT3_INFO = NULL;
TypeInfo *TYPE_MAT4_INFO = NULL;

// ============================================================================
// TYPE MAPPING TABLE
// ============================================================================

typedef struct {
    const char *name;
    BaseType base;
    uint8_t tgq_type;
    int size;
    int components;
    RegisterClass reg_class;
} TypeMapping;

static TypeMapping type_mappings[] = {
    // Scalars
    {"void",       TYPE_VOID,   0,           0,  0, REGCLASS_NONE},
    {"bool",       TYPE_BOOL,   TGQ_I8,      1,  1, REGCLASS_SCALAR_I8},
    {"int",        TYPE_INT,    TGQ_I32,     4,  1, REGCLASS_SCALAR_I32},
    {"float",      TYPE_FLOAT,  TGQ_FP32,    4,  1, REGCLASS_SCALAR_FP32},
    {"double",     TYPE_DOUBLE, TGQ_I64,     8,  1, REGCLASS_SCALAR_I64},
    {"char",       TYPE_CHAR,   TGQ_I8,      1,  1, REGCLASS_SCALAR_I8},
    {"fp16",       TYPE_FLOAT,  TGQ_FP16,    2,  1, REGCLASS_SCALAR_FP16},
    {"bf16",       TYPE_FLOAT,  TGQ_BF16,    2,  1, REGCLASS_SCALAR_BF16},

    // Float vectors
    {"vec2",       TYPE_VEC2,   TGQ_V4FP32,   16,  2, REGCLASS_VECTOR},
    {"vec3",       TYPE_VEC3,   TGQ_V4FP32,   16,  3, REGCLASS_VECTOR},
    {"vec4",       TYPE_VEC4,   TGQ_V4FP32,   16,  4, REGCLASS_VECTOR},

    // Int vectors
    {"ivec2",      TYPE_IVEC2,  TGQ_V4I32,       16,  2, REGCLASS_VECTOR},
    {"ivec3",      TYPE_IVEC3,  TGQ_V4I32,       16,  3, REGCLASS_VECTOR},
    {"ivec4",      TYPE_IVEC4,  TGQ_V4I32,       16,  4, REGCLASS_VECTOR},

    // Bool vectors
    {"bvec2",      TYPE_BVEC2,  TGQ_V4I32,    16,  2, REGCLASS_VECTOR},
    {"bvec3",      TYPE_BVEC3,  TGQ_V4I32,    16,  3, REGCLASS_VECTOR},
    {"bvec4",      TYPE_BVEC4,  TGQ_V4I32,    16,  4, REGCLASS_VECTOR},

    // Matrices
    {"mat2",       TYPE_MAT2,   TGQ_FP32,   16,  4, REGCLASS_MATRIX},
    {"mat3",       TYPE_MAT3,   TGQ_FP32,   36,  9, REGCLASS_MATRIX},
    {"mat4",       TYPE_MAT4,   TGQ_FP32,   64, 16, REGCLASS_MATRIX},

    // Samplers
    {"sampler2D",  TYPE_SAMPLER2D,   TGQ_I64, 8, 1, REGCLASS_SCALAR_I64},
    {"sampler3D",  TYPE_SAMPLER3D,   TGQ_I64, 8, 1, REGCLASS_SCALAR_I64},
    {"samplerCube",TYPE_SAMPLERCUBE, TGQ_I64, 8, 1, REGCLASS_SCALAR_I64},

    {NULL, 0, 0, 0, 0, 0}
};

// ============================================================================
// TYPE CREATION
// ============================================================================

static TypeInfo *type_alloc(void) {
    TypeInfo *t = calloc(1, sizeof(TypeInfo));
    return t;
}

static TypeInfo *type_create_basic(BaseType base, uint8_t tgq, int size,
                                    int components, RegisterClass rc) {
    TypeInfo *t = type_alloc();
    t->base = base;
    t->tgq_type = tgq;
    t->size = size;
    t->alignment = size > 4 ? 4 : size;
    t->components = components;
    t->reg_class = rc;
    t->array_length = -1;
    return t;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void types_init(void) {
    // Create predefined types
    TYPE_VOID_INFO  = type_create_basic(TYPE_VOID,  0,         0,  0, REGCLASS_NONE);
    TYPE_BOOL_INFO  = type_create_basic(TYPE_BOOL,  TGQ_I8,    1,  1, REGCLASS_SCALAR_I8);
    TYPE_INT_INFO   = type_create_basic(TYPE_INT,   TGQ_I32,   4,  1, REGCLASS_SCALAR_I32);
    TYPE_FLOAT_INFO = type_create_basic(TYPE_FLOAT, TGQ_FP32,  4,  1, REGCLASS_SCALAR_FP32);
    TYPE_FP16_INFO = type_create_basic(TYPE_FLOAT, TGQ_FP16,  2,  1, REGCLASS_SCALAR_FP16);
    TYPE_VEC2_INFO  = type_create_basic(TYPE_VEC2,  TGQ_V4FP32,  16,  2, REGCLASS_VECTOR);
    TYPE_VEC3_INFO  = type_create_basic(TYPE_VEC3,  TGQ_V4FP32, 16,  3, REGCLASS_VECTOR);
    TYPE_VEC4_INFO  = type_create_basic(TYPE_VEC4,  TGQ_V4FP32, 16, 4, REGCLASS_VECTOR);
    TYPE_IVEC2_INFO = type_create_basic(TYPE_IVEC2, TGQ_V4I32,   16,  2, REGCLASS_VECTOR);
    TYPE_IVEC3_INFO = type_create_basic(TYPE_IVEC3, TGQ_V4I32,  16,  3, REGCLASS_VECTOR);
    TYPE_IVEC4_INFO = type_create_basic(TYPE_IVEC4, TGQ_V4I32, 16, 4, REGCLASS_VECTOR);
    TYPE_MAT2_INFO  = type_create_basic(TYPE_MAT2,  TGQ_FP32, 16,  4, REGCLASS_MATRIX);
    TYPE_MAT3_INFO  = type_create_basic(TYPE_MAT3,  TGQ_FP32, 36,  9, REGCLASS_MATRIX);
    TYPE_MAT4_INFO  = type_create_basic(TYPE_MAT4,  TGQ_FP32, 64, 16, REGCLASS_MATRIX);
}

void types_cleanup(void) {
    free(TYPE_VOID_INFO);
    free(TYPE_BOOL_INFO);
    free(TYPE_INT_INFO);
    free(TYPE_FLOAT_INFO);
    free(TYPE_VEC2_INFO);
    free(TYPE_VEC3_INFO);
    free(TYPE_VEC4_INFO);
    free(TYPE_IVEC2_INFO);
    free(TYPE_IVEC3_INFO);
    free(TYPE_IVEC4_INFO);
    free(TYPE_MAT2_INFO);
    free(TYPE_MAT3_INFO);
    free(TYPE_MAT4_INFO);
}

// ============================================================================
// TYPE LOOKUP
// ============================================================================

TypeInfo *type_from_name(const char *name) {
    if (!name) return NULL;

    for (int i = 0; type_mappings[i].name != NULL; i++) {
        if (strcmp(name, type_mappings[i].name) == 0) {
            TypeMapping *m = &type_mappings[i];
            return type_create_basic(m->base, m->tgq_type, m->size,
                                     m->components, m->reg_class);
        }
    }
    return NULL;
}

// ============================================================================
// COMPLEX TYPE CREATION
// ============================================================================

TypeInfo *type_make_array(TypeInfo *element, int length) {
    TypeInfo *t = type_alloc();
    t->base = TYPE_ARRAY;
    t->element_type = element;
    t->array_length = length;
    t->size = element->size * length;
    t->alignment = element->alignment;
    t->components = length;
    t->reg_class = REGCLASS_NONE; // Arrays go to memory
    return t;
}

TypeInfo *type_make_struct(const char *name, StructField *fields, int field_count) {
    TypeInfo *t = type_alloc();
    t->base = TYPE_STRUCT;
    t->struct_name = strdup(name);

    StructInfo *s = calloc(1, sizeof(StructInfo));
    s->name = strdup(name);
    s->fields = malloc(sizeof(StructField) * field_count);
    s->field_count = field_count;

    int offset = 0;
    int max_align = 1;
    for (int i = 0; i < field_count; i++) {
        s->fields[i].name = strdup(fields[i].name);
        s->fields[i].type = fields[i].type;

        // Align field
        int align = fields[i].type->alignment;
        if (align > max_align) max_align = align;
        offset = (offset + align - 1) & ~(align - 1);

        s->fields[i].offset = offset;
        offset += fields[i].type->size;
    }

    s->total_size = offset;
    s->alignment = max_align;

    t->struct_info = s;
    t->size = s->total_size;
    t->alignment = s->alignment;
    t->reg_class = REGCLASS_NONE;

    return t;
}

TypeInfo *type_make_function(TypeInfo *return_type, TypeInfo **params, int param_count) {
    TypeInfo *t = type_alloc();
    t->base = TYPE_FUNCTION;
    t->return_type = return_type;
    t->param_types = malloc(sizeof(TypeInfo*) * param_count);
    memcpy(t->param_types, params, sizeof(TypeInfo*) * param_count);
    t->param_count = param_count;
    return t;
}

// ============================================================================
// TYPE CHECKING
// ============================================================================

bool types_equal(TypeInfo *a, TypeInfo *b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->base != b->base) return false;

    switch (a->base) {
        case TYPE_ARRAY:
            return a->array_length == b->array_length &&
                   types_equal(a->element_type, b->element_type);
        case TYPE_STRUCT:
            return strcmp(a->struct_name, b->struct_name) == 0;
        case TYPE_FUNCTION:
            if (!types_equal(a->return_type, b->return_type)) return false;
            if (a->param_count != b->param_count) return false;
            for (int i = 0; i < a->param_count; i++) {
                if (!types_equal(a->param_types[i], b->param_types[i])) return false;
            }
            return true;
        default:
            return true;
    }
}

bool types_compatible(TypeInfo *a, TypeInfo *b) {
    if (types_equal(a, b)) return true;

    // Int can convert to float
    if ((a->base == TYPE_INT && b->base == TYPE_FLOAT) ||
        (a->base == TYPE_FLOAT && b->base == TYPE_INT)) {
        return true;
    }

    // Bool can convert to int
    if ((a->base == TYPE_BOOL && b->base == TYPE_INT) ||
        (a->base == TYPE_INT && b->base == TYPE_BOOL)) {
        return true;
    }

    return false;
}

bool type_is_numeric(TypeInfo *t) {
    if (!t) return false;
    switch (t->base) {
        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
        case TYPE_VEC2:
        case TYPE_VEC3:
        case TYPE_VEC4:
        case TYPE_IVEC2:
        case TYPE_IVEC3:
        case TYPE_IVEC4:
        case TYPE_MAT2:
        case TYPE_MAT3:
        case TYPE_MAT4:
            return true;
        default:
            return false;
    }
}

bool type_is_vector(TypeInfo *t) {
    if (!t) return false;
    switch (t->base) {
        case TYPE_VEC2:
        case TYPE_VEC3:
        case TYPE_VEC4:
        case TYPE_IVEC2:
        case TYPE_IVEC3:
        case TYPE_IVEC4:
        case TYPE_BVEC2:
        case TYPE_BVEC3:
        case TYPE_BVEC4:
            return true;
        default:
            return false;
    }
}

bool type_is_matrix(TypeInfo *t) {
    if (!t) return false;
    return t->base == TYPE_MAT2 || t->base == TYPE_MAT3 || t->base == TYPE_MAT4;
}

bool type_is_scalar(TypeInfo *t) {
    if (!t) return false;
    switch (t->base) {
        case TYPE_BOOL:
        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
        case TYPE_CHAR:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// BINARY OPERATION TYPE INFERENCE
// ============================================================================

TypeInfo *type_binary_result(const char *op, TypeInfo *left, TypeInfo *right) {
    if (!left || !right) return NULL;

    // Comparison operators always return bool
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<") == 0  || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
        return TYPE_BOOL_INFO;
    }

    // Logical operators return bool
    if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
        return TYPE_BOOL_INFO;
    }

    // Arithmetic: if either is float, result is float
    if (type_is_scalar(left) && type_is_scalar(right)) {
        if (left->base == TYPE_FLOAT || right->base == TYPE_FLOAT) {
            return TYPE_FLOAT_INFO;
        }
        return TYPE_INT_INFO;
    }

    // Vector operations
    if (type_is_vector(left) && type_is_vector(right)) {
        if (left->components == right->components) {
            return left;
        }
    }

    // Scalar * vector = vector
    if (type_is_scalar(left) && type_is_vector(right)) {
        return right;
    }
    if (type_is_vector(left) && type_is_scalar(right)) {
        return left;
    }

    // Matrix operations
    if (type_is_matrix(left) && type_is_matrix(right)) {
        // Matrix * Matrix = Matrix
        if (strcmp(op, "*") == 0) {
            return left;
        }
        return left;
    }

    // Matrix * Vector
    if (type_is_matrix(left) && type_is_vector(right)) {
        if (left->base == TYPE_MAT4 && right->base == TYPE_VEC4) {
            return TYPE_VEC4_INFO;
        }
        if (left->base == TYPE_MAT3 && right->base == TYPE_VEC3) {
            return TYPE_VEC3_INFO;
        }
        if (left->base == TYPE_MAT2 && right->base == TYPE_VEC2) {
            return TYPE_VEC2_INFO;
        }
    }

    return left; // Default
}

TypeInfo *type_unary_result(const char *op, TypeInfo *operand) {
    if (!operand) return NULL;

    if (strcmp(op, "!") == 0) {
        return TYPE_BOOL_INFO;
    }

    // +, -, ++, -- preserve type
    return operand;
}

// ============================================================================
// MEMBER ACCESS
// ============================================================================

TypeInfo *type_get_member(TypeInfo *type, const char *member) {
    if (!type || !member) return NULL;

    // Struct member access
    if (type->base == TYPE_STRUCT && type->struct_info) {
        for (int i = 0; i < type->struct_info->field_count; i++) {
            if (strcmp(type->struct_info->fields[i].name, member) == 0) {
                return type->struct_info->fields[i].type;
            }
        }
        return NULL;
    }

    // Vector swizzle
    if (type_is_vector(type)) {
        int len = strlen(member);
        if (len == 1) {
            // Single component: .x, .y, .z, .w, .r, .g, .b, .a
            return TYPE_FLOAT_INFO;
        }
        if (len == 2) {
            return TYPE_VEC2_INFO;
        }
        if (len == 3) {
            return TYPE_VEC3_INFO;
        }
        if (len == 4) {
            return TYPE_VEC4_INFO;
        }
    }

    return NULL;
}

// ============================================================================
// SWIZZLE PARSING
// ============================================================================

static int swizzle_char_to_index(char c) {
    switch (c) {
        case 'x': case 'r': case 's': return 0;
        case 'y': case 'g': case 't': return 1;
        case 'z': case 'b': case 'p': return 2;
        case 'w': case 'a': case 'q': return 3;
        default: return -1;
    }
}

SwizzleInfo *swizzle_parse(const char *pattern, int source_components) {
    if (!pattern) return NULL;

    int len = strlen(pattern);
    if (len == 0 || len > 4) return NULL;

    SwizzleInfo *s = malloc(sizeof(SwizzleInfo));
    s->count = len;

    for (int i = 0; i < len; i++) {
        int idx = swizzle_char_to_index(pattern[i]);
        if (idx < 0 || idx >= source_components) {
            free(s);
            return NULL;
        }
        s->indices[i] = idx;
    }

    return s;
}

void swizzle_free(SwizzleInfo *s) {
    free(s);
}

// ============================================================================
// TARGET TYPE MAPPING
// ============================================================================

uint8_t type_to_tgq(TypeInfo *t) {
    if (!t) return TGQ_I32;
    return t->tgq_type;
}

RegisterClass type_register_class(TypeInfo *t) {
    if (!t) return REGCLASS_NONE;
    return t->reg_class;
}

uint16_t float32_to_fp16(float f) {
    uint32_t x = *(uint32_t*)&f;
    uint32_t sign = (x >> 16) & 0x8000;
    uint32_t exponent = ((x >> 23) & 0xFF) - 127 + 15;
    uint32_t mantissa = (x >> 13) & 0x3FF;

    if (exponent <= 0) {
        return sign;
    } else if (exponent >= 31) {
        return sign | 0x7C00;
    } else {
        return sign | (exponent << 10) | mantissa;
    }
}