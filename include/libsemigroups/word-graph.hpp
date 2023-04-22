//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2019-2023 Finn Smith
// Copyright (C) 2019-2023 James D. Mitchell
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

// This file contains an implementation of word graphs (which are basically
// deterministic automata without initial or accept states).

// TODO(later)
// * iwyu
// * More benchmarks
// * split into tpp file

#ifndef LIBSEMIGROUPS_WORD_GRAPH_HPP_
#define LIBSEMIGROUPS_WORD_GRAPH_HPP_

#include <algorithm>    // for uniform_int_distribution
#include <cstddef>      // for size_t
#include <cstdint>      // for uint64_t, int64_t
#include <iterator>     // for forward_iterator_tag, distance
#include <numeric>      // for accumulate
#include <ostream>      // for operator<<
#include <queue>        // for queue
#include <random>       // for mt19937
#include <stack>        // for stack
#include <string>       // for to_string
#include <tuple>        // for tie
#include <type_traits>  // for is_integral, is_unsigned
#include <utility>      // for pair
#include <variant>      // for variant
#include <vector>       // for vector

#include "config.hpp"      // for LIBSEMIGROUPS_EIGEN_ENABLED
#include "constants.hpp"   // for UNDEFINED
#include "containers.hpp"  // for DynamicArray2
#include "debug.hpp"       // for LIBSEMIGROUPS_ASSERT
#include "exception.hpp"   // for LIBSEMIGROUPS_EXCEPTION
#include "forest.hpp"      // for Forest
#include "int-range.hpp"   // for IntegralRange
#include "iterator.hpp"    // for ConstIteratorStateless
#include "matrix.hpp"      // for IntMat
#include "order.hpp"       // for Order
#include "types.hpp"       // for word_type
#include "uf.hpp"          // for
#include "words.hpp"       // for number_of_words

#include <rx/ranges.hpp>

#ifdef LIBSEMIGROUPS_EIGEN_ENABLED
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <Eigen/Core>
#include <unsupported/Eigen/MatrixFunctions>
#undef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

namespace libsemigroups {

  template <typename Node>
  class WordGraph;  // forward decl

  namespace word_graph {
    //! Undoc
    template <typename Node>
    using node_type = typename WordGraph<Node>::node_type;

    //! Undoc
    template <typename Node>
    using label_type = typename WordGraph<Node>::label_type;

    //! No doc
    // not noexcept because it throws an exception!
    template <typename Node>
    void validate_node(WordGraph<Node> const& ad, node_type<Node> v);

    //! No doc
    // not noexcept because it throws an exception!
    template <typename Node>
    void validate_label(WordGraph<Node> const& ad, label_type<Node> lbl);
  }  // namespace word_graph

  // TODO doc
  struct WordGraphBase {};

  //! Defined in ``digraph.hpp``.
  //!
  //! This class represents the digraph of an action of a semigroup on a set.
  //! If the digraph has \p n nodes, they are represented by the numbers
  //! \f${0, ..., n - 1}\f$, and every node has the same number \c m of
  //! out-edges (edges with source that node and range any other node). The
  //! number \c m is referred to as the *out-degree* of the digraph, or any of
  //! its nodes.
  //!
  //! \tparam Node the type of the nodes in the digraph, must be an unsigned
  //! integer type.
  //!
  //! \sa Action.
  template <typename Node>
  class WordGraph : private WordGraphBase {
    static_assert(std::is_integral<Node>(),
                  "the template parameter Node must be an integral type!");
    static_assert(
        std::is_unsigned<Node>(),
        "the template parameter Node must be an unsigned integral type!");

    template <typename N>
    friend class WordGraph;

    ////////////////////////////////////////////////////////////////////////
    // WordGraph - iterator - private
    ////////////////////////////////////////////////////////////////////////

   public:
    ////////////////////////////////////////////////////////////////////////
    // WordGraph - typedefs - public
    ////////////////////////////////////////////////////////////////////////

    //! The type of nodes in a digraph.
    using node_type = Node;

    //! The type of edge labels in a digraph.
    using label_type = Node;

    //! Unsigned integer type.
    using size_type = std::size_t;

#ifdef LIBSEMIGROUPS_EIGEN_ENABLED
    //! The type of the adjacency matrix.
    using adjacency_matrix_type
        = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
#else
    //! The type of the adjacency matrix.
    using adjacency_matrix_type = IntMat<0, 0, int64_t>;
#endif

    //! The type of an iterator pointing to the nodes of a digraph.
    using const_iterator_nodes = typename IntegralRange<Node>::const_iterator;

    //! The type of a reverse iterator pointing to the nodes of a digraph.
    using const_reverse_iterator_nodes =
        typename IntegralRange<Node>::const_reverse_iterator;

    //! The type of an iterator pointing to the out-edges of a node in a
    //! digraph.
    using const_iterator_edges =
        typename detail::DynamicArray2<Node>::const_iterator;

    ////////////////////////////////////////////////////////////////////////
    // WordGraph - constructors + destructor - public
    ////////////////////////////////////////////////////////////////////////

    //! Construct from number of nodes and out degree.
    //!
    //! \param m the number of nodes in the digraph (default: 0).
    //! \param n the out-degree of every node (default: 0).
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(mn)\f$ where \p m is the number of nodes, and \p n is
    //! the out-degree of the digraph.
    // Not noexcept
    explicit WordGraph(Node m = 0, Node n = 0);

    //! Re-initialize the digraph to have \p m nodes and out-degree \p n
    //!
    //! This functions puts a digraph into the state that it would have been in
    //! if it had just been newly constructed with the same parameters \p m and
    //! \p n.
    //!
    //! \param m the number of nodes in the digraph.
    //! \param n the out-degree of every node.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(mn)\f$ where \p m is the number of nodes, and \p n is the
    //! out-degree of the digraph.
    void init(Node m, Node n);

    //! Default copy constructor
    WordGraph(WordGraph const&);

    template <typename N>
    WordGraph(WordGraph<N> const&);

    //! Default move constructor
    WordGraph(WordGraph&&);

    //! Default copy assignment constructor
    WordGraph& operator=(WordGraph const&);

    //! Default move assignment constructor
    WordGraph& operator=(WordGraph&&);

    ~WordGraph();

    //! Construct a random digraph from number of nodes and out-degree.
    //!
    //! \param number_of_nodes the number of nodes
    //! \param out_degree the out-degree of every node
    //! \param mt a std::mt19937 used as a random source (defaults to:
    //! std::mt19937(std::random_device()()))
    //!
    //! \throws LibsemigroupsException if any of the following hold:
    //! * \p number_of_nodes is less than \c 2
    //! * \p out_degree is less than \c 2
    //!
    //! \par Complexity
    //! \f$O(mn)\f$ where \p m is the number of nodes, and \p n is
    //! the out-degree of the digraph.
    static WordGraph random(Node         number_of_nodes,
                            Node         out_degree,
                            std::mt19937 mt
                            = std::mt19937(std::random_device()()));

    //! Construct a random digraph from number of nodes, edges, and out-degree.
    //!
    //! \param number_of_nodes the number of nodes
    //! \param out_degree the out-degree of every node
    //! \param number_of_edges the out-degree of every node
    //! \param mt a std::mt19937 used as a random source (defaults to:
    //! std::mt19937(std::random_device()()))
    //!
    //! \throws LibsemigroupsException if any of the following hold:
    //! * \p number_of_nodes is less than \c 2
    //! * \p out_degree is less than \c 2
    //! * \p number_of_edges exceeds the product of \p number_of_nodes and \p
    //! out_degree
    //!
    //! \par Complexity
    //! At least \f$O(mn)\f$ where \p m is the number of nodes, and \p n is the
    //! out-degree of the digraph.
    static WordGraph random(Node         number_of_nodes,
                            Node         out_degree,
                            Node         number_of_edges,
                            std::mt19937 mt
                            = std::mt19937(std::random_device()()));

