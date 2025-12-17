#include "tgpu_quartz_symtab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// HASH FUNCTION
// ============================================================================

static unsigned int hash_string(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % SYMTAB_HASH_SIZE;
}

// ============================================================================
// SCOPE MANAGEMENT
// ============================================================================

static Scope *scope_create(Scope *parent, int level) {
    Scope *s = calloc(1, sizeof(Scope));
    s->parent = parent;
    s->scope_level = level;
    s->child_capacity = 4;
    s->children = malloc(sizeof(Scope*) * s->child_capacity);
    s->stack_offset = parent ? parent->stack_offset : 0;
    return s;
}

static void scope_add_child(Scope *parent, Scope *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = realloc(parent->children,
                                   sizeof(Scope*) * parent->child_capacity);
    }
    parent->children[parent->child_count++] = child;
}

static void scope_destroy(Scope *s) {
    if (!s) return;

    // Free symbols
    for (int i = 0; i < SYMTAB_HASH_SIZE; i++) {
        Symbol *sym = s->buckets[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            if (sym->params) free(sym->params);
            free(sym);
            sym = next;
        }
    }

    // Free children
    for (int i = 0; i < s->child_count; i++) {
        scope_destroy(s->children[i]);
    }
    free(s->children);
    free(s);
}

// ============================================================================
// SYMBOL TABLE CREATION
// ============================================================================

SymbolTable *symtab_create(void) {
    SymbolTable *st = calloc(1, sizeof(SymbolTable));
    st->global = scope_create(NULL, 0);
    st->current = st->global;
    st->scope_depth = 0;

    st->struct_capacity = 16;
    st->structs = malloc(sizeof(StructInfo*) * st->struct_capacity);

    st->func_capacity = 32;
    st->functions = malloc(sizeof(Symbol*) * st->func_capacity);

    return st;
}

void symtab_destroy(SymbolTable *st) {
    if (!st) return;
    scope_destroy(st->global);
    free(st->structs);
    free(st->functions);
    free(st);
}

// ============================================================================
// SCOPE OPERATIONS
// ============================================================================

void symtab_enter_scope(SymbolTable *st) {
    st->scope_depth++;
    Scope *child = scope_create(st->current, st->scope_depth);
    scope_add_child(st->current, child);
    st->current = child;
}

void symtab_exit_scope(SymbolTable *st) {
    if (st->current->parent) {
        st->current = st->current->parent;
        st->scope_depth--;
    }
}

int symtab_scope_depth(SymbolTable *st) {
    return st->scope_depth;
}

// ============================================================================
// SYMBOL DEFINITION
// ============================================================================

static Symbol *symbol_create(const char *name, SymbolKind kind,
                             TypeInfo *type, StorageClass storage, int level) {
    Symbol *sym = calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = kind;
    sym->type = type;
    sym->storage = storage;
    sym->scope_level = level;
    sym->reg_index = -1;
    sym->stack_offset = -1;
    return sym;
}

static void scope_insert(Scope *s, Symbol *sym) {
    unsigned int h = hash_string(sym->name);
    sym->next = s->buckets[h];
    s->buckets[h] = sym;
    s->symbol_count++;
}

Symbol *symtab_define(SymbolTable *st, const char *name, SymbolKind kind,
                      TypeInfo *type, StorageClass storage) {
    // Check for duplicate in current scope
    Symbol *existing = symtab_lookup_local(st, name);
    if (existing) {
        fprintf(stderr, "Error: redefinition of '%s'\n", name);
        return NULL;
    }

    Symbol *sym = symbol_create(name, kind, type, storage, st->scope_depth);

    // Allocate stack space for local variables
    if (storage == STORAGE_LOCAL && kind == SYM_VARIABLE) {
        sym->stack_offset = symtab_alloc_local(st, type->size, type->alignment);
    }

    scope_insert(st->current, sym);
    return sym;
}

Symbol *symtab_define_param(SymbolTable *st, const char *name, TypeInfo *type) {
    return symtab_define(st, name, SYM_PARAMETER, type, STORAGE_IN);
}

Symbol *symtab_define_function(SymbolTable *st, const char *name,
                               TypeInfo *return_type, Symbol **params, int param_count) {
    TypeInfo **param_types = NULL;
    if (param_count > 0) {
        param_types = malloc(sizeof(TypeInfo*) * param_count);
        for (int i = 0; i < param_count; i++) {
            param_types[i] = params[i]->type;
        }
    }

    TypeInfo *func_type = type_make_function(return_type, param_types, param_count);
    free(param_types);

    Symbol *sym = symtab_define(st, name, SYM_FUNCTION, func_type, STORAGE_GLOBAL);
    if (sym) {
        sym->params = params;
        sym->param_count = param_count;

        // Register function
        if (st->func_count >= st->func_capacity) {
            st->func_capacity *= 2;
            st->functions = realloc(st->functions, sizeof(Symbol*) * st->func_capacity);
        }
        st->functions[st->func_count++] = sym;
    }

    return sym;
}

