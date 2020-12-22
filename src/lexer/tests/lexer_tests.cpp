#include <stdio.h>
#include <regex>
#include <vector>
#include "lexer/lexer.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
;
using namespace std;

struct Token {
  enum Type {
    WHITESPACE,
    IDENT,
    INT,
    IF
  };
  explicit Token(Type type)
    : type(type) {
  }
  explicit Token()
    : type(Token::WHITESPACE) {
  }

  Type type;
};

struct IDToken : public Token {
  explicit IDToken(std::string &id, Token::Type type = Token::IDENT)
    : Token(type), id_name(id) {
  }
  std::string id_name;
};
struct ValueBase : public Token {
  using Token::Token;
};

struct IntValue : public ValueBase {
  explicit IntValue(int v)
    : ValueBase(INT), value(v) {
  }
  int value;
};

typedef compiler::Lexer<Token> BasicLanguageTokens;

TEST_CASE("Lexer Basics") {

  SECTION("Understanding c++ regexs") {
    std::string input =
        "This is has five white spaces";
    std::regex regex(" ");

    std::sregex_iterator re_begin(input.begin(), input.end(), regex);
    std::sregex_iterator re_end = std::sregex_iterator();
    REQUIRE(std::distance(re_begin, re_end) == 5);
  }
}