    //! Construct a random acyclic digraph from number of nodes, edges, and
    //! out-degree.
    //!
    //! \param number_of_nodes the number of nodes
    //! \param out_degree the out-degree of every node
    //! \param number_of_edges the out-degree of every node
    //! \param mt a std::mt19937 used as a random source (defaults to:
    //! std::mt19937(std::random_device()()))
    //!
    //! \throws LibsemigroupsException if any of the following hold:
    //! * \p number_of_nodes is less than \c 2
    //! * \p out_degree is less than \c 2
    //! * \p number_of_edges exceeds the product of \p number_of_nodes and \p
    //! out_degree
    //! * \p number_of_edges exceeds the product of \p number_of_nodes and \p
    //! number_of_nodes - 1 divided by 2.
    //!
    //! \par Complexity
    //! At least \f$O(mn)\f$ where \p m is the number of nodes, and \p n is the
    //! out-degree of the digraph.
    static WordGraph random_acyclic(Node         number_of_nodes,
                                    Node         out_degree,
                                    Node         number_of_edges,
                                    std::mt19937 mt
                                    = std::mt19937(std::random_device()()));

    ////////////////////////////////////////////////////////////////////////
    // WordGraph - modifiers - public
    ////////////////////////////////////////////////////////////////////////

    //! Adds a number of new nodes.
    //!
    //! \param nr the number of nodes to add.
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //! \strong_guarantee
    //!
    //! \complexity
    //! Linear in `number_of_nodes() + nr`.
    //!
    //! \iterator_validity
    //! \iterator_invalid
    // Not noexcept because DynamicArray2::add_rows isn't.
    void add_nodes(size_t nr);

    //! Adds to the out-degree.
    //!
    //! \param nr the number of new out-edges for every node.
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //! \strong_guarantee
    //!
    //! \complexity
    //! \f$O(mn)\f$ where \c m is the number of nodes, and \c n is the new out
    //! degree of the digraph.
    //!
    //! \iterator_validity
    //! \iterator_invalid
    // Not noexcept because DynamicArray2::add_cols isn't.
    void add_to_out_degree(size_t nr);

    //! Restrict the digraph to its first \p n nodes.
    //!
    //! \param n the number of nodes
    //!
    //! \returns (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! This function performs no checks whatsoever and will result in a
    //! corrupted digraph if there are any edges from the nodes \f$0, \ldots, n
    //! - 1\f$ to nodes larger than \f$n - 1\f$.
    // TODO(later) this is the nc version do a non-nc version also
    // Means restrict the number of nodes to the first 0, ... ,n - 1.
    // TODO(v3) rename to shrink_nodes_to
    void inline restrict(size_t n) {
      _nr_nodes = n;
      _dynamic_array_2.shrink_rows_to(n);
    }

    // Only valid if no edges incident to nodes in [first, last) point outside
    // [first, last)
    void induced_subdigraph(node_type first, node_type last);

    template <typename Iterator>
    void induced_subdigraph(Iterator first, Iterator last);

    //! Add an edge from one node to another with a given label.
    //!
    //! If \p i and \p j are nodes in \c this, and \p lbl is in the range `[0,
    //! out_degree())`, then this function adds an edge from \p i to \p j
    //! labelled \p lbl.
    //!
    //! \param i the source node
    //! \param j the range node
    //! \param lbl the label of the edge from \p i to \p j
    //!
    //! \returns
    //! (None)
    //!
    //! \throws LibsemigroupsException if \p i, \p j, or \p lbl is
    //! not valid.
    //! \strong_guarantee
    //!
    //! \complexity
    //! Constant.
    // Not noexcept because validate_node/label aren't
    void inline add_edge(node_type i, node_type j, label_type lbl) {
      word_graph::validate_node(*this, i);
      word_graph::validate_node(*this, j);
      word_graph::validate_label(*this, lbl);
      add_edge_no_checks(i, j, lbl);
    }

    void inline def_edge(node_type i, label_type lbl, node_type j) {
      word_graph::validate_node(*this, i);
      word_graph::validate_node(*this, j);
      word_graph::validate_label(*this, lbl);
      def_edge_no_checks(i, lbl, j);
    }

    //! Add an edge from one node to another with a given label.
    //!
    //! \param i the source node
    //! \param j the range node
    //! \param lbl the label of the edge from \p i to \p j
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Constant.
    //!
    //! \warning
    //! No checks whatsoever on the validity of the arguments are performed.

    // TODO remove this in v3
    void inline add_edge_no_checks(node_type i, node_type j, label_type lbl) {
      _dynamic_array_2.set(i, lbl, j);
    }

    void inline def_edge_no_checks(node_type i, label_type lbl, node_type j) {
      _dynamic_array_2.set(i, lbl, j);
    }

    //! Remove an edge from a node with a given label.
    //!
    //! \param i the source node
    //! \param lbl the label of the edge from \p i
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Constant.
    //!
    //! \warning
    //! No checks whatsoever on the validity of the arguments are performed.
    void inline remove_edge_no_checks(node_type i, label_type lbl) {
      _dynamic_array_2.set(i, lbl, UNDEFINED);
    }

    //! Remove all of the edges in the digraph.
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! \f$O(mn)\f$ where \p m is the number of nodes and \p n is the
    //! out-degree.
    //!
    //! \par Parameters
    //! (None)
    void inline remove_all_edges() {
      std::fill(_dynamic_array_2.begin(), _dynamic_array_2.end(), UNDEFINED);
    }

    void remove_label(label_type lbl) {
      word_graph::validate_label(*this, lbl);
      remove_label_no_checks(lbl);
    }

    void remove_label_no_checks(label_type lbl);

    //! Ensures that the digraph has capacity for a given number of nodes, and
    //! out-degree.
    //!
    //! \note
    //! Does not modify number_of_nodes() or out_degree().
    //!
    //! \param m the number of nodes
    //! \param n the out-degree
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! \f$O(mn)\f$ where \p m is the number of nodes and \p n is the
    //! out-degree.
    //!
    //! \iterator_validity
    //! \iterator_invalid
    // Not noexcept because DynamicArray2::add_cols isn't.
    void reserve(Node m, Node n) const {
      _dynamic_array_2.add_cols(n - _dynamic_array_2.number_of_cols());
      // What if add_cols throws, what guarantee can we offer then?
      _dynamic_array_2.add_rows(m - _dynamic_array_2.number_of_rows());
    }

    //! Swaps the edge with specified label from one node with another.
    //!
    //! This function swaps the target of the edge from the node \p u labelled
    //! \p a with the target of the edge from the node \p v labelled \p a.
    //!
    //! \param u the first node
    //! \param v the second node
    //! \param a the label
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Constant
    //!
    //! \warning
    //! No checks whatsoever on the validity of the arguments are performed.
    // swap u - a - > u' and v - a -> v'
    void swap_edges_no_checks(node_type u, node_type v, label_type a) {
      _dynamic_array_2.swap(u, a, v, a);
    }

    //! Check if two action digraphs are equal.
    //!
    //! \param that the action digraph for comparisonb
    //!
    //! \returns
    //! A `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! At worst \f$O(nm)\f$ where \f$n\f$ is the number of nodes and \f$m\f$
    //! is the out-degree of the digraph.
    // swap u - a - > u' and v - a -> v'
    bool operator==(WordGraph const& that) const {
      return _dynamic_array_2 == that._dynamic_array_2;
    }

    ////////////////////////////////////////////////////////////////////////
    // WordGraph - nodes, neighbors, etc - public
    ////////////////////////////////////////////////////////////////////////

    //! Get the range of the edge with given source node and label.
    //!
    //! \param v the node
    //! \param lbl the label
    //!
    //! \returns
    //! Returns the node adjacent to \p v via the edge labelled \p lbl, or
    //! libsemigroups::UNDEFINED; both are values of type \ref node_type.
    //!
    //! \complexity
    //! Constant.
    //!
    //! \exception LibsemigroupsException if \p v or \p lbl is not
    //! valid.
    // Not noexcept because validate_node/label aren't
    node_type inline neighbor(node_type v, label_type lbl) const {
      word_graph::validate_node(*this, v);
      word_graph::validate_label(*this, lbl);
      return _dynamic_array_2.get(v, lbl);
    }

