#include "tkcc.h"

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

bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
    return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *t = token;
  token = token->next;
  return t;
}


void expect(char *op) {

  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expect \"%s\"", op);
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

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;

  /* for debug
  printf("new_token kind %d\n", kind);
  printf("new_token str %s\n", str);
  printf("new_token len %d\n", len);
  */

  cur->next = tok;

  return tok;
}
bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}
bool is_alpha(char c)  {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_'; 

}
bool is_alnum(char c)  {
  return is_alpha(c) || ('0' <= c && c <= '9');  

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

    if (startswith(p,"return") && !is_alnum(p[6])) {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
    }

    if (startswith(p, "==") || startswith(p, "!=") ||
       startswith(p, "<=") || startswith(p, ">=")) { 

      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
 
    if (ispunct(*p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "invalid token");  
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}