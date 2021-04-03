#pragma once

#include "lexer/threaded_lexer.h"
#include <algorithm>

namespace lexer {

threaded_lexer::threaded_lexer(const regex_vector& regex_)
   {

    for(const std::regex& regex : regex_) {
      this->lexers.emplace_back(regex_vector{regex});
    }

    // std::transform(
    //   regex_.begin(),
    //   regex_.end(),
    //   this->lexers.begin(),
    //   [] (const std::regex& regex) {
    //     return generic_lexer();
    //   });
    
    // std::transform(
    //   this->regexs.begin(),
    //   this->regexs.end(),
    //   lexers.begin(),
    //   [this] (const regex_vector& regexv) {
    //     return generic_lexer(regexv);
    //   });
}

void threaded_lexer::tokenize(const std::string &input) {
  for(int i=0; i<this->lexers.size();++i) {
    generic_lexer& l = this->lexers[i];
    this->threads.emplace_back(&generic_lexer::tokenize, &l, input);
  }

  for(auto& thread : this->threads) {
    thread.join();
  }

  for(int i=0; i<this->lexers.size();++i) {
    generic_lexer& l = this->lexers[i];
    for (generic_lexer_node& n : l.nodes) {
      n.ri = i;
    }
    for (generic_lexer_node& n : l.error_nodes) {
      n.ri = i;
    }
  }

  const regex_vector& dummy{std::regex(" ")};
  generic_lexer final (dummy);

  for(int i=0; i<this->lexers.size();++i) {
    generic_lexer& l = this->lexers[i];
    final.nodes.insert(
      final.nodes.end(),
      l.nodes.begin(),
      l.nodes.end()
    );
  }

  final.sort();
  final.evict_overlapses();
  final.scan_errors();

  this->nodes = final.nodes;
  this->error_nodes = final.error_nodes;
}

}