    //! Get the range of the edge with given source node and label.
    //!
    //! \param v the node
    //! \param lbl the label
    //!
    //! \returns
    //! Returns the node adjacent to \p v via the edge labelled \p lbl, or
    //! libsemigroups::UNDEFINED; both are values of type \ref node_type.
    //!
    //! \complexity
    //! Constant.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning This function is unsafe because it does not verify \p v or \p
    //! lbl is valid.
    // Not noexcept because DynamicArray2::get is not
    node_type inline neighbor_no_checks(node_type v, label_type lbl) const {
      return _dynamic_array_2.get(v, lbl);
    }

    //! Get the next neighbor of a node that doesn't equal
    //! libsemigroups::UNDEFINED.
    //!
    //! If `neighbor(v, i)` equals libsemigroups::UNDEFINED for every value in
    //! the range \f$[i, n)\f$, where \f$n\f$ is the return value of
    //! out_degree() then \c x.first and \c x.second equal
    //! libsemigroups::UNDEFINED.
    //!
    //! \param v the node
    //! \param i the label
    //!
    //! \returns
    //! Returns a [std::pair](https://en.cppreference.com/w/cpp/utility/pair)
    //! \c x where:
    //! 1. \c x.first is adjacent to \p v via an edge labelled
    //!    \c x.second;
    //! 2. \c x.second is the minimum value in the range \f$[i, n)\f$ such that
    //!    `neighbor(v, x.second)` is not equal to libsemigroups::UNDEFINED
    //!    where \f$n\f$ is the return value of out_degree(); and
    //!
    //! \complexity
    //! At worst \f$O(n)\f$ where \f$n\f$ equals out_degree().
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \sa next_neighbor.
    //!
    //! \warning This function is unsafe because it is not verified that the
    //! parameter \p v represents a node of \c this.
    // Not noexcept because DynamicArray2::get is not
    std::pair<node_type, label_type> next_neighbor_no_checks(node_type,
                                                             label_type) const;

    //! Get the next neighbor of a node that doesn't equal
    //! libsemigroups::UNDEFINED.
    //!
    //! If `neighbor(v, i)` equals libsemigroups::UNDEFINED for every value in
    //! the range \f$[i, n)\f$, where \f$n\f$ is the return value of
    //! out_degree() then \c x.first and \c x.second equal
    //! libsemigroups::UNDEFINED.
    //!
    //! \param v the node
    //! \param i the label
    //!
    //! \returns
    //! Returns a [std::pair](https://en.cppreference.com/w/cpp/utility/pair)
    //! \c x where:
    //! 1. \c x.first is adjacent to \p v via an edge labelled
    //!    \c x.second;
    //! 2. \c x.second is the minimum value in the range \f$[i, n)\f$ such that
    //!    `neighbor(v, x.second)` is not equal to libsemigroups::UNDEFINED
    //!    where \f$n\f$ is the return value of out_degree(); and
    //!
    //! \complexity
    //! At worst \f$O(n)\f$ where \f$n\f$ equals out_degree().
    //!
    //! \throws LibsemigroupsException if \p v does not represent a node in \c
    //! this.
    //!
    //! \sa next_neighbor_no_checks.
    // Not noexcept because next_neighbor_no_checks is not
    std::pair<node_type, label_type> inline next_neighbor(node_type  v,
                                                          label_type i) const {
      word_graph::validate_node(*this, v);
      return next_neighbor_no_checks(v, i);
    }

    //! Returns the number of nodes.
    //!
    //! \returns
    //! The number of nodes, a value of type \c Node.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \par Parameters
    //! (None)
    Node inline number_of_nodes() const noexcept {
      return _nr_nodes;
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    WordGraph& number_of_active_nodes(size_type val) {
      _num_active_nodes = val;
      return *this;
    }

    size_type inline number_of_active_nodes() const noexcept {
      return _num_active_nodes;
    }
#endif

    //! Returns the number of edges.
    //!
    //! \returns
    //! The total number of edges, a value of type \c size_t.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! \f$O(mn)\f$ where \c m is number_of_nodes() and \c n is out_degree().
    //!
    //! \par Parameters
    //! (None)
    // Not noexcept because std::count isn't
    size_t number_of_edges() const;

    //! Returns the number of edges with given source node.
    //!
    //! \param n the node.
    //!
    //! \returns
    //! A value of type \c size_t.
    //!
    //! \throws LibsemigroupsException if \p n is not a node.
    //!
    //! \complexity
    //! \f$O(n)\f$ where \c n is out_degree().
    // TODO no_checks version
    size_t number_of_edges(node_type n) const {
      word_graph::validate_node(*this, n);
      return out_degree()
             - std::count(_dynamic_array_2.cbegin_row(n),
                          _dynamic_array_2.cend_row(n),
                          UNDEFINED);
    }

    //! Returns the out-degree.
    //!
    //! \returns
    //! The number of out-edges of every node, a value of type \c Node.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \par Parameters
    //! (None)
    Node out_degree() const noexcept {
      return _degree;
    }

    //! Check every node has exactly out_degree() out-edges.
    //!
    //! \returns
    //! A \c bool.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! \f$O(mn)\f$ where \c m is number_of_nodes() and \c n is out_degree().
    //!
    //! \par Parameters
    //! (None)
    // TODO rename this to is_complete and move to helper namespace
    bool validate() const noexcept {
      return number_of_edges() == number_of_nodes() * out_degree();
    }

    //! Returns a random access iterator pointing at the first node of the
    //! digraph.
    //!
    //! \returns
    //! An \ref const_iterator_nodes.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \par Parameters
    //! (None)
    const_iterator_nodes cbegin_nodes() const noexcept {
      // TODO remove IntegralRange
      return IntegralRange<Node>(0, number_of_nodes()).cbegin();
    }

    // no except correct?
    auto nodes() const noexcept {
      return rx::seq() | rx::take(number_of_nodes());
    }

    // no except correct?
    auto labels() const noexcept {
      return rx::seq() | rx::take(out_degree());
    }

    //! Returns a random access iterator pointing at the last node of the
    //! digraph.
    //!
    //! \returns
    //! An \ref const_reverse_iterator_nodes.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \par Parameters
    //! (None)
    // TODO remove (can just use node() | reverse())
    const_reverse_iterator_nodes crbegin_nodes() const noexcept {
      // TODO remove IntegralRange
      return IntegralRange<Node>(0, number_of_nodes()).crbegin();
    }

    //! Returns a random access iterator pointing one-past-the-first node of
    //! the digraph.
    //!
    //! \returns
    //! An \ref const_reverse_iterator_nodes.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \par Parameters
    //! (None)
    // TODO remove (can just use node() | reverse())
    const_reverse_iterator_nodes crend_nodes() const noexcept {
      // TODO remove IntegralRange
      return IntegralRange<Node>(0, number_of_nodes()).crend();
    }

    //! Returns a random access iterator pointing one-past-the-last node of the
    //! digraph.
    //!
    //! \returns
    //! An \ref const_iterator_nodes.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \par Parameters
    //! (None)
    const_iterator_nodes cend_nodes() const noexcept {
      // TODO remove IntegralRange
      return IntegralRange<Node>(0, number_of_nodes()).cend();
    }

    //! Returns a random access iterator pointing at the first neighbor of a
    //! node.
    //!
    //! \param i a node in the digraph.
    //!
    //! \returns
    //! A \ref const_iterator_edges.
    //!
    //! \throws LibsemigroupsException if \p i is not valid.
    //!
    //! \complexity
    //! Constant.
    // Not noexcept because validate_node isn't
    const_iterator_edges cbegin_edges(node_type i) const {
      word_graph::validate_node(*this, i);
      return cbegin_edges_no_checks(i);
    }

    //! Returns a random access iterator pointing at the first neighbor of a
    //! node.
    //!
    //! \param i a node in the digraph.
    //!
    //! \returns
    //! A \ref const_iterator_edges.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \warning
    //! No checks whatsoever on the validity of the arguments are performed.
    //!
    //! \sa
    //! \ref cbegin_edges.
    const_iterator_edges cbegin_edges_no_checks(node_type i) const noexcept {
      return _dynamic_array_2.cbegin_row(i);
    }

    //! Returns a random access iterator pointing one-past-the-last neighbor of
    //! a node.
    //!
    //! \param i a node in the digraph.
    //!
    //! \returns
    //! A \ref const_iterator_edges.
    //!
    //! \throws LibsemigroupsException if \p i is not valid.
    //!
    //! \complexity
    //! Constant.
    // Not noexcept because validate_node isn't
    const_iterator_edges cend_edges(node_type i) const {
      word_graph::validate_node(*this, i);
      return cend_edges_no_checks(i);
    }

    //! Returns a random access iterator pointing one-past-the-last neighbor of
    //! a node.
    //!
    //! \param i a node in the digraph.
    //!
    //! \returns
    //! A \ref const_iterator_edges.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    //!
    //! \warning
    //! No checks whatsoever on the validity of the arguments are performed.
    //!
    //! \sa
    //! \ref cend_edges.
    const_iterator_edges cend_edges_no_checks(node_type i) const noexcept {
      return _dynamic_array_2.cbegin_row(i) + _degree;
    }

    ////////////////////////////////////////////////////////////////////////
    // WordGraph - strongly connected components - public
    ////////////////////////////////////////////////////////////////////////

   protected:
    // TODO(v3) make this public, doc, and test it
    // TODO rename permute_nodes_no_checks
    template <typename S>
    void apply_row_permutation(S const& p) {
      _dynamic_array_2.apply_row_permutation(p);
    }

   private:
    ////////////////////////////////////////////////////////////////////////
    // WordGraph - data members - private
    ////////////////////////////////////////////////////////////////////////

    Node                                _degree;
    Node                                _nr_nodes;
    Node                                _num_active_nodes;
    mutable detail::DynamicArray2<Node> _dynamic_array_2;
  };

