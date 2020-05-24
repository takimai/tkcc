#include "tkcc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Invalid number of argument");
    return 1;
  }  

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  codegen(node); 
  return 0;
}

