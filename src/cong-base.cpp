//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2018 James D. Mitchell
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

#include "cong-base.hpp"

#include "internal/libsemigroups-exception.hpp"
#include "internal/stl.hpp"

#include "constants.hpp"
#include "froidure-pin-base.hpp"

namespace libsemigroups {

  ////////////////////////////////////////////////////////////////////////////
  // CongBase - constructors + destructor - public
  ////////////////////////////////////////////////////////////////////////////

  CongBase::CongBase(congruence_type type)
      : Runner(),
        // Protected
        _non_trivial_classes(),
        _nr_generating_pairs(0),
        // Private
        _delete_quotient(false),
        _init_ntc_done(false),
        _nrgens(UNDEFINED),
        _parent(nullptr),
        _quotient(nullptr),
        _type(type) {}

  CongBase::~CongBase() {
    reset_quotient();
  }

  ////////////////////////////////////////////////////////////////////////////
  // CongBase - non-pure virtual methods - public
  ////////////////////////////////////////////////////////////////////////////

  bool CongBase::contains(word_type const& w1, word_type const& w2) {
    return w1 == w2 || word_to_class_index(w1) == word_to_class_index(w2);
  }

  CongBase::result_type CongBase::const_contains(word_type const& u,
                                                 word_type const& v) const {
    if (const_word_to_class_index(u) == UNDEFINED
        || const_word_to_class_index(v) == UNDEFINED) {
      return result_type::UNKNOWN;
    } else if (const_word_to_class_index(u) == const_word_to_class_index(v)) {
      return result_type::TRUE;
    } else if (finished()) {
      return result_type::FALSE;
    } else {
      return result_type::UNKNOWN;
    }
  }

  bool CongBase::less(word_type const& w1, word_type const& w2) {
    return word_to_class_index(w1) < word_to_class_index(w2);
  }

  bool CongBase::is_quotient_obviously_finite() {
    return false;
  }

  bool CongBase::is_quotient_obviously_infinite() {
    return false;
  }

  void CongBase::set_nr_generators(size_t n) {
    if (nr_generators() != UNDEFINED) {
      throw LIBSEMIGROUPS_EXCEPTION(
          "the number of generators cannot be set more than once");
    }
    _nrgens = n;
  }

  /////////////////////////////////////////////////////////////////////////
  // CongBase - non-virtual methods - public
  /////////////////////////////////////////////////////////////////////////

  void CongBase::add_pair(std::initializer_list<size_t> l,
                          std::initializer_list<size_t> r) {
    add_pair(word_type(l), word_type(r));
  }

  std::vector<std::vector<word_type>>::const_iterator CongBase::cbegin_ntc() {
    init_non_trivial_classes();
    return _non_trivial_classes.cbegin();
  }

  std::vector<std::vector<word_type>>::const_iterator CongBase::cend_ntc() {
    init_non_trivial_classes();
    return _non_trivial_classes.cend();
  }

  size_t CongBase::nr_generators() const noexcept {
    return _nrgens;
  }

  size_t CongBase::nr_generating_pairs() const noexcept {
    return _nr_generating_pairs;
  }

  size_t CongBase::nr_non_trivial_classes() {
    init_non_trivial_classes();
    return _non_trivial_classes.size();
  }

  FroidurePinBase* CongBase::parent_semigroup() const {
    if (!has_parent()) {
      throw LIBSEMIGROUPS_EXCEPTION("the parent semigroup is not defined");
    }
    return get_parent();
  }

  congruence_type CongBase::type() const noexcept {
    return _type;
  }

  /////////////////////////////////////////////////////////////////////////
  // CongBase - non-virtual methods - protected
  /////////////////////////////////////////////////////////////////////////

  FroidurePinBase* CongBase::get_quotient() const noexcept {
    return _quotient;
  }

  bool CongBase::has_quotient() const noexcept {
    return _quotient != nullptr;
  }

  void CongBase::reset_quotient() {
    if (_delete_quotient) {
      delete _quotient;
    }
    _delete_quotient = false;
    _quotient        = nullptr;
  }

  void CongBase::set_quotient(FroidurePinBase* qtnt, bool delete_it) {
    LIBSEMIGROUPS_ASSERT(qtnt != nullptr);
    LIBSEMIGROUPS_ASSERT(_quotient == nullptr);
    LIBSEMIGROUPS_ASSERT(_type == congruence_type::TWOSIDED);
    // _delete_quotient can be either true or false, depending on whether qtnt
    // is coming from outside or inside.
    _delete_quotient = delete_it;
    _quotient        = qtnt;
  }

