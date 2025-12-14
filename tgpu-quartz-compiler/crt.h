#pragma once

// ============================================================================
// TOKEN DEFINITIONS
// ============================================================================

typedef enum {
    // Keywords
    TOK_KEYWORD,
    TOK_TYPE,
    // Identifiers and literals
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    // Operators
    TOK_OPERATOR,
    // Punctuation
    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_SEMICOLON, TOK_COMMA, TOK_DOT,
    // Special
    TOK_COMMENT,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int col;
} Token;

// ============================================================================
// AST NODE DEFINITIONS
// ============================================================================

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION_DECL,
    AST_VARIABLE_DECL,
    AST_STRUCT_DECL,
    AST_BLOCK_STMT,
    AST_EXPRESSION_STMT,
    AST_IF_STMT,
    AST_FOR_STMT,
    AST_WHILE_STMT,
    AST_RETURN_STMT,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_CALL_EXPR,
    AST_MEMBER_EXPR,
    AST_ARRAY_EXPR,
    AST_ASSIGNMENT_EXPR,
    AST_CONSTRUCTOR_EXPR,
    AST_IDENTIFIER,
    AST_LITERAL
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct {
    char *type;
    char *name;
} Parameter;

typedef struct {
    char *name;
    Parameter *fields;
    int field_count;
} StructDecl;

typedef struct {
    char **qualifiers;
    int qualifier_count;
    char *type;
    char *name;
    ASTNode *initializer;
    bool is_array;
    char *array_size;  // Can be a number or expression
} VariableDecl;

typedef struct {
    char **qualifiers;
    int qualifier_count;
    char *return_type;
    char *name;
    Parameter *params;
    int param_count;
    ASTNode *body;
} FunctionDecl;

typedef struct {
    ASTNode **statements;
    int statement_count;
} BlockStmt;

typedef struct {
    ASTNode *condition;
    ASTNode *consequent;
    ASTNode *alternate;
} IfStmt;

typedef struct {
    ASTNode *init;
    ASTNode *test;
    ASTNode *update;
    ASTNode *body;
} ForStmt;

typedef struct {
    ASTNode *test;
    ASTNode *body;
} WhileStmt;

typedef struct {
    ASTNode *argument;
} ReturnStmt;

typedef struct {
    char *operator;
    ASTNode *left;
    ASTNode *right;
} BinaryExpr;

typedef struct {
    char *operator;
    ASTNode *argument;
} UnaryExpr;

typedef struct {
    ASTNode *callee;
    ASTNode **arguments;
    int arg_count;
} CallExpr;

typedef struct {
    ASTNode *object;
    char *property;
} MemberExpr;

typedef struct {
    ASTNode *array;
    ASTNode *index;
} ArrayExpr;

typedef struct {
    char *operator;
    ASTNode *left;
    ASTNode *right;
} AssignmentExpr;

typedef struct {
    char *type_name;
    ASTNode **arguments;
    int arg_count;
} ConstructorExpr;

struct ASTNode {
    ASTNodeType type;
    int variable_declarations;
    int func_declarations;

    union {
        struct { ASTNode **declarations; int decl_count; } program;
        VariableDecl var_decl;
        FunctionDecl func_decl;
        StructDecl struct_decl;
        BlockStmt block_stmt;
        struct { ASTNode *expression; } expr_stmt;
        IfStmt if_stmt;
        ForStmt for_stmt;
        WhileStmt while_stmt;
        ReturnStmt return_stmt;
        BinaryExpr binary_expr;
        UnaryExpr unary_expr;
        CallExpr call_expr;
        MemberExpr member_expr;
        ArrayExpr array_expr;
        AssignmentExpr assign_expr;
        ConstructorExpr constructor_expr;
        struct { char *name; } identifier;
        struct { char *value; } literal;
    } data;
};

#include <stdio.h>
inline static void crt_warn(const char* msg) {
    printf("[Warn] %s", msg);
}

inline static void crt_err(const char* msg) {
    printf("[Err ] %s", msg);
}

//Code Gen

int gen_init(int flags);