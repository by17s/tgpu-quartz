#pragma once

#include "tgpu_quartz_types.h"
#include <stdbool.h>

// ============================================================================
// SYMBOL KINDS
// ============================================================================

typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_PARAMETER,
    SYM_STRUCT,
    SYM_BUILTIN_FUNC
} SymbolKind;

// ============================================================================
// STORAGE CLASSES
// ============================================================================

typedef enum {
    STORAGE_LOCAL,        // Local variable
    STORAGE_GLOBAL,       // Global variable
    STORAGE_UNIFORM,      // Uniform (constant across invocations)
    STORAGE_ATTRIBUTE,    // Vertex attribute input
    STORAGE_VARYING,      // Interpolated value
    STORAGE_IN,           // Input parameter
    STORAGE_OUT,          // Output parameter
    STORAGE_INOUT,        // Input/output parameter
    STORAGE_CONST,        // Compile-time constant
    STORAGE_REGISTER      // Already allocated to register
} StorageClass;

// ============================================================================
// SYMBOL STRUCTURE
// ============================================================================

typedef struct Symbol Symbol;
typedef struct Scope Scope;

struct Symbol {
    char *name;
    SymbolKind kind;
    TypeInfo *type;
    StorageClass storage;

    // Source location
    int def_line;
    int def_col;

    // Scope info
    int scope_level;

    // Code generation info
    int reg_index;           // Assigned register (-1 if in memory)
    RegisterClass reg_class; // Which register file
    int stack_offset;        // Offset in local memory

    // For functions
    struct ASTNode *func_body;    // Function body AST (forward decl)
    Symbol **params;              // Parameter symbols
    int param_count;
    int local_count;              // Number of local variables

    // Hash chain
    Symbol *next;
};

// ============================================================================
// SCOPE STRUCTURE
// ============================================================================

#define SYMTAB_HASH_SIZE 64

struct Scope {
    Symbol *buckets[SYMTAB_HASH_SIZE];
    int symbol_count;
    int scope_level;

    Scope *parent;
    Scope **children;
    int child_count;
    int child_capacity;

    // Stack allocation for this scope
    int stack_offset;
};

// ============================================================================
// SYMBOL TABLE
// ============================================================================

typedef struct {
    Scope *global;
    Scope *current;
    int scope_depth;

    // All user-defined structs
    StructInfo **structs;
    int struct_count;
    int struct_capacity;

    // All functions
    Symbol **functions;
    int func_count;
    int func_capacity;
} SymbolTable;

// ============================================================================
// SYMBOL TABLE API
// ============================================================================

// Create/destroy
SymbolTable *symtab_create(void);
void symtab_destroy(SymbolTable *st);

// Scope management
void symtab_enter_scope(SymbolTable *st);
void symtab_exit_scope(SymbolTable *st);
int symtab_scope_depth(SymbolTable *st);

// Symbol definition
Symbol *symtab_define(SymbolTable *st, const char *name, SymbolKind kind,
                      TypeInfo *type, StorageClass storage);
Symbol *symtab_define_param(SymbolTable *st, const char *name, TypeInfo *type);
Symbol *symtab_define_function(SymbolTable *st, const char *name,
                               TypeInfo *return_type, Symbol **params, int param_count);

// Symbol lookup
Symbol *symtab_lookup(SymbolTable *st, const char *name);
Symbol *symtab_lookup_local(SymbolTable *st, const char *name);
Symbol *symtab_lookup_function(SymbolTable *st, const char *name);

// Struct registration
void symtab_register_struct(SymbolTable *st, StructInfo *info);
StructInfo *symtab_find_struct(SymbolTable *st, const char *name);

// Stack allocation
int symtab_alloc_local(SymbolTable *st, int size, int alignment);

// Debug
void symtab_dump(SymbolTable *st, FILE *out);
