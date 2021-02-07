#include <stdio.h>
#include <regex>
#include <vector>
#include "lexer/lexer.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
;
using namespace std;

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

SCENARIO("When multiple regexs matches a string then multiple nodes are "
        "created which may overlap other nodes. No overlaps may occurr") {
  GIVEN("A language") {
    lexer::regex_vector tokens {
      regex("Aaa"),
      regex("(\\w+)"),
      regex("(\\s+)"),
    };

    lexer::generic_lexer lexer(tokens);

    GIVEN("A string that multiple regexs will match") {
      string text("Aaabbb Aaa bbbAaa");

      WHEN("Tokenizing the text") {
        lexer.tokenize(text);

        REQUIRE(lexer.nodes.size() == 6); // includes white space

        THEN("The first token is produced by the second regex because its match "
            "spans over a longer length than the first regex's match") {
          CHECK(lexer.nodes[0].ri == 1);
        }

        THEN("The second token is produced by the first regex because its index "
             "in the token list is lower (higher priority) than the other") {
          CHECK(lexer.nodes[1].ri == 0);
        }
      }
    }

    // WHEN("Evicting all nodes which overlap other nodes") {
    //     lexer.evict_overlapses();

    //     THEN("All nodes which overlap another node with higher weight are gone") {
    //       REQUIRE(lexer.nodes.size() == 3);
    //       lexer::generic_lexer_node n1 = lexer.nodes[0];
    //       lexer::generic_lexer_node n2 = lexer.nodes[1];
    //       lexer::generic_lexer_node n3 = lexer.nodes[2];

    //       CHECK(n1.ri == token::if_statement);
    //       CHECK(n2.ri == token::whitespace);
    //       CHECK(n3.ri == token::identifier);
    //     }
    //   }
  }
}

TEST_CASE("A basic language") {
  enum token {
    if_statement,
    integer,
    identifier,
    whitespace,
    number_of_tokens
  };

  lexer::regex_vector tokens(number_of_tokens);

  tokens[token::whitespace] = regex("\\s+");
  tokens[token::if_statement] = regex("\\b(if|else|then)\\b");
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

  SECTION("Some more complex input") {
    string str =
        "if ifif then 1 else 2";

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

  SECTION("Some errornous input") {
    string str =
        "if var then 1 else 2\n"
        "if ??? then x\n"
        "if var then 10\n"
        "if var then 10\n";

    lexer.scan(token::whitespace, str);
    lexer.scan(token::if_statement, str);
    lexer.scan(token::integer, str);
    lexer.scan(token::identifier, str);
    lexer.sort();
    lexer.evict_overlapses();
    lexer.scan_errors();
    // The '???' characters is an error
    REQUIRE(lexer.error_nodes.size() == 1);
    CHECK(lexer.error_nodes[0].p == 24);
    CHECK(lexer.error_nodes[0].n == 3);
  }
}
