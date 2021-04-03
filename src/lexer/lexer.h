//
//  CSSlexer.h
//
//
//  Created by Simon Warg on 27/11/15.
//
//

#pragma once

#include <stdio.h>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <string>
#include <vector>

namespace lexer {

// A node is used to store information of a substring in the input string
struct generic_lexer_node {
  std::string::size_type p;   // index position of where it starts
  std::string::size_type n;   // length of the substring
  std::size_t            ri;  // the regex index of which regex it was generated from
  std::string::size_type e;   // index position of where it ends
  // Operators used for sorting nodes.
  // The sorting is based on the position and the
  // length of the nodes
  friend bool operator<(const generic_lexer_node &a, const generic_lexer_node &b) {
    return (a.e < b.e) || ((a.e == b.e) && (a.n < b.n)) || ((a.e == b.e) && (a.n == b.n) && (a.ri > b.ri));
  }
  friend bool operator>(const generic_lexer_node &a, const generic_lexer_node &b) {
    return b < a;
  }
};

typedef std::vector<std::regex> regex_vector;
typedef std::vector<generic_lexer_node> node_vector;

/**
 * The lexer can be used to chop up an input string into to
 * user-defined C++ tokens. The lexer is constructed by a
 * language which is an array of pairs containing a regex and a
 * function that can construct a token from a substring matching that regex.
 *
 * Using the tokenize() method, the lexer will scan the input
 * string and output tokens based on the language. If there are more
 * matches for any given substring, then the regex matching the
 * most characters of the substring will be picked.
 */
class generic_lexer {
  public:
  explicit generic_lexer(const regex_vector& regexs)
    : regexs(regexs) {
    reset();
  }

  /**
   * Searches a string for all substrings matching a regex.
   * The results are stored as nodes in a member container
   * @param std::size_t i             the index of the regex
   * @param std::string &s            the string to search
   */
  void scan(std::size_t i, const std::string &s) {
    const std::regex &regex = this->regexs[i];
    auto        iter =
        std::sregex_iterator(s.begin(), s.end(), regex);
    auto end = std::sregex_iterator();

    for (; iter != end; ++iter) {
      std::smatch match = *iter;
      generic_lexer_node        n;
      n.p  = match.position();
      n.n  = match.length();
      n.ri = i;
      n.e = n.p + n.n;
      this->nodes.push_back(n);
    }
  }

  /**
   * Sorts the container of previously found nodes by position of appearance.
   * If more than one node starts at the same position, then
   * all but the one with highest length value will be evicted.
   */
  void sort() {
    if (this->nodes.size() < 2) {
      return;
    }
    std::sort(this->nodes.begin(), this->nodes.end());
  }

  /**
   * Removes nodes that overlap other nodes.
   * The nodes are assumed to have already been sorted.
   */
  void evict_overlapses() {
    auto it = this->nodes.rbegin();
    auto end = this->nodes.rend();
    std::size_t threshold = std::numeric_limits<std::size_t>::max();

    auto new_end = std::remove_if(it, end, [&threshold](const generic_lexer_node& node) {
      bool remove = false;
      if((node.p + node.n) > threshold) {
        remove = true;
      } else {
        threshold = node.p;
      }
      return remove;
    });
    this->nodes.erase(this->nodes.rend().base(), new_end.base());
  }

  /**
   * Find and return a list of positions of all line breaks, in
   * the order of appearence.
   * @param  {[type]} std::string &input        the input string
   * @param  {[type]} const       std::regex    &newline_regex the line break regex
   */
  std::vector<std::size_t> linebreaks(std::string &input, const std::regex &newline_regex) {
    auto newlines_begin =
        std::sregex_iterator(input.begin(), input.end(), newline_regex);
    auto newlines_end = std::sregex_iterator();

    std::vector<std::size_t> output;
    for (std::sregex_iterator i2 = newlines_begin; i2 != newlines_end; ++i2) {
      std::smatch match = *i2;
      output.push_back(match.position());
    }
    return output;
  }

  /**
   * Returns the line which the given character position is located.
   * @param  {[type]} const       std::vector<std::size_t> &lines        line positions
   * @param  {[type]} std::size_t char_position            position of character
   */
  std::size_t line_of_position(const std::vector<std::size_t> &lines, std::size_t char_position) {
    auto it = std::lower_bound(lines.begin(), lines.end(), char_position);
    if (it == lines.end()) {
      return lines.size() + 1;
    }
    return std::distance(lines.begin(), it) + 1;
  }

  /**
   * Stores all substrings that do not appear in the
   * node container.
   * @return {[type]} [description]
   */
  void scan_errors() {
    generic_lexer_node prev;
    prev.p = 0;
    prev.n = 0;
    for (auto &it : this->nodes) {
      std::size_t should_be_next = (prev.p + prev.n);
      if (it.p != should_be_next) {
        // error
        generic_lexer_node error;
        error.p = should_be_next;
        error.n = it.p - should_be_next;
        this->error_nodes.push_back(error);
      }
      prev = it;
    }
  }

  void tokenize(const std::string &input) {
    reset();

    // Collect all regex results and store them as nodes
    for (std::size_t i = 0; i < this->regexs.size(); ++i) {
      scan(i, input);
    }
    // Sort the container. This will pick the longest spanning
    // nodes for each character position in the input string
    sort();
    evict_overlapses();

    // Even though we may output valid tokens, the
    // input string may be inconsistent or contain
    // invalid substrings. Find them and store them
    scan_errors();
  }

  void reset() {
    this->nodes.clear();
    this->error_nodes.clear();
  }

  regex_vector    regexs;
  node_vector nodes, error_nodes;
};
}