  namespace detail {
    template <typename Thing>
    struct IsWordGraphHelper : std::false_type {};

    template <typename Node>
    struct IsWordGraphHelper<WordGraph<Node>> : std::true_type {};
  }  // namespace detail

  template <typename Thing>
  static constexpr bool IsWordGraph = detail::IsWordGraphHelper<Thing>::value;

  //////////////////////////////////////////////////////////////////////////
  // WordGraph - constructor/destructor implementations
  //////////////////////////////////////////////////////////////////////////

  //! Output the edges of an WordGraph to a stream.
  //!
  //! This function outputs the action digraph \p ad to the stream \p os. The
  //! digraph is represented by the out-neighbours of each node ordered
  //! according to their labels. The symbol `-` is used to denote that an edge
  //! is not defined. For example, the digraph with 1 nodes, out-degree 2, and
  //! a single loop labelled 1 from node 0 to 0 is represented as `{{-, 0}}`.
  //!
  //! \param os the ostream
  //! \param ad the action digraph
  //!
  //! \returns
  //! The first parameter \p os.
  //!
  //! \exceptions
  //! \no_libsemigroups_except
  template <typename Node>
  std::ostream& operator<<(std::ostream& os, WordGraph<Node> const& ad) {
    os << "{";
    std::string sep_n;
    for (auto n = ad.cbegin_nodes(); n != ad.cend_nodes(); ++n) {
      std::string sep_e;
      os << sep_n << "{";
      for (auto e = ad.cbegin_edges(*n); e != ad.cend_edges(*n); ++e) {
        os << sep_e << (*e == UNDEFINED ? "-" : std::to_string(*e));
        sep_e = ", ";
      }
      os << "}";
      sep_n = ", ";
    }
    os << "}";
    return os;
  }

  namespace word_graph {
    namespace detail {

#ifdef LIBSEMIGROUPS_EIGEN_ENABLED
      template <typename Node>
      void init_adjacency_matrix(
          WordGraph<Node> const&                                 ad,
          Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& mat) {
        size_t const N = ad.number_of_nodes();
        mat.resize(N, N);
        mat.fill(0);
      }

      static inline void
      identity(Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& x) {
        x.fill(0);
        for (size_t i = 0; i < static_cast<size_t>(x.rows()); ++i) {
          x(i, i) = 1;
        }
      }

#else
      template <typename Node>
      void init_adjacency_matrix(WordGraph<Node> const& ad,
                                 IntMat<0, 0, int64_t>& mat) {
        size_t const N = ad.number_of_nodes();
        mat            = IntMat<0, 0, int64_t>(N, N);
        std::fill(mat.begin(), mat.end(), 0);
      }
#endif
    }  // namespace detail

#ifdef LIBSEMIGROUPS_EIGEN_ENABLED
    static inline Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>
    pow(Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> const& x,
        size_t                                                       e) {
      using Mat = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
      return Eigen::MatrixPower<Mat>(x)(e);
    }
#endif

    template <typename Node>
    auto adjacency_matrix(WordGraph<Node> const& ad) {
      using Mat = typename WordGraph<Node>::adjacency_matrix_type;
      Mat mat;
      detail::init_adjacency_matrix(ad, mat);

      for (auto n = ad.cbegin_nodes(); n != ad.cend_nodes(); ++n) {
        for (auto e = ad.cbegin_edges(*n); e != ad.cend_edges(*n); ++e) {
          if (*e != UNDEFINED) {
            mat(*n, *e) += 1;
          }
        }
      }
      return mat;
    }
    // TODO doc
    // TODO tests
    // TODO return a range here not an unordered_set
    template <typename Node>
    std::unordered_set<Node>
    nodes_reachable_from(WordGraph<Node> const&              ad,
                         typename WordGraph<Node>::node_type source) {
      using node_type = typename WordGraph<Node>::node_type;

      std::unordered_set<node_type> seen;
      std::stack<node_type>         stack;
      stack.push(source);

      while (!stack.empty()) {
        auto n = stack.top();
        stack.pop();
        if (seen.insert(n).second) {
          for (auto it = ad.cbegin_edges(n); it != ad.cend_edges(n); ++it) {
            if (*it != UNDEFINED) {
              stack.push(*it);
            }
          }
        }
      }
      return seen;
    }

    // TODO doc
    // TODO tests
    template <typename T>
    size_t
    number_of_nodes_reachable_from(WordGraph<T> const&              ad,
                                   typename WordGraph<T>::node_type source) {
      return nodes_reachable_from(ad, source).size();
    }

    //! Find the node that a path starting at a given node leads to.
    //!
    //! \tparam T the type used as the template parameter for the WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //! \param first the starting node.
    //! \param path the path to follow.
    //!
    //! \returns
    //! A value of type WordGraph::node_type. If one or more edges in
    //! \p path are not defined, then \ref UNDEFINED is returned.
    //!
    //! \throw LibsemigroupsException if \p first is not a node in the digraph
    //! or \p path contains a value that is not an edge-label.
    //!
    //! \par Complexity
    //! Linear in the length of \p path.
    // TODO(later) example
    // not noexcept because WordGraph::neighbor isn't
    template <typename T>
    node_type<T> follow_path(WordGraph<T> const& ad,
                             node_type<T> const  first,
                             word_type const&    path) {
      // first is validated in neighbor
      node_type<T> last = first;
      for (auto it = path.cbegin(); it < path.cend() && last != UNDEFINED;
           ++it) {
        last = ad.neighbor(last, *it);
      }
      return last;
    }

    // TODO(later) follow_path non-nc version for iterators

    //! Follow the path from a specified node labelled by a word.
    //!
    //! This function returns the last node on the path in the action digraph
    //! \p ad starting at the node \p from labelled by \f$[first, last)\f$ or
    //! libsemigroups::UNDEFINED.
    //!
    //! \param ad an action digraph
    //! \param from the source node
    //! \param first iterator into a word
    //! \param last iterator into a word.
    //!
    //! \exception
    //! \noexcept
    //!
    //! \returns A value of type WordGraph::node_type.
    //!
    //! \complexity
    //! At worst the distance from \p first to \p last.
    //!
    //! \warning
    //! No checks on the arguments of this function are performed.
    namespace detail {  // TODO put this in stl or somewhere else
      template <typename A,
                typename B,
                typename = decltype(std::declval<A>() <= std::declval<B>())>
      struct has_less_equal : std::true_type {};
    }  // namespace detail

