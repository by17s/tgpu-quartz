#include "../crt.h"

#include "tgpu_quartz_emit.h"
#include "tgpu_quartz_symtab.h"

#include <stdlib.h>
#include <math.h>

EmitBuffer g_emitBufferCode;
EmitBuffer g_emitBufferData;
SymbolTable *g_symtab;
const char* g_current_block_name = "<Main>";

uint8_t g_local_reg[TGQ_TYPE_TOP] = {0};

int gen_reg_local(EmitBuffer *_data, uint8_t type, void* data) {
    int offset = _data->size;
    printf("Adding new local var to memory [BASE+%016x] (value i32=%d f32=%f)...\n\n", offset, *(uint32_t*)data, *(float*)data);
    switch (type)
    {
    case TGQ_I16:
    case TGQ_FP16:
    case TGQ_BF16:
        uint16_t x16 = *(uint32_t*)data;
        emit_u16(_data, x16);
        return offset;
    case TGQ_I32:
    case TGQ_FP32:
    case TGQ_BF32:
        uint32_t x32 = *(uint32_t*)data;
        emit_u32(_data, x32);
        return offset;
    default:
        return -1;
    }
}

int walk_vardecl(ASTNode *node) {
    TypeInfo* tinf = type_from_name(node->data.var_decl.type);
    if (tinf == NULL) {
        crt_err("Invalid type:");
        printf("Unknown type <%s> for %s\n", node->data.var_decl.type, node->data.var_decl.name);
        return 0;
    }

    Symbol* sym = symtab_define(
        g_symtab,
        node->data.var_decl.name,
        SYM_VARIABLE,
        tinf,
        g_current_block_name ? STORAGE_LOCAL : STORAGE_GLOBAL
    );

    if(sym == NULL) {
        return 0;
    }

    uint64_t default_val = 0;
    void* vval = &default_val;

    if(node->data.var_decl.initializer->type == AST_LITERAL) {
        switch (tinf->tgq_type)
        {
        case TGQ_I32:
            int xi = atoi(node->data.var_decl.initializer->data.literal.value);
            vval = &xi;
            break;
        case TGQ_FP32:
            float xf = atof(node->data.var_decl.initializer->data.literal.value);
            vval = &xf;
            break;
        case TGQ_BF32:
            float xbf32 = atof(node->data.var_decl.initializer->data.literal.value);
            vval = &xbf32;
            break;
        case TGQ_I16:
            float xi16 = atoi(node->data.var_decl.initializer->data.literal.value);
            vval = &xi16;
            break;
        case TGQ_FP16:
            uint16_t xfp16 = float32_to_fp16(atof(node->data.var_decl.initializer->data.literal.value));
            vval = &xfp16;
            break;
        default:
            break;
        }
    } else if (node->data.var_decl.initializer == NULL) {

    }

    sym->stack_offset = gen_reg_local(&g_emitBufferData, tinf->tgq_type, vval);
    if(sym->stack_offset == -1) {
        crt_err("Allocation failed:");
        printf("Details: typename=\"%s\" size=%s bytes\n", node->data.var_decl.type, tinf->size);
        exit(-1);
    }

    uint8_t reg = TGQ_R_GEN8(tinf->tgq_type, 0);
    switch (tinf->size) {
    case 1:
        emit_lconst8(&g_emitBufferCode, reg, (uint8_t)*(uint64_t*)vval);
        break;
    
    default:
        break;
    }
    return 1;
}

int walk_binexp(ASTNode *node) {

}