// ============================================================================
// SYMBOL LOOKUP
// ============================================================================

Symbol *symtab_lookup_local(SymbolTable *st, const char *name) {
    unsigned int h = hash_string(name);
    Symbol *sym = st->current->buckets[h];
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    return NULL;
}

Symbol *symtab_lookup(SymbolTable *st, const char *name) {
    unsigned int h = hash_string(name);

    // Search from current scope up to global
    Scope *s = st->current;
    while (s) {
        Symbol *sym = s->buckets[h];
        while (sym) {
            if (strcmp(sym->name, name) == 0) {
                return sym;
            }
            sym = sym->next;
        }
        s = s->parent;
    }

    return NULL;
}

Symbol *symtab_lookup_function(SymbolTable *st, const char *name) {
    for (int i = 0; i < st->func_count; i++) {
        if (strcmp(st->functions[i]->name, name) == 0) {
            return st->functions[i];
        }
    }
    return NULL;
}

// ============================================================================
// STRUCT REGISTRATION
// ============================================================================

void symtab_register_struct(SymbolTable *st, StructInfo *info) {
    if (st->struct_count >= st->struct_capacity) {
        st->struct_capacity *= 2;
        st->structs = realloc(st->structs, sizeof(StructInfo*) * st->struct_capacity);
    }
    st->structs[st->struct_count++] = info;

    // Also add to symbol table as a type
    TypeInfo *struct_type = type_make_struct(info->name, info->fields, info->field_count);
    symtab_define(st, info->name, SYM_STRUCT, struct_type, STORAGE_GLOBAL);
}

StructInfo *symtab_find_struct(SymbolTable *st, const char *name) {
    for (int i = 0; i < st->struct_count; i++) {
        if (strcmp(st->structs[i]->name, name) == 0) {
            return st->structs[i];
        }
    }
    return NULL;
}

// ============================================================================
// STACK ALLOCATION
// ============================================================================

int symtab_alloc_local(SymbolTable *st, int size, int alignment) {
    Scope *s = st->current;

    // Align offset
    int offset = (s->stack_offset + alignment - 1) & ~(alignment - 1);
    s->stack_offset = offset + size;

    return offset;
}

// ============================================================================
// DEBUG OUTPUT
// ============================================================================

static const char *storage_class_name(StorageClass sc) {
    switch (sc) {
        case STORAGE_LOCAL:     return "local";
        case STORAGE_GLOBAL:    return "global";
        case STORAGE_UNIFORM:   return "uniform";
        case STORAGE_ATTRIBUTE: return "attribute";
        case STORAGE_VARYING:   return "varying";
        case STORAGE_IN:        return "in";
        case STORAGE_OUT:       return "out";
        case STORAGE_INOUT:     return "inout";
        case STORAGE_CONST:     return "const";
        case STORAGE_REGISTER:  return "register";
        default:                return "unknown";
    }
}

static const char *symbol_kind_name(SymbolKind k) {
    switch (k) {
        case SYM_VARIABLE:     return "variable";
        case SYM_FUNCTION:     return "function";
        case SYM_PARAMETER:    return "parameter";
        case SYM_STRUCT:       return "struct";
        case SYM_BUILTIN_FUNC: return "builtin";
        default:               return "unknown";
    }
}

static void dump_scope(Scope *s, FILE *out, int indent) {
    for (int i = 0; i < SYMTAB_HASH_SIZE; i++) {
        Symbol *sym = s->buckets[i];
        while (sym) {
            fprintf(out, "%*s%s '%s' : %s %s",
                    indent, "",
                    symbol_kind_name(sym->kind),
                    sym->name,
                    storage_class_name(sym->storage),
                    sym->type ? "(has type)" : "(no type)");

            if (sym->reg_index >= 0) {
                fprintf(out, " reg=%d", sym->reg_index);
            }
            if (sym->stack_offset >= 0) {
                fprintf(out, " stack=%d", sym->stack_offset);
            }
            fprintf(out, "\n");

            sym = sym->next;
        }
    }

    for (int i = 0; i < s->child_count; i++) {
        fprintf(out, "%*s{\n", indent, "");
        dump_scope(s->children[i], out, indent + 2);
        fprintf(out, "%*s}\n", indent, "");
    }
}

void symtab_dump(SymbolTable *st, FILE *out) {
    fprintf(out, "=== Symbol Table ===\n");
    fprintf(out, "Structs: %d\n", st->struct_count);
    fprintf(out, "Functions: %d\n", st->func_count);
    fprintf(out, "--- Symbols ---\n");
    dump_scope(st->global, out, 0);
}