    template <typename T, typename S>
    node_type<T> follow_path_no_checks(WordGraph<T> const& ad,
                                       node_type<T>        from,
                                       S                   first,
                                       S                   last) noexcept {
      if constexpr (detail::has_less_equal<S, S>::value) {
        if (last <= first) {
          return from;
        }
      }
      node_type<T> to = from;
      for (auto it = first; it != last && to != UNDEFINED; ++it) {
        to = ad.neighbor_no_checks(to, *it);
      }
      return to;
    }

    //! Follow the path from a specified node labelled by a word.
    //!
    //! This function returns the last node on the path in the action digraph
    //! \p ad starting at the node \p from labelled by \p path or
    //! libsemigroups::UNDEFINED.
    //!
    //! \tparam T the node type of the action digraph
    //!
    //! \param ad an action digraph
    //! \param from the source node
    //! \param path the word
    //!
    //! \exception
    //! \noexcept
    //!
    //! \returns A value of type WordGraph::node_type.
    //!
    //! \complexity
    //! At worst the length of \p path.
    //!
    //! \warning
    //! No checks on the arguments of this function are performed.
    template <typename T>
    node_type<T> follow_path_no_checks(WordGraph<T> const& ad,
                                       node_type<T> const  from,
                                       word_type const&    path) noexcept {
      return follow_path_no_checks(ad, from, path.cbegin(), path.cend());
    }

    //! Returns the last node on the path labelled by a word and an iterator to
    //! the position in the word reached.
    //!
    //! \tparam T the node type of the action digraph
    //! \tparam S the type of the iterators into a word
    //!
    //! \param ad an action digraph
    //! \param from the source node
    //! \param first iterator into a word
    //! \param last iterator into a word.
    //!
    //! \exception
    //! \noexcept
    //!
    //! \returns A pair consisting of WordGraph::node_type and \p S.
    //!
    //! \complexity
    //! At worst the distance from \p first to \p last.
    //!
    //! \warning
    //! No checks on the arguments of this function are performed.
    template <typename T, typename S>
    std::pair<node_type<T>, S>
    last_node_on_path_no_checks(WordGraph<T> const& ad,
                                node_type<T>        from,
                                S                   first,
                                S                   last) noexcept {
      auto         it   = first;
      node_type<T> prev = from;
      node_type<T> to   = from;
      for (; it < last && to != UNDEFINED; ++it) {
        prev = to;
        to   = ad.neighbor_no_checks(to, *it);
      }
      if (it != last || to == UNDEFINED) {
        LIBSEMIGROUPS_ASSERT(prev != UNDEFINED);
        return {prev, it - 1};
      } else {
        return {to, it};
      }
    }

    //! Returns the last node on the path labelled by a word and an iterator to
    //! the position in the word reached.
    //!
    //! \tparam T the node type of the action digraph
    //! \tparam S the type of the iterators into a word
    //!
    //! \param ad an action digraph
    //! \param from the source node
    //! \param first iterator into a word
    //! \param last iterator into a word.
    //!
    //! \throws LibsemigroupsException if any of the letters in word `[first,
    //! last)` is out of bounds.
    //!
    //! \returns A pair consisting of WordGraph::node_type and \p S.
    //!
    //! \complexity
    //! At worst the distance from \p first to \p last.
    template <typename T, typename S>
    std::pair<node_type<T>, S> last_node_on_path(WordGraph<T> const& ad,
                                                 node_type<T>        from,
                                                 S                   first,
                                                 S                   last) {
      auto         it   = first;
      node_type<T> prev = from;
      node_type<T> to   = from;
      size_t const n    = ad.out_degree();
      for (; it < last && to != UNDEFINED; ++it) {
        prev = to;
        if (*it >= n) {
          to = UNDEFINED;
        } else {
          to = ad.neighbor_no_checks(to, *it);
        }
      }
      if (it != last || to == UNDEFINED) {
        LIBSEMIGROUPS_ASSERT(prev != UNDEFINED);
        return {prev, it - 1};
      } else {
        return {to, it};
      }
    }

    namespace detail {
      template <typename T>
      using stack_type  = std::stack<std::pair<node_type<T>, label_type<T>>>;
      using lookup_type = std::vector<uint8_t>;

      template <typename T>
      using topological_sort_type = std::vector<node_type<T>>;

      // Helper function for the two versions of is_acyclic below.
      // Not noexcept because std::stack::emplace isn't
      // This function does not really need to exist any longer, since
      // topological_sort can be used for the same computation, but we retain
      // it because it was already written and uses less space than
      // topological_sort.
      template <typename T>
      bool is_acyclic(WordGraph<T> const& ad,
                      std::stack<T>&      stck,
                      std::vector<T>&     preorder,
                      T&                  next_preorder_num,
                      std::vector<T>&     postorder,
                      T&                  next_postorder_num) {
        size_t const M = ad.out_degree();
        size_t const N = ad.number_of_nodes();
        node_type<T> v;
        while (!stck.empty()) {
          v = stck.top();
          stck.pop();
          if (v >= N) {
            postorder[v - N] = next_postorder_num++;
          } else {
            if (preorder[v] < next_preorder_num && postorder[v] == N) {
              // v is an ancestor of some vertex later in the search
              return false;
            } else if (preorder[v] == N) {
              // not seen v before
              preorder[v] = next_preorder_num++;
              // acts as a divider, so that we know when we've stopped
              // processing the out-neighbours of v
              stck.push(N + v);
              for (size_t label = 0; label < M; ++label) {
                auto w = ad.neighbor_no_checks(v, label);
                if (w != UNDEFINED) {
                  stck.push(w);
                }
              }
            }
          }
        }
        return true;
      }

      // helper function for the two public methods below
      template <typename T>
      bool topological_sort(WordGraph<T> const&       ad,
                            stack_type<T>&            stck,
                            lookup_type&              seen,
                            topological_sort_type<T>& order) {
        node_type<T>  m;
        node_type<T>  n;
        label_type<T> e;
      dive:
        LIBSEMIGROUPS_ASSERT(!stck.empty());
        LIBSEMIGROUPS_ASSERT(seen[stck.top().first] == 0);
        m       = stck.top().first;
        seen[m] = 2;
        e       = 0;
        do {
        rise:
          std::tie(n, e) = ad.next_neighbor_no_checks(m, e);
          if (n != UNDEFINED) {
            if (seen[n] == 0) {
              // never saw this node before, so dive
              stck.emplace(n, 0);
              goto dive;
            } else if (seen[n] == 1) {
              // => all descendants of n prev. explored and no cycles found
              // => try the next neighbour of m.
              ++e;
            } else {
              LIBSEMIGROUPS_ASSERT(seen[n] == 2);
              // => n is an ancestor and a descendant of m
              // => there's a cycle
              order.clear();
              return false;
            }
          }
        } while (e < ad.out_degree());
        // => all descendants of m were explored, and no cycles were found
        // => backtrack
        seen[m] = 1;
        order.push_back(m);
        stck.pop();
        if (stck.size() == 0) {
          return true;
        } else {
          m = stck.top().first;
          e = stck.top().second;
          goto rise;
        }
      }
    }  // namespace detail