void walk_ast_node(ASTNode *node, int indent, FILE *output) {
    if (!node) {
        fprintf(output, "Empety code....\n");
        return;
    }
    
    switch (node->type) {
        case AST_PROGRAM:
            fprintf(output, "Program:\n");
            for (int i = 0; i < node->data.program.decl_count; i++) {
                walk_ast_node(node->data.program.declarations[i], indent + 1, output);
            }
            break;
            
        case AST_FUNCTION_DECL:
            fprintf(output, "FunctionDeclaration: %s %s (",
                    node->data.func_decl.return_type,
                    node->data.func_decl.name);
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                fprintf(output, "%s %s", 
                        node->data.func_decl.params[i].type,
                        node->data.func_decl.params[i].name);
                if (i < node->data.func_decl.param_count - 1) fprintf(output, ", ");
            }
            fprintf(output, ")\n");
            walk_ast_node(node->data.func_decl.body, indent + 1, output);
            break;
            
        case AST_STRUCT_DECL:
            fprintf(output, "StructDeclaration: %s\n", node->data.struct_decl.name);
            for (int i = 0; i < node->data.struct_decl.field_count; i++) {

                fprintf(output, "Field: %s %s\n",
                        node->data.struct_decl.fields[i].type,
                        node->data.struct_decl.fields[i].name);
            }
            break;
            
        case AST_VARIABLE_DECL:
            fprintf(output, "VariableDeclaration: %s %s",
                    node->data.var_decl.type,
                    node->data.var_decl.name);
            if (node->data.var_decl.is_array) {
                fprintf(output, "[%s]", 
                        node->data.var_decl.array_size ? node->data.var_decl.array_size : "");
            }
            fprintf(output, "\n");
            if (node->data.var_decl.initializer) {

                fprintf(output, "Initializer:\n");
                walk_ast_node(node->data.var_decl.initializer, indent + 2, output);
            }
            walk_vardecl(node);
            break;
            
        case AST_BLOCK_STMT:
            fprintf(output, "BlockStatement:\n");
            for (int i = 0; i < node->data.block_stmt.statement_count; i++) {
                walk_ast_node(node->data.block_stmt.statements[i], indent + 1, output);
            }
            break;
            
        case AST_EXPRESSION_STMT:
            fprintf(output, "ExpressionStatement:\n");
            walk_ast_node(node->data.expr_stmt.expression, indent + 1, output);
            break;
            
        case AST_IF_STMT:
            fprintf(output, "IfStatement:\n");
            fprintf(output, "Condition:\n");
            walk_ast_node(node->data.if_stmt.condition, indent + 2, output);
            fprintf(output, "Consequent:\n");
            walk_ast_node(node->data.if_stmt.consequent, indent + 2, output);
            if (node->data.if_stmt.alternate) {

                fprintf(output, "Alternate:\n");
                walk_ast_node(node->data.if_stmt.alternate, indent + 2, output);
            }
            break;
            
        case AST_FOR_STMT:
            fprintf(output, "ForStatement:\n");
            fprintf(output, "Init:\n");
            walk_ast_node(node->data.for_stmt.init, indent + 2, output);
            fprintf(output, "Test:\n");
            walk_ast_node(node->data.for_stmt.test, indent + 2, output);
            fprintf(output, "Update:\n");
            walk_ast_node(node->data.for_stmt.update, indent + 2, output);
            fprintf(output, "Body:\n");
            walk_ast_node(node->data.for_stmt.body, indent + 2, output);
            break;
            
        case AST_WHILE_STMT:
            fprintf(output, "WhileStatement:\n");
            fprintf(output, "Test:\n");
            walk_ast_node(node->data.while_stmt.test, indent + 2, output);
            fprintf(output, "Body:\n");
            walk_ast_node(node->data.while_stmt.body, indent + 2, output);
            break;
            
        case AST_RETURN_STMT:
            fprintf(output, "ReturnStatement:\n");
            if (node->data.return_stmt.argument) {
                walk_ast_node(node->data.return_stmt.argument, indent + 1, output);
            }
            break;
            
        case AST_BINARY_EXPR:
            fprintf(output, "BinaryExpression: %s\n", node->data.binary_expr.operator);
            fprintf(output, "Left:\n");
            walk_ast_node(node->data.binary_expr.left, indent + 2, output);
            fprintf(output, "Right:\n");
            walk_ast_node(node->data.binary_expr.right, indent + 2, output);
            walk_binexp(node);
            break;
            
        case AST_UNARY_EXPR:
            fprintf(output, "UnaryExpression: %s\n", node->data.unary_expr.operator);
            walk_ast_node(node->data.unary_expr.argument, indent + 1, output);
            break;
            
        case AST_CALL_EXPR:
            fprintf(output, "CallExpression:\n");
            fprintf(output, "Callee:\n");
            walk_ast_node(node->data.call_expr.callee, indent + 2, output);
            fprintf(output, "Arguments:\n");
            for (int i = 0; i < node->data.call_expr.arg_count; i++) {
                walk_ast_node(node->data.call_expr.arguments[i], indent + 2, output);
            }
            break;
            
        case AST_MEMBER_EXPR:
            fprintf(output, "MemberExpression: .%s\n", node->data.member_expr.property);
            fprintf(output, "Object:\n");
            walk_ast_node(node->data.member_expr.object, indent + 2, output);
            break;
            
        case AST_ARRAY_EXPR:
            fprintf(output, "ArrayExpression:\n");
            fprintf(output, "Array:\n");
            walk_ast_node(node->data.array_expr.array, indent + 2, output);
            fprintf(output, "Index:\n");
            walk_ast_node(node->data.array_expr.index, indent + 2, output);
            break;
            
        case AST_ASSIGNMENT_EXPR:
            fprintf(output, "AssignmentExpression: %s\n", node->data.assign_expr.operator);
            fprintf(output, "Left:\n");
            walk_ast_node(node->data.assign_expr.left, indent + 2, output);
            fprintf(output, "Right:\n");
            walk_ast_node(node->data.assign_expr.right, indent + 2, output);
            break;
            
        case AST_CONSTRUCTOR_EXPR:
            fprintf(output, "ConstructorExpression: %s\n", 
                    node->data.constructor_expr.type_name);
            for (int i = 0; i < node->data.constructor_expr.arg_count; i++) {
                walk_ast_node(node->data.constructor_expr.arguments[i], indent + 1, output);
            }
            break;
            
        case AST_IDENTIFIER:
            fprintf(output, "Identifier: %s\n", node->data.identifier.name);
            break;
            
        case AST_LITERAL:
            fprintf(output, "Literal: %s\n", node->data.literal.value);
            break;
    }
}

int gen_init(int flags) {
    emit_init(&g_emitBufferCode);
    emit_init(&g_emitBufferData);
    types_init();
    g_symtab = symtab_create();

    printf("TGPU\n");
}

int gen_by_ast(ASTNode *root) {
    walk_ast_node(root, 2, stdout);

    emit_write_file(&g_emitBufferData, ".data.hex");
}