TEST_CASE("A basic language") {
  BasicLanguageTokens::RegexHandleMap re = {
    make_pair(regex("\\s+"), [](std::smatch &match) {
      return Token(Token::WHITESPACE);
    }),
    make_pair(regex("if|else|then"), [](std::smatch &match) {
      return Token(Token::IF);
    }),
    make_pair(regex("(\\d+)"), [](std::smatch &match) {
      int val = std::stoi(match[1]);
      return IntValue(val);
    }),
    make_pair(regex("(\\w+)"), [](std::smatch &match) {
      string s = match[0];
      return IDToken(s);
    })
  };

  typedef BasicLanguageTokens::node node;
  BasicLanguageTokens               lexer(re);
  REQUIRE(lexer.nodes.size() == 0);

  SECTION("Some input") {
    string                         script = "   if if if";
    BasicLanguageTokens            lexer(re);
    BasicLanguageTokens::TokenList tokens;
    lexer.tokenize(script, tokens);
    REQUIRE(tokens.size() == 6);
    REQUIRE(tokens[0].token.type == Token::WHITESPACE);
  }

  SECTION("scan") {
    string              script = "if not";
    BasicLanguageTokens lexer(re);
    REQUIRE(lexer.nodes.size() == 0);
    lexer.scan(0, script);
    REQUIRE(lexer.nodes.size() == 1);
    node n1 = lexer.nodes[0];
    CHECK(n1.p == 2);
    CHECK(n1.n == 1);
  }

  SECTION("scan") {
    string              script = "if if";
    BasicLanguageTokens lexer(re);
    REQUIRE(lexer.nodes.size() == 0);
    lexer.scan(1, script);
    node n1 = lexer.nodes[0];
    node n2 = lexer.nodes[1];
    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n2.p == 3);
    CHECK(n2.n == 2);
  }

  SECTION("linebreaks") {
    string              script = "if\nthen\nelse";
    BasicLanguageTokens lexer(re);
    std::vector<size_t> newline_positions = lexer.linebreaks(script, std::regex("\n", std::regex::basic));

    CHECK(newline_positions.size() == 2);
    CHECK(newline_positions[0] == 2);
    CHECK(newline_positions[1] == 7);

    size_t line = lexer.line_of_position(newline_positions, 0);
    CHECK(line == 1);

    line = lexer.line_of_position(newline_positions, 2);
    CHECK(line == 1);

    line = lexer.line_of_position(newline_positions, 3);
    CHECK(line == 2);

    line = lexer.line_of_position(newline_positions, 6);
    CHECK(line == 2);

    line = lexer.line_of_position(newline_positions, 8);
    CHECK(line == 3);
  }

  SECTION("sort") {
    string              script = "if thenter";
    BasicLanguageTokens lexer(re);
    REQUIRE(lexer.nodes.size() == 0);
    lexer.scan(0, script);
    lexer.scan(1, script);
    lexer.scan(3, script);
    REQUIRE(lexer.nodes.size() == 5);

    lexer.sort();

    CHECK(lexer.nodes.size() == 3);

    node n1 = lexer.nodes[0];
    node n2 = lexer.nodes[1];
    node n3 = lexer.nodes[2];

    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n1.ri == 1);

    CHECK(n2.p == 2);
    CHECK(n2.n == 1);
    CHECK(n2.ri == 0);

    CHECK(n3.p == 3);
    CHECK(n3.n == 7);
    CHECK(n3.ri == 3);
  }

  SECTION("sort") {
    string script = "if somevar then 1 else 2";
    lexer.scan(0, script);
    lexer.scan(1, script);
    lexer.scan(2, script);
    lexer.scan(3, script);
    lexer.sort();

    CHECK(lexer.nodes.size() == 11);

    node n1 = lexer.nodes[0];
    node n2 = lexer.nodes[1];

    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n1.ri == 1);

    CHECK(n2.p == 2);
    CHECK(n2.n == 1);
    CHECK(n2.ri == 0);
  }

  SECTION("sort") {
    string script = "if ifif then 1 else 2";
    lexer.scan(0, script);
    lexer.scan(1, script);
    lexer.scan(2, script);
    lexer.scan(3, script);
    lexer.sort();

    CHECK(lexer.nodes.size() == 11);

    node n1 = lexer.nodes[0];
    node n2 = lexer.nodes[1];

    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n1.ri == 1);

    CHECK(n2.p == 2);
    CHECK(n2.n == 1);
    CHECK(n2.ri == 0);
  }

  SECTION("Some more complex input") {
    string str =
        "if somevar then 1 else 2";

    BasicLanguageTokens            lexer(re);
    BasicLanguageTokens::TokenList tokens;
    lexer.tokenize(str, tokens);
    REQUIRE(tokens.size() == 11);
    CHECK(tokens[0].token.type == Token::IF);
    CHECK(tokens[1].token.type == Token::WHITESPACE);
    CHECK(tokens[2].token.type == Token::IDENT);
    CHECK(tokens[3].token.type == Token::WHITESPACE);
    CHECK(tokens[4].token.type == Token::IF);
    CHECK(tokens[5].token.type == Token::WHITESPACE);
    CHECK(tokens[6].token.type == Token::INT);
    CHECK(tokens[7].token.type == Token::WHITESPACE);
    CHECK(tokens[8].token.type == Token::IF);
    CHECK(tokens[9].token.type == Token::WHITESPACE);
    CHECK(tokens[10].token.type == Token::INT);
  }

  SECTION("Some more complex input") {
    string str =
        "if ifif then 1 else 2";

    BasicLanguageTokens            lexer(re);
    BasicLanguageTokens::TokenList tokens;
    lexer.tokenize(str, tokens);
    REQUIRE(tokens.size() == 11);
    CHECK(tokens[0].token.type == Token::IF);
    CHECK(tokens[1].token.type == Token::WHITESPACE);
    CHECK(tokens[2].token.type == Token::IDENT);
    CHECK(tokens[3].token.type == Token::WHITESPACE);
    CHECK(tokens[4].token.type == Token::IF);
    CHECK(tokens[5].token.type == Token::WHITESPACE);
    CHECK(tokens[6].token.type == Token::INT);
    CHECK(tokens[7].token.type == Token::WHITESPACE);
    CHECK(tokens[8].token.type == Token::IF);
    CHECK(tokens[9].token.type == Token::WHITESPACE);
    CHECK(tokens[10].token.type == Token::INT);
  }

  SECTION("Some errornous input") {
    string str =
        "if var then 1 else 2\n"
        "if ??? then x\n"
        "if var then 10\n"
        "if var then 10\n";

    BasicLanguageTokens            lexer(re);
    BasicLanguageTokens::TokenList tokens;
    lexer.scan(0, str);
    lexer.scan(1, str);
    lexer.scan(2, str);
    lexer.scan(3, str);
    lexer.sort();
    lexer.scan_errors();
    // The '???' characters is an error
    REQUIRE(lexer.error_nodes.size() == 1);
    CHECK(lexer.error_nodes[0].p == 24);
    CHECK(lexer.error_nodes[0].n == 3);
  }
}