    //! Check if a digraph is acyclic.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //!
    //! \returns
    //! A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! WordGraph \p ad and \f$n\f$ is the number of edges. Note that for
    //! WordGraph objects the number of edges is always at most \f$mk\f$
    //! where \f$k\f$ is the WordGraph::out_degree.
    //!
    //! A digraph is acyclic if every directed cycle on the digraph is
    //! trivial.
    //!
    //! \par Example
    //! \code
    //! WordGraph<size_t> ad;
    //! ad.add_nodes(2);
    //! ad.add_to_out_degree(1);
    //! ad.add_edge(0, 1, 0);
    //! ad.add_edge(1, 0, 0);
    //! word_graph::is_acyclic(ad); // returns false
    //! \endcode
    // Not noexcept because detail::is_acyclic isn't
    template <typename T>
    bool is_acyclic(WordGraph<T> const& ad) {
      if (ad.validate()) {
        return false;
      }
      auto const     N = ad.number_of_nodes();
      std::stack<T>  stck;
      std::vector<T> preorder(N, N);
      T              next_preorder_num = 0;
      std::vector<T> postorder(N, N);
      T              next_postorder_num = 0;

      for (node_type<T> m = 0; m < N; ++m) {
        if (preorder[m] == N) {
          stck.push(m);
          if (!detail::is_acyclic(ad,
                                  stck,
                                  preorder,
                                  next_preorder_num,
                                  postorder,
                                  next_postorder_num)) {
            return false;
          }
        }
      }
      return true;
    }

    //! Returns the nodes of the digraph in topological order (see below) if
    //! possible.
    //!
    //! If it is not empty, the returned vector has the property that if an
    //! edge from a node \c n points to a node \c m, then \c m occurs before
    //! \c n in the vector.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //!
    //! \returns
    //! A std::vector<WordGraph<T>::node_type> that contains the nodes of
    //! \p ad in topological order (if possible) and is otherwise empty.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! WordGraph \p ad and \f$n\f$ is the number of edges. Note that for
    //! WordGraph objects the number of edges is always at most \f$mk\f$
    //! where \f$k\f$ is the WordGraph::out_degree.
    template <typename T>
    detail::topological_sort_type<T> topological_sort(WordGraph<T> const& ad) {
      using node_type = typename WordGraph<T>::node_type;

      detail::topological_sort_type<T> order;
      if (ad.validate()) {
        return order;
      }

      size_t const          N = ad.number_of_nodes();
      detail::stack_type<T> stck;
      std::vector<uint8_t>  seen(N, 0);

      for (node_type m = 0; m < N; ++m) {
        if (seen[m] == 0) {
          stck.emplace(m, 0);
          if (!detail::topological_sort(ad, stck, seen, order)) {
            // digraph is not acyclic and so there's no topological order for
            // the nodes.
            LIBSEMIGROUPS_ASSERT(order.empty());
            return order;
          }
        }
      }
      LIBSEMIGROUPS_ASSERT(order.size() == ad.number_of_nodes());
      return order;
    }

    //! Returns the nodes of the digraph reachable from a given node in
    //! topological order (see below) if possible.
    //!
    //! If it is not empty, the returned vector has the property that
    //! if an edge from a node \c n points to a node \c m, then \c m occurs
    //! before \c n in the vector, and the last item in the vector is \p
    //! source.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //! \param source the source node.
    //!
    //! \returns
    //! A std::vector<WordGraph<T>::node_type> that contains the nodes of
    //! \p ad in topological order (if possible) and is otherwise empty.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! At worst \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! subdigraph of those nodes reachable from \p source
    //! and \f$n\f$ is the number of edges.
    template <typename T>
    detail::topological_sort_type<T> topological_sort(WordGraph<T> const& ad,
                                                      node_type<T> source) {
      detail::topological_sort_type<T> order;
      if (ad.validate()) {
        return order;
      }
      size_t const          N = ad.number_of_nodes();
      detail::stack_type<T> stck;
      std::vector<uint8_t>  seen(N, 0);

      stck.emplace(source, 0);
      detail::topological_sort(ad, stck, seen, order);
      return order;
    }

    //! Check if the subdigraph induced by the nodes reachable from a source
    //! node is acyclic.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //! \param source the source node.
    //!
    //! \returns
    //! A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! WordGraph \p ad and \f$n\f$ is the number of edges. Note that for
    //! WordGraph objects the number of edges is always at most \f$mk\f$
    //! where \f$k\f$ is the WordGraph::out_degree.
    //!
    //! A digraph is acyclic if every directed cycle on the digraph is
    //! trivial.
    //!
    //! \par Example
    //! \code
    //! WordGraph<size_t> ad;
    //! ad.add_nodes(4);
    //! ad.add_to_out_degree(1);
    //! ad.add_edge(0, 1, 0);
    //! ad.add_edge(1, 0, 0);
    //! ad.add_edge(2, 3, 0);
    //! word_graph::is_acyclic(ad); // returns false
    //! word_graph::is_acyclic(ad, 0); // returns false
    //! word_graph::is_acyclic(ad, 1); // returns false
    //! word_graph::is_acyclic(ad, 2); // returns true
    //! word_graph::is_acyclic(ad, 3); // returns true
    //! \endcode
    // Not noexcept because detail::is_acyclic isn't
    template <typename T>
    bool is_acyclic(WordGraph<T> const& ad, node_type<T> source) {
      validate_node(ad, source);
      auto const    N = ad.number_of_nodes();
      std::stack<T> stck;
      stck.push(source);
      std::vector<T> preorder(N, N);
      T              next_preorder_num = 0;
      std::vector<T> postorder(N, N);
      T              next_postorder_num = 0;
      return detail::is_acyclic(
          ad, stck, preorder, next_preorder_num, postorder, next_postorder_num);
    }

    //! Check if there is a path from one node to another.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //! \param source the source node.
    //! \param target the source node.
    //!
    //! \returns
    //! A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! WordGraph \p ad and \f$n\f$ is the number of edges. Note that for
    //! WordGraph objects the number of edges is always at most \f$mk\f$
    //! where \f$k\f$ is the WordGraph::out_degree.
    //!
    //! \note
    //! If \p source and \p target are equal, then, by convention, we consider
    //! \p target to be reachable from \p source, via the empty path.
    //!
    //! \par Example
    //! \code
    //! WordGraph<size_t> ad;
    //! ad.add_nodes(4);
    //! ad.add_to_out_degree(1);
    //! ad.add_edge(0, 1, 0);
    //! ad.add_edge(1, 0, 0);
    //! ad.add_edge(2, 3, 0);
    //! word_graph::is_reachable(ad, 0, 1); // returns true
    //! word_graph::is_reachable(ad, 1, 0); // returns true
    //! word_graph::is_reachable(ad, 1, 2); // returns false
    //! word_graph::is_reachable(ad, 2, 3); // returns true
    //! word_graph::is_reachable(ad, 3, 2); // returns false
    //! \endcode
    template <typename T>
    bool is_reachable(WordGraph<T> const& ad,
                      node_type<T> const  source,
                      node_type<T> const  target) {
      validate_node(ad, source);
      validate_node(ad, target);
      if (source == target) {
        return true;
      }
      label_type<T>             edge = 0;
      std::stack<node_type<T>>  nodes;
      std::stack<label_type<T>> edges;
      std::vector<bool>         seen(ad.number_of_nodes(), false);
      nodes.push(source);
      seen[source] = true;

      do {
        node_type<T> node;
        std::tie(node, edge) = ad.next_neighbor_no_checks(nodes.top(), edge);
        if (node == target) {
          return true;
        } else if (node != UNDEFINED) {
          if (!seen[node]) {
            // dive, dive, dive!!
            seen[node] = true;
            nodes.push(node);
            edges.push(edge);
            edge = 0;
          } else {
            ++edge;
          }
        } else {
          // backtrack
          nodes.pop();
          if (!edges.empty()) {
            edge = edges.top();
            edges.pop();
          }
        }
      } while (!nodes.empty());
      return false;
    }

    template <typename T>
    bool is_acyclic(WordGraph<T> const& ad,
                    node_type<T>        source,
                    node_type<T>        target) {
      validate_node(ad, source);
      validate_node(ad, target);
      if (!is_reachable(ad, source, target)) {
        return true;
      }
      auto const    N = ad.number_of_nodes();
      std::stack<T> stck;
      stck.push(source);
      std::vector<T> preorder(N, N);
      T              next_preorder_num = 0;
      std::vector<T> postorder(N, N);
      T              next_postorder_num = 0;
      // TODO(later) there should be a better way of doing this
      for (auto it = ad.cbegin_nodes(); it != ad.cend_nodes(); ++it) {
        if (!is_reachable(ad, *it, target)) {
          preorder[*it] = N + 1;
        }
      }
      return detail::is_acyclic(
          ad, stck, preorder, next_preorder_num, postorder, next_postorder_num);
    }

