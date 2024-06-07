//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2022-2024 James D. Mitchell
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

// This file contains the declaration of a class template for semigroup or
// monoid presentations. The idea is to provide a shallow wrapper around a
// vector of words, with some checks that the vector really defines a
// presentation, (i.e. it's consistent with its alphabet etc), and some related
// functionality.

#ifndef LIBSEMIGROUPS_PRESENTATION_HPP_
#define LIBSEMIGROUPS_PRESENTATION_HPP_

#include <algorithm>         // for reverse, sort
#include <cstring>           // for size_t, strlen
#include <initializer_list>  // for initializer_list
#include <iterator>          // for distance
#include <limits>            // for numeric_limits
#include <map>               // for map
#include <numeric>           // for accumulate
#include <string>            // for basic_string, operator==
#include <tuple>             // for tie, tuple
#include <type_traits>       // for enable_if_t
#include <unordered_map>     // for operator==, operator!=
#include <unordered_set>     // for unordered_set
#include <utility>           // for move, pair
#include <vector>            // for vector, operator!=

#include "adapters.hpp"   // for Hash, EqualTo
#include "constants.hpp"  // for Max, UNDEFINED, operator==
#include "debug.hpp"      // for LIBSEMIGROUPS_ASSERT
#include "exception.hpp"  // for LIBSEMIGROUPS_EXCEPTION
#include "order.hpp"      // for ShortLexCompare
#include "ranges.hpp"     // for seq, operator|, rx, take
#include "ranges.hpp"     // for chain, is_sorted
#include "types.hpp"      // for word_type
#include "ukkonen.hpp"    // for GreedyReduceHelper, Ukkonen
#include "words.hpp"      // for operator+

#include "detail/fmt.hpp"         // for format
#include "detail/formatters.hpp"  // for StaticVector1 formatter
#include "detail/report.hpp"      // for formatter<vector>
#include "detail/string.hpp"      // for maximum_common_prefix
#include "detail/uf.hpp"          // for Duf

namespace libsemigroups {

  //! \defgroup presentations_group Presentations
  //!
  //! This file contains documentation related to semigroup and monoid
  //! presentations in `libsemigroups`.
  //!
  //! There are two classes and two namespaces with functionality related to
  //! presentations:
  //! * \ref Presentation "the Presentation class"
  //! * \ref InversePresentation "the InversePresentation class"
  //! * \ref libsemigroups::presentation "Helper functions for presentations"
  //! * \ref libsemigroups::fpsemigroup "Presentations for standard examples"

  //! No doc
  struct PresentationBase {};

  //! \ingroup presentations_group
  //!
  //! \brief For an implementations of presentations for semigroups or monoids.
  //!
  //! Defined in ``presentation.hpp``.
  //!
  //! This class template can be used to construction presentations for
  //! semigroups or monoids and is intended to be used as the input to other
  //! algorithms in `libsemigroups`. The idea is to provide a shallow wrapper
  //! around a vector of words of type `Word`. We refer to this vector of words
  //! as the rules of the presentation. The Presentation class template also
  //! provide some checks that the rules really define a presentation, (i.e.
  //! it's consistent with its alphabet), and some related functionality is
  //! available in the namespace `libsemigroups::presentation`.
  //!
  //! \tparam Word the type of the underlying words.
  template <typename Word>
  class Presentation : public PresentationBase {
   public:
    //! \brief The type of the words in the rules of a Presentation object.
    using word_type = Word;

    //! \brief The type of the letters in the words that constitute the rules of
    //! a Presentation object.
    using letter_type = typename word_type::value_type;

    //! \brief Type of a const iterator to either side of a rule
    using const_iterator = typename std::vector<word_type>::const_iterator;

    //! \brief Type of an iterator to either side of a rule.
    using iterator = typename std::vector<word_type>::iterator;

    //! \brief Size type for rules.
    using size_type = typename std::vector<word_type>::size_type;

   private:
    word_type                                  _alphabet;
    std::unordered_map<letter_type, size_type> _alphabet_map;
    bool                                       _contains_empty_word;

   public:
    //! \brief Data member holding the rules of the presentation.
    //!
    //! The rules can be altered using the member functions of `std::vector`,
    //! and the presentation can be checked for validity using \ref validate.
    std::vector<word_type> rules;

    //! \brief Default constructor.
    //!
    //! Constructs an empty presentation with no rules and no alphabet.
    Presentation();

    //! \brief Remove the alphabet and all rules.
    //!
    //! This function clears the alphabet and all rules from the presentation,
    //! putting it back into the state it would be in if it was newly
    //! constructed.
    //!
    //! \returns
    //! A reference to `this`.
    Presentation& init();

    //! \brief Default copy constructor.
    //!
    //! Default copy constructor.
    Presentation(Presentation const&);

    //! \brief Default move constructor.
    //!
    //! Default move constructor.
    Presentation(Presentation&&);

    //! \brief Default copy assignment operator.
    //!
    //! Default copy assignment operator.
    Presentation& operator=(Presentation const&);

    //! \brief Default move assignment operator.
    //!
    //! Default move assignment operator.
    Presentation& operator=(Presentation&&);

    ~Presentation();

    //! \brief Returns the alphabet of the presentation.
    //!
    //! Returns the alphabet of the presentation.
    //!
    //! \returns A const reference to \c Presentation::word_type.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    [[nodiscard]] word_type const& alphabet() const noexcept {
      return _alphabet;
    }

    //! \brief Set the alphabet by size.
    //!
    //! Sets the alphabet to the range \f$[0, n)\f$ consisting of values of
    //! type \ref letter_type.
    //!
    //! \param n the size of the alphabet.
    //!
    //! \returns A const reference to \c *this.
    //!
    //! \throws LibsemigroupsException if the value of \p n is greater than the
    //! maximum number of letters supported by \ref letter_type.
    //!
    //! \warning
    //! This function does not verify that the rules in the presentation (if
    //! any) consist of letters belonging to the alphabet.
    //!
    //! \sa validate_alphabet, validate_rules, and \ref validate.
    Presentation& alphabet(size_type n);

    //! \brief Set the alphabet const reference.
    //!
    //! Sets the alphabet to be the letters in \p lphbt.
    //!
    //! \param lphbt the alphabet.
    //!
    //! \returns A const reference to \c *this.
    //!
    //! \throws LibsemigroupsException if there are duplicate letters in \p
    //! lphbt.
    //!
    //! \warning
    //! This function does not verify that the rules in the presentation (if
    //! any) consist of letters belonging to the alphabet.
    //!
    //! \sa validate_rules and \ref validate.
    Presentation& alphabet(word_type const& lphbt);

    //! \brief Set the alphabet from rvalue reference.
    //!
    //! Sets the alphabet to be the letters in \p lphbt.
    //!
    //! \param lphbt the alphabet.
    //!
    //! \returns A const reference to \c *this.
    //!
    //! \throws LibsemigroupsException if there are duplicate letters in \p
    //! lphbt.
    //!
    //! \warning
    //! This function does not verify that the rules in the presentation (if
    //! any) consist of letters belonging to the alphabet.
    //!
    //! \sa validate_rules and \ref validate.
    Presentation& alphabet(word_type&& lphbt);

    //! \brief Set the alphabet to be the letters in the rules.
    //!
    //! Sets the alphabet to be the letters in \ref rules.
    //!
    //! \returns A const reference to \c *this.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! At most \f$O(mn)\f$ where \f$m\f$ is the number of rules, and \f$n\f$ is
    //! the length of the longest rule.
    //!
    //! \sa validate_rules, and \ref validate.
    Presentation& alphabet_from_rules();

    //! \brief Get a letter in the alphabet by index.
    //!
    //! Returns the letter of the alphabet in position \p i.
    //!
    //! \param i the index.
    //!
    //! \returns A value of type \ref letter_type.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! This function performs no bound checks on the argument \p i.
    [[nodiscard]] letter_type letter_no_checks(size_type i) const {
      LIBSEMIGROUPS_ASSERT(i < _alphabet.size());
      return _alphabet[i];
    }

    //! \brief Get a letter in the alphabet by index.
    //!
    //! After checking that \p i is in the range \f$[0, n)\f$, where \f$n\f$ is
    //! the length of the alphabet, this function performs the same as
    //! `letter_no_checks(size_type i) const`.
    //!
    //! \throws LibsemigroupsException if \p i is not in the range \f$[0, n)\f$.
    //!
    //! \sa letter_no_checks.
    [[nodiscard]] letter_type letter(size_type i) const;