  FroidurePinBase* CongBase::get_parent() const noexcept {
    return _parent;
  }

  bool CongBase::has_parent() const noexcept {
    return _parent != nullptr;
  }

  void CongBase::set_parent(FroidurePinBase* prnt) {
    LIBSEMIGROUPS_ASSERT(prnt != nullptr || dead());
    if (prnt == _parent) {
      return;
    }
    LIBSEMIGROUPS_ASSERT(_parent == nullptr || dead());
    LIBSEMIGROUPS_ASSERT(prnt->nr_generators() == nr_generators()
                         || nr_generators() == UNDEFINED || dead());
    _parent = prnt;
    if (_nr_generating_pairs == 0) {
      _quotient        = prnt;
      _delete_quotient = false;
    }
  }

  /////////////////////////////////////////////////////////////////////////
  // CongBase - non-pure virtual methods - private
  /////////////////////////////////////////////////////////////////////////

  void CongBase::init_non_trivial_classes() {
    if (_init_ntc_done) {
      // There are no non-trivial classes, or we already found them.
      return;
    } else if (_parent == nullptr) {
      throw LIBSEMIGROUPS_EXCEPTION("there's no parent semigroup in which to "
                                    "find the non-trivial classes");
    }
    LIBSEMIGROUPS_ASSERT(nr_classes() != POSITIVE_INFINITY);

    _init_ntc_done = true;
    _non_trivial_classes.assign(nr_classes(), std::vector<word_type>());

    word_type w;
    for (size_t pos = 0; pos < _parent->size(); ++pos) {
      _parent->factorisation(w, pos);
      _non_trivial_classes[word_to_class_index(w)].push_back(w);
      LIBSEMIGROUPS_ASSERT(word_to_class_index(w)
                           < _non_trivial_classes.size());
    }

    _non_trivial_classes.erase(
        std::remove_if(_non_trivial_classes.begin(),
                       _non_trivial_classes.end(),
                       [](std::vector<word_type> const& klass) -> bool {
                         return klass.size() <= 1;
                       }),
        _non_trivial_classes.end());
  }

  CongBase::class_index_type
  CongBase::const_word_to_class_index(word_type const&) const {
    return UNDEFINED;
  }

  /////////////////////////////////////////////////////////////////////////
  // CongBase - non-virtual methods - protected
  /////////////////////////////////////////////////////////////////////////

  bool CongBase::validate_letter(letter_type c) const {
    if (nr_generators() == UNDEFINED) {
      throw LIBSEMIGROUPS_EXCEPTION("no generators have been defined");
    }
    return c < _nrgens;
  }

  void CongBase::validate_word(word_type const& w) const {
    for (auto l : w) {
      // validate_letter throws if no generators are defined
      if (!validate_letter(l)) {
        throw LIBSEMIGROUPS_EXCEPTION(
            "invalid letter " + to_string(l) + " in word " + to_string(w)
            + ", the valid range is [0, " + to_string(_nrgens) + ")");
      }
    }
  }

  void CongBase::validate_relation(word_type const& l,
                                   word_type const& r) const {
    validate_word(l);
    validate_word(r);
  }

  void CongBase::validate_relation(relation_type const& rel) const {
    validate_relation(rel.first, rel.second);
  }

  /////////////////////////////////////////////////////////////////////////
  // CongBase - non-virtual static methods - protected
  /////////////////////////////////////////////////////////////////////////

  std::string const& CongBase::congruence_type_to_string(congruence_type typ) {
    switch (typ) {
      case congruence_type::TWOSIDED:
        return STRING_TWOSIDED;
      case congruence_type::LEFT:
        return STRING_LEFT;
      case congruence_type::RIGHT:
        return STRING_RIGHT;
    }
  }

  /////////////////////////////////////////////////////////////////////////
  // CongBase - static data members - private
  /////////////////////////////////////////////////////////////////////////

  const std::string CongBase::STRING_TWOSIDED = "two-sided";
  const std::string CongBase::STRING_LEFT     = "left";
  const std::string CongBase::STRING_RIGHT    = "right";

}  // namespace libsemigroups