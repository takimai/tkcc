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

char *user_input;

// 現在着目しているトークン
Token *token;

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
    error_at("token->str, expect '%c'", op);
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

    if (*p == '+' || *p == '-') {
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

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Invalid number of argument");
    return 1;
  }  
  
  // トークナイズする
  user_input = argv[1];
  token = tokenize();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  
  // 式の最初は数でなければならないので、それをチェックして
  // 最初のmov命令を出力
  printf("  mov rax, %d\n", expect_number());
  
  // '+<数>'あるいは-<数>'というトークンの並びを消費しつつ
  // アセンブリを出力
  while(!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
    continue;
    }
    expect('-'); 
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
