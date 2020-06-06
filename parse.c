#include "tkcc.h"

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
Node *new_unary(NodeKind kind, Node *expr)  {
  Node *node = new_node(kind);
  node->lhs = expr;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *new_var_node(char name) {
  Node *node = new_node(ND_VAR);
  node->name = name;
  return node;
}


Node *program() {
  Node head = {};
  Node *cur = &head;
 
  while (!at_eof())  {
    cur->next = stmt();
    cur = cur->next;
  }

  return head.next;
}

// stmt = "return" expr ";"
Node *stmt()  {
  if (consume("return")) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }
  
  Node *node = new_unary(ND_EXPR_STMT, expr());
  expect(";");
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if(consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());  
    else if (consume("!=")) 
      node = new_binary(ND_NE, node, relational());
    else
      return node;   
   }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) 
      node = new_binary(ND_LT, node, add());  
    else if (consume("<=")) 
      node = new_binary(ND_LE, node, add()); 
    else if (consume(">")) 
      node = new_binary(ND_LT, add(), node); 
    else if (consume(">=")) 
      node = new_binary(ND_LE, add(), node); 
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
     node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
     node = new_binary(ND_SUB, node, mul());
    else
     return node;
  }


}
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
     node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
     node = new_binary(ND_DIV, node, unary());
    else
     return node;
  }
}

Node *unary() {
    if (consume("+"))
      return unary();
    if (consume("-"))
      return new_binary(ND_SUB, new_num(0), unary());
   
    return primary();
}

Node *primary() {

  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  
  Token *tok = consume_ident();
  if (tok)
    return new_var_node(*tok->str);

  return new_num(expect_number());
}