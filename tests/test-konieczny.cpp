// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2020 Finn Smith
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

// TODO(now) IWYU
#include "libsemigroups/bmat8.hpp"
#include "libsemigroups/element-adapters.hpp"
#include "libsemigroups/element.hpp"
#include "libsemigroups/froidure-pin.hpp"
#include "libsemigroups/konieczny.hpp"

#include "catch.hpp"
#include "test-main.hpp"

#include "data/test-konieczny-data.hpp"

namespace libsemigroups {

  constexpr bool REPORT = false;

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "000",
                          "regular elements and idempotents",
                          "[quick][no-valgrind]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> gens
        = {BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}})};
    Konieczny<BMat8>   KS(gens);
    FroidurePin<BMat8> S(gens);
    S.run();

    REQUIRE(KS.size() == 63904);
    REQUIRE(S.size() == 63904);

    size_t count = 0;
    for (auto it = S.cbegin(); it < S.cend(); ++it) {
      if (KS.is_regular_element(*it)) {
        count++;
      }
    }
    REQUIRE(count == 40408);

    size_t sum = 0;
    for (auto it = KS.cbegin_regular_D_classes();
         it < KS.cend_regular_D_classes();
         ++it) {
      sum += (*it)->size();
    }
    REQUIRE(sum == 40408);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny", "001", "regular D class 01", "[quick]") {
    auto                     rg   = ReportGuard(REPORT);
    const std::vector<BMat8> gens = {BMat8({{0, 1, 0}, {0, 0, 1}, {1, 0, 0}}),
                                     BMat8({{0, 1, 0}, {1, 0, 0}, {0, 0, 1}}),
                                     BMat8({{1, 0, 0}, {1, 1, 0}, {0, 0, 1}}),
                                     BMat8({{1, 1, 0}, {0, 1, 1}, {1, 0, 1}})};
    Konieczny<BMat8>         KS(gens);
    REQUIRE(KS.size() == 247);

    BMat8                           x({{1, 0, 0}, {1, 1, 0}, {1, 0, 1}});
    Konieczny<BMat8>::RegularDClass D = Konieczny<BMat8>::RegularDClass(&KS, x);
    REQUIRE(D.cend_left_indices() - D.cbegin_left_indices() == 3);
    REQUIRE(D.cend_right_indices() - D.cbegin_right_indices() == 3);
    REQUIRE(D.size() == 18);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny", "002", "regular D class 02", "[quick]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> gens
        = {BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}}),
           BMat8({{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 0, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}})};

    Konieczny<BMat8> KS(gens);
    KS.run();
    BMat8 idem(BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}));
    Konieczny<BMat8>::RegularDClass D
        = Konieczny<BMat8>::RegularDClass(&KS, idem);
    REQUIRE(D.size() == 24);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "004",
                          "regular D class 04: contains",
                          "[quick][no-valgrind]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> gens
        = {BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}}),
           BMat8({{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 0, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}})};

    Konieczny<BMat8>   KS(gens);
    FroidurePin<BMat8> S(gens);
    KS.run();
    S.run();
    BMat8 idem(BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}));
    Konieczny<BMat8>::RegularDClass D
        = Konieczny<BMat8>::RegularDClass(&KS, idem);

    // test that the top D class contains only permutation matrices
    for (auto it = S.cbegin(); it < S.cend(); it++) {
      REQUIRE(D.contains(*it) == (((*it) * (*it).transpose()) == gens[0]));
    }
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "005",
                          "non-regular D classes 01",
                          "[quick]") {
    auto                     rg    = ReportGuard(REPORT);
    const std::vector<BMat8> gens  = {BMat8({{0, 1, 0}, {0, 0, 1}, {1, 0, 0}}),
                                     BMat8({{0, 1, 0}, {1, 0, 0}, {0, 0, 1}}),
                                     BMat8({{1, 0, 0}, {1, 1, 0}, {0, 0, 1}}),
                                     BMat8({{1, 1, 0}, {0, 1, 1}, {1, 0, 1}})};
    const std::vector<BMat8> idems = {BMat8({{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}),
                                      BMat8({{1, 0, 0}, {1, 1, 0}, {0, 0, 1}}),
                                      BMat8({{1, 0, 0}, {1, 1, 1}, {0, 0, 1}}),
                                      BMat8({{1, 0, 0}, {1, 1, 0}, {1, 0, 1}}),
                                      BMat8({{1, 0, 0}, {1, 1, 0}, {1, 1, 1}}),
                                      BMat8({{1, 1, 0}, {1, 1, 0}, {0, 0, 1}}),
                                      BMat8({{1, 0, 0}, {1, 1, 1}, {1, 1, 1}}),
                                      BMat8({{1, 1, 0}, {1, 1, 0}, {1, 1, 1}}),
                                      BMat8({{1, 1, 1}, {1, 1, 1}, {1, 1, 1}})};

    Konieczny<BMat8> KS(gens);
    KS.run();

    REQUIRE(KS.cend_regular_D_classes() - KS.cbegin_regular_D_classes()
            == idems.size());

    size_t count = 0;
    for (BMat8 id : idems) {
      Konieczny<BMat8>::RegularDClass D(&KS, id);
      count += D.size();
    }

    REQUIRE(count == 142);

    std::vector<BMat8> non_reg_reps
        = {BMat8({{0, 0, 1}, {1, 0, 1}, {1, 1, 0}}),
           BMat8({{0, 0, 1}, {1, 1, 1}, {1, 1, 0}}),
           BMat8({{0, 1, 1}, {1, 0, 1}, {1, 1, 1}}),
           BMat8({{0, 1, 1}, {1, 1, 0}, {1, 0, 1}}),
           BMat8({{1, 0, 1}, {1, 0, 1}, {1, 1, 0}}),
           BMat8({{1, 1, 0}, {1, 1, 1}, {1, 1, 1}})};

    Konieczny<BMat8>::NonRegularDClass X(&KS, non_reg_reps[0]);
    REQUIRE(X.size() == 36);
    REQUIRE(X.size_H_class() == 1);
    REQUIRE(X.nr_left_reps() == 6);
    REQUIRE(X.nr_right_reps() == 6);

    Konieczny<BMat8>::NonRegularDClass Y(&KS, non_reg_reps[1]);
    REQUIRE(Y.size() == 18);
    REQUIRE(Y.size_H_class() == 1);
    REQUIRE(Y.nr_left_reps() == 3);
    REQUIRE(Y.nr_right_reps() == 6);

    Konieczny<BMat8>::NonRegularDClass Z(&KS, non_reg_reps[2]);
    REQUIRE(Z.size() == 18);
    REQUIRE(Z.size_H_class() == 2);
    REQUIRE(Z.nr_left_reps() == 3);
    REQUIRE(Z.nr_right_reps() == 3);

    Konieczny<BMat8>::NonRegularDClass A(&KS, non_reg_reps[3]);
    REQUIRE(A.size() == 6);
    REQUIRE(A.size_H_class() == 6);
    REQUIRE(A.nr_left_reps() == 1);
    REQUIRE(A.nr_right_reps() == 1);

    Konieczny<BMat8>::NonRegularDClass B(&KS, non_reg_reps[4]);
    REQUIRE(B.size() == 18);
    REQUIRE(B.size_H_class() == 1);
    REQUIRE(B.nr_left_reps() == 6);
    REQUIRE(B.nr_right_reps() == 3);

    Konieczny<BMat8>::NonRegularDClass C(&KS, non_reg_reps[5]);
    REQUIRE(C.size() == 9);
    REQUIRE(C.size_H_class() == 1);
    REQUIRE(C.nr_left_reps() == 3);
    REQUIRE(C.nr_right_reps() == 3);

    for (BMat8 x : non_reg_reps) {
      Konieczny<BMat8>::NonRegularDClass N(&KS, x);
      count += N.size();
    }

    REQUIRE(count == 247);

    REQUIRE(KS.size() == 247);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "006",
                          "non-regular D classes 02",
                          "[quick]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> gens
        = {BMat8({{0, 1, 0, 0}, {1, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 1, 1}, {0, 0, 1, 0}, {1, 1, 1, 1}}),
           BMat8({{1, 1, 0, 1}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 0}, {1, 0, 0, 0}}),
           BMat8({{1, 1, 0, 1}, {1, 1, 1, 1}, {1, 0, 1, 0}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 0, 0}, {0, 1, 1, 0}, {1, 0, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}, {0, 0, 1, 0}})};

    const std::vector<BMat8> idems
        = {BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{1, 1, 1, 1}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{1, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 1, 1}, {0, 0, 0, 0}}),
           BMat8({{1, 1, 1, 1}, {0, 1, 0, 1}, {0, 0, 1, 0}, {0, 0, 0, 0}}),
           BMat8({{1, 0, 0, 1}, {0, 1, 0, 0}, {0, 1, 1, 0}, {1, 0, 0, 1}}),
           BMat8({{1, 0, 0, 1}, {1, 1, 0, 1}, {1, 1, 1, 1}, {1, 0, 0, 1}}),
           BMat8({{1, 0, 0, 1}, {0, 1, 1, 0}, {0, 1, 1, 0}, {1, 0, 0, 1}}),
           BMat8({{1, 1, 1, 1}, {0, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}),
           BMat8({{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}),
           BMat8({{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}})};

    Konieczny<BMat8> KS(gens);
    KS.run();

    size_t count = 0;
    for (BMat8 id : idems) {
      Konieczny<BMat8>::RegularDClass D(&KS, id);
      count += D.size();
    }

    REQUIRE(KS.cend_regular_D_classes() - KS.cbegin_regular_D_classes()
            == idems.size());

    REQUIRE(count == 8712);

    std::vector<BMat8> non_reg_reps
        = {BMat8({{1, 1, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 0}}),
           BMat8({{0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 0}, {1, 0, 0, 0}}),
           BMat8({{1, 0, 0, 1}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 1, 0, 0}}),
           BMat8({{1, 0, 0, 0}, {1, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 1, 1}}),
           BMat8({{1, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 0}, {0, 0, 1, 0}}),
           BMat8({{0, 1, 1, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 0, 0, 0}, {0, 1, 0, 1}, {0, 1, 1, 1}, {1, 1, 1, 0}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 1, 1}, {0, 0, 0, 0}, {1, 0, 1, 0}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 1, 1}, {0, 0, 0, 0}, {1, 1, 1, 0}}),
           BMat8({{1, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {1, 1, 0, 1}}),
           BMat8({{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 1}, {1, 1, 0, 1}}),
           BMat8({{1, 0, 0, 1}, {1, 1, 0, 1}, {0, 0, 0, 0}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 1, 0}, {0, 1, 1, 1}, {0, 0, 0, 0}, {1, 1, 0, 1}}),
           BMat8({{1, 1, 0, 1}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 1, 1, 1}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 1, 0, 1}}),
           BMat8({{1, 1, 1, 0}, {0, 1, 0, 1}, {0, 0, 0, 0}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 1, 1}, {1, 1, 0, 1}, {0, 0, 0, 0}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 1, 1}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}}),
           BMat8({{1, 1, 1, 1}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 1, 1}}),
           BMat8({{0, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 0, 1}}),
           BMat8({{0, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 0, 1, 1}}),
           BMat8({{0, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 1}, {0, 0, 1, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 1, 0, 1}, {1, 1, 1, 1}, {1, 1, 1, 0}}),
           BMat8({{0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 1, 1}, {1, 0, 1, 0}}),
           BMat8({{1, 0, 0, 0}, {1, 0, 1, 0}, {0, 0, 1, 1}, {1, 0, 0, 0}}),
           BMat8({{0, 1, 0, 0}, {0, 1, 1, 0}, {1, 1, 1, 1}, {1, 1, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 1, 1, 0}, {1, 0, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 0, 0}, {1, 0, 1, 0}}),
           BMat8({{1, 0, 0, 0}, {0, 0, 1, 1}, {0, 0, 1, 1}, {1, 0, 1, 0}}),
           BMat8({{1, 0, 0, 0}, {1, 0, 1, 1}, {0, 0, 1, 1}, {1, 0, 1, 0}}),
           BMat8({{0, 1, 0, 0}, {1, 1, 1, 0}, {1, 1, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 1, 0}, {1, 1, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 0, 1}, {1, 1, 1, 0}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 1, 0}, {0, 1, 1, 0}, {1, 1, 0, 1}}),
           BMat8({{0, 1, 0, 1}, {0, 1, 1, 0}, {0, 1, 1, 0}, {1, 1, 1, 1}}),
           BMat8({{0, 1, 1, 0}, {0, 1, 0, 1}, {0, 1, 0, 1}, {1, 1, 1, 0}}),
           BMat8({{1, 1, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{1, 0, 1, 0}, {1, 1, 1, 0}, {0, 1, 1, 1}, {1, 1, 1, 0}}),
           BMat8({{1, 1, 1, 0}, {1, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 0, 1}}),
           BMat8({{1, 0, 1, 0}, {0, 1, 0, 1}, {0, 1, 0, 1}, {1, 1, 1, 0}}),
           BMat8({{0, 1, 0, 1}, {1, 1, 1, 1}, {1, 0, 1, 0}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 1, 1, 0}, {1, 1, 1, 0}, {0, 1, 1, 1}}),
           BMat8({{1, 1, 0, 1}, {1, 1, 1, 1}, {1, 0, 1, 0}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 0, 1}, {1, 1, 1, 1}, {1, 1, 1, 0}, {0, 1, 1, 1}}),
           BMat8({{1, 0, 1, 0}, {1, 1, 1, 1}, {1, 1, 0, 1}, {1, 1, 1, 0}}),
           BMat8({{0, 1, 1, 0}, {0, 1, 1, 0}, {1, 1, 0, 1}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 1, 0}, {0, 1, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 0}}),
           BMat8({{1, 1, 0, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 0}}),
           BMat8({{0, 1, 1, 0}, {1, 1, 1, 0}, {1, 1, 1, 1}, {0, 1, 1, 1}}),
           BMat8({{1, 0, 0, 1}, {1, 1, 1, 1}, {0, 1, 1, 0}, {1, 1, 0, 1}}),
           BMat8({{0, 1, 1, 0}, {1, 1, 0, 1}, {1, 1, 0, 1}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 1, 0}, {1, 1, 1, 1}, {1, 1, 0, 1}, {0, 1, 1, 1}}),
           BMat8({{0, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 0}})};

    Konieczny<BMat8>::NonRegularDClass X0
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[0]);
    REQUIRE(X0.size() == 180);
    REQUIRE(X0.size_H_class() == 2);
    REQUIRE(X0.nr_left_reps() == 2);
    REQUIRE(X0.nr_right_reps() == 45);

    Konieczny<BMat8>::NonRegularDClass X1
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[1]);
    REQUIRE(X1.size() == 16);
    REQUIRE(X1.size_H_class() == 1);
    REQUIRE(X1.nr_left_reps() == 4);
    REQUIRE(X1.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X2
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[2]);
    REQUIRE(X2.size() == 64);
    REQUIRE(X2.size_H_class() == 1);
    REQUIRE(X2.nr_left_reps() == 4);
    REQUIRE(X2.nr_right_reps() == 16);

    Konieczny<BMat8>::NonRegularDClass X3
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[3]);
    REQUIRE(X3.size() == 16);
    REQUIRE(X3.size_H_class() == 1);
    REQUIRE(X3.nr_left_reps() == 4);
    REQUIRE(X3.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X4
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[4]);
    REQUIRE(X4.size() == 32);
    REQUIRE(X4.size_H_class() == 1);
    REQUIRE(X4.nr_left_reps() == 4);
    REQUIRE(X4.nr_right_reps() == 8);

    Konieczny<BMat8>::NonRegularDClass X5
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[5]);
    REQUIRE(X5.size() == 64);
    REQUIRE(X5.size_H_class() == 2);
    REQUIRE(X5.nr_left_reps() == 4);
    REQUIRE(X5.nr_right_reps() == 8);

    Konieczny<BMat8>::NonRegularDClass X6
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[6]);
    REQUIRE(X6.size() == 16);
    REQUIRE(X6.size_H_class() == 1);
    REQUIRE(X6.nr_left_reps() == 4);
    REQUIRE(X6.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X7
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[7]);
    REQUIRE(X7.size() == 16);
    REQUIRE(X7.size_H_class() == 1);
    REQUIRE(X7.nr_left_reps() == 4);
    REQUIRE(X7.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X8
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[8]);
    REQUIRE(X8.size() == 16);
    REQUIRE(X8.size_H_class() == 1);
    REQUIRE(X8.nr_left_reps() == 4);
    REQUIRE(X8.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X9
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[9]);
    REQUIRE(X9.size() == 16);
    REQUIRE(X9.size_H_class() == 1);
    REQUIRE(X9.nr_left_reps() == 4);
    REQUIRE(X9.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X10
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[10]);
    REQUIRE(X10.size() == 16);
    REQUIRE(X10.size_H_class() == 1);
    REQUIRE(X10.nr_left_reps() == 4);
    REQUIRE(X10.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X11
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[11]);
    REQUIRE(X11.size() == 16);
    REQUIRE(X11.size_H_class() == 1);
    REQUIRE(X11.nr_left_reps() == 4);
    REQUIRE(X11.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X12
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[12]);
    REQUIRE(X12.size() == 16);
    REQUIRE(X12.size_H_class() == 1);
    REQUIRE(X12.nr_left_reps() == 4);
    REQUIRE(X12.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X13
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[13]);
    REQUIRE(X13.size() == 16);
    REQUIRE(X13.size_H_class() == 1);
    REQUIRE(X13.nr_left_reps() == 4);
    REQUIRE(X13.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X14
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[14]);
    REQUIRE(X14.size() == 16);
    REQUIRE(X14.size_H_class() == 1);
    REQUIRE(X14.nr_left_reps() == 4);
    REQUIRE(X14.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X15
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[15]);
    REQUIRE(X15.size() == 16);
    REQUIRE(X15.size_H_class() == 1);
    REQUIRE(X15.nr_left_reps() == 4);
    REQUIRE(X15.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X16
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[16]);
    REQUIRE(X16.size() == 16);
    REQUIRE(X16.size_H_class() == 1);
    REQUIRE(X16.nr_left_reps() == 4);
    REQUIRE(X16.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X17
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[17]);
    REQUIRE(X17.size() == 16);
    REQUIRE(X17.size_H_class() == 2);
    REQUIRE(X17.nr_left_reps() == 2);
    REQUIRE(X17.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X18
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[18]);
    REQUIRE(X18.size() == 32);
    REQUIRE(X18.size_H_class() == 1);
    REQUIRE(X18.nr_left_reps() == 8);
    REQUIRE(X18.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X19
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[19]);
    REQUIRE(X19.size() == 16);
    REQUIRE(X19.size_H_class() == 1);
    REQUIRE(X19.nr_left_reps() == 4);
    REQUIRE(X19.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X20
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[20]);
    REQUIRE(X20.size() == 156);
    REQUIRE(X20.size_H_class() == 2);
    REQUIRE(X20.nr_left_reps() == 39);
    REQUIRE(X20.nr_right_reps() == 2);

    Konieczny<BMat8>::NonRegularDClass X21
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[21]);
    REQUIRE(X21.size() == 16);
    REQUIRE(X21.size_H_class() == 1);
    REQUIRE(X21.nr_left_reps() == 4);
    REQUIRE(X21.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X22
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[22]);
    REQUIRE(X22.size() == 32);
    REQUIRE(X22.size_H_class() == 1);
    REQUIRE(X22.nr_left_reps() == 4);
    REQUIRE(X22.nr_right_reps() == 8);

    Konieczny<BMat8>::NonRegularDClass X23
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[23]);
    REQUIRE(X23.size() == 32);
    REQUIRE(X23.size_H_class() == 1);
    REQUIRE(X23.nr_left_reps() == 4);
    REQUIRE(X23.nr_right_reps() == 8);

    Konieczny<BMat8>::NonRegularDClass X24
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[24]);
    REQUIRE(X24.size() == 16);
    REQUIRE(X24.size_H_class() == 1);
    REQUIRE(X24.nr_left_reps() == 4);
    REQUIRE(X24.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X25
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[25]);
    REQUIRE(X25.size() == 16);
    REQUIRE(X25.size_H_class() == 1);
    REQUIRE(X25.nr_left_reps() == 4);
    REQUIRE(X25.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X26
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[26]);
    REQUIRE(X26.size() == 32);
    REQUIRE(X26.size_H_class() == 1);
    REQUIRE(X26.nr_left_reps() == 4);
    REQUIRE(X26.nr_right_reps() == 8);

    Konieczny<BMat8>::NonRegularDClass X27
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[27]);
    REQUIRE(X27.size() == 16);
    REQUIRE(X27.size_H_class() == 1);
    REQUIRE(X27.nr_left_reps() == 4);
    REQUIRE(X27.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X28
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[28]);
    REQUIRE(X28.size() == 16);
    REQUIRE(X28.size_H_class() == 1);
    REQUIRE(X28.nr_left_reps() == 4);
    REQUIRE(X28.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X29
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[29]);
    REQUIRE(X29.size() == 16);
    REQUIRE(X29.size_H_class() == 1);
    REQUIRE(X29.nr_left_reps() == 4);
    REQUIRE(X29.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X30
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[30]);
    REQUIRE(X30.size() == 16);
    REQUIRE(X30.size_H_class() == 1);
    REQUIRE(X30.nr_left_reps() == 4);
    REQUIRE(X30.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X31
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[31]);
    REQUIRE(X31.size() == 16);
    REQUIRE(X31.size_H_class() == 1);
    REQUIRE(X31.nr_left_reps() == 4);
    REQUIRE(X31.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X32
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[32]);
    REQUIRE(X32.size() == 32);
    REQUIRE(X32.size_H_class() == 1);
    REQUIRE(X32.nr_left_reps() == 8);
    REQUIRE(X32.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X33
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[33]);
    REQUIRE(X33.size() == 16);
    REQUIRE(X33.size_H_class() == 1);
    REQUIRE(X33.nr_left_reps() == 4);
    REQUIRE(X33.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X34
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[34]);
    REQUIRE(X34.size() == 32);
    REQUIRE(X34.size_H_class() == 1);
    REQUIRE(X34.nr_left_reps() == 8);
    REQUIRE(X34.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X35
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[35]);
    REQUIRE(X35.size() == 32);
    REQUIRE(X35.size_H_class() == 2);
    REQUIRE(X35.nr_left_reps() == 4);
    REQUIRE(X35.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X36
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[36]);
    REQUIRE(X36.size() == 16);
    REQUIRE(X36.size_H_class() == 1);
    REQUIRE(X36.nr_left_reps() == 4);
    REQUIRE(X36.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X37
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[37]);
    REQUIRE(X37.size() == 32);
    REQUIRE(X37.size_H_class() == 1);
    REQUIRE(X37.nr_left_reps() == 8);
    REQUIRE(X37.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X38
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[38]);
    REQUIRE(X38.size() == 16);
    REQUIRE(X38.size_H_class() == 1);
    REQUIRE(X38.nr_left_reps() == 4);
    REQUIRE(X38.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X39
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[39]);
    REQUIRE(X39.size() == 32);
    REQUIRE(X39.size_H_class() == 1);
    REQUIRE(X39.nr_left_reps() == 8);
    REQUIRE(X39.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X40
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[40]);
    REQUIRE(X40.size() == 16);
    REQUIRE(X40.size_H_class() == 1);
    REQUIRE(X40.nr_left_reps() == 4);
    REQUIRE(X40.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X41
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[41]);
    REQUIRE(X41.size() == 16);
    REQUIRE(X41.size_H_class() == 1);
    REQUIRE(X41.nr_left_reps() == 4);
    REQUIRE(X41.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X42
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[42]);
    REQUIRE(X42.size() == 16);
    REQUIRE(X42.size_H_class() == 1);
    REQUIRE(X42.nr_left_reps() == 4);
    REQUIRE(X42.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X43
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[43]);
    REQUIRE(X43.size() == 16);
    REQUIRE(X43.size_H_class() == 1);
    REQUIRE(X43.nr_left_reps() == 4);
    REQUIRE(X43.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X44
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[44]);
    REQUIRE(X44.size() == 16);
    REQUIRE(X44.size_H_class() == 1);
    REQUIRE(X44.nr_left_reps() == 4);
    REQUIRE(X44.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X45
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[45]);
    REQUIRE(X45.size() == 16);
    REQUIRE(X45.size_H_class() == 1);
    REQUIRE(X45.nr_left_reps() == 4);
    REQUIRE(X45.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X46
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[46]);
    REQUIRE(X46.size() == 16);
    REQUIRE(X46.size_H_class() == 1);
    REQUIRE(X46.nr_left_reps() == 4);
    REQUIRE(X46.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X47
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[47]);
    REQUIRE(X47.size() == 16);
    REQUIRE(X47.size_H_class() == 1);
    REQUIRE(X47.nr_left_reps() == 4);
    REQUIRE(X47.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X48
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[48]);
    REQUIRE(X48.size() == 16);
    REQUIRE(X48.size_H_class() == 1);
    REQUIRE(X48.nr_left_reps() == 4);
    REQUIRE(X48.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X49
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[49]);
    REQUIRE(X49.size() == 16);
    REQUIRE(X49.size_H_class() == 1);
    REQUIRE(X49.nr_left_reps() == 4);
    REQUIRE(X49.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X50
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[50]);
    REQUIRE(X50.size() == 16);
    REQUIRE(X50.size_H_class() == 1);
    REQUIRE(X50.nr_left_reps() == 4);
    REQUIRE(X50.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X51
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[51]);
    REQUIRE(X51.size() == 16);
    REQUIRE(X51.size_H_class() == 2);
    REQUIRE(X51.nr_left_reps() == 2);
    REQUIRE(X51.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X52
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[52]);
    REQUIRE(X52.size() == 16);
    REQUIRE(X52.size_H_class() == 1);
    REQUIRE(X52.nr_left_reps() == 4);
    REQUIRE(X52.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X53
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[53]);
    REQUIRE(X53.size() == 16);
    REQUIRE(X53.size_H_class() == 1);
    REQUIRE(X53.nr_left_reps() == 4);
    REQUIRE(X53.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X54
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[54]);
    REQUIRE(X54.size() == 16);
    REQUIRE(X54.size_H_class() == 1);
    REQUIRE(X54.nr_left_reps() == 4);
    REQUIRE(X54.nr_right_reps() == 4);

    Konieczny<BMat8>::NonRegularDClass X55
        = Konieczny<BMat8>::NonRegularDClass(&KS, non_reg_reps[55]);
    REQUIRE(X55.size() == 8);
    REQUIRE(X55.size_H_class() == 2);
    REQUIRE(X55.nr_left_reps() == 2);
    REQUIRE(X55.nr_right_reps() == 2);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny", "007", "RegularDClass", "[quick]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> gens
        = {BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 0, 1, 1}}),
           BMat8({{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {0, 1, 0, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}})};

    Konieczny<BMat8> KS(gens);
    KS.run();
    BMat8 x = BMat8({{0, 1, 0}, {1, 0, 0}, {0, 0, 0}});
    Konieczny<BMat8>::RegularDClass D = Konieczny<BMat8>::RegularDClass(&KS, x);
    REQUIRE(D.size() == 90);
    REQUIRE(D.nr_left_reps() == 5);
    REQUIRE(D.nr_right_reps() == 9);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "008",
                          "full bmat monoid 4",
                          "[quick][no-valgrind]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> bmat4_gens
        = {BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{1, 1, 1, 0}, {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
           BMat8({{1, 1, 0, 0}, {1, 0, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{1, 1, 0, 0}, {1, 0, 1, 0}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}}),
           BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}})};

    Konieczny<BMat8> S(bmat4_gens);
    REQUIRE(S.size() == 65536);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "009",
                          "full bmat monoid 5",
                          "[extreme]") {
    auto                     rg         = ReportGuard(REPORT);
    const std::vector<BMat8> bmat5_gens = {BMat8({{1, 0, 0, 0, 0},
                                                  {0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1},
                                                  {1, 0, 0, 0, 0}}),
                                           BMat8({{0, 1, 0, 0, 0},
                                                  {1, 0, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 0, 0, 0, 0},
                                                  {0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {1, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 0, 0, 0},
                                                  {1, 0, 1, 0, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 0, 0, 0},
                                                  {1, 0, 1, 0, 0},
                                                  {0, 1, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 0, 0, 0},
                                                  {1, 0, 1, 0, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 0, 1},
                                                  {0, 0, 0, 1, 1}}),
                                           BMat8({{1, 1, 1, 1, 0},
                                                  {1, 0, 0, 0, 1},
                                                  {0, 1, 0, 0, 1},
                                                  {0, 0, 1, 0, 1},
                                                  {0, 0, 0, 1, 1}}),
                                           BMat8({{1, 0, 0, 0, 0},
                                                  {0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 0}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 0, 1},
                                                  {0, 0, 0, 1, 1}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 0},
                                                  {1, 0, 0, 0, 1},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 0, 1}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 1},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 1, 0, 0, 1},
                                                  {0, 0, 1, 1, 0}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 1, 0, 1, 0},
                                                  {1, 0, 0, 0, 1},
                                                  {0, 1, 0, 0, 1},
                                                  {0, 0, 1, 1, 1}})};

    Konieczny<BMat8> T(bmat5_gens);
    REQUIRE(T.size() == 33554432);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "010",
                          "regular generated bmat monoid 4 idempotents",
                          "[quick][no-valgrind]") {
    auto                     rg = ReportGuard(REPORT);
    const std::vector<BMat8> reg_bmat4_gens
        = {BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}}),
           BMat8({{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}})};

    Konieczny<BMat8> S(reg_bmat4_gens);
    REQUIRE(S.size() == 63904);

    size_t reg_elts = 0;
    for (auto it = S.cbegin_regular_D_classes();
         it < S.cend_regular_D_classes();
         ++it) {
      reg_elts += (*it)->size();
    }
    REQUIRE(reg_elts == 40408);

    size_t idems = 0;
    for (auto it = S.cbegin_regular_D_classes();
         it < S.cend_regular_D_classes();
         ++it) {
      idems += (*it)->nr_idempotents();
    }
    REQUIRE(idems == 2360);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "011",
                          "regular generated bmat monoid 5",
                          "[extreme]") {
    auto                     rg             = ReportGuard(REPORT);
    const std::vector<BMat8> reg_bmat5_gens = {BMat8({{0, 1, 0, 0, 0},
                                                      {1, 0, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1}}),
                                               BMat8({{0, 1, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1},
                                                      {1, 0, 0, 0, 0}}),
                                               BMat8({{1, 0, 0, 0, 0},
                                                      {1, 1, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1}}),
                                               BMat8({{0, 0, 0, 0, 0},
                                                      {0, 1, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1}})};

    Konieczny<BMat8> T(reg_bmat5_gens);
    REQUIRE(T.size() == 32311832);

    size_t reg_elts = 0;
    for (auto it = T.cbegin_regular_D_classes();
         it < T.cend_regular_D_classes();
         ++it) {
      reg_elts += (*it)->size();
    }
    REQUIRE(reg_elts == 8683982);

    size_t idems = 0;
    for (auto it = T.cbegin_regular_D_classes();
         it < T.cend_regular_D_classes();
         ++it) {
      idems += (*it)->nr_idempotents();
    }
    REQUIRE(idems == 73023);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "012",
                          "my favourite example",
                          "[quick][finite][no-valgrind]") {
    auto                     rg   = ReportGuard(REPORT);
    const std::vector<BMat8> gens = {BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0}}),
                                     BMat8({{0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1}}),
                                     BMat8({{0, 0, 0, 1, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 1, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1}})};

    Konieczny<BMat8> S(gens);
    REQUIRE(S.size() == 597369);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "013",
                          "another large example",
                          "[quick][no-valgrind]") {
    auto                     rg   = ReportGuard(REPORT);
    const std::vector<BMat8> gens = {BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1}}),
                                     BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1}}),
                                     BMat8({{1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 1, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0}}),
                                     BMat8({{0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0}})};

    Konieczny<BMat8> S(gens);
    REQUIRE(S.size() == 201750);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "014",
                          "my favourite example transposed",
                          "[quick][no-valgrind]") {
    auto                     rg   = ReportGuard(REPORT);
    const std::vector<BMat8> gens = {BMat8({{0, 0, 0, 0, 1, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1}}),
                                     BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 1, 0},
                                            {1, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 1, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {1, 0, 1, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 1, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 1, 1, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 1, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 1, 0, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 1, 1, 1, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 1},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {1, 0, 0, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0}}),
                                     BMat8({{0, 0, 0, 1, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 1, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 1, 0, 0, 1, 0, 0},
                                            {0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 1, 0, 0, 0},
                                            {1, 1, 0, 0, 0, 0, 0, 1}})};

    Konieczny<BMat8> S(gens);
    REQUIRE(S.size() == 597369);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny", "015", "transformations", "[quick]") {
    auto                              rg = ReportGuard(REPORT);
    Konieczny<Transformation<size_t>> S(
        {Transformation<size_t>({1, 0, 2, 3, 4}),
         Transformation<size_t>({1, 2, 3, 4, 0}),
         Transformation<size_t>({0, 0, 2, 3, 4})});
    S.run();
    REQUIRE(S.size() == 3125);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "016",
                          "transformations - JDM favourite example",
                          "[quick][no-valgrind]") {
    auto                                    rg = ReportGuard(REPORT);
    Konieczny<Transformation<uint_fast8_t>> S(
        {Transformation<uint_fast8_t>({1, 7, 2, 6, 0, 4, 1, 5}),
         Transformation<uint_fast8_t>({2, 4, 6, 1, 4, 5, 2, 7}),
         Transformation<uint_fast8_t>({3, 0, 7, 2, 4, 6, 2, 4}),
         Transformation<uint_fast8_t>({3, 2, 3, 4, 5, 3, 0, 1}),
         Transformation<uint_fast8_t>({4, 3, 7, 7, 4, 5, 0, 4}),
         Transformation<uint_fast8_t>({5, 6, 3, 0, 3, 0, 5, 1}),
         Transformation<uint_fast8_t>({6, 0, 1, 1, 1, 6, 3, 4}),
         Transformation<uint_fast8_t>({7, 7, 4, 0, 6, 4, 1, 7})});
    REQUIRE(S.size() == 597369);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "017",
                          "non-pointer BooleanMat",
                          "[quick][boolmat][booleanmat]") {
    auto                    rg = ReportGuard(REPORT);
    std::vector<BooleanMat> gens
        = {BooleanMat({0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0}),
           BooleanMat({0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}),
           BooleanMat({0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1})};

    Konieczny<BooleanMat> S(gens);
    REQUIRE(S.size() == 26);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "018",
                          "non-pointer BooleanMat",
                          "[quick][boolmat][booleanmat][no-valgrind]") {
    auto                    rg   = ReportGuard(REPORT);
    std::vector<BooleanMat> gens = {
        BooleanMat({{1, 0, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}, {0, 1, 0, 0}}),
        BooleanMat({{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 1, 1}, {0, 1, 1, 0}}),
        BooleanMat({{1, 0, 1, 0}, {1, 0, 1, 1}, {0, 0, 1, 1}, {0, 1, 0, 1}}),
        BooleanMat({{0, 0, 0, 0}, {0, 1, 0, 1}, {1, 1, 1, 0}, {1, 0, 0, 1}}),
        BooleanMat({{0, 0, 0, 1}, {0, 0, 1, 0}, {1, 0, 0, 1}, {1, 1, 0, 0}})};

    Konieczny<BooleanMat> S(gens);
    REQUIRE(S.size() == 415);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "019",
                          "transformations - large example",
                          "[standard][no-valgrind]") {
    auto                                    rg = ReportGuard(REPORT);
    Konieczny<Transformation<uint_fast8_t>> S(
        {Transformation<uint_fast8_t>({2, 1, 0, 4, 2, 1, 1, 8, 0}),
         Transformation<uint_fast8_t>({1, 7, 6, 2, 5, 1, 1, 4, 3}),
         Transformation<uint_fast8_t>({1, 0, 7, 2, 1, 3, 1, 3, 7}),
         Transformation<uint_fast8_t>({0, 3, 8, 1, 2, 8, 1, 7, 0}),
         Transformation<uint_fast8_t>({0, 0, 0, 2, 7, 7, 5, 5, 3})});
    REQUIRE(S.size() == 232511);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "020",
                          "transformations - large example with stop",
                          "[standard][no-valgrind]") {
    auto                                    rg = ReportGuard(REPORT);
    Konieczny<Transformation<uint_fast8_t>> S(
        {Transformation<uint_fast8_t>({2, 1, 0, 4, 2, 1, 1, 8, 0}),
         Transformation<uint_fast8_t>({1, 7, 6, 2, 5, 1, 1, 4, 3}),
         Transformation<uint_fast8_t>({1, 0, 7, 2, 1, 3, 1, 3, 7}),
         Transformation<uint_fast8_t>({0, 3, 8, 1, 2, 8, 1, 7, 0}),
         Transformation<uint_fast8_t>({0, 0, 0, 2, 7, 7, 5, 5, 3})});
    S.run_for(std::chrono::milliseconds(100));
    REQUIRE(S.size() == 232511);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "021",
                          "regular generated bmat monoid 5 with stops",
                          "[extreme]") {
    auto                     rg             = ReportGuard(REPORT);
    const std::vector<BMat8> reg_bmat5_gens = {BMat8({{0, 1, 0, 0, 0},
                                                      {1, 0, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1}}),
                                               BMat8({{0, 1, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1},
                                                      {1, 0, 0, 0, 0}}),
                                               BMat8({{1, 0, 0, 0, 0},
                                                      {1, 1, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1}}),
                                               BMat8({{0, 0, 0, 0, 0},
                                                      {0, 1, 0, 0, 0},
                                                      {0, 0, 1, 0, 0},
                                                      {0, 0, 0, 1, 0},
                                                      {0, 0, 0, 0, 1}})};

    Konieczny<BMat8> T(reg_bmat5_gens);
    T.run_for(std::chrono::milliseconds(4000));
    size_t nr_classes = T.cend_D_classes() - T.cbegin_D_classes();
    REQUIRE(nr_classes > 0);
    T.run_for(std::chrono::milliseconds(2000));
    REQUIRE(T.cend_D_classes() - T.cbegin_D_classes() > nr_classes);

    REQUIRE(T.size() == 32311832);

    size_t reg_elts = 0;
    for (auto it = T.cbegin_regular_D_classes();
         it < T.cend_regular_D_classes();
         ++it) {
      reg_elts += (*it)->size();
    }
    REQUIRE(reg_elts == 8683982);

    size_t idems = 0;
    for (auto it = T.cbegin_regular_D_classes();
         it < T.cend_regular_D_classes();
         ++it) {
      idems += (*it)->nr_idempotents();
    }
    REQUIRE(idems == 73023);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "022",
                          "transformations - large example with run_until",
                          "[standard][no-valgrind]") {
    auto                                    rg = ReportGuard(REPORT);
    Konieczny<Transformation<uint_fast8_t>> S(
        {Transformation<uint_fast8_t>({2, 1, 0, 4, 2, 1, 1, 8, 0}),
         Transformation<uint_fast8_t>({1, 7, 6, 2, 5, 1, 1, 4, 3}),
         Transformation<uint_fast8_t>({1, 0, 7, 2, 1, 3, 1, 3, 7}),
         Transformation<uint_fast8_t>({0, 3, 8, 1, 2, 8, 1, 7, 0}),
         Transformation<uint_fast8_t>({0, 0, 0, 2, 7, 7, 5, 5, 3})});
    S.run_until([&S]() -> bool {
      return S.cend_D_classes() - S.cbegin_D_classes() > 20;
    });

    size_t nr_classes1 = S.cend_D_classes() - S.cbegin_D_classes();
    REQUIRE(nr_classes1 >= 20);
    S.run();
    size_t nr_classes2 = S.cend_D_classes() - S.cbegin_D_classes();
    REQUIRE(S.size() == 232511);
    REQUIRE(nr_classes1 < nr_classes2);
    REQUIRE(nr_classes2 == 2122);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "023",
                          "transformations - large example with stop in Action",
                          "[standard][no-valgrind]") {
    auto                                    rg = ReportGuard(REPORT);
    Konieczny<Transformation<uint_fast8_t>> S(
        {Transformation<uint_fast8_t>({2, 1, 0, 4, 2, 1, 1, 8, 0}),
         Transformation<uint_fast8_t>({1, 7, 6, 2, 5, 1, 1, 4, 3}),
         Transformation<uint_fast8_t>({1, 0, 7, 2, 1, 3, 1, 3, 7}),
         Transformation<uint_fast8_t>({0, 3, 8, 1, 2, 8, 1, 7, 0}),
         Transformation<uint_fast8_t>({0, 0, 0, 2, 7, 7, 5, 5, 3})});
    S.run_for(std::chrono::milliseconds(5));
    S.run_for(std::chrono::milliseconds(5));
    S.run_for(std::chrono::milliseconds(5));
    S.run_for(std::chrono::milliseconds(100));
    S.run_for(std::chrono::milliseconds(100));
    S.run();
    S.run_for(std::chrono::milliseconds(100));
    S.run_for(std::chrono::milliseconds(100));
    REQUIRE(S.size() == 232511);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "024",
                          "full bmat monoid 5 with stop in Action",
                          "[extreme]") {
    auto                     rg         = ReportGuard(REPORT);
    const std::vector<BMat8> bmat5_gens = {BMat8({{1, 0, 0, 0, 0},
                                                  {0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1},
                                                  {1, 0, 0, 0, 0}}),
                                           BMat8({{0, 1, 0, 0, 0},
                                                  {1, 0, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 0, 0, 0, 0},
                                                  {0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {1, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 0, 0, 0},
                                                  {1, 0, 1, 0, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 0, 0, 0},
                                                  {1, 0, 1, 0, 0},
                                                  {0, 1, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 1, 0},
                                                  {0, 0, 0, 0, 1}}),
                                           BMat8({{1, 1, 0, 0, 0},
                                                  {1, 0, 1, 0, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 0, 1},
                                                  {0, 0, 0, 1, 1}}),
                                           BMat8({{1, 1, 1, 1, 0},
                                                  {1, 0, 0, 0, 1},
                                                  {0, 1, 0, 0, 1},
                                                  {0, 0, 1, 0, 1},
                                                  {0, 0, 0, 1, 1}}),
                                           BMat8({{1, 0, 0, 0, 0},
                                                  {0, 1, 0, 0, 0},
                                                  {0, 0, 1, 0, 0},
                                                  {0, 0, 0, 1, 0},
                                                  {0, 0, 0, 0, 0}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 0},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 0, 1},
                                                  {0, 0, 0, 1, 1}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 0},
                                                  {1, 0, 0, 0, 1},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 0, 1, 0, 1}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 0, 0, 1, 1},
                                                  {0, 1, 0, 1, 0},
                                                  {0, 1, 0, 0, 1},
                                                  {0, 0, 1, 1, 0}}),
                                           BMat8({{1, 1, 1, 0, 0},
                                                  {1, 1, 0, 1, 0},
                                                  {1, 0, 0, 0, 1},
                                                  {0, 1, 0, 0, 1},
                                                  {0, 0, 1, 1, 1}})};

    Konieczny<BMat8> T(bmat5_gens);
    T.run_for(std::chrono::milliseconds(100));
    T.run_for(std::chrono::milliseconds(100));
    T.run_for(std::chrono::milliseconds(100));
    T.run_for(std::chrono::milliseconds(100));
    T.run_for(std::chrono::milliseconds(100));
    T.run();
    T.run_for(std::chrono::milliseconds(100));
    T.run_for(std::chrono::milliseconds(100));
    REQUIRE(T.size() == 33554432);
  }

  LIBSEMIGROUPS_TEST_CASE("Konieczny",
                          "025",
                          "generators from Sean Clark",
                          "[extreme]") {
    auto rg = ReportGuard(REPORT);
    // actual size unknown, this is more of an aspirational test for now...
    // Konieczny<BooleanMat> S(konieczny_data::clark_gens);
    // S.run();
    // REQUIRE(S.size() == 33554432);
  }
}  // namespace libsemigroups
