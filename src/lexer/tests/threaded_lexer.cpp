#include <stdio.h>
#include <regex>
#include <vector>
#include "lexer/threaded_lexer.h"
#include "catch.hpp"

using namespace std;


SCENARIO("When using a threaded lexer") {
  GIVEN("A language") {
    lexer::regex_vector tokens {
      regex("Aaa"),
      regex("(\\w+)"),
      regex("(\\s+)"),
    };

    lexer::threaded_lexer lexer(tokens);

    GIVEN("A string that multiple regexs will match") {
      string text("Aaabbb Aaa bbbAaa");

      WHEN("Tokenizing the text") {
        lexer.tokenize(text);

        REQUIRE(lexer.nodes.size() == 5); // includes white space

        THEN("The first token is produced by the second regex because its match "
            "spans over a longer length than the first regex's match") {
          CHECK(lexer.nodes[0].ri == 1);
        };


        THEN("The second token is produced by the first regex because its index "
             "in the token list is lower (higher priority) than the other") {
          CHECK(lexer.nodes[2].ri == 0);
        }

        THEN("The fith token is produced by the second regex because its match "
            "spans over a longer length than the first regex's match") {
          CHECK(lexer.nodes[4].ri == 1);
        }
      }
    }
  }
}
