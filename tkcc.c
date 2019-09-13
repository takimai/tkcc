#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;       // トークン文字列
};


// 現在着目しているトークン
Token *token;


char *user_input;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
void error_at(char *loc, char *fmt, ...)  {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input; 
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  //pos個の空白を出力
  fprintf(stderr,  "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを一つ読み進めて
// 真を返す。それ以外の場合は偽を返す。
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] !=op)
    return false;
  token = token->next;
    return true;
}
// 次のトークンが期待している記号のときには、トークンを一つ読み進める
// それ以外の場合には、エラーを報告する。
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "expect '%c'", op);
  token = token->next;
}

// 次のトークンが数値のときには、トークンを一つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;

}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur =&head;

  while(*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;

    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
    continue;
    }

    error_at(p, "expected a number");  
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

typedef enum {
  ND_ADD, // + 
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
}NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *mul();
Node *primary();

Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
     node = new_binary(ND_ADD, node, mul());
    else if (consume('-'))
     node = new_binary(ND_SUB, node, mul());
    else
     return node;
  }


}
Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*'))
     node = new_binary(ND_MUL, node, primary());
    else if (consume('/'))
     node = new_binary(ND_DIV, node, primary());
    else
     return node;
  }
}
Node *primary() {

  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }
  
  return new_num(expect_number());
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
  printf("  push %d\n", node->val);
  return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;

  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;

  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;

  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;

  }

  printf("  push rax\n");

}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Invalid number of argument");
    return 1;
  }  
  
  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  
  // 抽象構文木を下りながらコード生成
  gen(node); 

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;

}



