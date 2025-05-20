//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2025 James D. Mitchell
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

// This file contains TODO

#ifndef LIBSEMIGROUPS_DETAIL_CITOW_MAP_HPP_
#define LIBSEMIGROUPS_DETAIL_CITOW_MAP_HPP_

#include <cstddef>   // for size_t
#include <iterator>  // for pair

#include "libsemigroups/types.hpp"  // for letter_type

namespace libsemigroups {
  namespace detail {
    // The following is a class for wrapping iterators. This is used by the
    // member functions that accept iterators (that point at words that might
    // not be the word_type used by ThingImpl) to convert the values
    // pointed at into word_types, and in the class itow_map, to allow
    // assignment of these values too. CITOW = const_iterator_to_word
    template <typename Vector, typename Iterator>
    class citow_map {
     public:
      using internal_iterator_type = Iterator;
      using value_type             = letter_type;
      using reference              = letter_type;
      using const_reference        = value_type;
      using const_pointer          = value_type const*;
      using pointer                = value_type*;

      using size_type         = size_t;
      using difference_type   = std::ptrdiff_t;
      using iterator_category = std::bidirectional_iterator_tag;

     protected:
      Iterator      _it;
      Vector const* _external_letters;

     public:
      citow_map(Vector const* external_letters, Iterator it)
          : _it(it), _external_letters(external_letters) {}

      reference operator*() const {
        reference d = std::distance(_external_letters->cbegin(),
                                    std::find(_external_letters->cbegin(),
                                              _external_letters->cend(),
                                              *_it));
        LIBSEMIGROUPS_ASSERT(d < _external_letters->size());
        return d;
      }

      // TODO(1) operator-> ??

      bool operator==(citow_map const& that) const noexcept {
        return _it == that._it;
      }

      bool operator!=(citow_map const& that) const noexcept {
        return _it != that._it;
      }

      bool operator<=(citow_map const& that) const noexcept {
        return _it <= that._it;
      }

      bool operator>=(citow_map const& that) const noexcept {
        return _it >= that._it;
      }

      bool operator<(citow_map const& that) const noexcept {
        return _it < that._it;
      }

      bool operator>(citow_map const& that) const noexcept {
        return _it > that._it;
      }

      citow_map& operator++() {
        ++_it;
        return *this;
      }

      citow_map& operator+=(size_type val) noexcept {
        _it += val;
        return *this;
      }

      citow_map operator+(size_type val) const noexcept {
        citow_map result(*this);
        result += val;
        return result;
      }

      citow_map& operator--() {
        --_it;
        return *this;
      }

      citow_map& operator-=(size_type val) noexcept {
        _it -= val;
        return *this;
      }

      citow_map operator-(size_type val) const noexcept {
        citow_map result(*this);
        result -= val;
        return result;
      }

      [[nodiscard]] Iterator get() const noexcept {
        return _it;
      }
    };  // class citow_map

    template <typename Vector, typename Iterator>
    citow_map(Vector const*, Iterator) -> citow_map<Vector, Iterator>;

  }  // namespace detail
}  // namespace libsemigroups
#endif  // LIBSEMIGROUPS_DETAIL_CITOW_MAP_HPP_
