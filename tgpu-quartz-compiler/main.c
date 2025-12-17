/*
 * Options:
 *   -t, --tokens    Print tokens
 *   -a, --ast       Print AST
 *   -o <file>       Output to file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "crt.h"

// ============================================================================
// LEXER
// ============================================================================

typedef struct {
    char *code;
    int pos;
    int line;
    int col;
    int length;
} Lexer;

const char *keywords[] = {
    "if", "else", "for", "while", "do", "return", "break", "continue",
    "const", "struct",
    "uniform", "varying", "attribute",
    "in", "out", "inout",
    "precision", "mediump", "highp", "lowp",
    NULL
};

const char *types[] = {
    "void", "int", "float", "double", "bool", "char",
    "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4",
    "bvec2", "bvec3", "bvec4",
    "mat2", "mat3", "mat4",
    "sampler2D", "sampler3D", "samplerCube",
    NULL
};

bool is_keyword(const char *str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) return true;
    }
    return false;
}

bool is_type(const char *str) {
    for (int i = 0; types[i] != NULL; i++) {
        if (strcmp(str, types[i]) == 0) return true;
    }
    return false;
}

Lexer *lexer_create(const char *code) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->code = strdup(code);
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
    lexer->length = strlen(code);
    return lexer;
}

void lexer_free(Lexer *lexer) {
    free(lexer->code);
    free(lexer);
}

char lexer_current(Lexer *lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->code[lexer->pos];
}

char lexer_peek(Lexer *lexer, int n) {
    if (lexer->pos + n >= lexer->length) return '\0';
    return lexer->code[lexer->pos + n];
}

void lexer_advance(Lexer *lexer) {
    if (lexer_current(lexer) == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    lexer->pos++;
}

void lexer_skip_whitespace(Lexer *lexer) {
    while (isspace(lexer_current(lexer))) {
        lexer_advance(lexer);
    }
}

Token *token_create(TokenType type, const char *value, int line, int col) {
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = strdup(value);
    token->line = line;
    token->col = col;
    return token;
}

void token_free(Token *token) {
    free(token->value);
    free(token);
}

Token *lexer_read_comment(Lexer *lexer) {
    int line = lexer->line;
    int col = lexer->col;
    char buffer[4096] = {0};
    int idx = 0;
    
    if (lexer_current(lexer) == '/' && lexer_peek(lexer, 1) == '/') {
        buffer[idx++] = '/';
        buffer[idx++] = '/';
        lexer_advance(lexer);
        lexer_advance(lexer);
        
        while (lexer_current(lexer) != '\n' && lexer_current(lexer) != '\0') {
            buffer[idx++] = lexer_current(lexer);
            lexer_advance(lexer);
        }
        return token_create(TOK_COMMENT, buffer, line, col);
    }
    
    if (lexer_current(lexer) == '/' && lexer_peek(lexer, 1) == '*') {
        buffer[idx++] = '/';
        buffer[idx++] = '*';
        lexer_advance(lexer);
        lexer_advance(lexer);
        
        while (!(lexer_current(lexer) == '*' && lexer_peek(lexer, 1) == '/') && 
               lexer_current(lexer) != '\0') {
            buffer[idx++] = lexer_current(lexer);
            lexer_advance(lexer);
        }
        
        if (lexer_current(lexer) == '*') {
            buffer[idx++] = '*';
            buffer[idx++] = '/';
            lexer_advance(lexer);
            lexer_advance(lexer);
        }
        return token_create(TOK_COMMENT, buffer, line, col);
    }
    
    return NULL;
}

Token *lexer_read_number(Lexer *lexer) {
    int line = lexer->line;
    int col = lexer->col;
    char buffer[256] = {0};
    int idx = 0;
    
    while (isdigit(lexer_current(lexer)) || lexer_current(lexer) == '.') {
        buffer[idx++] = lexer_current(lexer);
        lexer_advance(lexer);
    }
    
    if (lexer_current(lexer) == 'f' || lexer_current(lexer) == 'F') {
        buffer[idx++] = lexer_current(lexer);
        lexer_advance(lexer);
    }
    
    return token_create(TOK_NUMBER, buffer, line, col);
}

Token *lexer_read_identifier(Lexer *lexer) {
    int line = lexer->line;
    int col = lexer->col;
    char buffer[256] = {0};
    int idx = 0;
    
    while (isalnum(lexer_current(lexer)) || lexer_current(lexer) == '_') {
        buffer[idx++] = lexer_current(lexer);
        lexer_advance(lexer);
    }
    
    TokenType type = TOK_IDENTIFIER;
    if (is_keyword(buffer)) {
        type = TOK_KEYWORD;
    } else if (is_type(buffer)) {
        type = TOK_TYPE;
    }
    // Note: User-defined types (struct names) will remain as IDENTIFIER
    // and will be handled by the parser context
    
    return token_create(type, buffer, line, col);
}

Token *lexer_read_string(Lexer *lexer) {
    int line = lexer->line;
    int col = lexer->col;
    char buffer[4096] = {0};
    int idx = 0;
    char quote = lexer_current(lexer);
    
    buffer[idx++] = quote;
    lexer_advance(lexer);
    
    while (lexer_current(lexer) != quote && lexer_current(lexer) != '\0') {
        if (lexer_current(lexer) == '\\') {
            buffer[idx++] = lexer_current(lexer);
            lexer_advance(lexer);
        }
        buffer[idx++] = lexer_current(lexer);
        lexer_advance(lexer);
    }
    
    if (lexer_current(lexer) == quote) {
        buffer[idx++] = quote;
        lexer_advance(lexer);
    }
    
    return token_create(TOK_STRING, buffer, line, col);
}

Token **lexer_tokenize(Lexer *lexer, int *token_count) {
    Token **tokens = malloc(sizeof(Token*) * 10000);
    int count = 0;
    
    while (lexer_current(lexer) != '\0') {
        lexer_skip_whitespace(lexer);
        
        if (lexer_current(lexer) == '\0') break;
        
        char ch = lexer_current(lexer);
        int line = lexer->line;
        int col = lexer->col;
        
        // Comments
        if (ch == '/' && (lexer_peek(lexer, 1) == '/' || lexer_peek(lexer, 1) == '*')) {
            Token *comment = lexer_read_comment(lexer);
            if (comment) tokens[count++] = comment;
            continue;
        }
        
        // Numbers
        if (isdigit(ch)) {
            tokens[count++] = lexer_read_number(lexer);
            continue;
        }
        
        // Identifiers
        if (isalpha(ch) || ch == '_') {
            tokens[count++] = lexer_read_identifier(lexer);
            continue;
        }
        
        // Strings
        if (ch == '"' || ch == '\'') {
            tokens[count++] = lexer_read_string(lexer);
            continue;
        }
        
        // Single character tokens
        if (ch == '(') {
            tokens[count++] = token_create(TOK_LPAREN, "(", line, col);
            lexer_advance(lexer);
        } else if (ch == ')') {
            tokens[count++] = token_create(TOK_RPAREN, ")", line, col);
            lexer_advance(lexer);
        } else if (ch == '{') {
            tokens[count++] = token_create(TOK_LBRACE, "{", line, col);
            lexer_advance(lexer);
        } else if (ch == '}') {
            tokens[count++] = token_create(TOK_RBRACE, "}", line, col);
            lexer_advance(lexer);
        } else if (ch == '[') {
            tokens[count++] = token_create(TOK_LBRACKET, "[", line, col);
            lexer_advance(lexer);
        } else if (ch == ']') {
            tokens[count++] = token_create(TOK_RBRACKET, "]", line, col);
            lexer_advance(lexer);
        } else if (ch == ';') {
            tokens[count++] = token_create(TOK_SEMICOLON, ";", line, col);
            lexer_advance(lexer);
        } else if (ch == ',') {
            tokens[count++] = token_create(TOK_COMMA, ",", line, col);
            lexer_advance(lexer);
        } else if (ch == '.') {
            tokens[count++] = token_create(TOK_DOT, ".", line, col);
            lexer_advance(lexer);
        } else {
            // Operators
            char buffer[3] = {ch, '\0', '\0'};
            lexer_advance(lexer);
            
            // Check for two-character operators
            char next = lexer_current(lexer);
            if ((ch == '=' && next == '=') || (ch == '!' && next == '=') ||
                (ch == '<' && next == '=') || (ch == '>' && next == '=') ||
                (ch == '&' && next == '&') || (ch == '|' && next == '|') ||
                (ch == '+' && next == '=') || (ch == '-' && next == '=') ||
                (ch == '*' && next == '=') || (ch == '/' && next == '=') ||
                (ch == '+' && next == '+') || (ch == '-' && next == '-') ||
                (ch == '<' && next == '<') || (ch == '>' && next == '>')) {
                buffer[1] = next;
                lexer_advance(lexer);
            }
            
            tokens[count++] = token_create(TOK_OPERATOR, buffer, line, col);
        }
    }
    
    tokens[count++] = token_create(TOK_EOF, "", lexer->line, lexer->col);
    *token_count = count;
    return tokens;
}

// ============================================================================
// PARSER
// ============================================================================

typedef struct {
    Token **tokens;
    int pos;
    int count;
} Parser;

Parser *parser_create(Token **tokens, int count) {
    Parser *parser = malloc(sizeof(Parser));
    
    // Filter out comments
    Token **filtered = malloc(sizeof(Token*) * count);
    int filtered_count = 0;
    
    for (int i = 0; i < count; i++) {
        if (tokens[i]->type != TOK_COMMENT) {
            filtered[filtered_count++] = tokens[i];
        }
    }
    
    parser->tokens = filtered;
    parser->pos = 0;
    parser->count = filtered_count;
    return parser;
}

void parser_free(Parser *parser) {
    free(parser->tokens);
    free(parser);
}

Token *parser_current(Parser *parser) {
    return parser->tokens[parser->pos];
}

void parser_advance(Parser *parser) {
    if (parser->pos < parser->count - 1) {
        parser->pos++;
    }
}

bool parser_match(Parser *parser, TokenType type) {
    return parser_current(parser)->type == type;
}

Token *parser_expect(Parser *parser, TokenType type) {
    Token *token = parser_current(parser);
    if (token->type != type) {
        fprintf(stderr, "Parse error at line %d:%d: expected token type %d, got %d\n",
                token->line, token->col, type, token->type);
        exit(1);
    }
    parser_advance(parser);
    return token;
}

// Forward declarations
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_statement(Parser *parser);
const char *token_type_to_string(TokenType type);

ASTNode *parse_primary(Parser *parser) {
    Token *token = parser_current(parser);
    
    if (token->type == TOK_NUMBER) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_LITERAL;
        node->data.literal.value = strdup(token->value);
        parser_advance(parser);
        return node;
    }
    
    if (token->type == TOK_IDENTIFIER) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_IDENTIFIER;
        node->data.identifier.name = strdup(token->value);
        parser_advance(parser);
        return node;
    }
    
    if (token->type == TOK_TYPE) {
        char *type_name = strdup(token->value);
        parser_advance(parser);
        parser_expect(parser, TOK_LPAREN);
        
        ASTNode **args = malloc(sizeof(ASTNode*) * 100);
        int arg_count = 0;
        
        while (!parser_match(parser, TOK_RPAREN)) {
            if (arg_count > 0) {
                parser_expect(parser, TOK_COMMA);
            }
            args[arg_count++] = parse_expression(parser);
        }
        
        parser_expect(parser, TOK_RPAREN);
        
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_CONSTRUCTOR_EXPR;
        node->data.constructor_expr.type_name = type_name;
        node->data.constructor_expr.arguments = args;
        node->data.constructor_expr.arg_count = arg_count;
        return node;
    }
    
    if (token->type == TOK_LPAREN) {
        parser_advance(parser);
        ASTNode *expr = parse_expression(parser);
        parser_expect(parser, TOK_RPAREN);
        return expr;
    }
    
    fprintf(stderr, "Parse error: unexpected token at line %d:%d\n", token->line, token->col);
    exit(1);
}

ASTNode *parse_postfix(Parser *parser) {
    ASTNode *expr = parse_primary(parser);
    
    while (true) {
        if (parser_match(parser, TOK_LPAREN)) {
            parser_advance(parser);
            
            ASTNode **args = malloc(sizeof(ASTNode*) * 100);
            int arg_count = 0;
            
            while (!parser_match(parser, TOK_RPAREN)) {
                if (arg_count > 0) {
                    parser_expect(parser, TOK_COMMA);
                }
                args[arg_count++] = parse_expression(parser);
            }
            
            parser_expect(parser, TOK_RPAREN);
            
            ASTNode *call_node = malloc(sizeof(ASTNode));
            call_node->type = AST_CALL_EXPR;
            call_node->data.call_expr.callee = expr;
            call_node->data.call_expr.arguments = args;
            call_node->data.call_expr.arg_count = arg_count;
            expr = call_node;
        } else if (parser_match(parser, TOK_DOT)) {
            parser_advance(parser);
            Token *prop = parser_expect(parser, TOK_IDENTIFIER);
            
            ASTNode *member_node = malloc(sizeof(ASTNode));
            member_node->type = AST_MEMBER_EXPR;
            member_node->data.member_expr.object = expr;
            member_node->data.member_expr.property = strdup(prop->value);
            expr = member_node;
        } else if (parser_match(parser, TOK_LBRACKET)) {
            parser_advance(parser);
            ASTNode *index = parse_expression(parser);
            parser_expect(parser, TOK_RBRACKET);
            
            ASTNode *array_node = malloc(sizeof(ASTNode));
            array_node->type = AST_ARRAY_EXPR;
            array_node->data.array_expr.array = expr;
            array_node->data.array_expr.index = index;
            expr = array_node;
        } else {
            break;
        }
    }
    
    return expr;
}

ASTNode *parse_unary(Parser *parser) {
    Token *token = parser_current(parser);
    
    if (token->type == TOK_OPERATOR) {
        const char *op = token->value;
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
            strcmp(op, "!") == 0 || strcmp(op, "++") == 0 || strcmp(op, "--") == 0) {
            parser_advance(parser);
            ASTNode *arg = parse_unary(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_UNARY_EXPR;
            node->data.unary_expr.operator = strdup(op);
            node->data.unary_expr.argument = arg;
            return node;
        }
    }
    
    return parse_postfix(parser);
}

ASTNode *parse_multiplicative(Parser *parser) {
    ASTNode *left = parse_unary(parser);
    
    while (parser_match(parser, TOK_OPERATOR)) {
        const char *op = parser_current(parser)->value;
        if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) {
            parser_advance(parser);
            ASTNode *right = parse_unary(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_BINARY_EXPR;
            node->data.binary_expr.operator = strdup(op);
            node->data.binary_expr.left = left;
            node->data.binary_expr.right = right;
            left = node;
        } else {
            break;
        }
    }
    
    return left;
}

ASTNode *parse_additive(Parser *parser) {
    ASTNode *left = parse_multiplicative(parser);
    
    while (parser_match(parser, TOK_OPERATOR)) {
        const char *op = parser_current(parser)->value;
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) {
            parser_advance(parser);
            ASTNode *right = parse_multiplicative(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_BINARY_EXPR;
            node->data.binary_expr.operator = strdup(op);
            node->data.binary_expr.left = left;
            node->data.binary_expr.right = right;
            left = node;
        } else {
            break;
        }
    }
    
    return left;
}

ASTNode *parse_comparison(Parser *parser) {
    ASTNode *left = parse_additive(parser);
    
    while (parser_match(parser, TOK_OPERATOR)) {
        const char *op = parser_current(parser)->value;
        if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
            strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
            strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
            parser_advance(parser);
            ASTNode *right = parse_additive(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_BINARY_EXPR;
            node->data.binary_expr.operator = strdup(op);
            node->data.binary_expr.left = left;
            node->data.binary_expr.right = right;
            left = node;
        } else {
            break;
        }
    }
    
    return left;
}

ASTNode *parse_logical_and(Parser *parser) {
    ASTNode *left = parse_comparison(parser);
    
    while (parser_match(parser, TOK_OPERATOR)) {
        const char *op = parser_current(parser)->value;
        if (strcmp(op, "&&") == 0) {
            parser_advance(parser);
            ASTNode *right = parse_comparison(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_BINARY_EXPR;
            node->data.binary_expr.operator = strdup(op);
            node->data.binary_expr.left = left;
            node->data.binary_expr.right = right;
            left = node;
        } else {
            break;
        }
    }
    
    return left;
}

ASTNode *parse_logical_or(Parser *parser) {
    ASTNode *left = parse_logical_and(parser);
    
    while (parser_match(parser, TOK_OPERATOR)) {
        const char *op = parser_current(parser)->value;
        if (strcmp(op, "||") == 0) {
            parser_advance(parser);
            ASTNode *right = parse_logical_and(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_BINARY_EXPR;
            node->data.binary_expr.operator = strdup(op);
            node->data.binary_expr.left = left;
            node->data.binary_expr.right = right;
            left = node;
        } else {
            break;
        }
    }
    
    return left;
}

ASTNode *parse_assignment(Parser *parser) {
    ASTNode *left = parse_logical_or(parser);
    
    if (parser_match(parser, TOK_OPERATOR)) {
        const char *op = parser_current(parser)->value;
        if (strcmp(op, "=") == 0 || strcmp(op, "+=") == 0 || 
            strcmp(op, "-=") == 0 || strcmp(op, "*=") == 0 || strcmp(op, "/=") == 0) {
            parser_advance(parser);
            ASTNode *right = parse_assignment(parser);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_ASSIGNMENT_EXPR;
            node->data.assign_expr.operator = strdup(op);
            node->data.assign_expr.left = left;
            node->data.assign_expr.right = right;
            return node;
        }
    }
    
    return left;
}

ASTNode *parse_expression(Parser *parser) {
    return parse_assignment(parser);
}

ASTNode *parse_block(Parser *parser) {
    parser_expect(parser, TOK_LBRACE);
    
    ASTNode **statements = malloc(sizeof(ASTNode*) * 1000);
    int stmt_count = 0;
    
    while (!parser_match(parser, TOK_RBRACE) && !parser_match(parser, TOK_EOF)) {
        statements[stmt_count++] = parse_statement(parser);
    }
    
    parser_expect(parser, TOK_RBRACE);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK_STMT;
    node->data.block_stmt.statements = statements;
    node->data.block_stmt.statement_count = stmt_count;
    return node;
}

ASTNode *parse_if_statement(Parser *parser) {
    parser_expect(parser, TOK_KEYWORD); // 'if'
    parser_expect(parser, TOK_LPAREN);
    ASTNode *condition = parse_expression(parser);
    parser_expect(parser, TOK_RPAREN);
    
    ASTNode *consequent = parse_statement(parser);
    ASTNode *alternate = NULL;
    
    if (parser_match(parser, TOK_KEYWORD) && 
        strcmp(parser_current(parser)->value, "else") == 0) {
        parser_advance(parser);
        alternate = parse_statement(parser);
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF_STMT;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.consequent = consequent;
    node->data.if_stmt.alternate = alternate;
    return node;
}

ASTNode *parse_for_statement(Parser *parser) {
    parser_expect(parser, TOK_KEYWORD); // 'for'
    parser_expect(parser, TOK_LPAREN);
    
    ASTNode *init = NULL;
    
    // Check if init is a variable declaration
    if (parser_match(parser, TOK_TYPE) || 
        (parser_match(parser, TOK_IDENTIFIER) && parser->pos + 1 < parser->count && 
         parser->tokens[parser->pos + 1]->type == TOK_IDENTIFIER)) {
        
        Token *type_token = parser_current(parser);
        parser_advance(parser);
        
        Token *name_token = parser_expect(parser, TOK_IDENTIFIER);
        
        ASTNode *initializer = NULL;
        bool is_array = false;
        char *array_size = NULL;
        
        // Check for array
        if (parser_match(parser, TOK_LBRACKET)) {
            parser_advance(parser);
            is_array = true;
            
            if (parser_match(parser, TOK_NUMBER)) {
                array_size = strdup(parser_current(parser)->value);
                parser_advance(parser);
            } else if (parser_match(parser, TOK_IDENTIFIER)) {
                array_size = strdup(parser_current(parser)->value);
                parser_advance(parser);
            }
            
            parser_expect(parser, TOK_RBRACKET);
        }
        
        if (parser_match(parser, TOK_OPERATOR) && 
            strcmp(parser_current(parser)->value, "=") == 0) {
            parser_advance(parser);
            initializer = parse_expression(parser);
        }
        
        init = malloc(sizeof(ASTNode));
        init->type = AST_VARIABLE_DECL;
        init->data.var_decl.qualifiers = NULL;
        init->data.var_decl.qualifier_count = 0;
        init->data.var_decl.type = strdup(type_token->value);
        init->data.var_decl.name = strdup(name_token->value);
        init->data.var_decl.initializer = initializer;
        init->data.var_decl.is_array = is_array;
        init->data.var_decl.array_size = array_size;
    } else {
        init = parse_expression(parser);
    }
    
    parser_expect(parser, TOK_SEMICOLON);
    
    ASTNode *test = parse_expression(parser);
    parser_expect(parser, TOK_SEMICOLON);
    
    ASTNode *update = parse_expression(parser);
    parser_expect(parser, TOK_RPAREN);
    
    ASTNode *body = parse_statement(parser);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FOR_STMT;
    node->data.for_stmt.init = init;
    node->data.for_stmt.test = test;
    node->data.for_stmt.update = update;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode *parse_while_statement(Parser *parser) {
    parser_expect(parser, TOK_KEYWORD); // 'while'
    parser_expect(parser, TOK_LPAREN);
    ASTNode *test = parse_expression(parser);
    parser_expect(parser, TOK_RPAREN);
    
    ASTNode *body = parse_statement(parser);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_STMT;
    node->data.while_stmt.test = test;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode *parse_return_statement(Parser *parser) {
    parser_expect(parser, TOK_KEYWORD); // 'return'
    
    ASTNode *argument = NULL;
    if (!parser_match(parser, TOK_SEMICOLON)) {
        argument = parse_expression(parser);
    }
    
    parser_expect(parser, TOK_SEMICOLON);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN_STMT;
    node->data.return_stmt.argument = argument;
    return node;
}

ASTNode *parse_statement(Parser *parser) {
    if (parser_match(parser, TOK_KEYWORD)) {
        const char *keyword = parser_current(parser)->value;
        if (strcmp(keyword, "if") == 0) {
            return parse_if_statement(parser);
        } else if (strcmp(keyword, "for") == 0) {
            return parse_for_statement(parser);
        } else if (strcmp(keyword, "while") == 0) {
            return parse_while_statement(parser);
        } else if (strcmp(keyword, "return") == 0) {
            return parse_return_statement(parser);
        } else if (strcmp(keyword, "const") == 0) {
            // Handle const variable declaration inside function
            parser_advance(parser); // skip 'const'
            
            Token *type_token = parser_expect(parser, TOK_TYPE);
            Token *name_token = parser_expect(parser, TOK_IDENTIFIER);
            
            ASTNode *initializer = NULL;
            if (parser_match(parser, TOK_OPERATOR) && 
                strcmp(parser_current(parser)->value, "=") == 0) {
                parser_advance(parser);
                initializer = parse_expression(parser);
            }
            
            parser_expect(parser, TOK_SEMICOLON);
            
            char **qualifiers = malloc(sizeof(char*) * 1);
            qualifiers[0] = strdup("const");
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_VARIABLE_DECL;
            node->data.var_decl.qualifiers = qualifiers;
            node->data.var_decl.qualifier_count = 1;
            node->data.var_decl.type = strdup(type_token->value);
            node->data.var_decl.name = strdup(name_token->value);
            node->data.var_decl.initializer = initializer;
            return node;
        }
    }
    
    if (parser_match(parser, TOK_LBRACE)) {
        return parse_block(parser);
    }
    
    // Check for local variable declaration (type followed by identifier)
    if (parser_match(parser, TOK_TYPE) || 
        (parser_match(parser, TOK_IDENTIFIER) && parser->pos + 1 < parser->count && 
         parser->tokens[parser->pos + 1]->type == TOK_IDENTIFIER)) {
        
        Token *type_token = parser_current(parser);
        parser_advance(parser);
        
        if (parser_match(parser, TOK_IDENTIFIER)) {
            Token *name_token = parser_current(parser);
            parser_advance(parser);
            
            ASTNode *initializer = NULL;
            bool is_array = false;
            char *array_size = NULL;
            
            // Check for array declaration
            if (parser_match(parser, TOK_LBRACKET)) {
                parser_advance(parser);
                is_array = true;
                
                if (parser_match(parser, TOK_NUMBER)) {
                    array_size = strdup(parser_current(parser)->value);
                    parser_advance(parser);
                } else if (parser_match(parser, TOK_IDENTIFIER)) {
                    array_size = strdup(parser_current(parser)->value);
                    parser_advance(parser);
                }
                
                parser_expect(parser, TOK_RBRACKET);
            }
            
            // Check for initialization
            if (parser_match(parser, TOK_OPERATOR) && 
                strcmp(parser_current(parser)->value, "=") == 0) {
                parser_advance(parser);
                initializer = parse_expression(parser);
            }
            
            parser_expect(parser, TOK_SEMICOLON);
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_VARIABLE_DECL;
            node->data.var_decl.qualifiers = NULL;
            node->data.var_decl.qualifier_count = 0;
            node->data.var_decl.type = strdup(type_token->value);
            node->data.var_decl.name = strdup(name_token->value);
            node->data.var_decl.initializer = initializer;
            node->data.var_decl.is_array = is_array;
            node->data.var_decl.array_size = array_size;
            return node;
        } else {
            // Not a variable declaration, backtrack
            // This is a problem - we can't easily backtrack in this parser
            // For now, treat as error
            fprintf(stderr, "Parse error: expected identifier after type\n");
            exit(1);
        }
    }
    
    // Expression statement
    ASTNode *expr = parse_expression(parser);
    parser_expect(parser, TOK_SEMICOLON);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_EXPRESSION_STMT;
    node->data.expr_stmt.expression = expr;
    return node;
}

ASTNode *parse_function(Parser *parser, char **qualifiers, int qual_count, 
                        const char *return_type, const char *name) {
    parser_expect(parser, TOK_LPAREN);
    
    Parameter *params = malloc(sizeof(Parameter) * 100);
    int param_count = 0;
    
    while (!parser_match(parser, TOK_RPAREN)) {
        if (param_count > 0) {
            parser_expect(parser, TOK_COMMA);
        }
        
        // Accept both TYPE and IDENTIFIER tokens as types (for user-defined types)
        Token *param_type = parser_current(parser);
        if (!parser_match(parser, TOK_TYPE) && !parser_match(parser, TOK_IDENTIFIER)) {
            fprintf(stderr, "Error: expected type in parameter list\n");
            exit(1);
        }
        parser_advance(parser);
        
        Token *param_name = parser_expect(parser, TOK_IDENTIFIER);
        
        params[param_count].type = strdup(param_type->value);
        params[param_count].name = strdup(param_name->value);
        param_count++;
    }
    
    parser_expect(parser, TOK_RPAREN);
    
    ASTNode *body = parse_block(parser);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_DECL;
    node->data.func_decl.qualifiers = qualifiers;
    node->data.func_decl.qualifier_count = qual_count;
    node->data.func_decl.return_type = strdup(return_type);
    node->data.func_decl.name = strdup(name);
    node->data.func_decl.params = params;
    node->data.func_decl.param_count = param_count;
    node->data.func_decl.body = body;
    return node;
}

ASTNode *parse_variable(Parser *parser, char **qualifiers, int qual_count,
                        const char *var_type, const char *name) {
    ASTNode *initializer = NULL;
    bool is_array = false;
    char *array_size = NULL;
    
    // Check for array declaration
    if (parser_match(parser, TOK_LBRACKET)) {
        parser_advance(parser);
        is_array = true;
        
        // Get array size (could be number or expression)
        if (parser_match(parser, TOK_NUMBER)) {
            array_size = strdup(parser_current(parser)->value);
            parser_advance(parser);
        } else if (parser_match(parser, TOK_IDENTIFIER)) {
            array_size = strdup(parser_current(parser)->value);
            parser_advance(parser);
        }
        
        parser_expect(parser, TOK_RBRACKET);
    }
    
    if (parser_match(parser, TOK_OPERATOR) && 
        strcmp(parser_current(parser)->value, "=") == 0) {
        parser_advance(parser);
        initializer = parse_expression(parser);
    }
    
    parser_expect(parser, TOK_SEMICOLON);
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE_DECL;
    node->data.var_decl.qualifiers = qualifiers;
    node->data.var_decl.qualifier_count = qual_count;
    node->data.var_decl.type = strdup(var_type);
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.initializer = initializer;
    node->data.var_decl.is_array = is_array;
    node->data.var_decl.array_size = array_size;
    return node;
}

ASTNode *parse_declaration(Parser *parser) {
    // Handle struct declarations
    if (parser_match(parser, TOK_KEYWORD) &&
        strcmp(parser_current(parser)->value, "struct") == 0) {
        parser_advance(parser); // skip 'struct'
        
        Token *name_token = parser_expect(parser, TOK_IDENTIFIER);
        parser_expect(parser, TOK_LBRACE);
        
        Parameter *fields = malloc(sizeof(Parameter) * 100);
        int field_count = 0;
        
        while (!parser_match(parser, TOK_RBRACE)) {
            // Accept both TYPE and IDENTIFIER (for user-defined types)
            Token *field_type = parser_current(parser);
            if (!parser_match(parser, TOK_TYPE) && !parser_match(parser, TOK_IDENTIFIER)) {
                fprintf(stderr, "Error: expected type in struct field\n");
                exit(1);
            }
            parser_advance(parser);
            
            Token *field_name = parser_expect(parser, TOK_IDENTIFIER);
            parser_expect(parser, TOK_SEMICOLON);
            
            fields[field_count].type = strdup(field_type->value);
            fields[field_count].name = strdup(field_name->value);
            field_count++;
        }
        
        parser_expect(parser, TOK_RBRACE);
        parser_expect(parser, TOK_SEMICOLON);
        
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_STRUCT_DECL;
        node->data.struct_decl.name = strdup(name_token->value);
        node->data.struct_decl.fields = fields;
        node->data.struct_decl.field_count = field_count;
        return node;
    }
    
    // Handle precision statements (GLSL specific)
    if (parser_match(parser, TOK_KEYWORD) && 
        strcmp(parser_current(parser)->value, "precision") == 0) {
        parser_advance(parser); // skip 'precision'
        
        if (parser_match(parser, TOK_KEYWORD)) {
            parser_advance(parser); // skip precision level (mediump, highp, etc)
        }
        
        if (parser_match(parser, TOK_TYPE)) {
            parser_advance(parser); // skip type (float, int, etc)
        }
        
        parser_expect(parser, TOK_SEMICOLON);
        
        // Return empty variable declaration for precision statements
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_VARIABLE_DECL;
        node->data.var_decl.qualifiers = NULL;
        node->data.var_decl.qualifier_count = 0;
        node->data.var_decl.type = strdup("precision");
        node->data.var_decl.name = strdup("statement");
        node->data.var_decl.initializer = NULL;
        node->data.var_decl.is_array = false;
        node->data.var_decl.array_size = NULL;
        return node;
    }
    
    // Handle const declarations
    if (parser_match(parser, TOK_KEYWORD) &&
        strcmp(parser_current(parser)->value, "const") == 0) {
        parser_advance(parser); // skip 'const'
        
        Token *type_token = parser_expect(parser, TOK_TYPE);
        Token *name_token = parser_expect(parser, TOK_IDENTIFIER);
        
        bool is_array = false;
        char *array_size = NULL;
        
        // Check for array
        if (parser_match(parser, TOK_LBRACKET)) {
            parser_advance(parser);
            is_array = true;
            
            if (parser_match(parser, TOK_NUMBER)) {
                array_size = strdup(parser_current(parser)->value);
                parser_advance(parser);
            } else if (parser_match(parser, TOK_IDENTIFIER)) {
                array_size = strdup(parser_current(parser)->value);
                parser_advance(parser);
            }
            
            parser_expect(parser, TOK_RBRACKET);
        }
        
        parser_expect(parser, TOK_OPERATOR); // '='
        ASTNode *initializer = parse_expression(parser);
        parser_expect(parser, TOK_SEMICOLON);
        
        char **qualifiers = malloc(sizeof(char*) * 1);
        qualifiers[0] = strdup("const");
        
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_VARIABLE_DECL;
        node->data.var_decl.qualifiers = qualifiers;
        node->data.var_decl.qualifier_count = 1;
        node->data.var_decl.type = strdup(type_token->value);
        node->data.var_decl.name = strdup(name_token->value);
        node->data.var_decl.initializer = initializer;
        node->data.var_decl.is_array = is_array;
        node->data.var_decl.array_size = array_size;
        return node;
    }
    
    char **qualifiers = malloc(sizeof(char*) * 10);
    int qual_count = 0;
    
    // Parse qualifiers
    while (parser_match(parser, TOK_KEYWORD)) {
        const char *kw = parser_current(parser)->value;
        if (strcmp(kw, "uniform") == 0 || strcmp(kw, "varying") == 0 ||
            strcmp(kw, "attribute") == 0 ||
            strcmp(kw, "in") == 0 || strcmp(kw, "out") == 0 || strcmp(kw, "inout") == 0) {
            qualifiers[qual_count++] = strdup(kw);
            parser_advance(parser);
        } else {
            break;
        }
    }
    
    // Check for type (including user-defined types like struct names)
    if (!parser_match(parser, TOK_TYPE) && !parser_match(parser, TOK_IDENTIFIER)) {
        Token *tok = parser_current(parser);
        fprintf(stderr, "Error at line %d:%d: Expected type, got '%s' (type=%s)\n",
                tok->line, tok->col, tok->value, token_type_to_string(tok->type));
        exit(1);
    }
    
    // Parse type (could be built-in type or user-defined type)
    Token *type_token = parser_current(parser);
    parser_advance(parser);
    const char *var_type = type_token->value;
    
    // Parse identifier
    Token *name_token = parser_expect(parser, TOK_IDENTIFIER);
    const char *name = name_token->value;
    
    // Check if function or variable
    if (parser_match(parser, TOK_LPAREN)) {
        return parse_function(parser, qualifiers, qual_count, var_type, name);
    } else {
        return parse_variable(parser, qualifiers, qual_count, var_type, name);
    }
}

ASTNode *parse_program(Parser *parser) {
    ASTNode **declarations = malloc(sizeof(ASTNode*) * 1000);
    int decl_count = 0;
    
    while (!parser_match(parser, TOK_EOF)) {
        declarations[decl_count++] = parse_declaration(parser);
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->data.program.declarations = declarations;
    node->data.program.decl_count = decl_count;
    return node;
}

// ============================================================================
// PRINTING FUNCTIONS
// ============================================================================

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOK_KEYWORD: return "KEYWORD";
        case TOK_TYPE: return "TYPE";
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_NUMBER: return "NUMBER";
        case TOK_STRING: return "STRING";
        case TOK_OPERATOR: return "OPERATOR";
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_LBRACKET: return "LBRACKET";
        case TOK_RBRACKET: return "RBRACKET";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_COMMA: return "COMMA";
        case TOK_DOT: return "DOT";
        case TOK_COMMENT: return "COMMENT";
        case TOK_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}

void print_tokens(Token **tokens, int count, FILE *output) {
    fprintf(output, "=== TOKENS ===\n");
    for (int i = 0; i < count; i++) {
        if (tokens[i]->type == TOK_EOF) break;
        if (tokens[i]->type == TOK_COMMENT) continue;
        fprintf(output, "%3d:%-3d %-15s %s\n", 
                tokens[i]->line, tokens[i]->col,
                token_type_to_string(tokens[i]->type),
                tokens[i]->value);
    }
    fprintf(output, "\n");
}

void print_ast_node(ASTNode *node, int indent, FILE *output);

void print_indent(int indent, FILE *output) {
    for (int i = 0; i < indent; i++) {
        fprintf(output, "  ");
    }
}

void print_ast_node(ASTNode *node, int indent, FILE *output) {
    if (!node) {
        print_indent(indent, output);
        fprintf(output, "NULL\n");
        return;
    }
    
    print_indent(indent, output);
    
    switch (node->type) {
        case AST_PROGRAM:
            fprintf(output, "Program:\n");
            for (int i = 0; i < node->data.program.decl_count; i++) {
                print_ast_node(node->data.program.declarations[i], indent + 1, output);
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
            print_ast_node(node->data.func_decl.body, indent + 1, output);
            break;
            
        case AST_STRUCT_DECL:
            fprintf(output, "StructDeclaration: %s\n", node->data.struct_decl.name);
            for (int i = 0; i < node->data.struct_decl.field_count; i++) {
                print_indent(indent + 1, output);
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
                print_indent(indent + 1, output);
                fprintf(output, "Initializer:\n");
                print_ast_node(node->data.var_decl.initializer, indent + 2, output);
            }
            break;
            
        case AST_BLOCK_STMT:
            fprintf(output, "BlockStatement:\n");
            for (int i = 0; i < node->data.block_stmt.statement_count; i++) {
                print_ast_node(node->data.block_stmt.statements[i], indent + 1, output);
            }
            break;
            
        case AST_EXPRESSION_STMT:
            fprintf(output, "ExpressionStatement:\n");
            print_ast_node(node->data.expr_stmt.expression, indent + 1, output);
            break;
            
        case AST_IF_STMT:
            fprintf(output, "IfStatement:\n");
            print_indent(indent + 1, output);
            fprintf(output, "Condition:\n");
            print_ast_node(node->data.if_stmt.condition, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Consequent:\n");
            print_ast_node(node->data.if_stmt.consequent, indent + 2, output);
            if (node->data.if_stmt.alternate) {
                print_indent(indent + 1, output);
                fprintf(output, "Alternate:\n");
                print_ast_node(node->data.if_stmt.alternate, indent + 2, output);
            }
            break;
            
        case AST_FOR_STMT:
            fprintf(output, "ForStatement:\n");
            print_indent(indent + 1, output);
            fprintf(output, "Init:\n");
            print_ast_node(node->data.for_stmt.init, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Test:\n");
            print_ast_node(node->data.for_stmt.test, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Update:\n");
            print_ast_node(node->data.for_stmt.update, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Body:\n");
            print_ast_node(node->data.for_stmt.body, indent + 2, output);
            break;
            
        case AST_WHILE_STMT:
            fprintf(output, "WhileStatement:\n");
            print_indent(indent + 1, output);
            fprintf(output, "Test:\n");
            print_ast_node(node->data.while_stmt.test, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Body:\n");
            print_ast_node(node->data.while_stmt.body, indent + 2, output);
            break;
            
        case AST_RETURN_STMT:
            fprintf(output, "ReturnStatement:\n");
            if (node->data.return_stmt.argument) {
                print_ast_node(node->data.return_stmt.argument, indent + 1, output);
            }
            break;
            
        case AST_BINARY_EXPR:
            fprintf(output, "BinaryExpression: %s\n", node->data.binary_expr.operator);
            print_indent(indent + 1, output);
            fprintf(output, "Left:\n");
            print_ast_node(node->data.binary_expr.left, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Right:\n");
            print_ast_node(node->data.binary_expr.right, indent + 2, output);
            break;
            
        case AST_UNARY_EXPR:
            fprintf(output, "UnaryExpression: %s\n", node->data.unary_expr.operator);
            print_ast_node(node->data.unary_expr.argument, indent + 1, output);
            break;
            
        case AST_CALL_EXPR:
            fprintf(output, "CallExpression:\n");
            print_indent(indent + 1, output);
            fprintf(output, "Callee:\n");
            print_ast_node(node->data.call_expr.callee, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Arguments:\n");
            for (int i = 0; i < node->data.call_expr.arg_count; i++) {
                print_ast_node(node->data.call_expr.arguments[i], indent + 2, output);
            }
            break;
            
        case AST_MEMBER_EXPR:
            fprintf(output, "MemberExpression: .%s\n", node->data.member_expr.property);
            print_indent(indent + 1, output);
            fprintf(output, "Object:\n");
            print_ast_node(node->data.member_expr.object, indent + 2, output);
            break;
            
        case AST_ARRAY_EXPR:
            fprintf(output, "ArrayExpression:\n");
            print_indent(indent + 1, output);
            fprintf(output, "Array:\n");
            print_ast_node(node->data.array_expr.array, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Index:\n");
            print_ast_node(node->data.array_expr.index, indent + 2, output);
            break;
            
        case AST_ASSIGNMENT_EXPR:
            fprintf(output, "AssignmentExpression: %s\n", node->data.assign_expr.operator);
            print_indent(indent + 1, output);
            fprintf(output, "Left:\n");
            print_ast_node(node->data.assign_expr.left, indent + 2, output);
            print_indent(indent + 1, output);
            fprintf(output, "Right:\n");
            print_ast_node(node->data.assign_expr.right, indent + 2, output);
            break;
            
        case AST_CONSTRUCTOR_EXPR:
            fprintf(output, "ConstructorExpression: %s\n", 
                    node->data.constructor_expr.type_name);
            for (int i = 0; i < node->data.constructor_expr.arg_count; i++) {
                print_ast_node(node->data.constructor_expr.arguments[i], indent + 1, output);
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

void print_ast(ASTNode *ast, FILE *output) {
    fprintf(output, "\n=== ABSTRACT SYNTAX TREE ===\n");
    print_ast_node(ast, 0, output);
}

// ============================================================================
// MAIN
// ============================================================================

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}

void print_usage(const char *program_name) {
    printf("C-like and GLSL Lexer + Parser\n");
    printf("Usage: %s <input_file> [options]\n", program_name);
    printf("\nOptions:\n");
    printf("  -t, --tokens       Print tokens\n");
    printf("  -a, --ast          Print AST\n");
    printf("  -o <file>          Output to file\n");
    printf("  -h, --help         Show this help message\n");
    printf("\nExample:\n");
    printf("  %s shader.glsl -t -a\n", program_name);
    printf("  %s program.c -a -o output.txt\n", program_name);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    bool show_tokens = false;
    bool show_ast = false;
    char *output_file = NULL;
    char *input_file = NULL;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokens") == 0) {
            show_tokens = true;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--ast") == 0) {
            show_ast = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires a filename\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }
    
    if (!input_file) {
        fprintf(stderr, "Error: no input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Default: show both if neither specified
    if (!show_tokens && !show_ast) {
        show_tokens = true;
        show_ast = true;
    }
    
    // Read input file
    char *code = read_file(input_file);
    if (!code) {
        return 1;
    }
    
    // Open output file or use stdout
    FILE *output = stdout;
    if (output_file) {
        output = fopen(output_file, "w");
        if (!output) {
            fprintf(stderr, "Error: could not open output file '%s'\n", output_file);
            free(code);
            return 1;
        }
    }
    
    // Lexer
    Lexer *lexer = lexer_create(code);
    int token_count;
    Token **tokens = lexer_tokenize(lexer, &token_count);
    
    if (show_tokens) {
        print_tokens(tokens, token_count, output);
    }
    
    // Parser
    Parser *parser = parser_create(tokens, token_count);
    ASTNode *ast = NULL;

    ast = parse_program(parser);

    gen_init(0);
    gen_by_ast(ast);
    
    // Try to parse, catch errors
    if (show_ast) {
        fprintf(output, "\nParsing AST...\n");
        fflush(output);
        
        // Debug: show first few tokens that will be parsed
        fprintf(output, "First tokens to parse:\n");
        for (int i = 0; i < parser->count && i < 10; i++) {
            fprintf(output, "  [%d] %s: '%s'\n", i, 
                    token_type_to_string(parser->tokens[i]->type),
                    parser->tokens[i]->value);
        }
        fprintf(output, "\n");
        fflush(output);
        
        
        print_ast(ast, output);
    }
    
    // Cleanup
    if (output != stdout) {
        fclose(output);
    }
    
    free(code);
    lexer_free(lexer);
    parser_free(parser);
    
    for (int i = 0; i < token_count; i++) {
        token_free(tokens[i]);
    }
    free(tokens);
    
    // Note: Should also free AST nodes recursively
    // (omitted for brevity, but should be implemented in production)
    
    return 0;
}