    //! \brief Get the index of a letter in the alphabet
    //!
    //! Get the index of a letter in the alphabet
    //!
    //! \param val the letter
    //!
    //! \returns A value of type \ref size_type
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Constant.
    //!
    //! \warning This function does not verify that its argument belongs to the
    //! alphabet.
    [[nodiscard]] size_type index_no_checks(letter_type val) const {
      return _alphabet_map.find(val)->second;
    }

    //! \brief Get the index of a letter in the alphabet
    //!
    //! After checking that \p val is in the the alphabet, this function
    //! performs the same as `index_no_checks(letter_type val) const`.
    //!
    //! \throws LibsemigroupsException if \p val does not belong to the
    //! alphabet.
    //!
    //! \sa \ref index_no_checks.
    [[nodiscard]] size_type index(letter_type val) const;

    //! \brief Check if a letter belongs to the alphabet or not.
    //!
    //! Check if a letter belongs to the alphabet or not.
    //!
    //! \param val the letter to check
    //!
    //! \returns a value of type \c bool.
    //!
    //! \exception
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Constant on average, worst case linear in the size of the alphabet.
    [[nodiscard]] bool in_alphabet(letter_type val) const {
      return _alphabet_map.find(val) != _alphabet_map.cend();
    }

    //! \brief Add a rule to the presentation
    //!
    //! Adds the rule with left-hand side `[lhs_begin, lhs_end)` and
    //! right-hand side `[rhs_begin, rhs_end)` to the rules. It is possible to
    //! add rules directly via the data member \ref rules, this function just
    //! exists to encourage adding rules with both sides defined at the same
    //! time.
    //!
    //! \tparam S the type of the first two parameters (iterators, or
    //! pointers).
    //! \tparam T the type of the second two parameters (iterators,
    //! or pointers).
    //!
    //! \param lhs_begin an iterator pointing to the first letter of the left
    //! hand side of the rule to be added.
    //! \param lhs_end an iterator pointing one past the last letter of the
    //! left-hand side of the rule to be added.
    //! \param rhs_begin an iterator pointing to the first letter of the right
    //! hand side of the rule to be added.
    //! \param rhs_end an iterator pointing one past the last letter of the
    //! right-hand side of the rule to be added
    //!
    //! \returns A const reference to \c *this
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Amortized constant.
    //!
    //! \warning
    //! It is not checked that the arguments describe words over the alphabet
    //! of the presentation.
    //!
    //! \sa
    //! add_rule
    template <typename Iterator1, typename Iterator2>
    Presentation& add_rule_no_checks(Iterator1 lhs_begin,
                                     Iterator1 lhs_end,
                                     Iterator2 rhs_begin,
                                     Iterator2 rhs_end) {
      rules.emplace_back(lhs_begin, lhs_end);
      rules.emplace_back(rhs_begin, rhs_end);
      return *this;
    }

    //! \brief Add a rule to the presentation and check it is valid.
    //!
    //! After checking that they left-hand side and right-hand side only contain
    //! letters in \ref alphabet, this function performs the same as
    //! `add_rule_no_checks(Iterator1 lhs_begin, Iterator1 lhs_end, Iterator2
    //! rhs_begin, Iterator2 rhs_end)`.
    //!
    //! \throws LibsemigroupsException if any letter does not below to the
    //! alphabet.
    //! \throws LibsemigroupsException if \ref contains_empty_word returns \c
    //! false and \p lhs_begin equals \p lhs_end or \p rhs_begin equals \p
    //! rhs_end.
    //!
    //! \sa
    //! add_rule_no_checks.
    template <typename Iterator1, typename Iterator2>
    Presentation& add_rule(Iterator1 lhs_begin,
                           Iterator1 lhs_end,
                           Iterator2 rhs_begin,
                           Iterator2 rhs_end) {
      validate_word(lhs_begin, lhs_end);
      validate_word(rhs_begin, rhs_end);
      return add_rule_no_checks(lhs_begin, lhs_end, rhs_begin, rhs_end);
    }

    //! \brief Check if the presentation should contain the empty word.
    //!
    //! Check if the presentation should contain the empty word.
    //!
    //! \returns A value of type `bool`.
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant.
    [[nodiscard]] bool contains_empty_word() const noexcept {
      return _contains_empty_word;
    }

    //! \brief Specify that the presentation should (not) contain the empty
    //! word.
    //!
    //! Specify that the presentation should (not) contain the empty word.
    //!
    //! \param val whether the presentation should contain the empty word
    //!
    //! \returns A const reference to \c *this
    //!
    //! \exceptions
    //! \noexcept
    //!
    //! \complexity
    //! Constant
    Presentation& contains_empty_word(bool val) noexcept {
      _contains_empty_word = val;
      return *this;
    }

    //! \brief Check if the alphabet is valid.
    //!
    //! Check if the alphabet is valid.
    //!
    //! \throws LibsemigroupsException if there are duplicate letters in the
    //! alphabet.
    //!
    //! \complexity
    //! Linear in the length of the alphabet.
    void validate_alphabet() const {
      decltype(_alphabet_map) alphabet_map;
      validate_alphabet(alphabet_map);
    }

    //! \brief Check if a letter belongs to the alphabet or not.
    //!
    //! Check if a letter belongs to the alphabet or not.
    //!
    //! \param c the letter to check.
    //!
    //! \throws LibsemigroupsException if \p c does not belong to the alphabet.
    //!
    //! \complexity
    //! Constant on average, worst case linear in the size of the alphabet.
    void validate_letter(letter_type c) const;

    //! \brief Check if every letter in a range belongs to the alphabet.
    //!
    //! Check if every letter in a range belongs to the alphabet.
    //!
    //! \tparam T the type of the arguments (iterators)
    //! \param first iterator pointing at the first letter to check
    //! \param last iterator pointing one beyond the last letter to check
    //!
    //! \throws LibsemigroupsException if there is a letter not in the
    //! alphabet between \p first and \p last
    //!
    //! \complexity
    //! Worst case \f$O(mn)\f$ where \f$m\f$ is the length of the longest
    //! word, and \f$n\f$ is the size of the alphabet.
    template <typename Iterator>
    void validate_word(Iterator first, Iterator last) const;

    //! \brief Check if every rule consists of letters belonging to the
    //! alphabet.
    //!
    //! Check if every rule consists of letters belonging to the alphabet.
    //!
    //! \throws LibsemigroupsException if any word contains a letter not in
    //! the alphabet.
    //!
    //! \complexity
    //! Worst case \f$O(mnt)\f$ where \f$m\f$ is the length of the longest
    //! word, \f$n\f$ is the size of the alphabet and \f$t\f$ is the number of
    //! rules.
    void validate_rules() const;

    //! \brief Check if the alphabet and rules are valid.
    //!
    //! Check if the alphabet and rules are valid.
    //!
    //! \throws LibsemigroupsException if \ref validate_alphabet or \ref
    //! validate_rules does.
    //!
    //! \complexity
    //! Worst case \f$O(mnp)\f$ where \f$m\f$ is the length of length of the
    //! word, \f$n\f$ is the size of the alphabet and \f$p\f$ is the number of
    //! rules.
    void validate() const {
      validate_alphabet();
      validate_rules();
    }

