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
//  return memcmp(p, q, strlen(q)) == 0;
  return strncmp(p, q, strlen(q)) == 0;

}
bool is_alpha(char c)  {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_'; 

}
bool is_alnum(char c)  {
  return is_alpha(c) || ('0' <= c && c <= '9');  
}

static char *starts_with_reserved(char *p) {

  static char *kw[] = {"return", "if", "else"};

  for (int i = 0 ; i < sizeof(kw) / sizeof(*kw); i++) {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len]))
      return kw[i];

  }

  static char *ops[] = {"==", "!=", "<=", ">="};

  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
    if (startswith(p, ops[i]))
      return ops[i]; 

  return NULL;
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

    char *kw = starts_with_reserved(p);
    if (kw) {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }


    if (is_alpha(*p)) {
      char *q = p++;
      while (is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p-q);
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