#include <stdio.h>
#include <regex>
#include <vector>
#include "lexer/lexer.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
;
using namespace std;

enum token {
  if_statement,
  integer,
  identifier,
  whitespace,
  number_of_tokens
};


TEST_CASE("lexer Basics") {

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
  lexer::regex_vector tokens(number_of_tokens);

  tokens[token::whitespace] = regex("\\s+");
  tokens[token::if_statement] = regex("\\bif|else|then\\b");
  tokens[token::integer] = regex("(\\d+)");
  tokens[token::identifier] = regex("(\\w+)");

  lexer::generic_lexer lexer(tokens);
  REQUIRE(lexer.nodes.size() == 0);

  SECTION("Some input") {
    string                         script = "   if if if";
    lexer.tokenize(script);

    REQUIRE(lexer.nodes.size() == 6);
    REQUIRE(lexer.nodes[0].ri == token::whitespace);
  }

  SECTION("scan") {
    string              script = "if not";
    REQUIRE(lexer.nodes.size() == 0);
    lexer.scan(token::whitespace, script);
    REQUIRE(lexer.nodes.size() == 1);
    lexer::generic_lexer_node n1 = lexer.nodes[0];
    CHECK(n1.p == 2);
    CHECK(n1.n == 1);
  }

  SECTION("scan") {
    string              script = "if if";
    REQUIRE(lexer.nodes.size() == 0);
    lexer.scan(token::if_statement, script);
    lexer::generic_lexer_node n1 = lexer.nodes[0];
    lexer::generic_lexer_node n2 = lexer.nodes[1];
    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n2.p == 3);
    CHECK(n2.n == 2);

    CHECK(n1.ri == token::if_statement);
    CHECK(n2.ri == token::if_statement);
  }

  SECTION("linebreaks") {
    string              script = "if\nthen\nelse";
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
    REQUIRE(lexer.nodes.size() == 0);
    lexer.scan(token::whitespace, script);
    lexer.scan(token::if_statement, script);
    lexer.scan(token::identifier, script);
    REQUIRE(lexer.nodes.size() == 4);

    lexer.sort();

    CHECK(lexer.nodes.size() == 3);

    lexer::generic_lexer_node n1 = lexer.nodes[0];
    lexer::generic_lexer_node n2 = lexer.nodes[1];
    lexer::generic_lexer_node n3 = lexer.nodes[2];

    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n1.ri == token::if_statement);

    CHECK(n2.p == 2);
    CHECK(n2.n == 1);
    CHECK(n2.ri == token::whitespace);

    CHECK(n3.p == 3);
    CHECK(n3.n == 7);
    CHECK(n3.ri == token::identifier);
  }

  SECTION("sort") {
    string script = "if somevar then 1 else 2";
    lexer.scan(token::whitespace, script);
    lexer.scan(token::if_statement, script);
    lexer.scan(token::integer, script);
    lexer.scan(token::identifier, script);
    lexer.sort();

    CHECK(lexer.nodes.size() == 11);

    lexer::generic_lexer_node n1 = lexer.nodes[0];
    lexer::generic_lexer_node n2 = lexer.nodes[1];

    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n1.ri == token::if_statement);

    CHECK(n2.p == 2);
    CHECK(n2.n == 1);
    CHECK(n2.ri == token::whitespace);
  }

  SECTION("sort") {
    string script = "if ifif then 1 else 2";
    lexer.scan(token::whitespace, script);
    lexer.scan(token::if_statement, script);
    lexer.scan(token::integer, script);
    lexer.scan(token::identifier, script);
    lexer.sort();

    CHECK(lexer.nodes.size() == 11);

    lexer::generic_lexer_node n1 = lexer.nodes[0];
    lexer::generic_lexer_node n2 = lexer.nodes[1];

    CHECK(n1.p == 0);
    CHECK(n1.n == 2);
    CHECK(n1.ri == token::if_statement);

    CHECK(n2.p == 2);
    CHECK(n2.n == 1);
    CHECK(n2.ri == token::whitespace);
  }

  SECTION("Some more complex input") {
    string str =
        "if somevar then 1 else 2";

    lexer.tokenize(str);
    REQUIRE(lexer.nodes.size() == 11);
    CHECK(lexer.nodes[0].ri == token::if_statement);
    CHECK(lexer.nodes[1].ri == token::whitespace);
    CHECK(lexer.nodes[2].ri == token::identifier);
    CHECK(lexer.nodes[3].ri == token::whitespace);
    CHECK(lexer.nodes[4].ri == token::if_statement);
    CHECK(lexer.nodes[5].ri == token::whitespace);
    CHECK(lexer.nodes[6].ri == token::integer);
    CHECK(lexer.nodes[7].ri == token::whitespace);
    CHECK(lexer.nodes[8].ri == token::if_statement);
    CHECK(lexer.nodes[9].ri == token::whitespace);
    CHECK(lexer.nodes[10].ri == token::integer);
  }

  // SECTION("Some more complex input") {
  //   string str =
  //       "if ifif then 1 else 2";

  //   BasicLanguageTokens            lexer(re);
  //   BasicLanguageTokens::TokenList tokens;
  //   lexer.tokenize(str, tokens);
  //   REQUIRE(tokens.size() == 11);
  //   CHECK(tokens[0].token.type == token::if_statement);
  //   CHECK(tokens[1].token.type == token::whitespace);
  //   CHECK(tokens[2].token.type == token::identifier);
  //   CHECK(tokens[3].token.type == token::whitespace);
  //   CHECK(tokens[4].token.type == token::if_statement);
  //   CHECK(tokens[5].token.type == token::whitespace);
  //   CHECK(tokens[6].token.type == token::integer);
  //   CHECK(tokens[7].token.type == token::whitespace);
  //   CHECK(tokens[8].token.type == token::if_statement);
  //   CHECK(tokens[9].token.type == token::whitespace);
  //   CHECK(tokens[10].token.type == token::integer);
  // }

  // SECTION("Some errornous input") {
  //   string str =
  //       "if var then 1 else 2\n"
  //       "if ??? then x\n"
  //       "if var then 10\n"
  //       "if var then 10\n";

  //   BasicLanguageTokens            lexer(re);
  //   BasicLanguageTokens::TokenList tokens;
  //   lexer.scan(0, str);
  //   lexer.scan(1, str);
  //   lexer.scan(2, str);
  //   lexer.scan(3, str);
  //   lexer.sort();
  //   lexer.scan_errors();
  //   // The '???' characters is an error
  //   REQUIRE(lexer.error_nodes.size() == 1);
  //   CHECK(lexer.error_nodes[0].p == 24);
  //   CHECK(lexer.error_nodes[0].n == 3);
  // }
}