   private:
    void try_set_alphabet(decltype(_alphabet_map)& alphabet_map,
                          word_type&               old_alphabet);
    void validate_alphabet(decltype(_alphabet_map)& alphabet_map) const;
  };

  //! \ingroup presentations_group
  //!
  //! \brief Namespace for Presentation helper functions.
  //!
  //! Defined in ``presentation.hpp``.
  //!
  //! This namespace contains various helper functions for the class
  //! \ref Presentation. These functions could be functions of \ref Presentation
  //! but they only use public member functions of \ref Presentation, and so
  //! they are declared as free functions instead.
  namespace presentation {

    //! \brief Validate rules against the alphabet of \p p.
    //!
    //! Check if every rule of in `[first, last)` consists of letters belonging
    //! to the alphabet of \p p.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \tparam Iterator the type of the second and third arguments.
    //! \param p the presentation whose alphabet is being validated against.
    //! \param first iterator pointing at the first rule to check.
    //! \param last iterator pointing one beyond the last rule to check.
    //!
    //! \throws LibsemigroupsException if any word contains a letter not in
    //! `p.alphabet()`.
    //!
    //! \complexity
    //! Worst case \f$O(mnt)\f$ where \f$m\f$ is the length of the longest
    //! word, \f$n\f$ is the size of the \p p 's alphabet and \f$t\f$ is the
    //! distance between \p first and \p last.
    template <typename Word, typename Iterator>
    void validate_rules(Presentation<Word> const& p,
                        Iterator                  first,
                        Iterator                  last) {
      for (auto it = first; it != last; ++it) {
        p.validate_word(it->cbegin(), it->cend());
      }
    }

    //! \brief Validate if \p vals act as semigroup inverses in \p p.
    //!
    //! Check if the values in \p vals act as semigroup inverses for the letters
    //! of the alphabet of \p p. Specifically, it checks that the \f$i\f$th
    //! value in \p vals acts as an inverse for the \f$i\f$th value in
    //! `p.alphabet()`.
    //!
    //! Let \f$x_i\f$ be the \f$i\f$th letter in `p.alphabet()`, and
    //! suppose that \f$x_i=v_j\f$ is in the \f$j\f$th position of \p vals. This
    //! function checks that \f$v_i = x_j\f$, and therefore that
    //! \f$(x_i^{-1})^{-1} = x\f$.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //! \param vals the values to check if the act as inverses.
    //!
    //! \throws Libsemigroups_Exception if any of the following apply:
    //! * the length of \p vals is not the same as the length of `p.alphabet()`
    //! * `p.validate_word(vals)` throws
    //! * \p vals contains duplicate letters
    //! * the values in \p vals do not serve as semigroup inverses.
    template <typename Word>
    void validate_semigroup_inverses(Presentation<Word> const& p,
                                     Word const&               vals);

    //! \brief Add a rule to the presentation by reference.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of the
    //! presentation are performed.
    template <typename Word>
    void add_rule_no_checks(Presentation<Word>& p,
                            Word const&         lhop,
                            Word const&         rhop) {
      p.add_rule_no_checks(lhop.begin(), lhop.end(), rhop.begin(), rhop.end());
    }

    //! \brief Add a rule to the presentation by reference and check.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules, after checking that \p lhop and \p rhop consist entirely
    //! of letters in the alphabet of \p p.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \throws LibsemigroupsException if \p lhop or \p rhop contains any
    //! letters not belonging to `p.alphabet()`.
    template <typename Word>
    void add_rule(Presentation<Word>& p, Word const& lhop, Word const& rhop) {
      p.add_rule(lhop.begin(), lhop.end(), rhop.begin(), rhop.end());
    }

    //! \brief Add a rule to the presentation by `char const*`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of the
    //! presentation are performed.
    inline void add_rule_no_checks(Presentation<std::string>& p,
                                   char const*                lhop,
                                   char const*                rhop) {
      add_rule_no_checks(p, std::string(lhop), std::string(rhop));
    }

    //! \brief Add a rule to the presentation by `char const*`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \throws LibsemigroupsException if \p lhop or \p rhop contains any
    //! letters not belonging to `p.alphabet()`.
    inline void add_rule(Presentation<std::string>& p,
                         char const*                lhop,
                         char const*                rhop) {
      add_rule(p, std::string(lhop), std::string(rhop));
    }

    //! \brief Add a rule to the presentation by `string const &` and `char
    //! const*`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \throws LibsemigroupsException if \p lhop or \p rhop contains any
    //! letters not belonging to `p.alphabet()`.
    inline void add_rule(Presentation<std::string>& p,
                         std::string const&         lhop,
                         char const*                rhop) {
      add_rule(p, lhop, std::string(rhop));
    }

    //! \brief Add a rule to the presentation by `char const*` and `string const
    //! &`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \throws LibsemigroupsException if \p lhop or \p rhop contains any
    //! letters not belonging to `p.alphabet()`.
    inline void add_rule(Presentation<std::string>& p,
                         char const*                lhop,
                         std::string const&         rhop) {
      add_rule(p, std::string(lhop), rhop);
    }

    //! \brief Add a rule to the presentation by `string const&` and `char
    //! const*`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of the
    //! presentation are performed.
    inline void add_rule_no_checks(Presentation<std::string>& p,
                                   std::string const&         lhop,
                                   char const*                rhop) {
      add_rule_no_checks(p, lhop, std::string(rhop));
    }

    //! \brief Add a rule to the presentation by `char const*` and `string
    //! const&`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of the
    //! presentation are performed.
    inline void add_rule_no_checks(Presentation<std::string>& p,
                                   char const*                lhop,
                                   std::string const&         rhop) {
      add_rule_no_checks(p, std::string(lhop), rhop);
    }

    //! \brief Add a rule to the presentation by `initializer_list`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \tparam Letter the type of the values in the `initializer_list`.
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of the
    //! presentation are performed.
    template <typename Word, typename Letter>
    void add_rule_no_checks(Presentation<Word>&           p,
                            std::initializer_list<Letter> lhop,
                            std::initializer_list<Letter> rhop) {
      p.add_rule_no_checks(lhop.begin(), lhop.end(), rhop.begin(), rhop.end());
    }

    //! \brief Add a rule to the presentation by `initializer_list`.
    //!
    //! Adds the rule with left-hand side \p lhop and right-hand side \p rhop
    //! to the rules.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \tparam Letter the type of the values in the `initializer_list`.
    //! \param p the presentation.
    //! \param lhop the left-hand side of the rule.
    //! \param rhop the right-hand side of the rule.
    //!
    //! \throws LibsemigroupsException if \p lhop or \p rhop contains any
    //! letters not belonging to `p.alphabet()`.
    template <typename Word, typename Letter>
    void add_rule(Presentation<Word>&           p,
                  std::initializer_list<Letter> lhop,
                  std::initializer_list<Letter> rhop) {
      p.add_rule(lhop.begin(), lhop.end(), rhop.begin(), rhop.end());
    }

    //! \brief Add a rules to the presentation.
    //!
    //! Adds the rules stored in the range `[first, last)`.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \tparam Iterator the type of the second and third arguments.
    //! \param p the presentation.
    //! \param first iterator pointing at the first rule to add.
    //! \param last iterator pointing one beyond the last rule to add.
    //!
    //! \throws LibsemigroupsException if any rule contains any letters not
    //! belonging to `p.alphabet()`.
    template <typename Word, typename Iterator>
    void add_rules(Presentation<Word>& p, Iterator first, Iterator last) {
      for (auto it = first; it != last; it += 2) {
        add_rule(p, *it, *(it + 1));
      }
    }

    //! \brief Add a rules to the presentation.
    //!
    //! Adds the rules stored in the range `[first, last)`.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \tparam Iterator the type of the second and third arguments.
    //! \param p the presentation.
    //! \param first iterator pointing at the first rule to add.
    //! \param last iterator pointing one beyond the last rule to add.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of the
    //! presentation are performed.
    template <typename Word, typename Iterator>
    void add_rules_no_checks(Presentation<Word>& p,
                             Iterator            first,
                             Iterator            last) {
      for (auto it = first; it != last; it += 2) {
        add_rule_no_checks(p, *it, *(it + 1));
      }
    }

    //! \brief Add a rule to the presentation from another presentation.
    //!
    //! Adds all the rules of the second argument to the first argument
    //! which is modified in-place.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param q the presentation with the rules to add
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    template <typename Word>
    void add_rules_no_checks(Presentation<Word>&       p,
                             Presentation<Word> const& q) {
      add_rules_no_checks(p, q.rules.cbegin(), q.rules.cend());
    }

    //! \brief Check if a presentation contains a rule
    //!
    //! Checks if the rule with left-hand side \p lhs and right-hand side \p rhs
    //! is contained in \p p.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //! \param lhs the left-hand side of the rule.
    //! \param rhs the right-hand side of the rule.
    //!
    //! \returns a value of type `bool`
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \complexity
    //! Linear in the number of rules
    template <typename Word>
    [[nodiscard]] bool contains_rule(Presentation<Word>& p,
                                     Word const&         lhs,
                                     Word const&         rhs);

    //! \brief Add rules for an identity element.
    //!
    //! Adds rules of the form \f$ae = ea = a\f$ for every letter \f$a\f$ in
    //! the alphabet of \p p, and where \f$e\f$ is the second parameter.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param e the identity element
    //!
    //! \throws LibsemigroupsException if \p e is not a letter in
    //! `p.alphabet()`.
    //!
    //! \complexity
    //! Linear in the number of rules
    template <typename Word>
    void add_identity_rules(Presentation<Word>&                      p,
                            typename Presentation<Word>::letter_type e);

    //! \brief Add rules for a zero element.
    //!
    //! Adds rules of the form \f$az = za = z\f$ for every letter \f$a\f$ in
    //! the alphabet of \p p, and where \f$z\f$ is the second parameter.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param z the zero element
    //!
    //! \throws LibsemigroupsException if \p z is not a letter in
    //! `p.alphabet()`.
    //!
    //! \complexity
    //! Linear in the number of rules.
    template <typename Word>
    void add_zero_rules(Presentation<Word>&                      p,
                        typename Presentation<Word>::letter_type z);

    //! \brief Add rules for inverses.
    //!
    //! The letter in \c a with index \c i in \p vals is the inverse of the
    //! letter in `alphabet()` with index \c i. The rules added are \f$a_ib_i
    //! = e\f$ where the alphabet is \f$\{a_1, \ldots, a_n\}\f$; the 2nd
    //! parameter \p vals is \f$\{b_1, \ldots, b_n\}\f$; and \f$e\f$ is the
    //! 3rd parameter.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param vals the inverses
    //! \param e the identity element (defaults to \ref UNDEFINED, meaning use
    //! the empty word)
    //!
    //! \throws LibsemigroupsException if any of the following apply:
    //! * the length of \p vals is not equal to `alphabet().size()`;
    //! * the letters in \p vals are not exactly those in `alphabet()`
    //! (perhaps in a different order);
    //! * \f$(a_i ^ {-1}) ^ {-1} = a_i\f$ does not hold for some \f$i\f$;
    //! * \f$e ^ {-1} = e\f$ does not hold
    //!
    //! \complexity
    //! \f$O(n)\f$ where \f$n\f$ is `p.alphabet().size()`.
    template <typename Word>
    void add_inverse_rules(Presentation<Word>&                      p,
                           Word const&                              vals,
                           typename Presentation<Word>::letter_type e
                           = UNDEFINED);

    //! \brief Add rules for inverses.
    //!
    //! The letter in \c a with index \c i in \p vals is the inverse of the
    //! letter in `alphabet()` with index \c i. The rules added are \f$a_ib_i
    //! = e\f$ where the alphabet is \f$\{a_1, \ldots, a_n\}\f$; the 2nd
    //! parameter \p vals is \f$\{b_1, \ldots, b_n\}\f$; and \f$e\f$ is the
    //! 3rd parameter.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param vals the inverses
    //! \param e the identity element (defaults to \ref UNDEFINED, meaning use
    //! the empty word)
    //!
    //! \throws LibsemigroupsException if any of the following apply:
    //! * the length of \p vals is not equal to `alphabet().size()`;
    //! * the letters in \p vals are not exactly those in `alphabet()`
    //! (perhaps in a different order);
    //! * \f$(a_i ^ {-1}) ^ {-1} = a_i\f$ does not hold for some \f$i\f$;
    //! * \f$e ^ {-1} = e\f$ does not hold
    //!
    //! \complexity
    //! \f$O(n)\f$ where \f$n\f$ is `p.alphabet().size()`.
    inline void add_inverse_rules(Presentation<std::string>& p,
                                  char const*                vals,
                                  char                       e = UNDEFINED) {
      add_inverse_rules(p, std::string(vals), e);
    }

    //! \brief Remove duplicate rules.
    //!
    //! Removes all but one instance of any duplicate rules (if any). Note
    //! that rules of the form \f$u = v\f$ and \f$v = u\f$ (if any) are
    //! considered duplicates. Also note that the rules may be reordered by
    //! this function even if there are no duplicate rules.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    //!
    //! \complexity
    //! Linear in the number of rules.
    template <typename Word>
    void remove_duplicate_rules(Presentation<Word>& p);

    //! \brief Remove rules consisting of identical words.
    //!
    //! Removes all instance of rules (if any) where the left-hand side and
    //! the right-hand side are identical.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    //!
    //! \complexity
    //! Linear in the number of rules.
    template <typename Word>
    void remove_trivial_rules(Presentation<Word>& p);

    //! \brief If there are rules \f$u = v\f$ and \f$v = w\f$ where \f$|w| <
    //! |v|\f$, then replace \f$u = v\f$ by \f$u = w\f$.
    //!
    //! Attempts to reduce the length of the words by finding the equivalence
    //! relation on the relation words generated by the pairs of identical
    //! relation words. If \f$\{u_1, u_2, \ldots, u_n\}\f$ are the distinct
    //! words in an equivalence class and \f$u_1\f$ is the short-lex minimum
    //! word in the class, then the relation words are replaced by \f$u_1 =
    //! u_2, u_1 = u_3, \cdots, u_1 = u_n\f$.
    //!
    //! The rules may be reordered by this function even if there are no
    //! reductions found.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    template <typename Word>
    void reduce_complements(Presentation<Word>& p);

    //! \brief Sort the left-hand and right-hand side of each rule by shortlex.
    //!
    //! Sort each rule \f$u = v\f$ so that the left-hand side is shortlex
    //! greater than the right-hand side.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation whose rules should be sorted.
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    //!
    //! \complexity
    //! Linear in the number of rules.
    template <typename Word>
    bool sort_each_rule(Presentation<Word>& p);

    //! \brief Sort the left-hand and right-hand side of each rule relative to
    //! \p cmp.
    //!
    //! Sort each rule \f$u = v\f$ so that the left-hand side is greater than
    //! the right-hand side with respect to \p cmp.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \tparam Compare the type of the compare function
    //! \param p the presentation whose rules should be sorted.
    //! \param cmp the comparision function
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    //!
    //! \complexity
    //! Linear in the number of rules
    template <typename Word, typename Compare>
    bool sort_each_rule(Presentation<Word>& p, Compare cmp);

    // TODO(later) is_each_rule_sorted?

    //! \brief Sort all of the rules by \p cmp.
    //!
    //! Sort the rules \f$u_1 = v_1, \ldots, u_n = v_n\f$ so that
    //! \f$u_1v_1 < \cdots < u_nv_n\f$ where \f$<\f$ is the order defined by
    //! \p cmp.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \tparam Compare the type of the compare function
    //! \param p the presentation to sort
    //! \param cmp the comparision function
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    template <typename Word, typename Compare>
    void sort_rules(Presentation<Word>& p, Compare cmp);

    //! \brief Sort all of the rules by shortlex.
    //!
    //! Sort the rules \f$u_1 = v_1, \ldots, u_n = v_n\f$ so that
    //! \f$u_1v_1 < \cdots < u_nv_n\f$ where \f$<\f$ is the shortlex order.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to sort
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    template <typename Word>
    void sort_rules(Presentation<Word>& p) {
      sort_rules(p, ShortLexCompare());
    }

    //! \brief Check the rules are sorted relative to \p cmp
    //!
    //! Check if the rules \f$u_1 = v_1, \ldots, u_n = v_n\f$ satisfy
    //! \f$u_1v_1 < \cdots < u_nv_n\f$ where \f$<\f$ is the order described by
    //! \p cmp.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \tparam Compare the type of the compare function
    //! \param p the presentation to check
    //! \param cmp the comparision function
    //!
    //! \returns A value of type `bool`.
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    //!
    //! \sa \ref sort_rules(Presentation<Word>& p, Compare cmp).
    template <typename Word, typename Compare>
    bool are_rules_sorted(Presentation<Word> const& p, Compare cmp);

    //! \brief Check the rules are sorted relative to shortlex
    //!
    //! Check if the rules \f$u_1 = v_1, \ldots, u_n = v_n\f$ satisfy
    //! \f$u_1v_1 < \cdots < u_nv_n\f$ where \f$<\f$ is shortlex order.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to check
    //!
    //! \returns A value of type `bool`.
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    //!
    //! \sa \ref sort_rules(Presentation<Word>& p).
    template <typename Word>
    bool are_rules_sorted(Presentation<Word> const& p) {
      return are_rules_sorted(p, ShortLexCompare());
    }

    //! \brief Returns the longest common subword of the rules.
    //!
    //! If it is possible to find a subword \f$w\f$ of the rules \f$u_1 = v_1,
    //! \ldots, u_n = v_n\f$ such that the introduction of a new generator
    //! \f$z\f$ and the relation \f$z = w\f$ reduces the `presentation::length`
    //! of the presentation, then this function returns the longest such word
    //! \f$w\f$. If no such word can be found, then a word of length \f$0\f$ is
    //! returned.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \returns A value of type \p Word.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    // TODO(later) complexity
    template <typename Word>
    Word longest_subword_reducing_length(Presentation<Word>& p);

    //! \brief Add a generator to \p p.
    //!
    //! Add the first letter not in the alphabet of \p p as a generator of \p p,
    //! and return this letter.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //!
    //! \returns a value of type \ref letter_type.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    template <typename Word>
    typename Presentation<Word>::letter_type
    add_generator(Presentation<Word>& p);

    //! \brief Add \p x as a generator of \p p.
    //!
    //! Add the letter \p x as a generator of \p p.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //! \param x the letter to add as a generator.
    //!
    //! \throws LibsemigroupsException if \p x is in `p.alphabet()`.
    template <typename Word>
    void add_generator_no_checks(Presentation<Word>&                      p,
                                 typename Presentation<Word>::letter_type x);

    //! \brief Add \p x as a generator of \p p.
    //!
    //! Add the letter \p x as a generator of \p p.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p the presentation.
    //! \param x the letter to add as a generator.
    //!
    //! \throws LibsemigroupsException if \p x is in `p.alphabet()`.
    template <typename Word>
    void add_generator(Presentation<Word>&                      p,
                       typename Presentation<Word>::letter_type x);

    //! \brief Replace non-overlapping instances of a subword via iterators.
    //!
    //! If \f$w=\f$`[first, last)` is a word, then this function replaces
    //! every non-overlapping instance of \f$w\f$ in every rule, adds a new
    //! generator \f$z\f$, and the rule \f$w = z\f$. The new generator and
    //! rule are added even if \f$w\f$ is not a subword of any rule.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \tparam T the type of the 2nd and 3rd parameters (iterators)
    //! \param p the presentation
    //! \param first the start of the subword to replace
    //! \param last one beyond the end of the subword to replace
    //!
    //! \throws LibsemigroupsException if `first == last`.
    // TODO(later) complexity
    // TODO do we need this, since there is no other overloaded function with
    // three parameters?
    template <typename Word,
              typename Iterator,
              typename = std::enable_if_t<!std::is_same<Iterator, Word>::value>>
    typename Presentation<Word>::letter_type
    replace_word_with_new_generator(Presentation<Word>& p,
                                    Iterator            first,
                                    Iterator            last);

    //! \brief Replace non-overlapping instances of a word with a new generator
    //! via const reference.
    //!
    //! If \f$w=\f$`[first, last)` is a word, then this function replaces
    //! every non-overlapping instance (from left to right) of \f$w\f$ in
    //! every rule, adds a new generator \f$z\f$, and the rule \f$w = z\f$.
    //! The new generator and rule are added even if \f$w\f$ is not a subword
    //! of any rule.
    //!
    //! \tparam Word the type of the words in the presentation.
    //!
    //! \param p the presentation.
    //! \param w the subword to replace.
    //!
    //! \returns The new generator added.
    //!
    //! \throws LibsemigroupsException if \p w is empty.
    // TODO(later) complexity
    template <typename Word>
    typename Presentation<Word>::letter_type
    replace_word_with_new_generator(Presentation<Word>& p, Word const& w) {
      return replace_word_with_new_generator(p, w.cbegin(), w.cend());
    }

    //! \brief Replace non-overlapping instances of a subword via `char const*`.
    //!
    //! If \f$w=\f$`[first, last)` is a word, then replaces every
    //! non-overlapping instance (from left to right) of \f$w\f$ in every
    //! rule, adds a new generator \f$z\f$, and the rule \f$w = z\f$. The new
    //! generator and rule are added even if \f$w\f$ is not a subword of any
    //! rule.
    //!
    //! \param p the presentation.
    //! \param w the subword to replace.
    //!
    //! \returns The new generator added.
    //!
    //! \throws LibsemigroupsException if \p w is empty.
    inline typename Presentation<std::string>::letter_type
    replace_word_with_new_generator(Presentation<std::string>& p,
                                    char const*                w) {
      return replace_word_with_new_generator(p, w, w + std::strlen(w));
    }

    //! \brief Replace non-overlapping instances of a subword by another word.
    //!
    //! If \p existing and \p replacement are words, then this function
    //! replaces every non-overlapping instance of \p existing in every rule
    //! by \p replacement. The presentation \p p is changed in-place.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //! \param existing the word to be replaced
    //! \param replacement the replacement word.
    //!
    //! \throws LibsemigroupsException if `existing` is empty.
    // TODO(later) complexity
    template <typename Word>
    void replace_subword(Presentation<Word>& p,
                         Word const&         existing,
                         Word const&         replacement);

    //! \brief Replace non-overlapping instances of a subword by another word.
    //!
    //! Replaces every non-overlapping instance of `[first_existing,
    //! last_existing)` in every rule by `[first_replacement,
    //! last_replacement)`. The presentation \p p  is changed in-place
    //!
    //! \tparam S the type of the first two parameters (iterators, or
    //! pointers)
    //! \tparam T the type of the second two parameters (iterators,
    //! or pointers)
    //!
    //! \param p the presentation
    //! \param first_existing an iterator pointing to the first letter of the
    //! existing subword to be replaced
    //! \param last_existing an iterator pointing one past the last letter of
    //! the existing subword to be replaced
    //! \param first_replacement an iterator pointing to the first letter of
    //! the replacement word
    //! \param last_replacement an iterator pointing one past the last letter
    //! of the replacement word
    //!
    //! \throws LibsemigroupsException if `first_existing == last_existing`.
    template <typename Word, typename Iterator1, typename Iterator2>
    void replace_subword(Presentation<Word>& p,
                         Iterator1           first_existing,
                         Iterator1           last_existing,
                         Iterator2           first_replacement,
                         Iterator2           last_replacement);

    //! \brief Replace non-overlapping instances of a subword by another word by
    //! `const chat*`.
    //!
    //! This function replaces every non-overlapping instance of \p existing in
    //! every rule by \p replacement. The presentation \p p is changed in-place.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //! \param existing the word to be replaced
    //! \param replacement the replacement word.
    //!
    //! \throws LibsemigroupsException if `existing` is empty.
    inline void replace_subword(Presentation<std::string>& p,
                                char const*                existing,
                                char const*                replacement) {
      replace_subword(p,
                      existing,
                      existing + std::strlen(existing),
                      replacement,
                      replacement + std::strlen(replacement));
    }

    //! \brief Replace instances of a word on either side of a rule by another
    //! word.
    //!
    //! If \p existing and \p replacement are words, then this function
    //! replaces every instance of \p existing in every rule of the form
    //! \p existing \f$= w\f$ or \f$w = \f$ \p existing, with the word
    //! \p replacement. The presentation \p p is changed in-place.
    //!
    //! \tparam Word the type of the words in the presentation.
    //!
    //! \param p the presentation.
    //! \param existing the word to be replaced.
    //! \param replacement the replacement word.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    template <typename Word>
    void replace_word(Presentation<Word>& p,
                      Word const&         existing,
                      Word const&         replacement);

    //! \brief The sum of the lengths of all values in the range `[first,
    //! last)`.
    //!
    //! The sum of the lengths of all values in the range `[first, last)`.
    //!
    //! \tparam Iterator the type of the first and second arguments (iterators).
    //! \param first iterator pointing at the first value to calculate the
    //! length of.
    //! \param last iterator pointing one beyond the last value to
    //! calculate the length of.
    //!
    //! \returns a value of type `size_t`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    template <typename Iterator>
    size_t length(Iterator first, Iterator last);

    //! \brief Return the sum of the lengths of the rules.
    //!
    //! Return the sum of the lengths of the rules.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \returns A value of type `size_t`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    template <typename Word>
    size_t length(Presentation<Word> const& p) {
      return length(p.rules.cbegin(), p.rules.cend());
    }

    //! \brief Reverse every rule.
    //!
    //! Reverse every rule.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    template <typename Word>
    void reverse(Presentation<Word>& p) {
      for (auto& rule : p.rules) {
        std::reverse(rule.begin(), rule.end());
      }
    }

    //! \brief Normalize the alphabet to \f$\{0, \ldots, n - 1\}\f$
    //!
    //! Modify the presentation in-place so that the alphabet is \f$\{0, \ldots,
    //! n - 1\}\f$ (or equivalent) and rewrites the rules to use this alphabet.
    //!
    //! If the alphabet is already normalized, then no changes are made to the
    //! presentation.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \throws LibsemigroupsException if \ref validate throws on the initial
    //! presentation.
    template <typename Word>
    void normalize_alphabet(Presentation<Word>& p);

    //! \brief Change or re-order the alphabet.
    //!
    //! This function replaces `p.alphabet()` with \p new_alphabet, where
    //! possible, and re-writes the rules in the presentation using the new
    //! alphabet.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //! \param new_alphabet the replacement alphabet
    //!
    //! \throws LibsemigroupsException if the size of `p.alphabet()` and \p
    //! new_alphabet do not agree.
    template <typename Word>
    void change_alphabet(Presentation<Word>& p, Word const& new_alphabet);

    //! \brief Change or re-order the alphabet.
    //!
    //! This function replaces `p.alphabet()` with \p new_alphabet where
    //! possible, and re-writes the rules in the presentation using the new
    //! alphabet.
    //!
    //! \param p the presentation
    //! \param new_alphabet the replacement alphabet
    //!
    //! \throws LibsemigroupsException if the size of `p.alphabet()` and \p
    //! new_alphabet do not agree.
    inline void change_alphabet(Presentation<std::string>& p,
                                char const*                new_alphabet) {
      change_alphabet(p, std::string(new_alphabet));
    }

    //! \brief Returns an iterator pointing at the left-hand side of the first
    //! rule of maximal length in the given range.
    //!
    //! Returns an iterator pointing at the left-hand side of the first
    //! rule of maximal length in the given range.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam T the type of the parameters
    //! \param first left-hand side of the first rule
    //! \param last one past the right-hand side of the last rule
    //!
    //! \returns A value of type `T`.
    //!
    //! \throws LibsemigroupsException if the distance between \p first and \p
    //! last is odd.
    template <typename Iterator>
    Iterator longest_rule(Iterator first, Iterator last);

    //! \brief Returns an iterator pointing at the left-hand side of the first
    //! rule in the presentation with maximal length.
    //!
    //! Returns an iterator pointing at the left-hand side of the first
    //! rule in the presentation with maximal length.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \returns A value of type `std::vector<Word>::const_iterator`.
    //!
    //! \throws LibsemigroupsException if the length of \p p.rules is odd.
    template <typename Word>
    typename std::vector<Word>::const_iterator
    longest_rule(Presentation<Word> const& p) {
      return longest_rule(p.rules.cbegin(), p.rules.cend());
    }

    //! \brief Returns an iterator pointing at the left-hand side of the first
    //! rule of minimal length in the given range.
    //!
    //! Returns an iterator pointing at the left-hand side of the first
    //! rule of minimal length in the given range.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam T the type of the parameters
    //! \param first left-hand side of the first rule
    //! \param last one past the right-hand side of the last rule
    //!
    //! \returns A value of type `T`.
    //!
    //! \throws LibsemigroupsException if the distance between \p first and \p
    //! last is odd.
    template <typename Iterator>
    Iterator shortest_rule(Iterator first, Iterator last);

    //! \brief Returns an iterator pointing at the left-hand side of the first
    //! rule in the presentation with minimal length.
    //!
    //! Returns an iterator pointing at the left-hand side of the first
    //! rule in the presentation with minimal length.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \returns A value of type `std::vector<Word>::const_iterator`.
    //!
    //! \throws LibsemigroupsException if the length of \p p.rules is odd.
    template <typename Word>
    typename std::vector<Word>::const_iterator
    shortest_rule(Presentation<Word> const& p) {
      return shortest_rule(p.rules.cbegin(), p.rules.cend());
    }

    //! \brief Returns the maximum length of a rule in the given range.
    //!
    //! Returns the maximum length of a rule in the given range.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam Iterator the type of the parameters
    //! \param first left-hand side of the first rule
    //! \param last one past the right-hand side of the last rule
    //!
    //! \returns A value of type `Iterator::value_type::size_type`.
    //!
    //! \throws LibsemigroupsException if the length of \p p.rules is odd.
    template <typename Iterator>
    auto longest_rule_length(Iterator first, Iterator last);

    //! \brief the maximum length of a rule in the presentation.
    //!
    //! Returns the maximum length of a rule in the presentation.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \returns A value of type `Word::size_type`.
    //!
    //! \throws LibsemigroupsException if the length of \p p.rules is odd.
    template <typename Word>
    auto longest_rule_length(Presentation<Word> const& p) {
      return longest_rule_length(p.rules.cbegin(), p.rules.cend());
    }

    //! \brief Returns the minimum length of a rule in the given range.
    //!
    //! Returns the minimum length of a rule in the given range.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam Iterator the type of the parameters
    //! \param first left-hand side of the first rule
    //! \param last one past the right-hand side of the last rule
    //!
    //! \returns A value of type `Iterator::value_type::size_type`.
    //!
    //! \throws LibsemigroupsException if the length of \p p.rules is odd.
    // TODO is this the correct return type?
    template <typename Iterator>
    auto shortest_rule_length(Iterator first, Iterator last);

    //! \brief Returns the minimum length of a rule in the presentation.
    //!
    //! Returns the minimum length of a rule in the presentation.
    //!
    //! The *length* of a rule is defined to be the sum of the lengths of its
    //! left-hand and right-hand sides.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \returns A value of type `Word::size_type`.
    //!
    //! \throws LibsemigroupsException if the length of \p p.rules is odd.
    template <typename Word>
    auto shortest_rule_length(Presentation<Word> const& p) {
      return shortest_rule_length(p.rules.cbegin(), p.rules.cend());
    }

    //! \brief Remove any trivially redundant generators.
    //!
    //! If one side of any of the rules in the presentation \p p is a letter
    //! \c a and the other side of the rule does not contain \c a, then this
    //! function replaces every occurrence of \c a in every rule by the other
    //! side of the rule. This substitution is performed for every such
    //! rule in the presentation; and the trivial rules (with both sides being
    //! identical) are removed. If both sides of a rule are letters, then the
    //! greater letter is replaced by the lesser one.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //!
    //! \throws LibsemigroupsException if `p.rules.size()` is odd.
    template <typename Word>
    void remove_redundant_generators(Presentation<Word>& p);

    //! \brief Return a possible letter by index.
    //!
    //! This function returns the \f$i\f$-th letter in the alphabet consisting
    //! of all possible letters of type Presentation<Word>::letter_type. This
    //! function exists so that visible ASCII characters occur before
    //! invisible ones, so that when manipulating presentations over
    //! `std::string`s the human readable characters are used before
    //! non-readable ones.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation (unused)
    //! \param i the index of the letter to find
    //!
    //! \returns A `letter_type`.
    //!
    //! \throws LibsemigroupsException if `i` exceeds the number of letters in
    //! supported by `letter_type`.
    // TODO(later) move to words.*pp
    template <typename Word>
    typename Presentation<Word>::letter_type
    human_readable_letter(Presentation<Word> const& p, size_t i);

    //! \brief Return a possible letter by index.
    //!
    //! This function returns the \f$i\f$-th letter in the alphabet consisting
    //! of all possible letters of type Presentation<std::string>::letter_type.
    //! This function exists so that visible ASCII characters occur before
    //! invisible ones, so that when manipulating presentations over
    //! `std::string`s the human readable characters are used before
    //! non-readable ones.
    //!
    //! \param p a presentation (unused)
    //! \param i the index
    //!
    //! \returns A `letter_type`.
    //!
    //! \throws LibsemigroupsException if \p i exceeds the number of letters
    //! in supported by `letter_type`.
    // TODO(later) move to words.*pp
    typename Presentation<std::string>::letter_type
    human_readable_letter(Presentation<std::string> const& p, size_t i);

    //! \brief Return the first letter **not** in the alphabet of a
    //! presentation.
    //!
    //! This function returns `letter(p, i)` when `i` is the least possible
    //! value such that `!p.in_alphabet(letter(p, i))` if such a letter
    //! exists.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //!
    //! \returns A `letter_type`.
    //!
    //! \throws LibsemigroupsException if \p p already has an alphabet of
    //! the maximum possible size supported by `letter_type`.
    template <typename Word>
    typename Presentation<Word>::letter_type
    first_unused_letter(Presentation<Word> const& p);

    //! \brief Convert a monoid presentation to a semigroup presentation.
    //!
    //! This function modifies its argument in-place by replacing the empty
    //! word in all relations by a new generator, and the identity rules for
    //! that new generator. If `p.contains_empty_word()` is `false`, then the
    //! presentation is not modified and \ref UNDEFINED is returned. If a new
    //! generator is added as the identity, then this generator is returned.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //!
    //! \returns The new generator added, if any, and \ref UNDEFINED if not.
    //!
    //! \throws LibsemigroupsException if `replace_word` or
    //!  `add_identity_rules` does.
    template <typename Word>
    typename Presentation<Word>::letter_type
    make_semigroup(Presentation<Word>& p);

    //! \brief Greedily reduce the length of the presentation using
    //! `longest_subword_reducing_length`.
    //!
    //! This function repeatedly calls `longest_subword_reducing_length` and
    //! `replace_subword` to introduce a new generator and reduce the length
    //! of the presentation \p p until `longest_subword_reducing_length` returns
    //! the empty word.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //!
    //! \throws LibsemigroupsException if `longest_subword_reducing_length` or
    //!  `replace_word` does.
    template <typename Word>
    void greedy_reduce_length(Presentation<Word>& p);

    //! \brief Greedily reduce the length and number of generators of the
    //! presentation
    //!
    //! This function repeatedly calls `longest_subword_reducing_length` and
    //! `replace_subword` to temporarily introduce a new generator and try to
    //! reduce the length of the presentation \p p.  If, for a given subword,
    //! this operation is successfully reduces the length of the presentation,
    //! the changes are kept. Otherwise, the presentation is reverted and the
    //! next subword is tried. This is done  until
    //! `longest_subword_reducing_length` returns the empty word.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //!
    //! \throws LibsemigroupsException if `longest_subword_reducing_length` or
    //!  `replace_word` does.
    template <typename Word>
    void greedy_reduce_length_and_number_of_gens(Presentation<Word>& p);

    //! \brief Returns `true` if the \f$1\f$-relation presentation can be
    //! strongly compressed.
    //!
    //! A \f$1\f$-relation presentation is *strongly compressible* if both
    //! relation words start with the same letter and end with the same
    //! letter. In other words, if the alphabet of the presentation \p p is
    //! \f$A\f$ and the relation words are of the form \f$aub = avb\f$ where
    //! \f$a, b\in A\f$ (possibly \f$ a = b\f$) and \f$u, v\in A ^ *\f$, then
    //! \p p is strongly compressible. See [Section
    //! 3.2](https://doi.org/10.1007/s00233-021-10216-8) for details.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //!
    //! \returns A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \sa `strongly_compress`
    // not noexcept because std::vector::operator[] isn't
    template <typename Word>
    bool is_strongly_compressible(Presentation<Word> const& p);

    //! \brief Strongly compress a \f$1\f$-relation presentation.
    //!
    //! Strongly compress a \f$1\f$-relation presentation.
    //!
    //! Returns `true` if the \f$1\f$-relation presentation \p p has been
    //! modified and `false` if not. The word problem is solvable for the
    //! input presentation if it is solvable for the modified version.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //!
    //! \returns A value of type `bool`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \sa `is_strongly_compressible`
    // not noexcept because is_strongly_compressible isn't
    template <typename Word>
    bool strongly_compress(Presentation<Word>& p);

    //! \brief Reduce the number of generators in a \f$1\f$-relation
    //! presentation to `2`.
    //!
    //! Reduce the number of generators in a \f$1\f$-relation presentation to
    //! `2`.
    //!
    //! Returns `true` if the \f$1\f$-relation presentation \p p has been
    //! modified and `false` if not.
    //!
    //! A \f$1\f$-relation presentation is *left cycle-free* if the
    //! relation words start with distinct letters. In other words, if the
    //! alphabet of the presentation \p p is \f$A\f$ and the relation words are
    //! of the form \f$au = bv\f$ where \f$a, b\in A\f$ with \f$a \neq b\f$ and
    //! \f$u, v \in A ^ *\f$, then \p p is left cycle-free. The word problem for
    //! a left cycle-free \f$1\f$-relation monoid is solvable if the word
    //! problem for the modified version obtained from this function is
    //! solvable.
    //!
    //! \tparam Word the type of the words in the presentation
    //!
    //! \param p the presentation
    //! \param index determines the choice of letter to use, `0` uses
    //! `p.rules[0].front()` and `1` uses `p.rules[1].front()` (defaults to:
    //! `0`).
    //!
    //! \returns A value of type `bool`.
    //!
    //! \throws LibsemigroupsException if \p index is not `0` or `1`.
    template <typename Word>
    bool reduce_to_2_generators(Presentation<Word>& p, size_t index = 0);

    //! \brief Add rules that assert each letter is idempotent.
    //!
    //! Adds rules to \p p of the form \f$a^2 = a\f$ for every letter \f$a\f$ in
    //! \p letters.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param letters the letters to make idempotent
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    template <typename Word>
    void add_idempotent_rules_no_checks(Presentation<Word>& p,
                                        Word const&         letters) {
      for (auto x : letters) {
        add_rule_no_checks(p, {x, x}, {x});
      }
    }

    //! \brief Add rules that respect transpositions.
    //!
    //! Adds rules to \p p of the form \f$a^2 = \varepsilon\f$ for every letter
    //! \f$a\f$ in \p letters.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param letters the letters to add transposition rules for
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    // TODO does this doc make any sense?
    template <typename Word>
    void add_transposition_rules_no_checks(Presentation<Word>& p,
                                           word_type const&    letters) {
      for (auto x : letters) {
        add_rule_no_checks(p, {x, x}, {});
      }
    }

    //! \brief Add rules that assert specific letters commute
    //!
    //! Adds rules to \p p of the form \f$uv = vu\f$ for every letter \f$u\f$ in
    //! \p letters1 and \f$v\f$ in \p letters2.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param letters1 the first collection of letters to add rules for
    //! \param letters2 the second collection of letters to add rules for
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    template <typename Word>
    void add_commutes_rules_no_checks(Presentation<Word>& p,
                                      Word const&         letters1,
                                      Word const&         letters2);

    //! \brief Add rules that assert specific letters commute
    //!
    //! Adds rules to \p p of the form \f$uv = vu\f$ for every pair of letters
    //! \f$u, v\f$ in \p letters.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param letters the collection of letters to add rules for
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    template <typename Word>
    void add_commutes_rules_no_checks(Presentation<Word>& p,
                                      Word const&         letters) {
      add_commutes_rules_no_checks(p, letters, letters);
    }

    //! \brief Add rules that assert specific letters commute with specific
    //! words
    //!
    //! Adds rules to \p p of the form \f$uv = vu\f$ for every letter \f$u\f$ in
    //! \p letters and \f$v\f$ in \p words.
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation to add rules to
    //! \param letters the collection of letters to add rules for
    //! \param words the collection of words to add rules for
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    template <typename Word>
    void add_commutes_rules_no_checks(Presentation<Word>&         p,
                                      Word const&                 letters,
                                      std::initializer_list<Word> words);

    //! \brief TODO
    //!
    //! TODO
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //! \param letters TODO
    //! \param inverses TODO
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    template <typename Word>
    void balance_no_checks(Presentation<Word>& p,
                           Word const&         letters,
                           Word const&         inverses);

    //! \brief TODO
    //!
    //! TODO
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //! \param letters TODO
    //! \param inverses TODO
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! No checks that the arguments describe words over the alphabet of \p p
    //! are performed.
    inline void balance_no_checks(Presentation<std::string>& p,
                                  char const*                letters,
                                  char const*                inverses) {
      balance_no_checks(p, std::string(letters), std::string(inverses));
    }

    //! \brief TODO
    //!
    //! TODO
    //!
    //! \tparam Word the type of the words in the presentation
    //! \param p the presentation
    //! \param lhs TODO
    //! \param rhs TODO
    //!
    //! \throws LibsemigroupsException if either \p lhs or \p rhs contain
    //! letters not in `p.alphabet()`
    // TODO(later) do a proper version of this
    template <typename Word>
    void add_cyclic_conjugates(Presentation<Word>& p,
                               Word const&         lhs,
                               Word const&         rhs);

    //! \brief TODO
    //!
    //! TODO
    //!
    //! \param p the presentation
    //! \param var_name TODO
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    std::string to_gap_string(Presentation<word_type> const& p,
                              std::string const&             var_name);

    //! \brief TODO
    //!
    //! TODO
    //!
    //! \param p the presentation
    //! \param var_name TODO
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    std::string to_gap_string(Presentation<std::string> const& p,
                              std::string const&               var_name);

  }  // namespace presentation

  //! \ingroup presentations_group
  //!
  //! \brief For an implementations of inverse presentations for semigroups or
  //! monoids.
  //!
  //! Defined in ``presentation.hpp``.
  //!
  //! This class template can be used to construction inverse presentations for
  //! semigroups or monoids and is intended to be used as the input to other
  //! algorithms in `libsemigroups`. This class inherits from \ref
  //! Presentation<Word>.
  //!
  //! \tparam Word the type of the underlying words.
  template <typename Word>
  class InversePresentation : public Presentation<Word> {
   public:
    //! \brief The type of the words in the rules of an InversePresentation
    //! object.
    using word_type = typename Presentation<Word>::word_type;

    //! \brief The type of the letters in the words that constitute the rules of
    //! an InversePresentation object.
    using letter_type = typename Presentation<Word>::letter_type;

    //! \brief Type of a const iterator to either side of a rule.
    using const_iterator = typename Presentation<Word>::const_iterator;

    //! \brief Type of an iterator to either side of a rule.
    using iterator = typename Presentation<Word>::iterator;

    //! \brief Size type for rules.
    using size_type = typename Presentation<Word>::size_type;

   private:
    word_type _inverses;

   public:
    using Presentation<Word>::Presentation;

    // TODO init functions

    //! \brief Construct an InversePresentation from a Presentation reference
    //!
    //! Construct an InversePresentation, initially with empty inverses, from a
    //! reference to a Presentation.
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p a reference to the Presentation to construct from.
    InversePresentation<Word>(Presentation<Word> const& p)
        : Presentation<Word>(p), _inverses() {}

    //! \brief Construct an InversePresentation from a Presentation rvalue
    //! reference.
    //!
    //! Construct an InversePresentation, initially with empty inverses, from a
    //! Presentation rvalue reference
    //!
    //! \tparam Word the type of the words in the presentation.
    //! \param p an rvalue reference to the Presentation to construct from.
    InversePresentation<Word>(Presentation<Word>&& p)
        : Presentation<Word>(p), _inverses() {}

    //! \brief Set the inverse of each letter in the alphabet.
    //!
    //! Set the inverse of each letter in the alphabet.
    //!
    //! \param w a word containing the inverses
    //!
    //! \returns a reference to `this`.
    //!
    //! \exceptions
    //! \no_libsemigroups_except
    //!
    //! \warning
    //! This function does no checks on its argument. In particular, it does not
    //! check that the letters in \p w belong to the alphabet, nor does it add
    //! new letters to the alphabet.
    // TODO(later) validate that checks that inverses are set
    InversePresentation& inverses_no_checks(word_type const& w);

    //! \brief Return the inverse of each letter in the alphabet.
    //!
    //! Return the inverse of each letter in the alphabet.
    //!
    //! \returns a value of type \ref word_type.
    //!
    //! \exceptions
    //! \noexcept
    word_type const& inverses() const noexcept {
      return _inverses;
    }

    //! \brief Return the inverse of a letter in the alphabet.
    //!
    //! Return the inverse of the letter \p x.
    //!
    //! \param x the letter whose index is sought.
    //!
    //! \returns a value of type \ref letter_type
    //!
    //! \throws LibsemigroupsException if no inverses have been set, or if
    //! `index(letter_type x) const` throws.
    letter_type inverse(letter_type x) const;

    //! \brief Check if the InversePresentation is valid.
    //!
    //! Check if the InversePresentation is valid. Specifically, check that the
    //! alphabet does not contain duplicate letters, that all rules only contain
    //! letters defined in the alphabet, and that the inverses act as semigroup
    //! inverses.
    //!
    //! \throw LibsemigroupsException if:
    //! * the alphabet contains duplicate letters
    //! * the rules contain letters not defined in the alphabet
    //! * the inverses do not act as semigroup inverses
    //!
    //! \sa
    //! \ref Presentation<Word>::validate and \ref
    //! presentation::validate_semigroup_inverses.
    void validate() const {
      Presentation<Word>::validate();
      presentation::validate_semigroup_inverses(*this, inverses());
    }
  };

  //! \ingroup presentations_group
  //!
  //! \brief Compare for equality.
  //!
  //! Returns \c true if \p lhop equals \p rhop by comparing the
  //! the alphabets and the rules.
  //!
  //! \tparam Word the type of the words in the presentations.
  //! \param lhop a presentation that is to be compared.
  //! \param rhop a presentation that is to be compared.
  //!
  //! \returns
  //! A value of type \c bool.
  //!
  //! \exceptions
  //! \no_libsemigroups_except
  //!
  //! \complexity
  //! At worst linear in the sum of the alphabet sizes and numbers of rules.
  // TODO(later) also we could do a more sophisticated version of this
  template <typename Word>
  bool operator==(Presentation<Word> const& lhop,
                  Presentation<Word> const& rhop) {
    return lhop.alphabet() == rhop.alphabet() && lhop.rules == rhop.rules;
  }

  //! \ingroup presentations_group
  //!
  //! \brief Compare for inequality.
  //!
  //! Returns \c true if \p lhop does not equal \p rhop by comparing the
  //! the alphabets and the rules.
  //!
  //! \tparam Word the type of the words in the presentations.
  //! \param lhop a presentation that is to be compared.
  //! \param rhop a presentation that is to be compared.
  //!
  //! \returns
  //! A value of type \c bool.
  //!
  //! \exceptions
  //! \no_libsemigroups_except
  //!
  //! \complexity
  //! At worst linear in the sum of the alphabet sizes and numbers of rules.
  template <typename Word>
  bool operator!=(Presentation<Word> const& lhop,
                  Presentation<Word> const& rhop) {
    return !(lhop == rhop);
  }

  //! \ingroup presentations_group
  //! \brief Convert a string to a word_type.
  //!
  //! This function converts its second argument \p input into a \ref word_type
  //! and stores the result in the third argument \p output. The characters of
  //! \p input are converted using `p.index()`, so that each letter
  //! is mapped to the corresponding index in `p.alphabet()`.
  //!
  //! The content of the third argument \p output, if any, is removed.
  //!
  //! \param p the presentation whose alphabet will be used to convert
  //! \param input the string to convert
  //! \param output word to hold the result
  //!
  //! \throws LibsemigroupsException if any letter in \p input is not contained
  //! in `p.alphabet()`.
  //!
  //! \sa \ref Presentation::index.
  // TODO(later) could do a no_check version
  void to_word(Presentation<std::string> const& p,
               std::string const&               input,
               word_type&                       output);

  //! \ingroup presentations_group
  //! \brief Convert a string to a word_type.
  //!
  //! This function converts its second argument \p input into a \ref word_type
  //! and returns this result. The characters of \p input are converted using
  //! `p.index()`, so that each letter is mapped to the corresponding index in
  //! `p.alphabet()`.
  //!
  //! \param p the presentation whose alphabet will be used to convert
  //! \param input the string to convert
  //!
  //! \throws LibsemigroupsException if any letter in \p input is not contained
  //! in `p.alphabet()`.
  //!
  //! \sa \ref Presentation::index.
  // TODO(later) could do a no_check version
  [[nodiscard]] word_type to_word(Presentation<std::string> const& p,
                                  std::string const&               input);

  //! \ingroup presentations_group
  //! \brief Convert a word_type to a string.
  //!
  //! This function converts its second argument \p input into a `string`
  //! and stores the result in the third argument \p output. The indices in
  //! \p input are converted using `p.letter()`, so that each index
  //! is mapped to the corresponding letter.
  //!
  //! The content of the third argument \p output, if any, is removed.
  //!
  //! \param p the presentation whose alphabet will be used to convert.
  //! \param input the \ref word_type to convert.
  //! \param output the `string` to hold the result.
  //!
  //! \throws LibsemigroupsException if any index in \p input is not in the
  //! range `[0, p.alphabet().size())`
  //!
  //! \sa \ref Presentation::letter.
  // TODO(later) could do a no_check version
  void to_string(Presentation<std::string> const& p,
                 word_type const&                 input,
                 std::string&                     output);

  //! \ingroup presentations_group
  //! \brief Convert a word_type to a string.
  //!
  //! This function converts its second argument \p input into a `string`
  //! and returns this result. The indices in \p input are converted using
  //! `p.letter()`, so that each index is mapped to the corresponding letter.
  //!
  //! \param p the presentation whose alphabet will be used to convert.
  //! \param input the \ref word_type to convert.
  //!
  //! \throws LibsemigroupsException if any index in \p input is not in the
  //! range `[0, p.alphabet().size())`
  //!
  //! \sa \ref Presentation::letter.
  // TODO(later) could do a no check version
  std::string to_string(Presentation<std::string> const& p,
                        word_type const&                 input);

  namespace detail {
    template <typename T>
    struct IsPresentationHelper : std::false_type {};

    template <typename T>
    struct IsPresentationHelper<Presentation<T>> : std::true_type {};

    template <typename T>
    struct IsPresentationHelper<InversePresentation<T>> : std::true_type {};

    template <typename T>
    struct IsInversePresentationHelper : std::false_type {};

    template <typename T>
    struct IsInversePresentationHelper<InversePresentation<T>>
        : std::true_type {};
  }  // namespace detail

  //! \ingroup presentations_group
  //!
  //! \brief Helper variable template.
  //!
  //! Helper variable template.
  //!
  //! The value of this variable is \c true if the template parameter \p T is
  //! \ref InversePresentation.
  //!
  //! \tparam T a type.
  template <typename T>
  static constexpr bool IsInversePresentation
      = detail::IsInversePresentationHelper<T>::value;

  //! \ingroup presentations_group
  //!
  //! \brief Helper variable template.
  //!
  //! Helper variable template.
  //!
  //! The value of this variable is \c true if the template parameter \p T is
  //! \ref InversePresentation.
  //!
  //! \tparam T a type.
  template <typename T>
  static constexpr bool IsPresentation = detail::IsPresentationHelper<T>::value;

}  // namespace libsemigroups

#include "presentation.tpp"

#endif  // LIBSEMIGROUPS_PRESENTATION_HPP_
