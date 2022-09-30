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

typedef struct Token Token;

struct Token {
  TokenKind kind; // token type
  Token *next;
  int val;   // if kind is TK_NUM , the value
  char *str; // token string
};

// now watching token
Token *token;

// error handling function like printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
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

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("Could not tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "argment is Failed\n");
    return 1;
  }

  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // output first mov order
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());

    printf("  ret\n");
    return 0;
  }
  return 0;
}
