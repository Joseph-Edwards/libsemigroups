//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2023-2025 Joseph Edwards + James D. Mitchell
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// This file contains the implementation of the AhoCorasick class.
#include "libsemigroups/aho-corasick-class.hpp"

#include <algorithm>    // for max, copy, reverse
#include <array>        // for array
#include <string>       // for basic_string, string, to_string
#include <string_view>  // for basic_string_view, string_view

#include "libsemigroups/constants.hpp"   // for Undefined, UNDEFINED, operator==
#include "libsemigroups/debug.hpp"       // for LIBSEMIGROUPS_ASSERT
#include "libsemigroups/detail/fmt.hpp"  // for format
#include "libsemigroups/dot.hpp"         // for Dot, Dot::Edge, Dot::Node
#include "libsemigroups/exception.hpp"   // for LIBSEMIGROUPS_EXCEPTION
#include "libsemigroups/types.hpp"       // for word_type, letter_type

namespace libsemigroups {

  std::string to_human_readable_repr(AhoCorasick const& ac) {
    using detail::group_digits;
    auto n_nodes = ac.number_of_nodes();
    return fmt::format("<AhoCorasick with {} node{}>",
                       group_digits(n_nodes),
                       n_nodes == 1 ? "" : "s");
  }

  // [[nodiscard]] Dot aho_corasick::dot(AhoCorasick& ac) {
  //   auto to_word = [](word_type const& w) {
  //     if (w.empty()) {
  //       return std::string("&#949;");
  //     }
  //     std::string result;
  //     for (auto a : w) {
  //       result += std::to_string(a);
  //     }
  //     return result;
  //   };

  //   Dot result;
  //   result.kind(Dot::Kind::digraph).add_attr("node [shape=\"box\"]");

  //   word_type w;
  //   for (auto index : ac.active_nodes()) {
  //     ac.signature_no_checks(w, index);
  //     auto& node = result.add_node(index).add_attr("label", to_word(w));
  //     if (ac.node_no_checks(index).is_terminal()) {
  //       node.add_attr("peripheries", "2");
  //     }
  //   }

  //   for (auto index : ac.active_nodes()) {
  //     for (auto [label, child] : ac.node_no_checks(index).children()) {
  //       result
  //           .add_edge(index, child)
  //           // FIXME properly
  //           .add_attr("color", result.colors[label % result.colors.size()])
  //           .add_attr("label", label);
  //     }
  //     result.add_edge(index, ac.suffix_link_no_checks(index))
  //         .add_attr("color", "black")
  //         .add_attr("style", "dashed")
  //         .add_attr("constraint", "false");
  //   }
  //   return result;
  // }

}  // namespace libsemigroups