    //! Adds a cycle involving the specified range of nodes.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph. \tparam U the type of an iterator pointing to nodes of
    //! an WordGraph
    //!
    //! \param ad the WordGraph object to add a cycle to.
    //! \param first a const iterator to nodes of \p ad
    //! \param last a const iterator to nodes of \p ad
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m)\f$ where \f$m\f$ is the distance between \p first and \p last.
    //!
    //! \note
    //! The edges added by this function are all labelled \c 0.
    template <typename T, typename U>
    void add_cycle(WordGraph<T>& ad, U first, U last) {
      for (auto it = first; it < last - 1; ++it) {
        ad.add_edge(*it, *(it + 1), 0);
      }
      ad.add_edge(*(last - 1), *first, 0);
    }

    //! Adds a cycle consisting of \p N new nodes.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to add a cycle to.
    //! \param N the length of the cycle and number of new nodes to add.
    //!
    //! \returns
    //! (None)
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(N)\f$ where \f$N\f$ is the second parameter.
    //!
    //! \note
    //! The edges added by this function are all labelled \c 0.
    template <typename T>
    void add_cycle(WordGraph<T>& ad, size_t N) {
      size_t M = ad.number_of_nodes();
      ad.add_nodes(N);
      add_cycle(ad, ad.cbegin_nodes() + M, ad.cend_nodes());
    }

    namespace detail {
      template <typename T>
      std::string to_string(WordGraph<T>& ad) {
        std::string out = "WordGraph<size_t> ad;\n";
        out += "ad.add_nodes(" + std::to_string(ad.number_of_nodes()) + ");\n";
        out += "ad.add_to_out_degree(" + std::to_string(ad.out_degree())
               + ");\n";
        for (auto n = ad.cbegin_nodes(); n < ad.cend_nodes(); ++n) {
          for (auto e = ad.cbegin_edges(*n); e < ad.cend_edges(*n); ++e) {
            if (*e != UNDEFINED) {
              out += "ad.add_edge(" + std::to_string(*n) + ", "
                     + std::to_string(*e) + ", "
                     + std::to_string(e - ad.cbegin_edges(*n)) + ");\n";
            }
          }
        }
        return out;
      }
    }  // namespace detail

    //! Check if a digraph is connected.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //!
    //! \returns
    //! A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! WordGraph \p ad and \f$n\f$ is the number of edges. Note that for
    //! WordGraph objects the number of edges is always at most \f$mk\f$
    //! where \f$k\f$ is the WordGraph::out_degree.
    //!
    //! A digraph is *connected* if for every pair of nodes \f$u\f$ and
    //! \f$v\f$ there exists a sequence \f$u_0 := u, \ldots, u_{n - 1} := v\f$
    //! such that either  \f$(u_i, u_{i + 1})\f$ or \f$(u_{i + 1}, u_i)\f$ is
    //! an edge. Note that \f$u\f$ and \f$v\f$ can be equal, and the sequence
    //! above can be of length \f$0\f$.
    //!
    //! \par Example
    //! \code
    //! auto ad = to_word_graph<uint8_t>(
    //!     5, {{0, 0}, {1, 1}, {2}, {3, 3}});
    //! word_graph::is_connected(ad);  // returns false
    //! \endcode
    template <typename T>
    bool is_connected(WordGraph<T> const& ad) {
      using node_type = typename WordGraph<T>::node_type;

      auto const N = ad.number_of_nodes();
      if (N == 0) {
        return true;
      }

      ::libsemigroups::detail::Duf<> uf(N);
      for (node_type n = 0; n < N; ++n) {
        for (auto it = ad.cbegin_edges(n); it != ad.cend_edges(n); ++it) {
          if (*it != UNDEFINED) {
            uf.unite(n, *it);
          }
        }
      }
      return uf.number_of_blocks() == 1;
    }

    //! Check if a digraph is strictly cyclic.
    //!
    //! \tparam T the type used as the template parameter for the
    //! WordGraph.
    //!
    //! \param ad the WordGraph object to check.
    //!
    //! \returns
    //! A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \par Complexity
    //! \f$O(m + n)\f$ where \f$m\f$ is the number of nodes in the
    //! WordGraph \p ad and \f$n\f$ is the number of edges. Note that for
    //! WordGraph objects the number of edges is always at most \f$mk\f$
    //! where \f$k\f$ is the WordGraph::out_degree.
    //!
    //! A digraph is *strictly cyclic* if there exists a node \f$v\f$ from
    //! which every node is reachable (including \f$v\f$). There must be a
    //! path of length at least \f$1\f$ from the original node \f$v\f$ to
    //! itself (i.e. \f$v\f$ is not considered to be reachable from itself by
    //! default).
    //!
    //! \par Example
    //! \code
    //! auto ad = to_word_graph<uint8_t>(
    //!     5, {{0, 0}, {1, 1}, {2}, {3, 3}});
    //! word_graph::is_strictly_cyclic(ad);  // returns false
    //! \endcode
    template <typename T>
    bool is_strictly_cyclic(WordGraph<T> const& ad) {
      using node_type = typename WordGraph<T>::node_type;
      auto const N    = ad.number_of_nodes();

      if (N == 0) {
        return true;
      }

      std::vector<bool> seen(N, false);
      std::stack<T>     stack;

      for (node_type m = 0; m < N; ++m) {
        stack.push(m);
        size_t count = 0;
        while (!stack.empty()) {
          auto n = stack.top();
          stack.pop();
          if (!seen[n]) {
            seen[n] = true;
            if (++count == N) {
              return true;
            }
            for (auto it = ad.cbegin_edges(n); it != ad.cend_edges(n); ++it) {
              if (*it != UNDEFINED) {
                stack.push(*it);
              }
            }
          }
        }
        std::fill(seen.begin(), seen.end(), false);
      }
      return false;
    }

    namespace detail {
      // TODO(now) to tpp file
      template <typename Node>
      bool shortlex_standardize(Node& d, Forest& f) {
        LIBSEMIGROUPS_ASSERT(d.number_of_nodes() != 0);
        LIBSEMIGROUPS_ASSERT(f.number_of_nodes() == 0);

        using node_type = typename Node::node_type;
        f.add_nodes(1);

        node_type    t      = 0;
        size_t const n      = d.out_degree();
        bool         result = false;

        // p : new -> old and q : old -> new
        std::vector<node_type> p(d.number_of_nodes(), 0);
        std::iota(p.begin(), p.end(), 0);
        std::vector<node_type> q(p);

        for (node_type s = 0; s <= t; ++s) {
          for (letter_type x = 0; x < n; ++x) {
            node_type r = d.neighbor_no_checks(p[s], x);
            if (r != UNDEFINED) {
              r = q[r];  // new
              if (r > t) {
                t++;
                f.add_nodes(1);
                if (r > t) {
                  std::swap(p[t], p[r]);
                  std::swap(q[p[t]], q[p[r]]);
                  result = true;
                }
                f.set(t, (s == t ? r : s), x);
              }
            }
          }
        }
        d.permute_nodes_no_checks(p, q);
        return result;
      }

      template <typename Node>
      bool lex_standardize(Node& d, Forest& f) {
        LIBSEMIGROUPS_ASSERT(d.number_of_nodes() != 0);
        LIBSEMIGROUPS_ASSERT(f.number_of_nodes() == 0);

        using node_type  = typename Node::node_type;
        using label_type = typename Node::label_type;

        f.add_nodes(1);

        node_type  s = 0, t = 0;
        label_type x      = 0;
        auto const n      = d.out_degree();
        bool       result = false;

        // p : new -> old and q : old -> new
        std::vector<node_type> p(d.number_of_nodes(), 0);
        std::iota(p.begin(), p.end(), 0);
        std::vector<node_type> q(p);

        // Perform a DFS through d
        while (s <= t) {
          node_type r = d.neighbor_no_checks(p[s], x);
          if (r != UNDEFINED) {
            r = q[r];  // new
            if (r > t) {
              t++;
              f.add_nodes(1);
              if (r > t) {
                std::swap(p[t], p[r]);
                std::swap(q[p[t]], q[p[r]]);
                result = true;
              }
              f.set(t, (s == t ? r : s), x);
              s = t;
              x = 0;
              continue;
            }
          }
          x++;
          if (x == n) {  // backtrack
            x = f.label(s);
            s = f.parent(s);
          }
        }
        d.permute_nodes_no_checks(p, q);
        return result;
      }

