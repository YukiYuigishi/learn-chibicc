#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token kinds
typedef enum {
  TK_RESERVED, // symbol
  TK_NUM,      // integer token
  TK_EOF       // EOF token
} TokenKind;

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
} NodeKind;
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node type
  Node *lhs;     // left
  Node *rhs;     // right
  int val;       // if kind is ND_NUM use
};
typedef struct Token Token;

struct Token {
  TokenKind kind; // token type
  Token *next;
  int val;   // if kind is TK_NUM , the value
  char *str; // token string
};

Node *expr();
Node *mul();
Node *primary();

// now watching token
Token *token;

// 入力プログラム
char *user_input;

// error handling function like printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;

  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s\n", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
// if next token is expected symbol, advance token and return true
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// if next token is expecting , advance 1 token,
// else report error
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error("not %c", op);
  }

  token = token->next;
}

// if next token is integer , advance 1 token and return the number,
// else report error
int expect_number() {
  if (token->kind != TK_NUM)
    error("not number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// create new token and connect cur.
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
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
  case ND_NUM:
    break;
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

// tokenize inputed string

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {

    if (isspace(*p)) {
      p++;
      continue;
    }

    // Punctuator
    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p,"invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *primary() {

  // if the next token is "(" m it should be "(" expr ")".
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  // else the next token is number
  return new_node_num(expect_number());
}

Node *mul() {
  Node *node = primary();
  for (;;) {
    if (consume('*'))
      node = new_node(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_node(ND_DIV, node, primary());
    else
      return node;
  }
}

Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "argment is Failed\n");
    return 1;
  }

  // トーク内図してパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコードを生成
  gen(node);

  // スタックとドロップに敷き全体の値が残っているはずなのでそれをRAXにロード
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
