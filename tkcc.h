#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c

typedef enum {
  TK_RESERVED, // Keywords or punctuators
  TK_IDENT,     // Indentifiers
  TK_NUM,      // Integer Literals
  TK_EOF,      // End-of-file markers
} TokenKind;

typedef struct Token Token;
//トークン型
struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next token
  int val;         // If kind is TK_NUM, its value
  char *str;       // Token string
  int len;         // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
int expect_number(void);
bool at_eof(void);

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();

extern char *user_input;
extern Token *token;

// Local variable
typedef struct Var Var;
struct Var {
  Var *next;
  char *name; // Variable name
  int offset; // Offset from RBP
};


// AST node
typedef enum {
  ND_ADD,       // + 
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,     // =
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       // Variable
  ND_NUM,       // Integer
}NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // ノードの型
  Node *next;
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  
  Node *cond;
  Node *then;
  Node *els;

  
  Var *var;     // Use if kind==ND_VAR
  int val;       // Use if kind==ND_NUM

};

typedef struct Function Function;
struct Function{
  Node *node;
  Var *locals;
  int stack_size;
};

Function *program();

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void codegen(Function *prog);