      template <typename Node>
      bool recursive_standardize(Node& d, Forest& f) {
        LIBSEMIGROUPS_ASSERT(d.number_of_nodes() != 0);
        LIBSEMIGROUPS_ASSERT(f.number_of_nodes() == 0);

        using node_type = typename Node::node_type;

        f.add_nodes(1);

        std::vector<word_type> words;
        size_t const           n = d.out_degree();
        letter_type            a = 0;
        node_type              s = 0, t = 0;

        std::vector<node_type> p(d.number_of_nodes(), 0);
        std::iota(p.begin(), p.end(), 0);
        std::vector<node_type> q(p);

        size_t max_t = number_of_nodes_reachable_from(d, 0) - 1;

        // TODO move this out of here and use it in the other standardize
        // functions
        auto swap_if_necessary = [&d, &f, &p, &q](node_type const   s,
                                                  node_type&        t,
                                                  letter_type const x) {
          node_type r      = d.neighbor_no_checks(p[s], x);
          bool      result = false;
          if (r != UNDEFINED) {
            r = q[r];  // new
            if (r > t) {
              t++;
              f.add_nodes(1);
              if (r > t) {
                std::swap(p[t], p[r]);
                std::swap(q[p[t]], q[p[r]]);
              }
              result = true;
              f.set(t, (s == t ? r : s), x);
            }
          }
          return result;
        };

        bool result = false;

        while (s <= t) {
          if (swap_if_necessary(s, t, 0)) {
            words.push_back(word_type(t, a));
            result = true;
          }
          s++;
        }
        a++;
        bool new_generator = true;
        int  x, u, w;
        while (a < n && t < max_t) {
          if (new_generator) {
            w = -1;  // -1 is the empty word
            if (swap_if_necessary(0, t, a)) {
              result = true;
              words.push_back({a});
            }
            x             = words.size() - 1;
            u             = words.size() - 1;
            new_generator = false;
          }

          node_type const uu = word_graph::follow_path_no_checks(
              d, 0, words[u].begin(), words[u].end());
          if (uu != UNDEFINED) {
            for (int v = 0; v < x; v++) {
              node_type const uuv = word_graph::follow_path_no_checks(
                  d, uu, words[v].begin(), words[v].end() - 1);
              if (uuv != UNDEFINED) {
                s = q[uuv];
                if (swap_if_necessary(s, t, words[v].back())) {
                  result        = true;
                  word_type nxt = words[u];
                  nxt.insert(nxt.end(), words[v].begin(), words[v].end());
                  words.push_back(std::move(nxt));
                }
              }
            }
          }
          w++;
          if (static_cast<size_t>(w) < words.size()) {
            node_type const ww = word_graph::follow_path_no_checks(
                d, 0, words[w].begin(), words[w].end());
            if (ww != UNDEFINED) {
              s = q[ww];
              if (swap_if_necessary(s, t, a)) {
                result        = true;
                u             = words.size();
                word_type nxt = words[w];
                nxt.push_back(a);
                words.push_back(std::move(nxt));
              }
            }
          } else {
            a++;
            new_generator = true;
          }
        }
        d.permute_nodes_no_checks(p, q);
        return result;
      }
    }  // namespace detail

    // Return value indicates whether or not the graph was modified.
    // TODO(now) to tpp file
    template <typename Graph>
    bool standardize(Graph& d, Forest& f, Order val) {
      static_assert(std::is_base_of_v<WordGraphBase, Graph>,
                    "the template parameter Graph must be "
                    "derived from WordGraphBase");
      if (!f.empty()) {
        f.clear();
      }
      if (d.number_of_nodes() == 0) {
        return false;
      }

      switch (val) {
        case Order::none:
          return false;
        case Order::shortlex:
          return detail::shortlex_standardize(d, f);
        case Order::lex:
          return detail::lex_standardize(d, f);
        case Order::recursive:
          return detail::recursive_standardize(d, f);
        default:
          return false;
      }
    }

    template <typename Graph>
    std::pair<bool, Forest> standardize(Graph& d, Order val = Order::shortlex) {
      static_assert(
          std::is_base_of<WordGraphBase, Graph>::value,
          "the template parameter Graph must be derived from WordGraphBase");
      Forest f;
      bool   result = standardize(d, f, val);
      return std::make_pair(result, f);
    }

    template <typename Node, typename Iterator1, typename Iterator2>
    bool is_complete(WordGraph<Node> const& d,
                     Iterator1              first_node,
                     Iterator2              last_node) {
      size_t const n = d.out_degree();
      for (auto it = first_node; it != last_node; ++it) {
        for (size_t a = 0; a < n; ++a) {
          if (d.neighbor_no_checks(*it, a) == UNDEFINED) {
            return false;
          }
        }
      }
      return true;
    }

    template <typename Node,
              typename Iterator1,
              typename Iterator2,
              typename Iterator3>
    bool is_compatible(WordGraph<Node> const& d,
                       Iterator1              first_node,
                       Iterator2              last_node,
                       Iterator3              first_rule,
                       Iterator3              last_rule) {
      for (auto nit = first_node; nit != last_node; ++nit) {
        for (auto rit = first_rule; rit != last_rule; ++rit) {
          auto l = word_graph::follow_path_no_checks(
              d, *nit, rit->cbegin(), rit->cend());
          if (l == UNDEFINED) {
            return true;
          }
          ++rit;
          auto r = word_graph::follow_path_no_checks(
              d, *nit, rit->cbegin(), rit->cend());
          if (r == UNDEFINED) {
            return true;
          }
          if (l != r) {
            return false;
          }
        }
      }
      return true;
    }

  }  // namespace word_graph

  //! Constructs a digraph from number of nodes and an \c initializer_list.
  //!
  //! This function constructs an WordGraph from its arguments whose
  //! out-degree is specified by the length of the first \c initializer_list
  //! in the 2nd parameter.
  //!
  //! \tparam Node the type of the nodes of the digraph
  //!
  //! \param num_nodes the number of nodes in the digraph.
  //! \param il the out-neighbors of the digraph.
  //!
  //! \returns A value of type WordGraph.
  //!
  //! \throws LibsemigroupsException
  //! if WordGraph<Node>::add_edge throws when adding edges from \p il.
  //!
  //! \complexity
  //! \f$O(mn)\f$ where \f$m\f$ is the length of \p il and \f$n\f$ is the
  //! parameter \p num_nodes.
  //!
  //! \par Example
  //! \code
  //! // Construct an action digraph with 5 nodes and 10 edges (7 specified)
  //! to_word_graph<uint8_t>(
  //!     5, {{0, 0}, {1, 1}, {2}, {3, 3}});
  //! \endcode
  template <typename Node>
  WordGraph<Node>
  to_word_graph(size_t num_nodes,
                    std::initializer_list<std::initializer_list<Node>> il) {
    WordGraph<Node> result(num_nodes, il.begin()->size());
    for (size_t i = 0; i < il.size(); ++i) {
      for (size_t j = 0; j < (il.begin() + i)->size(); ++j) {
        auto val = *((il.begin() + i)->begin() + j);
        if (val != UNDEFINED) {
          result.add_edge(i, *((il.begin() + i)->begin() + j), j);
        }
      }
    }
    return result;
  }

}  // namespace libsemigroups

#include "word-graph.tpp"

#endif  // LIBSEMIGROUPS_WORD_GRAPH_HPP_