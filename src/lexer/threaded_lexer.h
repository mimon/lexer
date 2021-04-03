#pragma once

#include <stdio.h>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <string>
#include <vector>
#include <thread>
#include "lexer/lexer.h"

namespace lexer {

typedef std::vector<std::thread> thread_vector;
typedef std::vector<generic_lexer> lexer_vector;

class threaded_lexer {
  public:
  explicit threaded_lexer(const regex_vector& regexs);

  void tokenize(const std::string &input);

  std::vector<regex_vector>    regexs;
  lexer_vector lexers;

  node_vector nodes, error_nodes;
  thread_vector threads;
};
}
