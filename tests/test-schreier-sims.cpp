// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2019-2025 James D. Mitchell
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

// These tests are derived from:
//
//   https://github.com/james-d-mitchell/libsemigroups/pull/39
//
// by Alex Levine and Luna Elliott. Many examples here taken from
//
//   http://brauer.maths.qmul.ac.uk/Atlas/

#include <algorithm>         // for find_if, find, fill
#include <array>             // for array
#include <cstddef>           // for size_t
#include <cstdint>           // for uint64_t, uint8_t
#include <initializer_list>  // for begin, end
#include <iterator>          // for distance, reverse_ite...
#include <memory>            // for unique_ptr, make_unique
#include <numeric>           // for iota
#include <string>            // for operator+, operator==
#include <type_traits>       // for remove_reference_t
#include <utility>

#include "Catch2-3.8.0/catch_amalgamated.hpp"  // for REQUIRE etc
#include "test-main.hpp"                       // for LIBSEMIGROUPS_TEST_CASE

#include "libsemigroups/config.hpp"     // for LIBSEMIGROUPS_HPCOMBI_ENABLED
#include "libsemigroups/constants.hpp"  // for UNDEFINED
#include "libsemigroups/exception.hpp"  // for LibsemigroupsException
#include "libsemigroups/hpcombi.hpp"    // for HPCombi
#include "libsemigroups/schreier-sims.hpp"  // for SchreierSims, SchreierSims<>::ele...
#include "libsemigroups/transf.hpp"         // for Perm

#include "libsemigroups/detail/report.hpp"  // for ReportGuard

namespace libsemigroups {
  struct LibsemigroupsException;

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "000",
                          "trivial perm. group (degree 1)",
                          "[quick][schreier-sims]") {
    auto            rg = ReportGuard(false);
    SchreierSims<1> S;
    using Perm = decltype(S)::element_type;
    S.add_generator(make<Perm>({0}));
    REQUIRE(S.size() == 1);
    REQUIRE(S.contains(make<Perm>({0})));
    // REQUIRE(!S.contains(make<Perm>({1, 0})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "001",
                          "trivial perm. group (degree 2)",
                          "[quick][schreier-sims]") {
    auto            rg = ReportGuard(false);
    SchreierSims<2> S;
    using Perm = SchreierSims<2>::element_type;
    S.add_generator(make<Perm>({0, 1}));
    REQUIRE(S.size() == 1);
    REQUIRE(S.sift(make<Perm>({1, 0})) == make<Perm>({1, 0}));
    REQUIRE(S.sift(make<Perm>({0, 1})) == S.one());
    REQUIRE(!S.contains(make<Perm>({1, 0})));
    REQUIRE(S.contains(make<Perm>({0, 1})));
    REQUIRE(
        to_human_readable_repr(S)
        == "<partially enumerated SchreierSims with 0 generators & base ()>");
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "002",
                          "trivial perm. group (degree 500)",
                          "[quick][schreier-sims]") {
    auto rg            = ReportGuard(false);
    size_t constexpr N = 100;
    // We allocate using "new" to avoid allocation on the stack,
    // since 100 is too large, and causes this test to seg fault.
    auto S     = std::make_unique<SchreierSims<N>>();
    using Perm = std::remove_reference_t<decltype(*S)>::element_type;

    auto p = Perm::one(N);

    S->add_generator(p);

    REQUIRE(S->size() == 1);
    REQUIRE(S->contains(p));

    std::swap(p[30], p[31]);
    REQUIRE(!S->contains(p));
    std::swap(p[73], p[32]);
    REQUIRE(!S->contains(p));
    std::swap(p[73], p[32]);
    std::swap(p[30], p[31]);
    std::swap(p[0], p[99]);
    REQUIRE(!S->contains(p));
  }

#ifndef LIBSEMIGROUPS_HPCOMBI_ENABLED
#define PERM_TYPES Perm<>
#else
#define PERM_TYPES Perm<>, HPCombi::Perm16
#endif

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "003",
                                   "symmetric group (degree 5)",
                                   "[quick][schreier-sims]",
                                   Perm<5>,
                                   PERM_TYPES) {
    auto                               rg = ReportGuard(false);
    SchreierSims<5, uint8_t, TestType> S;
    S.add_generator(make<TestType>({1, 0, 2, 3, 4}));
    S.add_generator(make<TestType>({1, 2, 3, 4, 0}));
    REQUIRE(S.number_of_generators() == 2);
    REQUIRE(S.size() == 120);
    REQUIRE(
        to_human_readable_repr(S)
        == "<SchreierSims with 2 generators, base (0, 1, 2, 3) & size 120>");
  }

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "004",
                                   "symmetric group (degree 8)",
                                   "[quick][schreier-sims]",
                                   Perm<8>,
                                   PERM_TYPES) {
    auto                               rg = ReportGuard(false);
    SchreierSims<8, uint8_t, TestType> S;
    S.add_generator(make<TestType>({0, 6, 2, 3, 4, 5, 1, 7}));

    auto W = S;
    REQUIRE(W.size() == 2);

    decltype(S) T(S);
    REQUIRE(!S.finished());
    REQUIRE(!T.finished());

    S.add_generator(make<TestType>({1, 2, 3, 4, 5, 6, 7, 0}));
    T.add_generator(make<TestType>({1, 2, 3, 4, 5, 6, 7, 0}));
    REQUIRE(S.size() == 40'320);
    REQUIRE(T.size() == 40'320);
    REQUIRE(to_human_readable_repr(S)
            == "<SchreierSims with 2 generators, base size 7 & size 40,320>");

    decltype(T) U(T);
    REQUIRE(U.finished());
    REQUIRE(U.size() == T.size());
  }

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "005",
                                   "symmetric group (degree 9)",
                                   "[quick][schreier-sims]",
                                   Perm<9>,
                                   PERM_TYPES) {
    auto                               rg = ReportGuard(false);
    SchreierSims<9, uint8_t, TestType> S;
    S.add_generator(make<TestType>({1, 0, 2, 3, 4, 5, 6, 7, 8}));
    S.add_generator(make<TestType>({1, 2, 3, 4, 5, 6, 7, 8, 0}));
    REQUIRE(S.size() == 362'880);
  }

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "006",
                                   "alternating group (degree 12)",
                                   "[quick][schreier-sims]",
                                   Perm<12>,
                                   PERM_TYPES) {
    auto                                rg = ReportGuard(false);
    SchreierSims<12, uint8_t, TestType> S;
    S.add_generator(make<TestType>({1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
    S.add_generator(make<TestType>({0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 1}));

    REQUIRE(
        !S.contains(make<TestType>({11, 10, 0, 6, 8, 2, 3, 5, 4, 7, 9, 1})));
    REQUIRE(S.contains(make<TestType>({7, 11, 2, 3, 0, 6, 9, 10, 8, 5, 4, 1})));
    if constexpr (std::is_same_v<TestType, Perm<>>) {
      REQUIRE(!S.contains(
          make<TestType>({11, 10, 0, 6, 8, 2, 3, 5, 4, 7, 9, 1, 12})));
    } else {
      // Extended to 16 points to exceed the number of points in HPCombi::Perm16
      REQUIRE_THROWS_AS(
          make<TestType>(
              {11, 10, 0, 6, 8, 2, 3, 5, 4, 7, 9, 1, 12, 13, 14, 15, 16}),
          LibsemigroupsException);
    }
    REQUIRE(S.size() == 239'500'800);
  }

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "007",
                                   "symmetric group (degree 16)",
                                   "[quick][schreier-sims]",
                                   Perm<16>,
                                   PERM_TYPES) {
    auto                                rg = ReportGuard(false);
    SchreierSims<16, uint8_t, TestType> S;
    S.add_generator(
        make<TestType>({1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
    S.add_generator(
        make<TestType>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0}));
    REQUIRE(S.size() == static_cast<uint64_t>(20'922'789'888'000));
  }

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "008",
                                   "alternating group (degree 15)",
                                   "[quick][schreier-sims]",
                                   Perm<15>,
                                   PERM_TYPES) {
    auto                                rg = ReportGuard(false);
    SchreierSims<15, uint8_t, TestType> S;
    S.add_generator(
        make<TestType>({1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}));
    S.add_generator(
        make<TestType>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0}));
    REQUIRE(S.size() == static_cast<uint64_t>(653'837'184'000));

    REQUIRE(S.contains(
        make<TestType>({0, 1, 7, 8, 9, 10, 11, 12, 13, 14, 2, 3, 4, 5, 6})));
    REQUIRE(S.contains(
        make<TestType>({1, 12, 0, 13, 14, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11})));
    REQUIRE(S.contains(
        make<TestType>({12, 0, 1, 13, 14, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11})));
    REQUIRE(!S.contains(
        make<TestType>({12, 0, 1, 13, 14, 2, 3, 5, 4, 6, 7, 8, 9, 10, 11})));
    REQUIRE(!S.contains(
        make<TestType>({1, 12, 0, 14, 13, 3, 2, 5, 4, 6, 7, 8, 9, 10, 11})));
    REQUIRE(!S.contains(
        make<TestType>({0, 1, 7, 9, 8, 11, 10, 12, 13, 14, 2, 3, 6, 5, 4})));
  }

  LIBSEMIGROUPS_TEMPLATE_TEST_CASE("SchreierSims",
                                   "009",
                                   "alternating group (degree 16)",
                                   "[quick][schreier-sims]",
                                   Perm<16>,
                                   PERM_TYPES) {
    auto                                rg = ReportGuard(false);
    SchreierSims<16, uint8_t, TestType> S;
    S.add_generator(
        make<TestType>({1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
    S.add_generator(
        make<TestType>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15}));
    S.add_generator(
        make<TestType>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13}));
    REQUIRE(to_human_readable_repr(S)
            == "<partially enumerated SchreierSims with 3 generators & base "
               "size 13>");
    REQUIRE(S.size() == static_cast<uint64_t>(10'461'394'944'000));
    REQUIRE(S.contains(make<TestType>(
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13})));
    REQUIRE(!S.contains(make<TestType>(
        {1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13})));
    REQUIRE(S.contains(make<TestType>(
        {1, 0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13})));

    REQUIRE(S.contains(make<TestType>(
        {1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})));
    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(S.generator(1)));
    REQUIRE(S.contains(S.generator(2)));
    REQUIRE(S.contains(make<TestType>(
        {0, 1, 7, 8, 9, 10, 11, 12, 13, 14, 2, 3, 4, 5, 6, 15})));
    REQUIRE(S.contains(make<TestType>(
        {1, 12, 0, 13, 14, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15})));
    REQUIRE(S.contains(make<TestType>(
        {12, 0, 1, 13, 14, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15})));
    REQUIRE(!S.contains(make<TestType>(
        {12, 0, 1, 13, 14, 2, 3, 5, 4, 6, 7, 8, 9, 10, 11, 15})));
    REQUIRE(!S.contains(make<TestType>(
        {1, 12, 0, 14, 13, 3, 2, 5, 4, 6, 7, 8, 9, 10, 11, 15})));
    REQUIRE(!S.contains(make<TestType>(
        {0, 1, 7, 9, 8, 11, 10, 12, 13, 14, 2, 3, 6, 5, 4, 15})));

    REQUIRE(!S.contains(make<TestType>(
        {1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13})));
    S.add_generator(
        make<TestType>({1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13}));
    REQUIRE(S.contains(make<TestType>(
        {1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 13})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "010",
                          "dihedral perm. group (order 10)",
                          "[quick][schreier-sims]") {
    auto            rg = ReportGuard(false);
    SchreierSims<5> S;
    using Perm = decltype(S)::element_type;
    S.add_generator(make<Perm>({0, 4, 3, 2, 1}));
    S.add_generator(make<Perm>({1, 2, 3, 4, 0}));

    REQUIRE(S.size() == 10);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "011",
                          "dihedral perm. group (order 200)",
                          "[quick][schreier-sims]") {
    auto rg = ReportGuard(false);
    // At N = 4'000 or so, this starts to take an appreciable amount of time.
    constexpr size_t N = 200;
    auto             S = std::make_unique<SchreierSims<N>>();
    using Perm         = std::remove_reference_t<decltype(*S)>::element_type;

    Perm::container_type cntnr;
    std::fill(cntnr.begin(), cntnr.end(), 0);
    std::iota(cntnr.begin(), cntnr.end() - 1, 1);
    S->add_generator(make<Perm>(cntnr));
    std::iota(cntnr.rbegin(), cntnr.rend() - 1, 1);
    cntnr[0] = 0;
    S->add_generator(make<Perm>(cntnr));
    REQUIRE(S->size() == 2 * N);

    REQUIRE(S->contains(S->generator(0)));
    REQUIRE(S->contains(S->generator(1)));
    REQUIRE(S->contains(S->generator(1) * S->generator(0)));

    std::iota(cntnr.begin(), cntnr.end(), 0);
    for (auto it = cntnr.begin(); it < cntnr.end(); it += 2) {
      std::swap(*it, *(it + 1));
    }
    REQUIRE(!S->contains(make<Perm>(cntnr)));
    std::swap(*cntnr.begin(), *(cntnr.begin() + 1));
    REQUIRE(!S->contains(make<Perm>(cntnr)));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "012",
                          "perm. group T (order 12)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    SchreierSims<12> S;
    using Perm = decltype(S)::element_type;
    S.add_generator(make<Perm>({1, 2, 3, 4, 5, 0, 7, 8, 9, 10, 11, 6}));
    S.add_generator(make<Perm>({6, 11, 10, 9, 8, 7, 3, 2, 1, 0, 5, 4}));
    REQUIRE(S.size() == 12);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "013",
                          "quaternion perm. group (order 8)",
                          "[quick][schreier-sims]") {
    auto            rg = ReportGuard(false);
    SchreierSims<9> S;
    using Perm = decltype(S)::element_type;
    S.add_generator(make<Perm>({0, 2, 4, 6, 7, 3, 8, 1, 5}));
    S.add_generator(make<Perm>({0, 3, 5, 4, 8, 7, 2, 6, 1}));
    REQUIRE(S.generator(0) == make<Perm>({0, 2, 4, 6, 7, 3, 8, 1, 5}));

    REQUIRE(S.size() == 8);
    REQUIRE(S.sift(S.generator(0)) == S.one());
    REQUIRE(S.sift(S.generator(1)) == S.one());
    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(S.generator(1)));
    REQUIRE(S.contains(make<Perm>({0, 6, 3, 7, 5, 1, 4, 8, 2})));
    REQUIRE(S.contains(make<Perm>({0, 8, 6, 1, 3, 2, 7, 5, 4})));
    REQUIRE(!S.contains(make<Perm>({0, 1, 5, 4, 8, 7, 2, 6, 3})));
    REQUIRE(!S.contains(make<Perm>({3, 5, 4, 6, 1, 8, 2, 7, 0})));
    REQUIRE(!S.contains(make<Perm>({1, 3, 2, 5, 7, 8, 6, 4, 0})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "014",
                          "perm. group (order 84'129'611'558'952'960)",
                          "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 729;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
         14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
         28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
         42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
         70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
         84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,
         98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
         112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
         126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
         140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
         154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
         168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
         182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195,
         196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
         210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
         224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
         238, 239, 240, 241, 242, 486, 487, 488, 489, 490, 491, 492, 493, 494,
         495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508,
         509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522,
         523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536,
         537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550,
         551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564,
         565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578,
         579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592,
         593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606,
         607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620,
         621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634,
         635, 636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648,
         649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662,
         663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676,
         677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690,
         691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704,
         705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718,
         719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 243, 244, 245, 246,
         247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260,
         261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274,
         275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288,
         289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302,
         303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316,
         317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330,
         331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344,
         345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358,
         359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372,
         373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386,
         387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400,
         401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414,
         415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428,
         429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442,
         443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456,
         457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470,
         471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484,
         485}));
    S->add_generator(make<Perm>(
        {0,   6,   3,   18,  24,  21,  9,   15,  12,  54,  60,  57,  72,  78,
         75,  63,  69,  66,  27,  33,  30,  45,  51,  48,  36,  42,  39,  162,
         168, 165, 180, 186, 183, 171, 177, 174, 216, 222, 219, 234, 240, 237,
         225, 231, 228, 189, 195, 192, 207, 213, 210, 198, 204, 201, 81,  87,
         84,  99,  105, 102, 90,  96,  93,  135, 141, 138, 153, 159, 156, 144,
         150, 147, 108, 114, 111, 126, 132, 129, 117, 123, 120, 486, 492, 489,
         504, 510, 507, 495, 501, 498, 540, 546, 543, 558, 564, 561, 549, 555,
         552, 513, 519, 516, 531, 537, 534, 522, 528, 525, 648, 654, 651, 666,
         672, 669, 657, 663, 660, 702, 708, 705, 720, 726, 723, 711, 717, 714,
         675, 681, 678, 693, 699, 696, 684, 690, 687, 567, 573, 570, 585, 591,
         588, 576, 582, 579, 621, 627, 624, 639, 645, 642, 630, 636, 633, 594,
         600, 597, 612, 618, 615, 603, 609, 606, 243, 249, 246, 261, 267, 264,
         252, 258, 255, 297, 303, 300, 315, 321, 318, 306, 312, 309, 270, 276,
         273, 288, 294, 291, 279, 285, 282, 405, 411, 408, 423, 429, 426, 414,
         420, 417, 459, 465, 462, 477, 483, 480, 468, 474, 471, 432, 438, 435,
         450, 456, 453, 441, 447, 444, 324, 330, 327, 342, 348, 345, 333, 339,
         336, 378, 384, 381, 396, 402, 399, 387, 393, 390, 351, 357, 354, 369,
         375, 372, 360, 366, 363, 487, 493, 490, 505, 511, 508, 496, 502, 499,
         541, 547, 544, 559, 565, 562, 550, 556, 553, 514, 520, 517, 532, 538,
         535, 523, 529, 526, 649, 655, 652, 667, 673, 670, 658, 664, 661, 703,
         709, 706, 721, 727, 724, 712, 718, 715, 676, 682, 679, 694, 700, 697,
         685, 691, 688, 568, 574, 571, 586, 592, 589, 577, 583, 580, 622, 628,
         625, 640, 646, 643, 631, 637, 634, 595, 601, 598, 613, 619, 616, 604,
         610, 607, 244, 250, 247, 262, 268, 265, 253, 259, 256, 298, 304, 301,
         316, 322, 319, 307, 313, 310, 271, 277, 274, 289, 295, 292, 280, 286,
         283, 406, 412, 409, 424, 430, 427, 415, 421, 418, 460, 466, 463, 478,
         484, 481, 469, 475, 472, 433, 439, 436, 451, 457, 454, 442, 448, 445,
         325, 331, 328, 343, 349, 346, 334, 340, 337, 379, 385, 382, 397, 403,
         400, 388, 394, 391, 352, 358, 355, 370, 376, 373, 361, 367, 364, 1,
         7,   4,   19,  25,  22,  10,  16,  13,  55,  61,  58,  73,  79,  76,
         64,  70,  67,  28,  34,  31,  46,  52,  49,  37,  43,  40,  163, 169,
         166, 181, 187, 184, 172, 178, 175, 217, 223, 220, 235, 241, 238, 226,
         232, 229, 190, 196, 193, 208, 214, 211, 199, 205, 202, 82,  88,  85,
         100, 106, 103, 91,  97,  94,  136, 142, 139, 154, 160, 157, 145, 151,
         148, 109, 115, 112, 127, 133, 130, 118, 124, 121, 245, 251, 248, 263,
         269, 266, 254, 260, 257, 299, 305, 302, 317, 323, 320, 308, 314, 311,
         272, 278, 275, 290, 296, 293, 281, 287, 284, 407, 413, 410, 425, 431,
         428, 416, 422, 419, 461, 467, 464, 479, 485, 482, 470, 476, 473, 434,
         440, 437, 452, 458, 455, 443, 449, 446, 326, 332, 329, 344, 350, 347,
         335, 341, 338, 380, 386, 383, 398, 404, 401, 389, 395, 392, 353, 359,
         356, 371, 377, 374, 362, 368, 365, 2,   8,   5,   20,  26,  23,  11,
         17,  14,  56,  62,  59,  74,  80,  77,  65,  71,  68,  29,  35,  32,
         47,  53,  50,  38,  44,  41,  164, 170, 167, 182, 188, 185, 173, 179,
         176, 218, 224, 221, 236, 242, 239, 227, 233, 230, 191, 197, 194, 209,
         215, 212, 200, 206, 203, 83,  89,  86,  101, 107, 104, 92,  98,  95,
         137, 143, 140, 155, 161, 158, 146, 152, 149, 110, 116, 113, 128, 134,
         131, 119, 125, 122, 488, 494, 491, 506, 512, 509, 497, 503, 500, 542,
         548, 545, 560, 566, 563, 551, 557, 554, 515, 521, 518, 533, 539, 536,
         524, 530, 527, 650, 656, 653, 668, 674, 671, 659, 665, 662, 704, 710,
         707, 722, 728, 725, 713, 719, 716, 677, 683, 680, 695, 701, 698, 686,
         692, 689, 569, 575, 572, 587, 593, 590, 578, 584, 581, 623, 629, 626,
         641, 647, 644, 632, 638, 635, 596, 602, 599, 614, 620, 617, 605, 611,
         608}));

    REQUIRE(S->size() == static_cast<uint64_t>(84'129'611'558'952'960));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "015",
                          "perm. group SL(3, 5) (order 372'000)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 126;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,   1,   76,  101, 26,  51,  6,   81,  106, 31,  56,  11,  86,  111,
         36,  61,  16,  91,  116, 41,  66,  21,  96,  121, 46,  71,  2,   77,
         102, 27,  52,  7,   82,  107, 32,  57,  12,  87,  112, 37,  62,  17,
         92,  117, 42,  67,  22,  97,  122, 47,  72,  3,   78,  103, 28,  53,
         8,   83,  108, 33,  58,  13,  88,  113, 38,  63,  18,  93,  118, 43,
         68,  23,  98,  123, 48,  73,  4,   79,  104, 29,  54,  9,   84,  109,
         34,  59,  14,  89,  114, 39,  64,  19,  94,  119, 44,  69,  24,  99,
         124, 49,  74,  5,   80,  105, 30,  55,  10,  85,  110, 35,  60,  15,
         90,  115, 40,  65,  20,  95,  120, 45,  70,  25,  100, 125, 50,  75}));
    S->add_generator(make<Perm>(
        {0,   1,   77,  103, 29,  55,  6,   82,  108, 34,  60,  11,  87,  113,
         39,  65,  16,  92,  118, 44,  70,  21,  97,  123, 49,  75,  2,   78,
         105, 26,  54,  7,   83,  110, 31,  59,  12,  88,  115, 36,  64,  17,
         93,  120, 41,  69,  22,  98,  125, 46,  74,  3,   80,  104, 27,  51,
         8,   85,  109, 32,  56,  13,  90,  114, 37,  61,  18,  95,  119, 42,
         66,  23,  100, 124, 47,  71,  4,   76,  102, 30,  53,  9,   81,  107,
         35,  58,  14,  86,  112, 40,  63,  19,  91,  117, 45,  68,  24,  96,
         122, 50,  73,  5,   79,  101, 28,  52,  10,  84,  106, 33,  57,  15,
         89,  111, 38,  62,  20,  94,  116, 43,  67,  25,  99,  121, 48,  72}));
    S->add_generator(make<Perm>(
        {0,   1,   81,  111, 41,  71,  6,   86,  121, 26,  66,  11,  96,  116,
         31,  51,  16,  76,  106, 46,  61,  21,  91,  101, 36,  56,  2,   82,
         112, 42,  72,  7,   87,  122, 27,  67,  12,  97,  117, 32,  52,  17,
         77,  107, 47,  62,  22,  92,  102, 37,  57,  3,   83,  113, 43,  73,
         8,   88,  123, 28,  68,  13,  98,  118, 33,  53,  18,  78,  108, 48,
         63,  23,  93,  103, 38,  58,  4,   84,  114, 44,  74,  9,   89,  124,
         29,  69,  14,  99,  119, 34,  54,  19,  79,  109, 49,  64,  24,  94,
         104, 39,  59,  5,   85,  115, 45,  75,  10,  90,  125, 30,  70,  15,
         100, 120, 35,  55,  20,  80,  110, 50,  65,  25,  95,  105, 40,  60}));
    S->add_generator(make<Perm>(
        {0,   1,   76,  101, 26,  51,  7,   82,  107, 32,  57,  13,  88,  113,
         38,  63,  19,  94,  119, 44,  69,  25,  100, 125, 50,  75,  2,   77,
         102, 27,  52,  8,   83,  108, 33,  58,  15,  90,  115, 40,  65,  16,
         91,  116, 41,  66,  24,  99,  124, 49,  74,  3,   78,  103, 28,  53,
         10,  85,  110, 35,  60,  14,  89,  114, 39,  64,  17,  92,  117, 42,
         67,  21,  96,  121, 46,  71,  4,   79,  104, 29,  54,  6,   81,  106,
         31,  56,  12,  87,  112, 37,  62,  20,  95,  120, 45,  70,  23,  98,
         123, 48,  73,  5,   80,  105, 30,  55,  9,   84,  109, 34,  59,  11,
         86,  111, 36,  61,  18,  93,  118, 43,  68,  22,  97,  122, 47,  72}));

    REQUIRE(S->size() == 372'000);

    REQUIRE(S->contains(S->generator(0)));
    REQUIRE(S->contains(S->generator(1)));
    REQUIRE(S->contains(S->generator(2)));
    REQUIRE(S->contains(S->generator(3)));
    REQUIRE(!S->contains(make<Perm>(
        {0,  1,  76,  101, 26, 51, 7,  82,  107, 32, 57, 13, 88, 113, 38,
         63, 19, 94,  119, 44, 69, 25, 100, 125, 50, 75, 2,  77, 102, 27,
         52, 8,  83,  108, 33, 58, 15, 90,  115, 40, 65, 16, 91, 116, 41,
         66, 24, 99,  124, 49, 74, 3,  78,  103, 28, 53, 10, 85, 110, 35,
         60, 14, 89,  114, 39, 64, 17, 92,  117, 42, 67, 21, 96, 121, 46,
         71, 4,  79,  104, 29, 54, 6,  81,  106, 31, 56, 12, 87, 112, 37,
         62, 20, 120, 95,  45, 70, 23, 98,  123, 48, 73, 5,  80, 105, 30,
         55, 9,  84,  109, 34, 59, 11, 86,  111, 36, 61, 18, 93, 118, 43,
         68, 22, 97,  122, 47, 72})));

    REQUIRE(!S->contains(make<Perm>(
        {1,  77, 103, 109, 55, 6,  82, 108, 34,  60, 11, 87,  113, 39,  65,
         16, 92, 118, 44,  70, 21, 97, 123, 49,  75, 2,  78,  105, 26,  54,
         7,  83, 110, 31,  59, 12, 88, 115, 36,  64, 17, 93,  120, 41,  69,
         22, 98, 125, 46,  74, 3,  80, 104, 27,  51, 8,  85,  29,  32,  56,
         13, 90, 114, 37,  61, 18, 95, 119, 42,  66, 23, 100, 124, 47,  71,
         4,  76, 0,   102, 30, 53, 9,  81,  107, 35, 58, 14,  86,  112, 40,
         63, 19, 91,  117, 45, 68, 24, 96,  122, 50, 73, 5,   79,  101, 28,
         52, 10, 84,  106, 33, 57, 15, 89,  111, 38, 62, 20,  94,  116, 43,
         67, 25, 99,  121, 48, 72})));

    REQUIRE(!S->contains(make<Perm>(
        {0,  1,  80, 77,  103, 109, 55, 6,  82,  108, 34, 60, 11,  87,  113,
         39, 65, 16, 92,  118, 44,  70, 21, 97,  123, 49, 75, 2,   78,  105,
         26, 54, 7,  83,  110, 31,  59, 12, 88,  115, 36, 64, 17,  93,  120,
         41, 69, 22, 98,  125, 46,  74, 3,  104, 27,  51, 8,  85,  29,  32,
         56, 13, 90, 114, 37,  61,  18, 95, 119, 42,  66, 23, 100, 124, 47,
         71, 4,  76, 102, 30,  53,  9,  81, 107, 35,  58, 14, 86,  112, 40,
         63, 19, 91, 117, 45,  68,  24, 96, 122, 50,  73, 5,  79,  101, 28,
         52, 10, 84, 106, 33,  57,  15, 89, 111, 38,  62, 20, 94,  116, 43,
         67, 25, 99, 121, 48,  72})));

    REQUIRE(S->contains(make<Perm>(
        {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
         13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
         41,  42,  43,  44,  45,  26,  27,  28,  29,  30,  31,  32,  33,
         34,  35,  46,  47,  48,  49,  50,  36,  37,  38,  39,  40,  71,
         72,  73,  74,  75,  66,  67,  68,  69,  70,  51,  52,  53,  54,
         55,  61,  62,  63,  64,  65,  56,  57,  58,  59,  60,  81,  82,
         83,  84,  85,  86,  87,  88,  89,  90,  96,  97,  98,  99,  100,
         76,  77,  78,  79,  80,  91,  92,  93,  94,  95,  111, 112, 113,
         114, 115, 121, 122, 123, 124, 125, 116, 117, 118, 119, 120, 106,
         107, 108, 109, 110, 101, 102, 103, 104, 105})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "016",
                          "perm. group PSL(4, 8) (order 34'558'531'338'240)",
                          "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 585;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {1,   9,   17,  25,  33,  41,  49,  57,  65,  73,  81,  89,  97,  105,
         113, 121, 129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 209, 217,
         225, 233, 241, 249, 257, 265, 273, 281, 289, 297, 305, 313, 321, 329,
         337, 345, 353, 361, 369, 377, 385, 393, 401, 409, 417, 425, 433, 441,
         449, 457, 465, 473, 481, 489, 497, 505, 513, 521, 529, 537, 545, 553,
         561, 569, 577, 74,  82,  90,  98,  106, 114, 122, 130, 138, 146, 154,
         162, 170, 178, 186, 194, 202, 210, 218, 226, 234, 242, 250, 258, 266,
         274, 282, 290, 298, 306, 314, 322, 330, 338, 346, 354, 362, 370, 378,
         386, 394, 402, 410, 418, 426, 434, 442, 450, 458, 466, 474, 482, 490,
         498, 506, 514, 522, 530, 538, 546, 554, 562, 570, 578, 0,   2,   8,
         7,   6,   5,   4,   3,   10,  18,  26,  34,  42,  50,  58,  66,  16,
         72,  24,  32,  40,  48,  56,  64,  15,  63,  71,  23,  31,  39,  47,
         55,  14,  54,  62,  70,  22,  30,  38,  46,  13,  45,  53,  61,  69,
         21,  29,  37,  12,  36,  44,  52,  60,  68,  20,  28,  11,  27,  35,
         43,  51,  59,  67,  19,  78,  118, 126, 134, 86,  94,  102, 110, 398,
         438, 446, 454, 406, 414, 422, 430, 462, 502, 510, 518, 470, 478, 486,
         494, 526, 566, 574, 582, 534, 542, 550, 558, 142, 182, 190, 198, 150,
         158, 166, 174, 206, 246, 254, 262, 214, 222, 230, 238, 270, 310, 318,
         326, 278, 286, 294, 302, 334, 374, 382, 390, 342, 350, 358, 366, 75,
         91,  99,  107, 115, 123, 131, 83,  203, 219, 227, 235, 243, 251, 259,
         211, 267, 283, 291, 299, 307, 315, 323, 275, 331, 347, 355, 363, 371,
         379, 387, 339, 395, 411, 419, 427, 435, 443, 451, 403, 459, 475, 483,
         491, 499, 507, 515, 467, 523, 539, 547, 555, 563, 571, 579, 531, 139,
         155, 163, 171, 179, 187, 195, 147, 80,  136, 88,  96,  104, 112, 120,
         128, 528, 584, 536, 544, 552, 560, 568, 576, 144, 200, 152, 160, 168,
         176, 184, 192, 208, 264, 216, 224, 232, 240, 248, 256, 272, 328, 280,
         288, 296, 304, 312, 320, 336, 392, 344, 352, 360, 368, 376, 384, 400,
         456, 408, 416, 424, 432, 440, 448, 464, 520, 472, 480, 488, 496, 504,
         512, 76,  100, 108, 116, 124, 132, 84,  92,  268, 292, 300, 308, 316,
         324, 276, 284, 332, 356, 364, 372, 380, 388, 340, 348, 396, 420, 428,
         436, 444, 452, 404, 412, 460, 484, 492, 500, 508, 516, 468, 476, 524,
         548, 556, 564, 572, 580, 532, 540, 140, 164, 172, 180, 188, 196, 148,
         156, 204, 228, 236, 244, 252, 260, 212, 220, 77,  109, 117, 125, 133,
         85,  93,  101, 333, 365, 373, 381, 389, 341, 349, 357, 397, 429, 437,
         445, 453, 405, 413, 421, 461, 493, 501, 509, 517, 469, 477, 485, 525,
         557, 565, 573, 581, 533, 541, 549, 141, 173, 181, 189, 197, 149, 157,
         165, 205, 237, 245, 253, 261, 213, 221, 229, 269, 301, 309, 317, 325,
         277, 285, 293, 79,  127, 135, 87,  95,  103, 111, 119, 463, 511, 519,
         471, 479, 487, 495, 503, 527, 575, 583, 535, 543, 551, 559, 567, 143,
         191, 199, 151, 159, 167, 175, 183, 207, 255, 263, 215, 223, 231, 239,
         247, 271, 319, 327, 279, 287, 295, 303, 311, 335, 383, 391, 343, 351,
         359, 367, 375, 399, 447, 455, 407, 415, 423, 431, 439}));
    S->add_generator(make<Perm>(
        {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   11,  12,  13,  14,
         15,  16,  10,  25,  27,  28,  29,  30,  31,  32,  26,  33,  35,  36,
         37,  38,  39,  40,  34,  41,  43,  44,  45,  46,  47,  48,  42,  49,
         51,  52,  53,  54,  55,  56,  50,  57,  59,  60,  61,  62,  63,  64,
         58,  65,  67,  68,  69,  70,  71,  72,  66,  17,  19,  20,  21,  22,
         23,  24,  18,  73,  80,  74,  75,  76,  77,  78,  79,  129, 136, 130,
         131, 132, 133, 134, 135, 81,  88,  82,  83,  84,  85,  86,  87,  89,
         96,  90,  91,  92,  93,  94,  95,  97,  104, 98,  99,  100, 101, 102,
         103, 105, 112, 106, 107, 108, 109, 110, 111, 113, 120, 114, 115, 116,
         117, 118, 119, 121, 128, 122, 123, 124, 125, 126, 127, 457, 464, 458,
         459, 460, 461, 462, 463, 513, 520, 514, 515, 516, 517, 518, 519, 465,
         472, 466, 467, 468, 469, 470, 471, 473, 480, 474, 475, 476, 477, 478,
         479, 481, 488, 482, 483, 484, 485, 486, 487, 489, 496, 490, 491, 492,
         493, 494, 495, 497, 504, 498, 499, 500, 501, 502, 503, 505, 512, 506,
         507, 508, 509, 510, 511, 521, 528, 522, 523, 524, 525, 526, 527, 577,
         584, 578, 579, 580, 581, 582, 583, 529, 536, 530, 531, 532, 533, 534,
         535, 537, 544, 538, 539, 540, 541, 542, 543, 545, 552, 546, 547, 548,
         549, 550, 551, 553, 560, 554, 555, 556, 557, 558, 559, 561, 568, 562,
         563, 564, 565, 566, 567, 569, 576, 570, 571, 572, 573, 574, 575, 137,
         144, 138, 139, 140, 141, 142, 143, 193, 200, 194, 195, 196, 197, 198,
         199, 145, 152, 146, 147, 148, 149, 150, 151, 153, 160, 154, 155, 156,
         157, 158, 159, 161, 168, 162, 163, 164, 165, 166, 167, 169, 176, 170,
         171, 172, 173, 174, 175, 177, 184, 178, 179, 180, 181, 182, 183, 185,
         192, 186, 187, 188, 189, 190, 191, 201, 208, 202, 203, 204, 205, 206,
         207, 257, 264, 258, 259, 260, 261, 262, 263, 209, 216, 210, 211, 212,
         213, 214, 215, 217, 224, 218, 219, 220, 221, 222, 223, 225, 232, 226,
         227, 228, 229, 230, 231, 233, 240, 234, 235, 236, 237, 238, 239, 241,
         248, 242, 243, 244, 245, 246, 247, 249, 256, 250, 251, 252, 253, 254,
         255, 265, 272, 266, 267, 268, 269, 270, 271, 321, 328, 322, 323, 324,
         325, 326, 327, 273, 280, 274, 275, 276, 277, 278, 279, 281, 288, 282,
         283, 284, 285, 286, 287, 289, 296, 290, 291, 292, 293, 294, 295, 297,
         304, 298, 299, 300, 301, 302, 303, 305, 312, 306, 307, 308, 309, 310,
         311, 313, 320, 314, 315, 316, 317, 318, 319, 329, 336, 330, 331, 332,
         333, 334, 335, 385, 392, 386, 387, 388, 389, 390, 391, 337, 344, 338,
         339, 340, 341, 342, 343, 345, 352, 346, 347, 348, 349, 350, 351, 353,
         360, 354, 355, 356, 357, 358, 359, 361, 368, 362, 363, 364, 365, 366,
         367, 369, 376, 370, 371, 372, 373, 374, 375, 377, 384, 378, 379, 380,
         381, 382, 383, 393, 400, 394, 395, 396, 397, 398, 399, 449, 456, 450,
         451, 452, 453, 454, 455, 401, 408, 402, 403, 404, 405, 406, 407, 409,
         416, 410, 411, 412, 413, 414, 415, 417, 424, 418, 419, 420, 421, 422,
         423, 425, 432, 426, 427, 428, 429, 430, 431, 433, 440, 434, 435, 436,
         437, 438, 439, 441, 448, 442, 443, 444, 445, 446, 447}));

    REQUIRE(S->size() == static_cast<uint64_t>(34'558'531'338'240));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "017",
                          "perm. Mathieu group M11 (order 7'920)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 11;
    using Perm          = Perm<0, size_t>;
    auto S              = SchreierSims<N, size_t, Perm>();

    S.add_generator(make<Perm>({0, 9, 2, 10, 6, 5, 4, 8, 7, 1, 3}));
    S.add_generator(make<Perm>({3, 4, 7, 2, 5, 8, 6, 0, 1, 9, 10}));

    REQUIRE(S.size() == 7'920);
    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(S.generator(1)));
    REQUIRE(S.contains(make<Perm>({10, 8, 5, 0, 1, 2, 4, 9, 7, 6, 3})));
    REQUIRE(S.contains(make<Perm>({3, 6, 0, 9, 4, 10, 7, 5, 2, 8, 1})));
    REQUIRE(!S.contains(make<Perm>({4, 5, 6, 7, 8, 9, 10, 0, 1, 2, 3})));
    REQUIRE(!S.contains(make<Perm>({6, 7, 8, 9, 10, 0, 2, 1, 3, 5, 4})));
    REQUIRE(!S.contains(make<Perm>({9, 10, 1, 2, 3, 4, 0, 5, 6, 7, 8})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "018",
                          "perm. Mathieu group M24 (order 244'823'040)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 24;
    auto             S  = SchreierSims<N>();
    using Perm          = decltype(S)::element_type;

    S.add_generator(
        make<Perm>({1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 0,  23}));
    S.add_generator(make<Perm>({0,  1,  16, 12, 3, 5,  8, 17, 2,  6,  11, 22,
                                13, 18, 19, 14, 9, 10, 4, 21, 15, 20, 7,  23}));

    S.add_generator(make<Perm>({23, 22, 11, 15, 17, 9, 19, 13, 20, 5,  16, 2,
                                21, 7,  18, 3,  10, 4, 14, 6,  8,  12, 1,  0}));

    REQUIRE(S.size() == 244'823'040);
    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(S.generator(1)));
    REQUIRE(
        S.contains(make<Perm>({1,  16, 12, 3, 5,  8, 17, 2,  6,  11, 22, 13,
                               18, 19, 14, 9, 10, 4, 21, 15, 20, 7,  0,  23})));
    REQUIRE(
        S.contains(make<Perm>({12, 16, 18, 10, 20, 14, 21, 6, 17, 3, 22, 8, 19,
                               4,  11, 5,  15, 7,  9,  13, 2, 23, 0, 1})));
    REQUIRE(
        S.contains(make<Perm>({19, 9, 21, 13, 10, 22, 6, 16, 7,  8, 11, 17,
                               12, 5, 15, 1,  14, 0,  4, 18, 20, 3, 23, 2})));
    REQUIRE(!S.contains(
        make<Perm>({0,  3,  2,  1,  4,  5,  6,  7,  8,  9,  10, 11,
                    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23})));
    REQUIRE(!S.contains(
        make<Perm>({0,  1,  3,  4,  2,  5,  6,  7,  8,  9,  10, 11,
                    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23})));
    REQUIRE(!S.contains(
        make<Perm>({0,  1,  3,  4,  2,  6,  5,  7,  8,  9,  10, 11,
                    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "019",
                          "perm. Janko Group J1 (order 175'560)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 267;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,   262, 107, 21,  213, 191, 22,  133, 234, 232, 151, 139, 176, 202,
         253, 222, 16,  195, 206, 68,  55,  3,   6,   179, 217, 216, 256, 87,
         70,  131, 44,  105, 170, 77,  104, 198, 137, 243, 56,  124, 223, 134,
         42,  174, 30,  45,  51,  128, 94,  250, 264, 46,  183, 231, 115, 20,
         38,  85,  233, 261, 95,  235, 177, 249, 91,  247, 155, 67,  19,  219,
         28,  237, 211, 84,  192, 130, 251, 33,  78,  260, 112, 193, 156, 242,
         73,  57,  238, 27,  143, 168, 148, 64,  119, 212, 48,  60,  150, 199,
         140, 189, 180, 147, 111, 159, 34,  31,  162, 2,   194, 166, 200, 102,
         80,  120, 141, 54,  182, 181, 225, 92,  113, 254, 125, 146, 39,  122,
         208, 221, 47,  210, 75,  29,  255, 7,   41,  135, 175, 36,  207, 11,
         98,  114, 240, 88,  172, 185, 123, 101, 90,  224, 96,  10,  169, 241,
         190, 66,  82,  214, 161, 103, 236, 158, 106, 239, 229, 230, 109, 188,
         89,  152, 32,  258, 144, 186, 43,  136, 12,  62,  245, 23,  100, 117,
         116, 52,  205, 145, 173, 228, 167, 99,  154, 5,   74,  81,  108, 17,
         196, 203, 35,  97,  110, 252, 13,  197, 204, 184, 18,  138, 126, 248,
         129, 72,  93,  4,   157, 259, 25,  24,  246, 69,  227, 127, 15,  40,
         149, 118, 226, 220, 187, 164, 165, 53,  9,   58,  8,   61,  160, 71,
         86,  163, 142, 153, 83,  37,  244, 178, 218, 65,  209, 63,  49,  76,
         201, 14,  121, 132, 26,  263, 171, 215, 79,  59,  1,   257, 50,  266,
         265}));
    S->add_generator(make<Perm>(
        {0,   146, 132, 3,   156, 242, 107, 125, 245, 174, 241, 264, 248, 36,
         116, 47,  178, 170, 197, 233, 121, 1,   228, 48,  201, 15,  136, 212,
         6,   175, 77,  237, 30,  226, 31,  129, 44,  161, 232, 219, 78,  139,
         9,   211, 13,  222, 97,  25,  173, 70,  153, 186, 29,  203, 35,  169,
         140, 260, 91,  199, 108, 208, 206, 11,  55,  103, 65,  95,  73,  151,
         131, 41,  221, 225, 18,  143, 7,   32,  159, 217, 93,  181, 2,   258,
         163, 154, 182, 38,  133, 117, 33,  243, 191, 122, 27,  205, 20,  135,
         98,  229, 138, 61,  194, 66,  104, 149, 62,  28,  164, 123, 17,  137,
         16,  69,  37,  238, 128, 247, 57,  167, 134, 96,  80,  193, 185, 76,
         83,  218, 14,  54,  8,   49,  82,  215, 189, 46,  190, 183, 188, 71,
         230, 231, 239, 202, 224, 158, 21,  119, 214, 184, 250, 113, 72,  200,
         213, 22,  166, 102, 220, 40,  92,  114, 257, 177, 60,  179, 4,   147,
         168, 64,  110, 171, 148, 23,  42,  52,  195, 84,  112, 246, 19,  252,
         196, 111, 105, 265, 209, 24,  100, 120, 26,  160, 39,  109, 157, 266,
         86,  74,  204, 227, 50,  187, 75,  216, 207, 67,  106, 198, 101, 51,
         141, 251, 94,  85,  172, 88,  53,  254, 261, 192, 145, 152, 240, 262,
         249, 68,  90,  59,  155, 263, 56,  210, 87,  180, 12,  115, 142, 34,
         235, 236, 45,  244, 253, 58,  10,  130, 165, 89,  234, 144, 259, 43,
         81,  5,   79,  223, 162, 256, 126, 150, 118, 127, 255, 99,  63,  124,
         176}));

    REQUIRE(S->size() == 175'560);
    REQUIRE(S->contains(S->generator(0)));
    REQUIRE(S->contains(S->generator(1)));
    REQUIRE(S->contains(make<Perm>(
        {0,   174, 56,  262, 210, 214, 81,  21,  160, 230, 148, 170, 67,  91,
         126, 238, 25,  85,  260, 32,  16,  29,  181, 66,  248, 143, 167, 39,
         241, 115, 98,  139, 226, 20,  12,  73,  152, 10,  141, 165, 121, 117,
         169, 84,  11,  211, 112, 116, 234, 38,  131, 48,  219, 222, 68,  217,
         9,   265, 188, 176, 74,  173, 69,  255, 163, 40,  77,  45,  264, 147,
         225, 63,  33,  58,  212, 17,  51,  261, 3,   119, 59,  65,  6,   41,
         103, 80,  231, 43,  184, 201, 151, 189, 47,  49,  42,  245, 124, 92,
         206, 71,  123, 61,  259, 239, 105, 178, 113, 183, 96,  104, 111, 195,
         199, 194, 192, 216, 83,  97,  64,  180, 94,  130, 8,   156, 134, 144,
         246, 89,  110, 221, 82,  243, 190, 76,  154, 28,  120, 249, 114, 87,
         23,  102, 150, 218, 34,  202, 57,  235, 127, 26,  253, 257, 258, 108,
         205, 37,  132, 177, 208, 18,  191, 233, 54,  35,  182, 251, 55,  27,
         247, 254, 19,  266, 44,  99,  5,   122, 186, 88,  60,  136, 24,  30,
         242, 166, 50,  263, 75,  256, 118, 52,  187, 158, 53,  1,   237, 209,
         240, 14,  4,   204, 140, 155, 90,  145, 146, 153, 86,  198, 175, 252,
         236, 125, 109, 193, 223, 250, 227, 215, 72,  129, 93,  13,  15,  213,
         7,   168, 172, 224, 128, 203, 244, 22,  200, 133, 161, 142, 171, 135,
         138, 31,  2,   106, 197, 157, 196, 185, 164, 159, 137, 79,  107, 149,
         228, 62,  179, 101, 100, 229, 162, 220, 70,  232, 95,  78,  36,  46,
         207})));

    REQUIRE(S->contains(make<Perm>(
        {0,   56,  118, 229, 217, 125, 79,  100, 1,   259, 190, 122, 60,  260,
         52,  206, 189, 71,  120, 23,  44,  160, 186, 148, 265, 170, 243, 195,
         204, 151, 179, 202, 254, 161, 169, 201, 235, 96,  46,  200, 37,  90,
         121, 54,  3,   212, 130, 248, 156, 143, 141, 246, 106, 146, 43,  234,
         116, 171, 178, 105, 64,  20,  29,  164, 137, 113, 182, 209, 31,  163,
         159, 227, 197, 262, 107, 66,  22,  59,  58,  244, 73,  230, 115, 2,
         165, 152, 6,   240, 158, 167, 4,   264, 36,  255, 191, 101, 261, 192,
         239, 62,  117, 231, 30,  17,  241, 14,  75,  144, 40,  147, 119, 132,
         203, 174, 133, 136, 78,  16,  155, 18,  139, 95,  11,  68,  258, 228,
         13,  8,   5,   177, 193, 124, 238, 50,  112, 70,  157, 218, 61,  224,
         85,  55,  166, 104, 142, 97,  84,  232, 129, 74,  24,  266, 123, 215,
         108, 49,  173, 87,  127, 126, 45,  28,  245, 249, 47,  69,  226, 21,
         138, 196, 63,  57,  72,  91,  53,  111, 140, 48,  88,  149, 199, 187,
         216, 222, 32,  172, 33,  253, 131, 225, 81,  256, 221, 237, 109, 211,
         210, 250, 114, 183, 150, 15,  236, 154, 181, 39,  25,  214, 134, 207,
         175, 220, 251, 145, 180, 233, 77,  7,   94,  80,  162, 213, 247, 67,
         103, 41,  102, 110, 27,  184, 38,  185, 205, 198, 153, 99,  252, 242,
         26,  83,  219, 98,  10,  34,  263, 128, 12,  223, 35,  65,  42,  93,
         176, 76,  168, 257, 51,  89,  135, 92,  188, 208, 82,  86,  19,  194,
         9})));

    REQUIRE(S->contains(make<Perm>(
        {0,   201, 243, 31,  229, 136, 122, 223, 164, 155, 185, 45,  3,   81,
         166, 48,  14,  105, 162, 169, 228, 94,  212, 43,  167, 190, 93,  202,
         224, 55,  56,  29,  118, 116, 194, 161, 184, 39,  134, 22,  181, 66,
         8,   216, 112, 110, 58,  183, 131, 156, 103, 244, 144, 203, 5,   59,
         9,   231, 108, 238, 52,  111, 255, 178, 226, 23,  92,  82,  173, 51,
         145, 26,  146, 138, 196, 143, 163, 37,  249, 246, 53,  191, 117, 245,
         200, 4,   30,  106, 247, 32,  192, 263, 49,  142, 171, 34,  188, 151,
         65,  260, 36,  6,   68,  259, 265, 123, 20,  73,  193, 63,  258, 100,
         2,   41,  16,  7,   124, 132, 230, 86,  135, 113, 83,  17,  149, 33,
         256, 84,  42,  128, 180, 236, 47,  187, 11,  74,  253, 61,  67,  209,
         211, 70,  237, 198, 25,  266, 130, 182, 125, 213, 250, 257, 141, 199,
         147, 13,  175, 102, 91,  27,  205, 75,  222, 242, 153, 206, 76,  88,
         1,   140, 96,  18,  69,  85,  189, 10,  50,  208, 227, 21,  139, 165,
         40,  87,  35,  204, 179, 99,  158, 176, 79,  251, 38,  109, 57,  89,
         221, 90,  24,  219, 252, 214, 159, 157, 239, 262, 129, 121, 160, 232,
         80,  97,  77,  60,  54,  44,  177, 195, 19,  154, 28,  225, 114, 62,
         240, 248, 150, 120, 126, 133, 168, 119, 218, 64,  172, 217, 197, 98,
         170, 78,  174, 15,  186, 101, 220, 115, 254, 137, 46,  207, 261, 95,
         152, 235, 148, 215, 107, 72,  264, 234, 210, 127, 71,  104, 241, 12,
         233})));

    REQUIRE(!S->contains(make<Perm>(
        {0,   250, 199, 41,  146, 23,  54,  53,  254, 33,  40,  80,  184, 78,
         34,  92,  102, 263, 45,  195, 153, 106, 58,  192, 127, 93,  209, 70,
         126, 143, 152, 166, 85,  87,  112, 49,  222, 159, 42,  210, 238, 150,
         191, 239, 52,  179, 174, 55,  75,  51,  142, 165, 1,   81,  56,  9,
         198, 225, 64,  197, 234, 38,  178, 132, 183, 226, 265, 202, 241, 129,
         26,  36,  116, 103, 28,  90,  252, 57,  119, 67,  171, 211, 161, 60,
         94,  18,  180, 134, 236, 136, 131, 176, 201, 66,  167, 182, 124, 137,
         220, 181, 262, 50,  27,  256, 135, 74,  113, 245, 118, 91,  160, 109,
         186, 97,  215, 105, 20,  175, 24,  149, 71,  17,  144, 125, 207, 43,
         59,  224, 242, 235, 82,  189, 84,  13,  227, 156, 120, 7,   208, 139,
         117, 257, 2,   30,  35,  244, 6,   114, 217, 72,  44,  63,  89,  145,
         31,  266, 76,  121, 204, 240, 115, 163, 98,  264, 233, 260, 162, 65,
         219, 253, 194, 151, 111, 123, 5,   39,  46,  212, 99,  108, 16,  22,
         138, 83,  104, 148, 69,  203, 187, 157, 206, 3,   246, 29,  243, 232,
         101, 168, 229, 221, 140, 213, 231, 47,  228, 188, 100, 248, 185, 128,
         200, 88,  155, 122, 14,  107, 95,  73,  77,  110, 169, 79,  193, 32,
         223, 249, 11,  205, 173, 190, 251, 196, 10,  218, 62,  37,  21,  86,
         147, 258, 214, 130, 237, 164, 133, 19,  172, 247, 177, 255, 12,  158,
         48,  261, 259, 170, 216, 154, 230, 96,  15,  141, 4,   61,  25,  68,
         8})));

    REQUIRE(!S->contains(make<Perm>(
        {0,   89,  18,  6,   159, 73,  100, 91,  110, 216, 66,  56,  144, 22,
         62,  15,  23,  146, 188, 11,  245, 148, 86,  96,  156, 114, 221, 103,
         217, 7,   129, 26,  111, 51,  232, 12,  141, 243, 212, 167, 225, 219,
         177, 34,  104, 64,  54,  255, 165, 250, 136, 202, 227, 20,  246, 124,
         134, 65,  84,  61,  77,  97,  162, 259, 247, 263, 215, 139, 150, 48,
         196, 181, 106, 187, 95,  44,  2,   33,  5,   260, 19,  127, 195, 69,
         58,  75,  142, 32,  203, 224, 45,  251, 4,   99,  88,  233, 184, 189,
         173, 85,  163, 249, 16,  43,  93,  27,  264, 140, 208, 166, 101, 39,
         211, 14,  206, 192, 230, 107, 87,  63,  102, 204, 205, 70,  115, 145,
         8,   3,   198, 182, 24,  153, 168, 174, 200, 154, 55,  40,  53,  59,
         125, 30,  112, 38,  265, 236, 223, 242, 262, 76,  190, 133, 28,  194,
         183, 213, 130, 60,  257, 123, 171, 253, 234, 149, 94,  109, 83,  118,
         256, 126, 176, 164, 31,  228, 151, 42,  261, 201, 226, 248, 244, 78,
         258, 220, 120, 178, 239, 71,  90,  67,  119, 197, 50,  199, 143, 175,
         92,  52,  29,  9,   80,  82,  157, 160, 147, 240, 49,  122, 191, 170,
         252, 13,  131, 209, 161, 254, 266, 74,  113, 41,  179, 116, 21,  185,
         1,   137, 17,  108, 214, 193, 172, 238, 105, 152, 218, 47,  117, 25,
         222, 35,  79,  180, 210, 138, 46,  37,  241, 81,  135, 169, 237, 128,
         121, 98,  158, 132, 235, 10,  36,  68,  207, 155, 231, 72,  57,  186,
         229})));

    REQUIRE(!S->contains(make<Perm>(
        {0,   56,  118, 229, 217, 125, 79,  100, 1,   259, 190, 122, 60,  260,
         52,  206, 189, 71,  120, 23,  44,  160, 186, 148, 265, 170, 243, 195,
         204, 151, 179, 202, 254, 161, 169, 201, 235, 96,  46,  200, 37,  90,
         121, 54,  4,   212, 130, 248, 156, 143, 141, 246, 106, 146, 43,  234,
         116, 171, 178, 105, 64,  20,  29,  164, 137, 113, 182, 209, 31,  163,
         159, 227, 197, 262, 107, 66,  22,  59,  58,  244, 73,  230, 115, 3,
         165, 152, 2,   240, 158, 167, 5,   264, 36,  255, 191, 101, 261, 192,
         239, 62,  117, 231, 30,  17,  241, 14,  75,  144, 40,  147, 119, 132,
         203, 174, 133, 136, 78,  16,  155, 18,  139, 95,  11,  68,  258, 228,
         13,  8,   6,   177, 193, 124, 238, 50,  112, 70,  157, 218, 61,  224,
         85,  55,  166, 104, 142, 97,  84,  232, 129, 74,  24,  266, 123, 215,
         108, 49,  173, 87,  127, 126, 45,  28,  245, 249, 47,  69,  226, 21,
         138, 196, 63,  57,  72,  91,  53,  111, 140, 48,  88,  149, 199, 187,
         216, 222, 32,  172, 33,  253, 131, 225, 81,  256, 221, 237, 109, 211,
         210, 250, 114, 183, 150, 15,  236, 154, 181, 39,  25,  214, 134, 207,
         175, 220, 251, 145, 180, 233, 77,  7,   94,  80,  162, 213, 247, 67,
         103, 41,  102, 110, 27,  184, 38,  185, 205, 198, 153, 99,  252, 242,
         26,  83,  219, 98,  10,  34,  263, 128, 12,  223, 35,  65,  42,  93,
         176, 76,  168, 257, 51,  89,  135, 92,  188, 208, 82,  86,  19,  194,
         9})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "020",
                          "perm. Hall-Janko group (order 604'800)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 101;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,  84, 20, 48, 56, 82, 67, 55, 41, 35, 40, 78, 100, 49, 37, 94, 76,
         19, 44, 17, 2,  34, 85, 92, 57, 75, 28, 64, 26, 90,  97, 38, 68, 69,
         21, 9,  53, 14, 31, 61, 10, 8,  73, 91, 18, 86, 81,  89, 3,  13, 93,
         96, 72, 36, 74, 7,  4,  24, 99, 95, 63, 39, 83, 60,  27, 70, 88, 6,
         32, 33, 65, 87, 52, 42, 54, 25, 16, 98, 11, 80, 79,  46, 5,  62, 1,
         22, 45, 71, 66, 47, 29, 43, 23, 50, 15, 59, 51, 30,  77, 58, 12}));
    S->add_generator(make<Perm>(
        {0,  80, 9,  53, 23, 51, 37, 7,  27, 11, 62, 2,   65, 64, 61, 98, 73,
         39, 5,  13, 97, 96, 1,  78, 6,  15, 93, 60, 57,  71, 69, 12, 16, 17,
         86, 28, 36, 24, 59, 33, 43, 41, 68, 91, 42, 30,  85, 10, 76, 92, 66,
         18, 14, 87, 95, 29, 54, 35, 20, 94, 8,  52, 47,  74, 19, 31, 88, 21,
         44, 45, 81, 55, 63, 32, 72, 70, 90, 49, 4,  100, 22, 75, 34, 79, 84,
         89, 82, 3,  50, 46, 48, 40, 77, 99, 38, 56, 67,  58, 25, 26, 83}));

    REQUIRE(S->size() == 604'800);
    REQUIRE(S->contains(S->generator(0)));
    REQUIRE(S->contains(S->generator(1)));
    REQUIRE(S->contains(make<Perm>(
        {0,  22, 11, 87, 78, 18, 24, 7,  60, 2,  47, 9,  31, 19, 52, 25,  32,
         33, 51, 64, 58, 67, 80, 4,  37, 98, 99, 8,  35, 55, 45, 65, 73,  39,
         82, 57, 36, 6,  94, 17, 91, 41, 44, 40, 68, 69, 89, 62, 90, 77,  88,
         5,  61, 3,  56, 71, 95, 28, 97, 38, 27, 14, 10, 72, 13, 12, 50,  96,
         42, 30, 75, 29, 74, 16, 63, 81, 48, 92, 23, 83, 1,  70, 86, 100, 84,
         46, 34, 53, 66, 85, 76, 43, 49, 26, 59, 54, 21, 20, 15, 93, 79})));
    REQUIRE(S->contains(make<Perm>(
        {0,  59, 20,  76, 38, 35, 13, 73, 45, 94, 84, 82, 10, 14, 58, 39, 68,
         95, 37, 54,  52, 16, 89, 26, 51, 81, 50, 23, 18, 71, 60, 43, 7,  25,
         9,  88, 48,  97, 2,  96, 46, 92, 53, 62, 32, 66, 80, 40, 5,  28, 57,
         90, 93, 29,  70, 55, 33, 77, 6,  78, 74, 19, 31, 12, 24, 72, 99, 17,
         30, 42, 100, 3,  27, 67, 21, 98, 36, 41, 8,  15, 79, 56, 34, 75, 47,
         91, 86, 44,  69, 49, 87, 65, 85, 4,  22, 64, 63, 11, 83, 61, 1})));
    REQUIRE(S->contains(make<Perm>(
        {0,  74, 78, 68,  70, 20, 98, 18, 21, 75, 85, 44, 43, 67, 50, 37, 42,
         31, 72, 60, 54,  3,  83, 9,  34, 47, 28, 13, 32, 36, 52, 91, 87, 46,
         24, 93, 71, 86,  99, 82, 65, 56, 53, 12, 73, 88, 62, 89, 45, 58, 14,
         7,  66, 16, 100, 55, 97, 94, 51, 35, 4,  69, 15, 80, 26, 39, 30, 27,
         2,  76, 38, 29,  49, 57, 5,  23, 48, 63, 8,  95, 84, 11, 6,  77, 22,
         10, 33, 96, 61,  17, 90, 25, 79, 59, 81, 41, 64, 92, 40, 19, 1})));
    REQUIRE(!S->contains(make<Perm>(
        {0,  2,  1,  4,  3,  5,  6,  8,  9,  10, 7,  11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
         51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
         68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
         85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100})));
    REQUIRE(!S->contains(make<Perm>(
        {0,  3,  100, 4,  8,  5,  6,  7,  1,  33, 10, 11, 12, 13, 14, 15, 16,
         17, 18, 19,  20, 21, 22, 23, 24, 25, 89, 27, 28, 29, 30, 31, 32, 2,
         34, 35, 36,  37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
         51, 52, 53,  54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
         68, 69, 70,  71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 93, 83, 84,
         85, 86, 87,  88, 82, 90, 91, 92, 26, 94, 95, 96, 97, 98, 99, 9})));
    REQUIRE(!S->contains(make<Perm>(
        {0,  5,  3,  1,  4,  100, 6,  7,  23, 9,  19, 11, 12, 13, 14, 15, 16,
         17, 18, 8,  20, 21, 22,  45, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39,  40, 41, 42, 43, 44, 10, 46, 47, 48, 49, 50,
         51, 52, 53, 54, 55, 56,  57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
         68, 69, 70, 71, 72, 73,  74, 75, 76, 77, 78, 79, 80, 81, 82, 2,  84,
         85, 86, 87, 88, 89, 90,  91, 92, 93, 94, 95, 96, 97, 98, 99, 83})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "021",
                          "perm. Hall-Janko group (alt.) (order 604'800)",
                          "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 841;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,   2,   1,   5,   7,   3,   10,  4,   12,  13,  6,   16,  8,   9,
         20,  21,  11,  24,  25,  27,  14,  15,  31,  32,  17,  18,  36,  19,
         38,  39,  41,  22,  23,  45,  46,  48,  26,  51,  28,  29,  55,  30,
         57,  58,  59,  33,  34,  63,  35,  65,  66,  37,  67,  68,  70,  40,
         73,  42,  43,  44,  77,  78,  80,  47,  83,  49,  50,  52,  53,  89,
         54,  91,  72,  56,  92,  93,  95,  60,  61,  99,  62,  101, 102, 64,
         103, 104, 106, 108, 110, 69,  113, 71,  74,  75,  118, 76,  120, 121,
         123, 79,  126, 81,  82,  84,  85,  132, 86,  134, 87,  136, 88,  138,
         139, 90,  140, 141, 142, 144, 94,  147, 96,  97,  151, 98,  153, 154,
         100, 155, 156, 158, 160, 162, 105, 165, 107, 168, 109, 171, 111, 112,
         114, 115, 116, 177, 117, 179, 180, 119, 181, 182, 184, 122, 187, 124,
         125, 127, 128, 193, 129, 195, 130, 197, 131, 199, 200, 133, 201, 202,
         135, 205, 206, 137, 207, 208, 210, 212, 214, 143, 217, 145, 146, 148,
         149, 223, 150, 224, 186, 152, 225, 226, 228, 230, 192, 157, 234, 159,
         237, 161, 240, 163, 164, 166, 167, 204, 203, 169, 170, 172, 173, 251,
         174, 253, 175, 255, 176, 257, 258, 178, 259, 260, 262, 264, 266, 183,
         185, 188, 189, 272, 190, 274, 191, 276, 277, 278, 194, 279, 280, 196,
         283, 284, 198, 285, 286, 288, 290, 292, 294, 295, 297, 250, 249, 209,
         302, 211, 305, 213, 308, 215, 216, 218, 219, 314, 220, 316, 221, 318,
         222, 320, 321, 322, 323, 325, 227, 328, 229, 331, 231, 232, 233, 235,
         236, 282, 281, 238, 239, 241, 242, 342, 243, 344, 244, 345, 245, 347,
         246, 247, 350, 248, 298, 352, 353, 354, 252, 355, 356, 254, 359, 360,
         256, 361, 362, 364, 366, 368, 261, 371, 263, 374, 265, 377, 267, 268,
         269, 270, 382, 271, 384, 385, 273, 386, 387, 275, 390, 391, 392, 335,
         395, 397, 398, 400, 402, 379, 287, 406, 289, 291, 409, 293, 412, 414,
         296, 407, 299, 300, 301, 303, 304, 358, 357, 306, 307, 309, 310, 422,
         311, 423, 312, 425, 313, 427, 428, 315, 429, 430, 317, 433, 434, 319,
         435, 341, 437, 439, 324, 442, 326, 327, 329, 330, 389, 388, 332, 333,
         334, 450, 451, 336, 417, 337, 338, 454, 339, 456, 340, 458, 459, 460,
         343, 351, 461, 346, 410, 462, 348, 465, 349, 467, 469, 396, 471, 473,
         420, 476, 363, 365, 480, 367, 483, 369, 370, 372, 373, 432, 431, 375,
         376, 378, 493, 380, 495, 381, 497, 498, 383, 444, 443, 501, 503, 504,
         506, 507, 393, 394, 511, 470, 399, 487, 401, 516, 403, 404, 405, 408,
         411, 523, 524, 413, 527, 415, 530, 416, 453, 418, 533, 419, 535, 536,
         421, 538, 539, 540, 424, 543, 544, 426, 545, 546, 548, 455, 488, 513,
         550, 532, 509, 436, 518, 438, 556, 440, 441, 560, 500, 445, 561, 446,
         447, 564, 448, 449, 566, 492, 567, 452, 552, 489, 568, 569, 457, 572,
         494, 573, 575, 577, 578, 463, 464, 581, 582, 466, 583, 576, 468, 586,
         491, 472, 590, 474, 475, 594, 477, 478, 479, 542, 541, 481, 482, 484,
         485, 604, 486, 606, 490, 608, 512, 609, 610, 611, 496, 612, 613, 615,
         499, 502, 617, 619, 505, 622, 508, 510, 514, 515, 571, 570, 517, 519,
         629, 520, 529, 521, 522, 632, 633, 525, 526, 528, 637, 638, 531, 607,
         639, 640, 534, 641, 642, 643, 537, 644, 645, 647, 649, 651, 602, 653,
         600, 656, 547, 659, 549, 587, 551, 553, 554, 555, 557, 558, 668, 559,
         669, 562, 672, 563, 636, 674, 565, 675, 676, 678, 661, 681, 682, 574,
         685, 687, 579, 580, 691, 693, 620, 584, 585, 588, 589, 591, 592, 593,
         595, 596, 699, 597, 701, 598, 684, 599, 704, 601, 706, 707, 603, 709,
         710, 605, 711, 626, 662, 703, 712, 714, 697, 717, 614, 616, 721, 722,
         618, 725, 621, 623, 624, 728, 625, 679, 729, 627, 628, 732, 650, 630,
         686, 631, 733, 724, 735, 634, 737, 635, 739, 740, 742, 666, 745, 646,
         720, 648, 749, 663, 652, 751, 654, 655, 727, 657, 658, 660, 664, 713,
         665, 756, 757, 667, 759, 760, 700, 670, 671, 762, 689, 673, 763, 708,
         677, 680, 765, 767, 683, 688, 770, 690, 772, 692, 774, 694, 695, 776,
         696, 778, 779, 698, 781, 782, 775, 702, 783, 705, 752, 784, 755, 754,
         715, 716, 786, 718, 719, 789, 723, 726, 773, 730, 793, 731, 795, 796,
         734, 798, 736, 764, 738, 748, 741, 801, 743, 744, 803, 746, 747, 750,
         753, 806, 758, 808, 809, 761, 805, 812, 813, 766, 816, 768, 769, 819,
         771, 820, 821, 777, 807, 780, 814, 790, 785, 802, 787, 788, 822, 825,
         791, 792, 804, 827, 794, 828, 829, 797, 799, 800, 810, 830, 833, 811,
         834, 815, 817, 818, 823, 832, 831, 824, 826, 835, 839, 837, 840, 836,
         838}));
    S->add_generator(make<Perm>(
        {0,   3,   4,   6,   8,   9,   1,   11,  2,   14,  15,  17,  18,  19,
         5,   22,  23,  7,   26,  28,  29,  30,  10,  33,  34,  35,  12,  37,
         13,  40,  42,  43,  44,  16,  47,  49,  50,  52,  53,  54,  20,  56,
         21,  59,  60,  61,  62,  24,  64,  25,  51,  36,  27,  69,  71,  72,
         74,  75,  76,  31,  32,  79,  81,  82,  84,  85,  86,  87,  88,  38,
         90,  39,  73,  55,  41,  94,  96,  97,  98,  45,  100, 46,  83,  63,
         48,  105, 107, 109, 111, 112, 114, 115, 116, 117, 57,  119, 58,  122,
         124, 125, 127, 128, 129, 130, 131, 65,  133, 66,  135, 67,  137, 68,
         113, 89,  70,  142, 143, 145, 146, 148, 149, 150, 77,  152, 78,  126,
         99,  80,  157, 159, 161, 163, 164, 166, 167, 169, 170, 172, 173, 174,
         175, 176, 91,  92,  178, 93,  147, 118, 95,  183, 185, 186, 188, 189,
         190, 191, 192, 101, 194, 102, 196, 103, 198, 104, 165, 132, 106, 203,
         204, 108, 171, 136, 110, 209, 211, 213, 215, 216, 218, 219, 220, 221,
         222, 120, 223, 121, 187, 151, 123, 227, 229, 231, 232, 233, 235, 236,
         238, 239, 241, 242, 243, 244, 245, 134, 246, 247, 248, 249, 250, 138,
         252, 139, 254, 140, 256, 141, 217, 177, 144, 261, 263, 265, 267, 268,
         269, 270, 271, 153, 273, 154, 275, 155, 156, 234, 193, 158, 281, 282,
         160, 240, 197, 162, 287, 289, 291, 293, 168, 296, 298, 299, 300, 301,
         303, 304, 306, 307, 309, 310, 311, 312, 313, 179, 315, 180, 317, 181,
         319, 182, 184, 323, 324, 326, 327, 329, 330, 332, 333, 276, 334, 335,
         336, 195, 337, 338, 339, 340, 341, 199, 343, 200, 295, 201, 346, 202,
         348, 349, 205, 351, 206, 207, 208, 302, 251, 210, 357, 358, 212, 308,
         255, 214, 363, 365, 367, 369, 370, 372, 373, 375, 376, 378, 379, 380,
         381, 224, 225, 383, 226, 328, 272, 228, 388, 389, 230, 277, 393, 394,
         396, 237, 399, 401, 403, 404, 405, 354, 407, 408, 410, 411, 413, 290,
         344, 353, 352, 297, 288, 415, 416, 253, 417, 418, 419, 420, 421, 257,
         422, 258, 424, 259, 426, 260, 371, 314, 262, 431, 432, 264, 377, 318,
         266, 436, 438, 440, 441, 386, 443, 444, 325, 445, 274, 446, 447, 448,
         449, 278, 279, 452, 280, 453, 430, 283, 455, 284, 457, 285, 286, 406,
         342, 350, 409, 345, 292, 463, 464, 294, 466, 468, 470, 305, 472, 474,
         475, 477, 478, 479, 481, 482, 484, 485, 486, 487, 488, 316, 489, 490,
         491, 492, 320, 494, 321, 496, 322, 442, 382, 499, 500, 502, 331, 505,
         507, 508, 509, 510, 512, 513, 429, 514, 515, 517, 518, 519, 520, 521,
         522, 347, 525, 526, 528, 529, 355, 531, 356, 532, 359, 534, 360, 361,
         537, 362, 364, 541, 542, 366, 483, 425, 368, 547, 549, 454, 398, 374,
         551, 552, 553, 458, 554, 555, 557, 558, 559, 384, 385, 498, 387, 562,
         563, 390, 565, 391, 392, 567, 511, 451, 395, 397, 400, 570, 571, 402,
         493, 574, 576, 578, 579, 580, 523, 412, 527, 465, 414, 584, 585, 587,
         588, 589, 591, 592, 593, 595, 596, 597, 598, 423, 599, 600, 601, 602,
         603, 427, 605, 428, 607, 433, 434, 435, 437, 556, 495, 439, 614, 501,
         560, 616, 618, 620, 621, 623, 624, 450, 625, 626, 456, 627, 628, 609,
         459, 630, 460, 631, 461, 462, 524, 634, 635, 636, 467, 586, 530, 469,
         471, 590, 533, 473, 643, 594, 536, 476, 646, 648, 650, 480, 652, 654,
         655, 657, 658, 660, 661, 640, 662, 663, 664, 665, 666, 667, 497, 668,
         670, 671, 503, 673, 504, 622, 564, 506, 677, 679, 680, 516, 683, 684,
         686, 688, 689, 690, 692, 674, 687, 649, 694, 608, 550, 695, 696, 535,
         697, 698, 538, 700, 539, 702, 540, 703, 543, 705, 544, 545, 708, 546,
         659, 604, 548, 678, 639, 573, 713, 715, 716, 718, 719, 720, 561, 723,
         724, 726, 582, 710, 727, 566, 606, 568, 569, 730, 731, 572, 685, 629,
         575, 583, 577, 734, 736, 735, 581, 738, 740, 741, 743, 744, 746, 669,
         747, 748, 637, 728, 750, 752, 753, 676, 754, 709, 755, 693, 732, 610,
         712, 611, 612, 758, 613, 615, 699, 761, 721, 617, 725, 672, 619, 707,
         651, 764, 766, 768, 714, 769, 632, 771, 633, 773, 711, 775, 638, 641,
         777, 642, 644, 780, 645, 647, 749, 701, 751, 704, 653, 783, 656, 675,
         785, 778, 787, 742, 788, 722, 790, 733, 791, 792, 681, 794, 682, 763,
         797, 691, 772, 774, 737, 799, 800, 759, 786, 802, 804, 805, 760, 706,
         784, 807, 757, 717, 782, 810, 811, 729, 814, 815, 817, 818, 796, 798,
         770, 739, 801, 776, 803, 779, 745, 822, 823, 756, 808, 821, 824, 762,
         826, 812, 765, 816, 793, 767, 830, 825, 831, 832, 781, 827, 789, 834,
         813, 806, 835, 836, 795, 837, 809, 833, 819, 838, 840, 820, 828, 839,
         829}));

    REQUIRE(S->size() == 604'800);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "022",
                          "perm. Conway group Co3 (order 495'766'656'000)",
                          "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 277;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,   245, 42,  112, 15,  131, 7,   188, 75,  132, 10,  11,  187, 186,
         265, 22,  159, 256, 43,  101, 123, 134, 4,   32,  209, 238, 35,  45,
         235, 126, 5,   19,  60,  66,  80,  154, 251, 117, 206, 71,  118, 93,
         87,  167, 271, 221, 261, 182, 155, 47,  230, 172, 236, 109, 191, 76,
         156, 73,  116, 147, 23,  127, 231, 38,  53,  122, 210, 24,  68,  86,
         255, 196, 139, 149, 21,  111, 203, 252, 72,  262, 114, 214, 9,   181,
         174, 85,  95,  2,   250, 257, 243, 90,  158, 170, 148, 69,  105, 249,
         263, 16,  54,  31,  115, 51,  104, 125, 219, 92,  46,  64,  204, 8,
         266, 225, 34,  175, 145, 161, 180, 237, 241, 224, 169, 269, 12,  96,
         129, 189, 190, 29,  17,  30,  82,  143, 74,  168, 13,  227, 217, 78,
         258, 220, 178, 228, 146, 58,  254, 273, 215, 57,  106, 77,  110, 50,
         26,  248, 260, 274, 107, 99,  253, 37,  25,  272, 44,  52,  119, 18,
         201, 65,  41,  233, 103, 246, 200, 102, 160, 198, 207, 157, 40,  223,
         49,  267, 79,  1,   136, 124, 6,   61,  268, 100, 70,  98,  171, 121,
         39,  62,  211, 208, 84,  135, 97,  55,  152, 141, 63,  142, 259, 67,
         33,  177, 173, 14,  242, 94,  113, 240, 264, 150, 205, 27,  183, 83,
         195, 216, 163, 247, 133, 36,  153, 197, 140, 194, 120, 270, 165, 166,
         162, 218, 138, 234, 81,  91,  89,  185, 212, 137, 48,  202, 276, 229,
         151, 176, 144, 192, 130, 244, 232, 199, 56,  108, 184, 193, 239, 213,
         3,   222, 128, 20,  28,  164, 226, 59,  179, 275, 88}));
    S->add_generator(make<Perm>(
        {0,   204, 203, 33,  236, 5,   172, 77,  76,  47,  146, 133, 224, 229,
         53,  84,  16,  223, 228, 130, 131, 252, 190, 13,  263, 242, 10,  32,
         196, 199, 65,  246, 209, 40,  99,  241, 198, 269, 251, 75,  118, 176,
         271, 183, 116, 197, 238, 22,  29,  178, 26,  174, 129, 2,   153, 272,
         257, 41,  12,  59,  20,  27,  175, 106, 159, 218, 259, 137, 258, 261,
         164, 262, 189, 45,  177, 260, 85,  25,  15,  226, 96,  24,  1,   274,
         148, 264, 132, 48,  117, 36,  60,  171, 201, 101, 253, 95,  120, 142,
         213, 165, 51,  115, 44,  103, 167, 243, 66,  141, 108, 88,  97,  276,
         30,  139, 222, 166, 173, 231, 3,   73,  239, 56,  170, 82,  162, 163,
         207, 145, 128, 52,  104, 90,  216, 220, 155, 74,  237, 28,  4,   113,
         273, 230, 270, 248, 180, 206, 50,  250, 78,  127, 150, 54,  232, 217,
         121, 69,  156, 6,   125, 210, 86,  89,  46,  184, 211, 265, 93,  19,
         138, 23,  126, 43,  188, 102, 244, 219, 192, 256, 83,  58,  144, 181,
         187, 91,  158, 205, 235, 147, 157, 114, 9,   152, 57,  39,  64,  143,
         67,  119, 161, 87,  200, 111, 79,  14,  123, 21,  149, 122, 191, 61,
         194, 266, 225, 31,  81,  62,  160, 151, 112, 215, 254, 234, 72,  17,
         179, 105, 267, 227, 18,  169, 249, 109, 208, 275, 68,  233, 168, 55,
         124, 80,  240, 35,  7,   212, 100, 245, 98,  195, 247, 107, 182, 42,
         185, 94,  11,  255, 135, 154, 221, 63,  193, 134, 71,  214, 8,   34,
         70,  202, 268, 37,  110, 38,  136, 140, 49,  186, 92}));

    REQUIRE(S->size() == static_cast<uint64_t>(495'766'656'000));
    REQUIRE(S->contains(S->generator(0)));
    REQUIRE(S->contains(S->generator(1)));

    REQUIRE(S->contains(make<Perm>(
        {0,   123, 14,  40,  168, 5,   188, 25,  85,  22,  50,  220, 179, 169,
         2,   148, 16,  17,  18,  104, 90,  185, 9,   229, 214, 7,   146, 209,
         67,  87,  218, 98,  61,  118, 165, 35,  161, 37,  42,  260, 3,   192,
         38,  91,  173, 119, 124, 190, 199, 83,  10,  244, 52,  203, 217, 136,
         154, 176, 224, 59,  131, 32,  219, 66,  210, 112, 63,  28,  221, 134,
         211, 71,  114, 197, 256, 193, 264, 242, 84,  267, 120, 263, 204, 49,
         78,  8,   216, 29,  231, 198, 20,  43,  111, 115, 94,  95,  239, 270,
         31,  265, 174, 166, 116, 103, 19,  212, 259, 230, 108, 117, 142, 92,
         65,  113, 72,  93,  102, 109, 33,  45,  80,  257, 126, 1,   46,  184,
         122, 206, 128, 129, 167, 60,  160, 254, 69,  177, 55,  196, 236, 139,
         140, 249, 110, 247, 144, 149, 26,  182, 15,  145, 150, 153, 208, 151,
         56,  261, 156, 172, 163, 194, 132, 36,  238, 158, 266, 34,  101, 130,
         4,   13,  207, 183, 157, 44,  100, 215, 57,  135, 274, 12,  180, 181,
         147, 171, 125, 21,  233, 250, 6,   222, 47,  232, 41,  75,  159, 248,
         137, 73,  89,  48,  200, 276, 226, 53,  82,  252, 127, 170, 152, 27,
         64,  70,  105, 246, 24,  175, 86,  54,  30,  62,  11,  68,  189, 223,
         58,  243, 202, 227, 228, 23,  107, 88,  191, 186, 258, 275, 138, 272,
         162, 96,  240, 241, 77,  225, 51,  245, 213, 143, 195, 141, 187, 271,
         205, 253, 133, 255, 74,  121, 234, 106, 39,  155, 262, 81,  76,  99,
         164, 79,  268, 269, 97,  251, 237, 273, 178, 235, 201})));
    REQUIRE(S->contains(make<Perm>(
        {0,   205, 48,  70,  190, 65,  157, 172, 276, 1,   146, 133, 162, 237,
         31,  236, 165, 104, 19,  246, 37,  177, 84,  20,  137, 46,  121, 234,
         110, 52,  90,  115, 13,  194, 222, 10,  169, 89,  106, 67,  144, 126,
         203, 228, 211, 32,  108, 178, 247, 187, 217, 103, 265, 159, 51,  14,
         193, 127, 206, 140, 209, 114, 119, 149, 88,  23,  40,  61,  258, 95,
         57,  75,  15,  41,  155, 76,  272, 54,  113, 158, 99,  7,   216, 17,
         200, 264, 261, 271, 92,  100, 171, 212, 141, 176, 62,  132, 163, 79,
         39,  210, 152, 130, 219, 188, 167, 120, 150, 125, 134, 2,   232, 260,
         33,  160, 96,  44,  12,  269, 118, 93,  68,  143, 218, 131, 147, 243,
         199, 27,  268, 207, 135, 5,   47,  18,  252, 111, 235, 195, 240, 189,
         208, 21,  122, 220, 11,  173, 180, 59,  253, 45,  215, 185, 123, 249,
         241, 29,  257, 58,  201, 16,  192, 231, 124, 267, 38,  168, 55,  183,
         74,  170, 101, 64,  174, 225, 148, 166, 94,  266, 270, 49,  3,   274,
         22,  72,  71,  245, 229, 224, 77,  145, 128, 153, 255, 214, 275, 179,
         262, 109, 256, 63,  244, 138, 107, 85,  97,  254, 251, 83,  87,  263,
         259, 161, 98,  34,  24,  78,  105, 4,   80,  66,  230, 197, 202, 181,
         56,  139, 136, 28,  248, 42,  26,  175, 221, 43,  35,  196, 129, 73,
         242, 8,   151, 239, 81,  60,  154, 204, 102, 227, 69,  142, 117, 198,
         25,  86,  50,  164, 223, 36,  273, 191, 156, 238, 226, 213, 112, 53,
         30,  91,  9,   82,  233, 116, 184, 250, 6,   186, 182})));
    REQUIRE(S->contains(make<Perm>(
        {0,   205, 48,  70,  190, 65,  157, 172, 276, 1,   146, 133, 162, 237,
         31,  236, 165, 104, 19,  246, 37,  177, 84,  20,  137, 46,  121, 234,
         110, 52,  90,  115, 13,  194, 222, 10,  169, 89,  106, 67,  144, 126,
         203, 228, 211, 32,  108, 178, 247, 187, 217, 103, 265, 159, 51,  14,
         193, 127, 206, 140, 209, 114, 119, 149, 88,  23,  40,  61,  258, 95,
         57,  75,  15,  41,  155, 76,  272, 54,  113, 158, 99,  7,   216, 17,
         200, 264, 261, 271, 92,  100, 171, 212, 141, 176, 62,  132, 163, 79,
         39,  210, 152, 130, 219, 188, 167, 120, 150, 125, 134, 2,   232, 260,
         33,  160, 96,  44,  12,  269, 118, 93,  68,  143, 218, 131, 147, 243,
         199, 27,  268, 207, 135, 5,   47,  18,  252, 111, 235, 195, 240, 189,
         208, 21,  122, 220, 11,  173, 180, 59,  253, 45,  215, 185, 123, 249,
         241, 29,  257, 58,  201, 16,  192, 231, 124, 267, 38,  168, 55,  183,
         74,  170, 101, 64,  174, 225, 148, 166, 94,  266, 270, 49,  3,   274,
         22,  72,  71,  245, 229, 224, 77,  145, 128, 153, 255, 214, 275, 179,
         262, 109, 256, 63,  244, 138, 107, 85,  97,  254, 251, 83,  87,  263,
         259, 161, 98,  34,  24,  78,  105, 4,   80,  66,  230, 197, 202, 181,
         56,  139, 136, 28,  248, 42,  26,  175, 221, 43,  35,  196, 129, 73,
         242, 8,   151, 239, 81,  60,  154, 204, 102, 227, 69,  142, 117, 198,
         25,  86,  50,  164, 223, 36,  273, 191, 156, 238, 226, 213, 112, 53,
         30,  91,  9,   82,  233, 116, 184, 250, 6,   186, 182})));
    REQUIRE(!S->contains(make<Perm>(
        {0,   185, 87,  266, 22,  30,  188, 6,   111, 82,  10,  13,  124, 136,
         213, 100, 99,  130, 167, 31,  269, 74,  15,  60,  67,  162, 154, 221,
         270, 129, 131, 101, 23,  210, 114, 26,  229, 161, 63,  196, 180, 170,
         3,   18,  164, 27,  108, 49,  248, 182, 153, 103, 165, 64,  230, 203,
         260, 149, 145, 273, 32,  189, 197, 206, 109, 169, 33,  209, 68,  95,
         192, 39,  78,  57,  134, 8,   55,  151, 139, 184, 34,  242, 132, 223,
         200, 85,  69,  42,  276, 244, 91,  243, 107, 41,  215, 86,  125, 202,
         193, 159, 191, 19,  175, 172, 104, 96,  150, 158, 261, 53,  152, 75,
         5,   216, 80,  102, 58,  37,  40,  166, 234, 195, 65,  20,  187, 105,
         29,  61,  268, 126, 256, 1,   9,   228, 2,   201, 186, 247, 240, 72,
         232, 205, 207, 133, 254, 116, 144, 59,  94,  73,  219, 252, 204, 11,
         35,  48,  56,  179, 92,  16,  176, 117, 238, 226, 271, 236, 237, 43,
         135, 122, 93,  194, 51,  212, 84,  115, 253, 211, 142, 274, 118, 83,
         47,  222, 262, 245, 17,  12,  7,   127, 128, 54,  255, 263, 233, 224,
         71,  231, 177, 259, 174, 168, 249, 76,  110, 220, 38,  178, 199, 24,
         66,  198, 246, 265, 81,  148, 225, 138, 239, 106, 141, 45,  267, 181,
         121, 113, 272, 137, 143, 251, 50,  62,  258, 171, 241, 28,  52,  119,
         25,  264, 217, 120, 214, 90,  257, 21,  173, 227, 155, 97,  88,  4,
         77,  160, 146, 70,  36,  89,  140, 208, 156, 46,  79,  98,  218, 14,
         112, 183, 190, 123, 235, 44,  163, 147, 157, 275, 250})));
    REQUIRE(!S->contains(make<Perm>(
        {0,   123, 14,  40,  168, 2,   188, 25,  85,  22,  50,  220, 179, 169,
         1,   148, 16,  36,  18,  104, 90,  185, 9,   229, 214, 7,   146, 209,
         67,  87,  218, 98,  61,  118, 165, 35,  161, 37,  42,  260, 21,  192,
         38,  91,  173, 119, 124, 190, 199, 83,  10,  244, 52,  203, 217, 136,
         154, 176, 224, 59,  131, 32,  219, 66,  210, 112, 63,  28,  221, 134,
         211, 71,  114, 197, 256, 193, 264, 242, 84,  267, 120, 263, 204, 49,
         78,  8,   216, 29,  231, 198, 20,  43,  111, 115, 94,  95,  239, 270,
         31,  265, 174, 166, 116, 103, 19,  212, 259, 11,  108, 117, 142, 92,
         65,  113, 72,  93,  102, 109, 33,  45,  80,  257, 126, 3,   46,  184,
         122, 206, 128, 129, 167, 60,  160, 254, 69,  177, 55,  196, 236, 139,
         140, 249, 110, 247, 144, 149, 26,  182, 15,  145, 150, 153, 208, 151,
         56,  261, 156, 172, 163, 194, 132, 4,   238, 158, 266, 34,  101, 130,
         100, 17,  207, 183, 157, 44,  230, 215, 57,  135, 274, 12,  180, 181,
         147, 171, 125, 5,   233, 250, 6,   222, 47,  232, 41,  75,  159, 248,
         137, 73,  89,  48,  200, 276, 226, 53,  82,  252, 127, 170, 152, 27,
         64,  70,  105, 246, 24,  175, 86,  54,  30,  62,  13,  68,  189, 223,
         58,  243, 202, 227, 228, 23,  107, 88,  191, 186, 258, 275, 138, 272,
         162, 96,  240, 241, 77,  225, 51,  245, 213, 143, 195, 141, 187, 271,
         205, 253, 133, 255, 74,  121, 234, 106, 39,  155, 262, 81,  76,  99,
         164, 79,  268, 269, 97,  251, 237, 273, 178, 235, 201})));
    REQUIRE(!S->contains(make<Perm>(
        {0,   185, 87,  266, 22,  30,  188, 6,   111, 82,  10,  100, 124, 136,
         213, 17,  99,  130, 167, 31,  269, 74,  15,  60,  67,  162, 154, 221,
         270, 129, 131, 101, 23,  210, 114, 26,  229, 161, 63,  196, 180, 170,
         1,   18,  164, 27,  108, 49,  248, 182, 153, 103, 165, 64,  36,  203,
         260, 149, 145, 273, 32,  189, 197, 206, 109, 169, 33,  209, 68,  95,
         192, 39,  78,  57,  134, 8,   55,  151, 139, 184, 34,  242, 132, 223,
         200, 85,  69,  42,  276, 244, 91,  243, 107, 41,  215, 86,  125, 202,
         193, 159, 191, 19,  175, 172, 104, 96,  150, 158, 261, 53,  152, 75,
         21,  216, 80,  102, 58,  37,  40,  166, 234, 195, 65,  20,  187, 105,
         29,  61,  268, 126, 256, 2,   9,   228, 5,   201, 186, 247, 240, 72,
         232, 205, 207, 133, 254, 116, 144, 59,  94,  73,  219, 252, 204, 4,
         35,  48,  56,  179, 92,  16,  176, 117, 238, 226, 271, 236, 237, 43,
         135, 122, 93,  194, 51,  212, 84,  115, 253, 211, 142, 274, 118, 83,
         47,  222, 262, 245, 230, 12,  7,   127, 128, 54,  255, 263, 233, 224,
         71,  231, 177, 259, 174, 168, 249, 76,  110, 220, 38,  178, 199, 24,
         66,  198, 246, 265, 81,  148, 225, 138, 239, 106, 141, 45,  267, 181,
         121, 113, 272, 137, 143, 251, 50,  62,  258, 171, 241, 28,  52,  119,
         25,  264, 217, 120, 214, 90,  257, 3,   173, 227, 155, 97,  88,  13,
         77,  160, 146, 70,  11,  89,  140, 208, 156, 46,  79,  98,  218, 14,
         112, 183, 190, 123, 235, 44,  163, 147, 157, 275, 250})));
  }

  LIBSEMIGROUPS_TEST_CASE(
      "SchreierSims",
      "023",
      "perm. Conway group Co3 (alt.) (order 495'766'656'000) ",
      "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 553;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,   3,   4,   5,   6,   1,   2,   10,  12,  14,  16,  18,  20,  21,
         23,  25,  7,   27,  29,  31,  8,   34,  36,  9,   39,  41,  43,  45,
         47,  11,  50,  52,  54,  56,  13,  59,  61,  63,  64,  66,  67,  15,
         70,  71,  73,  17,  76,  78,  80,  81,  83,  84,  19,  87,  88,  90,
         92,  94,  95,  97,  99,  22,  102, 104, 105, 93,  24,  107, 109, 110,
         112, 26,  115, 117, 119, 120, 122, 124, 28,  127, 129, 130, 118, 30,
         132, 134, 135, 137, 32,  139, 141, 143, 33,  145, 147, 149, 150, 35,
         152, 154, 156, 155, 157, 159, 37,  38,  162, 40,  165, 167, 169, 121,
         42,  173, 174, 176, 178, 44,  180, 182, 184, 185, 46,  186, 151, 189,
         188, 190, 192, 48,  49,  195, 51,  198, 200, 202, 96,  53,  206, 208,
         209, 55,  212, 214, 215, 65,  218, 57,  219, 58,  136, 77,  223, 225,
         60,  227, 229, 62,  232, 234, 235, 236, 238, 240, 241, 243, 245, 68,
         220, 69,  249, 250, 252, 254, 256, 257, 72,  260, 262, 263, 82,  266,
         74,  267, 75,  111, 270, 272, 274, 276, 79,  279, 281, 282, 283, 285,
         287, 288, 290, 292, 85,  268, 86,  296, 297, 222, 300, 302, 89,  304,
         172, 306, 286, 308, 91,  231, 312, 314, 316, 317, 318, 320, 258, 98,
         322, 324, 325, 101, 328, 100, 330, 144, 331, 333, 103, 336, 338, 340,
         106, 177, 342, 344, 346, 108, 348, 350, 352, 353, 265, 310, 355, 207,
         210, 357, 113, 359, 114, 361, 205, 363, 239, 364, 116, 278, 368, 301,
         371, 372, 373, 375, 123, 376, 378, 379, 126, 382, 125, 384, 179, 385,
         387, 128, 390, 392, 394, 131, 142, 396, 307, 399, 133, 401, 403, 405,
         406, 217, 366, 408, 255, 409, 138, 248, 251, 412, 140, 414, 415, 197,
         418, 420, 170, 397, 423, 424, 295, 425, 146, 148, 168, 246, 391, 381,
         380, 429, 153, 430, 271, 432, 434, 435, 436, 158, 438, 400, 440, 442,
         160, 269, 161, 305, 444, 445, 163, 446, 164, 448, 450, 280, 451, 386,
         166, 388, 319, 455, 457, 171, 327, 458, 460, 298, 462, 175, 464, 465,
         466, 468, 203, 343, 471, 472, 473, 181, 183, 201, 293, 337, 326, 477,
         187, 478, 224, 480, 482, 483, 417, 191, 485, 347, 487, 489, 193, 221,
         194, 362, 491, 492, 196, 493, 494, 496, 233, 497, 332, 199, 334, 374,
         501, 503, 204, 504, 506, 411, 507, 329, 339, 211, 428, 277, 213, 511,
         512, 419, 513, 216, 515, 437, 516, 427, 517, 488, 226, 519, 356, 345,
         228, 413, 230, 315, 402, 490, 404, 377, 521, 522, 237, 453, 367, 476,
         433, 449, 242, 244, 525, 341, 474, 247, 422, 518, 253, 459, 528, 461,
         529, 383, 393, 259, 261, 531, 524, 467, 532, 264, 534, 484, 535, 475,
         508, 441, 273, 537, 321, 398, 275, 463, 370, 349, 443, 351, 323, 538,
         539, 284, 499, 311, 481, 495, 289, 291, 523, 395, 426, 294, 470, 536,
         299, 505, 542, 303, 447, 543, 510, 421, 309, 456, 452, 313, 500, 416,
         354, 545, 520, 335, 486, 533, 365, 514, 530, 547, 358, 360, 546, 469,
         502, 498, 369, 454, 407, 549, 389, 439, 509, 550, 410, 540, 544, 431,
         526, 551, 548, 479, 552, 527, 541}));

    S->add_generator(make<Perm>(
        {0,   2,   1,   4,   3,   7,   8,   9,   11,  13,  15,  17,  19,  5,
         22,  24,  26,  6,   28,  30,  32,  33,  35,  37,  38,  40,  42,  44,
         46,  48,  49,  51,  53,  55,  57,  58,  60,  62,  10,  65,  25,  68,
         69,  54,  72,  74,  75,  77,  79,  12,  82,  31,  85,  86,  43,  89,
         91,  93,  14,  96,  98,  100, 101, 103, 81,  39,  106, 34,  108, 16,
         111, 113, 114, 116, 118, 18,  121, 123, 125, 126, 128, 64,  50,  131,
         45,  133, 20,  136, 138, 21,  140, 142, 144, 67,  146, 148, 59,  122,
         151, 153, 155, 23,  127, 158, 160, 152, 161, 163, 164, 166, 168, 170,
         171, 172, 27,  175, 177, 179, 84,  181, 183, 76,  97,  154, 187, 188,
         29,  102, 191, 193, 186, 194, 196, 197, 199, 201, 203, 204, 205, 207,
         95,  210, 211, 213, 92,  216, 217, 182, 90,  220, 221, 36,  222, 224,
         47,  226, 228, 230, 231, 233, 112, 232, 237, 239, 41,  242, 244, 246,
         110, 247, 248, 104, 251, 253, 255, 120, 258, 259, 261, 117, 264, 265,
         147, 115, 268, 269, 252, 271, 273, 275, 277, 278, 280, 137, 279, 284,
         286, 52,  289, 291, 293, 135, 294, 295, 129, 298, 299, 301, 256, 303,
         305, 56,  200, 307, 309, 310, 311, 313, 315, 134, 312, 319, 321, 270,
         302, 323, 61,  326, 327, 329, 157, 63,  66,  332, 334, 335, 337, 339,
         285, 341, 320, 343, 345, 347, 267, 349, 351, 290, 70,  354, 317, 71,
         356, 297, 358, 314, 208, 360, 362, 73,  167, 344, 365, 366, 367, 369,
         370, 109, 368, 374, 223, 359, 377, 78,  380, 381, 383, 190, 80,  83,
         386, 388, 389, 391, 393, 238, 395, 375, 397, 398, 400, 219, 402, 404,
         243, 87,  407, 372, 88,  250, 410, 357, 99,  411, 413, 384, 363, 416,
         417, 419, 421, 422, 420, 94,  409, 414, 426, 206, 218, 427, 387, 105,
         227, 428, 225, 241, 424, 431, 433, 371, 176, 418, 437, 429, 439, 441,
         346, 423, 396, 443, 325, 107, 430, 340, 447, 445, 449, 202, 350, 452,
         453, 260, 454, 234, 456, 401, 130, 139, 459, 124, 461, 463, 330, 306,
         436, 467, 469, 470, 468, 119, 464, 474, 173, 266, 475, 333, 274, 476,
         272, 288, 472, 479, 481, 316, 141, 466, 484, 477, 486, 488, 399, 471,
         342, 490, 379, 132, 478, 394, 492, 495, 169, 403, 498, 499, 212, 500,
         281, 502, 348, 174, 505, 487, 262, 508, 318, 509, 143, 510, 493, 462,
         149, 480, 145, 514, 322, 511, 276, 150, 324, 287, 518, 156, 263, 328,
         300, 483, 520, 159, 390, 353, 257, 519, 304, 162, 457, 165, 523, 178,
         364, 485, 524, 521, 507, 408, 352, 501, 526, 473, 516, 482, 527, 440,
         214, 517, 373, 530, 446, 412, 184, 432, 180, 533, 376, 531, 229, 185,
         378, 240, 536, 189, 215, 382, 254, 435, 192, 336, 406, 209, 537, 361,
         195, 503, 198, 525, 308, 438, 512, 538, 529, 355, 405, 455, 540, 425,
         535, 434, 541, 245, 442, 496, 494, 491, 415, 532, 236, 534, 544, 489,
         392, 235, 448, 451, 546, 385, 465, 331, 249, 460, 458, 292, 450, 444,
         513, 283, 515, 548, 338, 282, 497, 543, 296, 506, 504, 550, 528, 549,
         547, 551, 542, 545, 552, 522, 539}));

    REQUIRE(S->size() == static_cast<uint64_t>(495'766'656'000));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "024",
                          "perm. Suzuki group (order 448'345'497'600)",
                          "[no-valgrind][quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 1'783;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,    926,  1366, 1336, 1696, 1688, 610,  1412, 1390, 374,  1774, 1572,
         1588, 1690, 1127, 616,  197,  240,  761,  935,  301,  1776, 1078, 23,
         588,  1388, 799,  69,   1318, 839,  438,  517,  1510, 33,   1241, 611,
         843,  791,  1526, 162,  1053, 474,  706,  1012, 1076, 739,  556,  937,
         296,  1755, 208,  151,  225,  883,  952,  1405, 149,  396,  1781, 949,
         181,  1008, 1529, 63,   743,  243,  500,  288,  1027, 27,   1579, 723,
         1622, 1172, 1710, 1179, 1328, 1377, 638,  217,  356,  986,  1183, 519,
         1203, 377,  241,  294,  1484, 1198, 717,  630,  311,  481,  1252, 95,
         213,  118,  280,  823,  247,  786,  726,  721,  1750, 1369, 367,  325,
         334,  238,  573,  427,  1525, 830,  1278, 600,  912,  1536, 97,   119,
         828,  1226, 1555, 1550, 410,  1761, 148,  522,  928,  1338, 888,  161,
         797,  1075, 1176, 1769, 840,  1272, 869,  1574, 462,  732,  865,  495,
         1586, 223,  1590, 375,  126,  56,   1745, 51,   546,  488,  154,  1108,
         1652, 443,  1051, 1122, 1732, 131,  39,   1212, 609,  192,  333,  1110,
         446,  340,  922,  613,  1514, 1573, 399,  1775, 1042, 363,  646,  1339,
         1770, 60,   1262, 467,  793,  496,  528,  430,  858,  661,  405,  838,
         165,  980,  552,  606,  947,  16,   568,  401,  1468, 1143, 1589, 1503,
         1044, 720,  1599, 1235, 50,   985,  1493, 1378, 957,  96,   1628, 1089,
         1749, 79,   738,  835,  1634, 368,  1006, 145,  1606, 52,   1054, 1242,
         964,  774,  1251, 1661, 579,  1354, 341,  1100, 837,  1610, 109,  862,
         17,   86,   392,  65,   788,  746,  1545, 100,  1679, 735,  1167, 1723,
         523,  1639, 1694, 1605, 718,  433,  1553, 815,  1568, 939,  1289, 1223,
         1061, 946,  1672, 1535, 1299, 729,  740,  1752, 1020, 1418, 1132, 1773,
         887,  329,  429,  702,  98,   1511, 1446, 619,  898,  518,  801,  369,
         67,   992,  466,  1146, 960,  318,  87,   1569, 48,   472,  304,  493,
         780,  20,   1116, 593,  298,  1625, 944,  307,  545,  1505, 457,  92,
         1022, 1546, 768,  1415, 1693, 543,  293,  625,  492,  1428, 1296, 1292,
         680,  107,  1576, 1542, 436,  277,  1455, 744,  1285, 166,  108,  800,
         1213, 458,  1441, 1169, 169,  234,  1356, 449,  1206, 1135, 1374, 1248,
         417,  695,  594,  1219, 395,  1683, 1626, 1598, 80,   491,  851,  787,
         431,  582,  1671, 177,  1221, 758,  917,  106,  221,  287,  1105, 1236,
         1325, 539,  9,    147,  1200, 85,   1417, 875,  426,  1346, 587,  1192,
         813,  802,  1410, 1365, 550,  1364, 475,  423,  242,  1692, 479,  352,
         57,   525,  1580, 174,  1281, 199,  1102, 1471, 1189, 190,  1751, 1190,
         658,  1259, 124,  411,  1298, 1389, 1582, 896,  1700, 348,  1277, 520,
         731,  982,  1623, 391,  1578, 1072, 380,  111,  1085, 278,  187,  360,
         1077, 257,  1400, 967,  328,  1015, 30,   1125, 979,  1350, 455,  157,
         1187, 1184, 168,  694,  654,  343,  1335, 831,  1643, 1063, 1368, 442,
         688,  310,  337,  1007, 1396, 586,  140,  1243, 1091, 1495, 290,  183,
         1230, 469,  750,  784,  297,  808,  41,   390,  592,  1344, 524,  394,
         480,  93,   1757, 1343, 1725, 961,  1633, 508,  153,  1040, 864,  357,
         320,  299,  1011, 143,  185,  549,  1759, 1552, 66,   557,  1577, 1268,
         1004, 642,  1109, 1095, 487,  620,  510,  1161, 736,  715,  704,  834,
         971,  31,   285,  83,   419,  1596, 127,  252,  478,  397,  1250, 853,
         186,  1113, 551,  760,  1332, 1779, 910,  535,  1523, 1469, 712,  373,
         585,  945,  1607, 317,  709,  308,  152,  663,  1275, 497,  388,  530,
         194,  558,  1066, 969,  46,   501,  553,  1119, 993,  1740, 1150, 1258,
         873,  1686, 759,  657,  198,  956,  936,  1720, 766,  110,  1482, 1649,
         1019, 859,  1363, 232,  1018, 621,  361,  659,  1559, 540,  461,  382,
         24,   902,  1571, 1462, 476,  303,  350,  1651, 675,  1403, 725,  1048,
         115,  1129, 927,  1068, 1302, 1156, 195,  1512, 1608, 164,  6,    35,
         1098, 171,  1300, 1079, 15,   966,  1486, 283,  509,  581,  754,  1090,
         1566, 319,  1130, 1456, 1039, 1762, 91,   1084, 1411, 1009, 730,  1327,
         1501, 814,  78,   1765, 640,  1508, 505,  643,  924,  1287, 178,  1297,
         1440, 1164, 1506, 1246, 1001, 1768, 448,  655,  842,  567,  408,  583,
         1204, 189,  1031, 547,  1430, 1467, 1715, 890,  1731, 1474, 1756, 1271,
         1630, 1408, 881,  596,  1530, 1322, 1584, 930,  324,  681,  1101, 1466,
         792,  1291, 1205, 1117, 456,  1123, 915,  948,  1443, 1669, 447,  349,
         974,  1283, 900,  1734, 1112, 1705, 279,  876,  514,  1237, 42,   1653,
         1670, 544,  1153, 1025, 538,  1301, 1321, 513,  1663, 90,   256,  763,
         205,  103,  1724, 71,   891,  598,  102,  905,  773,  269,  634,  420,
         141,  1228, 1666, 249,  512,  999,  218,  45,   270,  1464, 1667, 64,
         331,  809,  245,  1128, 1660, 1706, 470,  1431, 1217, 1279, 622,  1662,
         1477, 1516, 365,  566,  531,  18,   1207, 719,  1028, 806,  572,  1406,
         314,  1637, 1222, 1245, 1094, 728,  229,  775,  866,  1029, 1185, 1249,
         300,  1502, 1280, 1708, 471,  962,  101,  359,  244,  973,  1311, 37,
         684,  184,  794,  846,  1052, 132,  820,  26,   335,  286,  385,  824,
         989,  1445, 765,  1640, 473,  745,  1358, 1707, 1587, 384,  637,  259,
         1449, 1736, 1157, 1675, 798,  1379, 844,  99,   803,  1433, 1611, 1380,
         120,  832,  113,  451,  829,  1037, 515,  219,  1170, 236,  191,  29,
         136,  919,  656,  36,   822,  940,  795,  1106, 1058, 913,  863,  358,
         1617, 527,  1778, 1498, 1677, 1729, 188,  577,  1737, 959,  239,  850,
         490,  142,  776,  1017, 904,  138,  925,  1453, 1155, 564,  972,  379,
         703,  995,  970,  1470, 1404, 674,  1032, 53,   1713, 1728, 1307, 276,
         130,  1427, 667,  724,  1261, 893,  1333, 1544, 415,  1414, 284,  1452,
         698,  997,  589,  1370, 868,  727,  1632, 1465, 1444, 1081, 534,  1227,
         116,  849,  1121, 690,  1551, 366,  1103, 841,  1320, 1730, 170,  1210,
         644,  870,  1,    602,  128,  1485, 679,  1648, 1162, 1655, 934,  19,
         570,  47,   1305, 261,  845,  1093, 1744, 1293, 306,  541,  265,  196,
         691,  59,   1303, 1392, 54,   1522, 1138, 1416, 569,  212,  1402, 861,
         292,  485,  785,  1188, 228,  1458, 617,  435,  1527, 555,  878,  516,
         874,  789,  696,  1255, 1564, 1483, 1533, 440,  193,  1581, 421,  1460,
         1082, 209,  81,   1664, 1211, 804,  1432, 1005, 289,  560,  1673, 877,
         1615, 901,  1254, 737,  1375, 652,  1064, 1196, 504,  991,  222,  459,
         61,   633,  1438, 494,  43,   1434, 1014, 437,  1602, 867,  580,  576,
         272,  1743, 312,  1257, 1086, 711,  1202, 68,   764,  777,  1083, 662,
         882,  1238, 1034, 1681, 1036, 833,  1647, 628,  489,  1140, 176,  1420,
         204,  1201, 1609, 1385, 599,  1763, 1348, 158,  796,  40,   226,  1540,
         1515, 1627, 848,  1229, 1631, 264,  1087, 453,  1002, 1317, 554,  1638,
         603,  1371, 1273, 1656, 425,  1703, 1247, 133,  44,   432,  22,   615,
         1563, 909,  984,  1030, 631,  428,  1024, 1062, 1126, 215,  623,  464,
         1507, 941,  772,  507,  1439, 1480, 612,  1738, 235,  682,  402,  918,
         1104, 370,  847,  1680, 155,  506,  167,  1111, 700,  529,  1313, 1240,
         302,  687,  1233, 559,  1163, 914,  159,  689,  1148, 439,  1088, 14,
         747,  601,  626,  1195, 274,  1342, 1435, 345,  1547, 1312, 954,  1721,
         1041, 1697, 1193, 201,  1620, 1319, 291,  1554, 1124, 1423, 562,  1450,
         1152, 710,  1355, 872,  605,  818,  1209, 1614, 1476, 511,  932,  1120,
         649,  1314, 1597, 250,  1391, 339,  836,  1687, 73,   1334, 1174, 1359,
         134,  1711, 1436, 75,   1181, 1180, 1182, 82,   445,  778,  1186, 444,
         963,  404,  407,  1357, 383,  1142, 1315, 1131, 1003, 1384, 89,   1777,
         376,  1045, 1026, 84,   660,  686,  344,  762,  1329, 1158, 923,  988,
         163,  336,  1214, 1603, 1270, 752,  1674, 351,  1726, 364,  770,  263,
         1459, 1437, 121,  911,  733,  1059, 468,  1764, 1678, 1118, 1539, 207,
         371,  705,  1033, 1447, 1115, 34,   227,  463,  1442, 771,  651,  1074,
         347,  779,  526,  230,  94,   1717, 998,  975,  1518, 1023, 563,  409,
         1282, 892,  182,  1422, 1294, 1487, 1601, 1665, 503,  1585, 1216, 671,
         137,  1070, 1636, 548,  1767, 418,  114,  753,  782,  400,  1260, 697,
         1489, 332,  1409, 645,  1504, 262,  1290, 685,  323,  943,  1264, 1488,
         322,  647,  412,  268,  614,  713,  604,  950,  1337, 938,  1772, 886,
         1593, 1641, 1654, 790,  1137, 1114, 1165, 1194, 1766, 1065, 28,   1145,
         920,  714,  677,  1753, 1616, 372,  1531, 635,  76,   1208, 1397, 1448,
         532,  894,  1173, 450,  3,    1304, 129,  179,  1519, 1642, 1133, 483,
         477,  1746, 381,  1701, 1050, 1407, 441,  1457, 1500, 1401, 233,  1154,
         342,  1191, 810,  1175, 1646, 1650, 1689, 578,  389,  387,  2,    1538,
         454,  105,  903,  1069, 1695, 1386, 346,  1000, 1398, 77,   211,  821,
         827,  1760, 1413, 1383, 1197, 1047, 1373, 1712, 25,   413,  8,    1168,
         951,  1478, 1583, 1635, 460,  1330, 1376, 1520, 434,  1353, 958,  597,
         880,  55,   767,  1349, 673,  1286, 386,  632,  7,    1382, 897,  315,
         955,  378,  273,  1419, 1043, 1421, 1263, 1149, 1698, 1619, 1685, 889,
         321,  1497, 664,  751,  990,  825,  1013, 1134, 1178, 1225, 1010, 1096,
         648,  338,  1244, 692,  908,  805,  282,  1239, 1331, 816,  1151, 1532,
         899,  871,  1558, 330,  627,  1351, 965,  1224, 983,  1490, 591,  1644,
         741,  907,  683,  665,  200,  537,  879,  403,  1754, 1716, 669,  1612,
         1160, 756,  1393, 1733, 1097, 1537, 574,  977,  88,   929,  618,  1265,
         1295, 1284, 1461, 1714, 1704, 210,  1494, 465,  1541, 1429, 855,  1565,
         1352, 636,  781,  203,  1288, 309,  650,  1092, 641,  1682, 32,   281,
         607,  1524, 172,  1056, 757,  1517, 1256, 1340, 1399, 1556, 953,  536,
         1513, 112,  38,   968,  1621, 62,   676,  1326, 1451, 978,  1534, 267,
         117,  1481, 1367, 1234, 1055, 1496, 327,  1629, 895,  246,  313,  1136,
         1684, 1549, 123,  916,  499,  258,  1147, 122,  1521, 1575, 1454, 584,
         1780, 1657, 1562, 1080, 976,  1499, 624,  1591, 260,  295,  1748, 590,
         11,   173,  139,  1557, 326,  502,  424,  70,   398,  981,  414,  1394,
         678,  1269, 144,  812,  12,   202,  146,  1567, 1604, 1308, 1659, 1618,
         521,  1166, 355,  206,  1719, 1266, 1016, 1215, 1592, 255,  224,  542,
         608,  1046, 237,  826,  1475, 1702, 1159, 996,  1324, 852,  1595, 1425,
         1144, 1528, 72,   422,  1624, 305,  354,  1057, 214,  1543, 672,  1060,
         906,  486,  220,  1395, 1274, 769,  1067, 253,  807,  1309, 1341, 452,
         1463, 1722, 1360, 1038, 931,  575,  1361, 595,  156,  707,  1310, 933,
         1071, 1561, 1735, 1594, 748,  231,  755,  716,  987,  1267, 734,  742,
         1668, 693,  708,  362,  266,  994,  1218, 819,  1782, 856,  1232, 248,
         1107, 1035, 1509, 353,  1548, 1426, 565,  1171, 5,    1362, 13,   1747,
         393,  316,  254,  1372, 4,    1141, 1424, 1742, 416,  1347, 1613, 1073,
         1492, 701,  749,  811,  783,  1758, 74,   1177, 1387, 884,  1491, 666,
         1473, 1253, 1739, 1600, 571,  1139, 1645, 251,  722,  484,  1220, 1771,
         885,  857,  921,  668,  160,  1479, 699,  1658, 817,  860,  1099, 1718,
         561,  1741, 1699, 1021, 942,  150,  1345, 1691, 1570, 216,  104,  406,
         271,  1323, 1472, 49,   670,  482,  1709, 498,  1381, 125,  629,  1049,
         1231, 639,  1316, 1276, 653,  135,  180,  1727, 1306, 275,  10,   175,
         21,   1199, 854,  533,  1560, 58,   1676}));
    S->add_generator(make<Perm>(
        {0,    28,   33,   42,   49,   51,   17,   59,   14,   70,   73,   81,
         84,   87,   22,   99,   101,  26,   15,   117,  120,  124,  8,    133,
         135,  140,  6,    145,  40,   156,  160,  72,   163,  47,   134,  179,
         3,    186,  188,  192,  1,    57,   36,   4,    210,  213,  137,  2,
         222,  43,   231,  68,   240,  247,  250,  89,   75,   255,  261,  79,
         37,   276,  279,  287,  290,  299,  302,  305,  5,    311,  94,   324,
         97,   98,   333,  331,  340,  342,  347,  7,    62,   108,  359,  361,
         112,  372,  375,  104,  23,   329,  391,  399,  402,  407,  9,    416,
         38,   31,   10,   18,   427,  131,  438,  445,  13,   448,  451,  453,
         11,   462,  465,  468,  12,   304,  481,  486,  377,  150,  498,  500,
         154,  511,  518,  521,  159,  529,  532,  535,  543,  545,  548,  16,
         556,  88,   220,  171,  288,  274,  573,  576,  177,  588,  591,  289,
         597,  184,  609,  616,  415,  623,  19,   632,  608,  638,  20,   647,
         199,  658,  491,  21,   204,  674,  676,  208,  136,  690,  400,  700,
         702,  707,  710,  24,   351,  118,  722,  725,  729,  25,   228,  229,
         430,  745,  748,  752,  27,   759,  60,   182,  96,   774,  596,  779,
         245,  787,  792,  795,  803,  806,  810,  29,   817,  621,  823,  828,
         30,   837,  840,  843,  32,   848,  268,  859,  862,  272,  872,  587,
         882,  141,  449,  612,  34,   785,  285,  896,  422,  708,  280,  904,
         885,  35,   627,  297,  921,  926,  503,  933,  936,  940,  943,  864,
         281,  82,   218,  905,  959,  39,   963,  318,  881,  975,  322,  986,
         506,  991,  238,  41,   1001, 559,  1010, 1012, 525,  338,  293,  1029,
         61,   1036, 1038, 193,  44,   622,  1052, 1054, 45,   1062, 46,   1065,
         264,  76,   169,  80,   1073, 52,   1042, 875,  1030, 48,   1083, 355,
         164,  738,  371,  1093, 1096, 1099, 1105, 1108, 1053, 50,   1117, 382,
         1125, 605,  386,  1135, 603,  389,  1143, 1146, 1147, 1090, 1153, 397,
         1157, 717,  571,  1164, 1166, 1171, 53,   1180, 561,  1022, 54,   1190,
         414,  1110, 426,  452,  306,  55,   1204, 56,   1210, 425,  1218, 198,
         1221, 1224, 58,   1228, 277,  1234, 436,  1245, 1247, 719,  1154, 443,
         421,  515,  851,  883,  1268, 1271, 797,  63,   505,  215,  1276, 241,
         1004, 460,  1281, 457,  1282, 670,  203,  352,  455,  138,  1290, 64,
         473,  1301, 309,  477,  814,  619,  1313, 485,  1025, 244,  65,   358,
         1329, 1333, 66,   798,  1338, 67,   441,  496,  1116, 671,  861,  107,
         469,  69,   1352, 437,  857,  1273, 509,  1047, 1371, 1372, 1373, 516,
         1382, 1387, 1144, 162,  246,  1393, 71,   528,  527,  1384, 1398, 1085,
         1324, 995,  1086, 628,  639,  74,   1396, 541,  723,  982,  912,  1109,
         1424, 1430, 1416, 1399, 77,   91,   554,  1440, 1168, 1443, 1286, 78,
         691,  563,  1452, 1215, 566,  242,  1310, 570,  1207, 395,  119,  1464,
         1104, 804,  553,  624,  83,   546,  583,  1468, 1296, 586,  1201, 1155,
         574,  1474, 1077, 1476, 1478, 85,   1482, 1233, 1327, 86,   558,  1492,
         1148, 607,  1497, 917,  1500, 1202, 614,  1507, 530,  1512, 1514, 816,
         1515, 1187, 625,  821,  90,   1375, 173,  1519, 454,  533,  1521, 1123,
         360,  1460, 878,  1531, 1534, 92,   1538, 645,  618,  1150, 974,  1388,
         93,   1549, 654,  296,  1552, 656,  937,  1558, 600,  1149, 216,  95,
         148,  665,  773,  1562, 669,  1496, 1568, 672,  1537, 1265, 1266, 1576,
         730,  100,  934,  594,  1581, 684,  1311, 1072, 688,  562,  1590, 1469,
         1592, 1445, 102,  630,  698,  1596, 1481, 781,  947,  1380, 1601, 103,
         459,  1222, 105,  522,  1606, 405,  106,  1356, 1610, 369,  111,  1615,
         720,  895,  409,  819,  784,  221,  889,  109,  1256, 332,  110,  357,
         217,  219,  768,  736,  1427, 1314, 128,  1098, 951,  743,  1626, 756,
         1138, 251,  1631, 113,  344,  1328, 1627, 114,  786,  758,  1504, 1308,
         589,  1013, 115,  1640, 765,  898,  993,  116,  860,  999,  1255, 772,
         564,  1633, 1417, 1103, 1334, 617,  1078, 227,  783,  1340, 1361, 1651,
         1199, 712,  790,  1572, 1659, 1212, 1337, 1541, 1170, 121,  801,  802,
         1665, 1463, 1467, 1198, 1302, 1261, 122,  1670, 123,  1046, 815,  939,
         1049, 668,  1158, 1415, 1240, 125,  575,  1189, 1677, 126,  836,  1517,
         127,  315,  834,  1434, 411,  1341, 362,  1516, 517,  606,  599,  1381,
         129,  1264, 1689, 1169, 130,  1690, 855,  1470, 298,  901,  1075, 990,
         641,  994,  132,  1317, 825,  343,  870,  1694, 793,  1484, 1693, 278,
         1088, 766,  880,  1242, 1699, 1700, 185,  1546, 847,  1354, 408,  1439,
         139,  307,  893,  1397, 263,  888,  886,  1248, 1448, 900,  1040, 664,
         1557, 497,  1041, 580,  142,  1707, 143,  911,  1447, 1114, 716,  144,
         1711, 919,  927,  270,  187,  1342, 1102, 626,  924,  1684, 1094, 1404,
         682,  1582, 146,  714,  1501, 1674, 1493, 256,  1718, 147,  918,  1696,
         1252, 1269, 653,  308,  149,  488,  949,  539,  800,  1254, 1502, 955,
         732,  257,  1330, 151,  735,  581,  152,  267,  1681, 1641, 153,  1679,
         968,  1035, 973,  972,  965,  962,  1064, 1128, 1283, 1284, 155,  981,
         363,  992,  891,  283,  824,  1005, 335,  489,  964,  1617, 1232, 157,
         158,  997,  961,  1423, 753,  869,  1236, 1006, 1736, 167,  1692, 1738,
         366,  762,  967,  1632, 706,  1528, 161,  1612, 365,  1019, 1494, 487,
         1023, 1374, 1733, 1027, 1300, 1473, 746,  742,  1034, 1367, 1551, 1365,
         1749, 1067, 897,  165,  1425, 166,  1080, 1045, 1251, 1288, 1048, 1089,
         1145, 1151, 844,  1653, 1413, 495,  168,  1747, 1060, 1032, 833,  807,
         677,  1142, 252,  1437, 170,  1137, 526,  172,  1364, 178,  1598, 1654,
         175,  954,  273,  197,  418,  174,  398,  1044, 1081, 1176, 629,  699,
         176,  1346, 1559, 388,  631,  1008, 849,  873,  728,  749,  1229, 1636,
         180,  1723, 1664, 1322, 1566, 1426, 709,  181,  1748, 1115, 1697, 1051,
         183,  1106, 1122, 846,  1306, 667,  1339, 744,  537,  1131, 1238, 1635,
         1002, 567,  464,  1705, 1140, 970,  439,  254,  957,  1018, 1113, 976,
         1715, 189,  370,  190,  1249, 303,  582,  191,  1520, 1394, 796,  381,
         1623, 1613, 354,  412,  1753, 958,  1230, 1275, 194,  1237, 1756, 1175,
         195,  1758, 1272, 1179, 560,  711,  1181, 1569, 1028, 196,  1410, 662,
         740,  703,  601,  1518, 1235, 275,  1141, 1195, 1420, 512,  1480, 348,
         633,  200,  838,  201,  689,  763,  236,  1459, 504,  1766, 202,  1524,
         243,  1535, 1213, 1347, 1216, 1177, 841,  852,  392,  551,  1709, 205,
         687,  948,  1385, 206,  1752, 1495, 295,  207,  1076, 724,  1092, 928,
         907,  1600, 209,  704,  1057, 1456, 1243, 1525, 540,  1461, 1280, 466,
         577,  211,  1703, 1058, 212,  1671, 858,  1629, 747,  519,  1260, 513,
         1603, 265,  403,  914,  214,  1644, 890,  1639, 387,  989,  1011, 1303,
         1432, 1401, 1522, 979,  1498, 226,  330,  1586, 980,  1431, 555,  456,
         1050, 223,  868,  1287, 1331, 1318, 224,  1604, 225,  1444, 374,  552,
         284,  1294, 1159, 1768, 1298, 1039, 1550, 262,  1253, 867,  1761, 230,
         1079, 1307, 1565, 1616, 1026, 1257, 1395, 482,  1548, 1775, 1435, 232,
         1016, 692,  1223, 339,  592,  826,  233,  234,  1024, 1326, 1152, 1070,
         1226, 930,  1722, 235,  1683, 922,  1345, 953,  310,  248,  524,  1716,
         237,  695,  1556, 328,  1402, 239,  721,  771,  1597, 260,  1055, 1442,
         1390, 1136, 1441, 1583, 760,  1350, 983,  754,  1436, 916,  1217, 1132,
         673,  1450, 1363, 1770, 1594, 1020, 1370, 1369, 971,  1720, 508,  1172,
         854,  613,  727,  249,  1378, 1655, 1160, 1490, 523,  1100, 1239, 1577,
         420,  929,  1392, 1686, 1455, 507,  1087, 253,  1289, 1191, 1595, 1488,
         1539, 1043, 379,  686,  1074, 484,  1751, 327,  1309, 1706, 585,  385,
         696,  258,  1112, 1409, 259,  1726, 1412, 1673, 1295, 1267, 1698, 1773,
         1418, 1735, 799,  1769, 1119, 1717, 1406, 902,  376,  1451, 1428, 1642,
         1638, 1561, 542,  1250, 731,  1667, 977,  266,  463,  701,  845,  604,
         1178, 1348, 1186, 620,  590,  1185, 1483, 269,  1704, 431,  1278, 835,
         271,  770,  1429, 1743, 1391, 932,  1419, 336,  367,  1325, 856,  1376,
         514,  1529, 1725, 830,  383,  1220, 1355, 1742, 282,  678,  1465, 776,
         646,  471,  1422, 286,  394,  1728, 950,  884,  433,  446,  291,  1277,
         938,  378,  292,  1646, 866,  1386, 1593, 853,  1353, 1173, 1031, 294,
         1611, 1446, 1605, 461,  1710, 1297, 1591, 1772, 1332, 899,  419,  598,
         634,  1299, 1661, 1408, 1188, 769,  300,  1527, 301,  1505, 1407, 1487,
         1454, 1211, 423,  1745, 1687, 1542, 903,  1129, 996,  876,  909,  1621,
         913,  1554, 693,  1066, 952,  813,  312,  404,  894,  1656, 313,  1258,
         314,  1279, 1545, 1650, 1263, 1320, 683,  316,  1291, 350,  1485, 906,
         1731, 317,  644,  1351, 569,  1544, 1014, 733,  353,  648,  319,  1009,
         320,  1362, 718,  842,  1526, 321,  373,  578,  349,  1682, 1126, 685,
         323,  1532, 944,  325,  326,  428,  892,  1411, 1130, 1746, 410,  960,
         1708, 1165, 966,  782,  1740, 447,  429,  435,  334,  1082, 1509, 1734,
         1587, 751,  337,  538,  805,  1645, 442,  579,  1584, 1270, 483,  1120,
         341,  1713, 1292, 470,  1489, 879,  1741, 741,  1182, 1433, 557,  345,
         942,  346,  865,  390,  863,  458,  1208, 984,  908,  1061, 1405, 475,
         1293, 1139, 1316, 1466, 1563, 536,  850,  1588, 356,  734,  956,  1652,
         368,  364,  1192, 1730, 1244, 1017, 444,  761,  1063, 1246, 396,  1567,
         1285, 550,  1622, 1414, 697,  478,  1624, 777,  1658, 1630, 1007, 1084,
         636,  1068, 1589, 1688, 1510, 829,  998,  380,  501,  1111, 547,  544,
         1205, 1156, 1511, 572,  1647, 384,  1343, 839,  1513, 1479, 479,  778,
         811,  1543, 1540, 1523, 1744, 393,  1777, 1648, 1462, 1649, 1127, 492,
         1486, 1097, 417,  476,  874,  401,  615,  1193, 413,  871,  1174, 1379,
         1573, 1457, 1662, 1574, 490,  1625, 642,  1508, 406,  1755, 1620, 635,
         1214, 680,  595,  1458, 1669, 1774, 1134, 1227, 877,  780,  1184, 1231,
         655,  969,  1675, 1037, 1579, 925,  1161, 1781, 1666, 1071, 568,  602,
         424,  1680, 1200, 737,  775,  493,  1614, 1602, 1643, 1499, 1739, 467,
         432,  1312, 694,  434,  1003, 1183, 1477, 593,  1091, 652,  440,  651,
         1438, 480,  726,  1779, 1712, 549,  1564, 1760, 1194, 450,  1570, 920,
         1421, 1359, 1503, 611,  1259, 818,  1724, 666,  1685, 1349, 1368, 1782,
         715,  1506, 472,  1403, 474,  643,  1319, 681,  1225, 923,  1366, 1571,
         1637, 494,  1599, 1729, 739,  831,  822,  1472, 1530, 1765, 988,  1678,
         1449, 1608, 1321, 610,  499,  502,  1609, 1547, 649,  1101, 1672, 978,
         1304, 827,  705,  1727, 1315, 915,  510,  1389, 1059, 1377, 534,  1780,
         1219, 520,  1560, 660,  887,  1668, 1471, 531,  1274, 661,  1580, 1695,
         1536, 1702, 1663, 1163, 820,  1107, 1197, 1719, 1767, 1203, 1000, 1778,
         1121, 832,  985,  791,  1764, 1133, 565,  637,  1754, 1206, 788,  657,
         584,  659,  1762, 1575, 755,  945,  1585, 931,  1033, 1262, 1167, 1021,
         1453, 910,  1763, 1336, 1305, 1335, 1400, 1344, 1578, 1676, 1162, 640,
         1196, 935,  650,  1618, 1619, 1383, 663,  1628, 808,  1771, 1657, 757,
         1323, 675,  1357, 1360, 679,  1721, 1069, 1533, 1607, 1015, 1691, 1701,
         1124, 812,  713,  1358, 941,  1737, 794,  1553, 1056, 750,  1209, 764,
         767,  789,  809,  1491, 1776, 1555, 1714, 1757, 1660, 1118, 1759, 946,
         1095, 987,  1750, 1634, 1241, 1475, 1732}));

    REQUIRE(S->size() == static_cast<uint64_t>(448'345'497'600));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "025",
                          "perm. Fischer group Fi22 (order 64'561'751'654'400)",
                          "[standard][schreier-sims]") {
    auto rg = ReportGuard(false);
    // Slower than GAP
    constexpr size_t N = 3'511;
    auto             S = std::make_unique<SchreierSims<N>>();
    using Perm         = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,    524,  2,    789,  2130, 777,  491,  3059, 3096, 2636, 527,  11,
         822,  3013, 14,   872,  2392, 2504, 108,  2816, 20,   3295, 851,  23,
         2196, 1503, 2131, 2285, 1061, 1382, 174,  1063, 32,   3057, 490,  846,
         827,  1494, 1640, 3025, 2769, 2858, 42,   1987, 237,  45,   1962, 3393,
         1038, 2974, 50,   1272, 52,   3089, 425,  1109, 1275, 821,  1121, 2314,
         638,  1900, 1649, 63,   64,   1132, 1370, 1590, 68,   2968, 2775, 71,
         3423, 2393, 2154, 75,   1070, 3010, 2642, 79,   2135, 1764, 463,  83,
         417,  2407, 3424, 2748, 3377, 1184, 776,  670,  2072, 653,  3183, 2690,
         96,   1821, 98,   1427, 2804, 1028, 2873, 397,  1196, 207,  3108, 2619,
         18,   2960, 110,  111,  2400, 2492, 245,  2216, 116,  3015, 632,  712,
         3451, 2055, 3036, 2178, 124,  2418, 3030, 127,  1453, 732,  283,  3283,
         273,  2183, 561,  135,  1936, 806,  571,  261,  1460, 669,  142,  1071,
         1431, 145,  3252, 785,  1177, 1469, 327,  1116, 1863, 3087, 3298, 2215,
         156,  2518, 3389, 1289, 160,  1967, 162,  1519, 164,  3210, 166,  3135,
         2507, 169,  3236, 1659, 3403, 299,  30,   2365, 2082, 3080, 415,  1790,
         180,  3359, 3028, 3014, 1876, 2774, 2151, 1444, 188,  1633, 190,  2808,
         1326, 2401, 3308, 988,  1949, 2261, 198,  2246, 200,  2829, 202,  203,
         204,  421,  3148, 105,  2958, 1489, 1542, 211,  814,  1166, 1231, 1334,
         216,  217,  911,  624,  2306, 3055, 1573, 223,  687,  2649, 759,  3215,
         2603, 229,  306,  368,  3195, 2689, 234,  1245, 2009, 44,   1777, 305,
         2260, 3434, 1101, 3292, 1964, 114,  2032, 2014, 918,  249,  474,  1760,
         252,  3119, 254,  1384, 3045, 2132, 3022, 825,  3051, 139,  1745, 263,
         3257, 2647, 266,  1533, 268,  2849, 270,  2606, 866,  132,  1322, 1012,
         1831, 2834, 694,  279,  2912, 2620, 689,  130,  3073, 1862, 286,  848,
         1859, 289,  290,  291,  3297, 2593, 294,  2036, 2395, 339,  1675, 173,
         3198, 1312, 3344, 3238, 2660, 239,  230,  739,  3490, 530,  952,  1521,
         2880, 313,  2628, 905,  3507, 2101, 318,  1681, 2956, 1935, 2699, 1491,
         953,  958,  326,  150,  1536, 2723, 2321, 1671, 2369, 1986, 2140, 1945,
         2113, 3505, 556,  297,  1302, 1963, 1602, 1361, 1386, 345,  346,  371,
         807,  2842, 973,  351,  885,  2719, 2561, 1441, 356,  1984, 358,  359,
         803,  1403, 365,  3373, 1424, 362,  1359, 1508, 231,  1466, 2382, 347,
         1566, 373,  2126, 375,  2991, 2718, 3035, 1451, 2920, 1750, 382,  1554,
         384,  2493, 3040, 1767, 1692, 1068, 2021, 1565, 3110, 393,  3182, 1913,
         1304, 103,  3340, 1197, 3294, 2514, 2687, 2836, 1141, 1828, 2851, 917,
         1713, 409,  2973, 2773, 2417, 611,  1696, 178,  1890, 84,   1955, 3156,
         1877, 205,  3240, 1513, 1162, 54,   426,  427,  2891, 3348, 1232, 630,
         2979, 2162, 2817, 2992, 578,  1131, 3289, 813,  2397, 2819, 1472, 2665,
         838,  1108, 3479, 2744, 3112, 950,  450,  888,  770,  479,  3181, 2462,
         456,  542,  458,  3253, 3070, 3442, 3136, 82,   1179, 1225, 3031, 3127,
         468,  1126, 783,  471,  2426, 473,  250,  2560, 2289, 477,  1150, 453,
         2006, 481,  482,  2908, 484,  1784, 486,  2445, 2905, 1971, 34,   6,
         492,  3027, 494,  3321, 3362, 709,  1161, 1260, 3438, 501,  2308, 503,
         504,  505,  2041, 3245, 2771, 1813, 510,  623,  512,  2934, 2825, 515,
         2931, 2743, 2473, 1135, 3388, 521,  2310, 717,  1,    525,  526,  10,
         1709, 529,  309,  1013, 3432, 533,  1346, 3363, 765,  537,  3016, 2323,
         3081, 541,  457,  543,  2494, 2923, 1376, 1761, 1866, 2508, 2040, 551,
         1994, 1609, 3176, 3235, 338,  886,  3186, 2982, 2625, 134,  562,  563,
         1707, 1477, 566,  567,  568,  1147, 570,  138,  2324, 2458, 2794, 1406,
         1992, 577,  436,  579,  2480, 581,  582,  583,  2146, 3462, 1801, 587,
         588,  2726, 2252, 2727, 1230, 593,  594,  2410, 3415, 856,  3171, 3456,
         1096, 755,  3137, 2618, 1817, 2214, 1735, 2868, 608,  1549, 1788, 413,
         2283, 1220, 3376, 615,  616,  617,  2906, 747,  3124, 826,  1010, 511,
         219,  3023, 2278, 3139, 1855, 629,  431,  3293, 118,  796,  1606, 2052,
         2529, 2338, 60,   2386, 2302, 1251, 642,  1823, 3497, 1966, 3416, 2631,
         1570, 3333, 3190, 651,  2424, 93,   828,  2478, 1550, 1958, 658,  3160,
         990,  1595, 2878, 1267, 793,  1835, 1564, 667,  3303, 141,  91,   1423,
         993,  1119, 2759, 1942, 2350, 2919, 3046, 1128, 834,  681,  1358, 683,
         3218, 3338, 2156, 224,  2137, 282,  2758, 1800, 1772, 2039, 278,  2481,
         3209, 1861, 698,  2573, 2557, 2367, 702,  2199, 2616, 2045, 1931, 2909,
         1204, 497,  3427, 2076, 119,  713,  714,  762,  716,  523,  718,  2864,
         3206, 721,  722,  891,  2938, 3174, 2056, 727,  728,  2643, 730,  1829,
         129,  2211, 1605, 735,  2785, 3169, 738,  307,  3009, 2892, 742,  1787,
         744,  1098, 746,  619,  3260, 2074, 3012, 1222, 1909, 753,  3473, 601,
         756,  757,  2525, 226,  3214, 2926, 715,  1380, 3287, 536,  1476, 919,
         768,  889,  452,  2610, 2447, 773,  2972, 775,  90,   5,    778,  779,
         1693, 781,  1325, 470,  1258, 147,  2016, 787,  1944, 3,    1163, 3018,
         2187, 664,  1045, 2062, 633,  2528, 798,  1253, 2372, 801,  1281, 360,
         2889, 3422, 137,  348,  2066, 2213, 810,  1397, 931,  439,  212,  1896,
         2706, 817,  960,  819,  1537, 57,   12,   2870, 824,  259,  621,  36,
         654,  2584, 830,  831,  832,  833,  680,  852,  3406, 837,  444,  839,
         987,  2537, 1341, 843,  3384, 2688, 35,   2231, 287,  1144, 2364, 22,
         835,  2389, 1922, 3185, 597,  1183, 858,  1889, 860,  3396, 2053, 2844,
         2821, 3097, 272,  2253, 2127, 2930, 2205, 2501, 15,   873,  874,  2702,
         876,  877,  1661, 2377, 1051, 1978, 882,  883,  908,  352,  557,  1102,
         451,  769,  3159, 723,  1879, 893,  894,  1555, 1921, 1103, 898,  899,
         1516, 1387, 1626, 2513, 3301, 315,  3350, 1187, 884,  909,  936,  218,
         912,  3021, 2398, 2124, 1724, 407,  248,  767,  1716, 2442, 922,  3410,
         2020, 2746, 1704, 2093, 928,  929,  930,  812,  1153, 1686, 1151, 1138,
         910,  937,  1528, 1865, 3037, 3378, 3399, 2413, 944,  1229, 1685, 2818,
         1799, 1324, 449,  2320, 310,  324,  954,  3092, 1703, 1878, 325,  3251,
         818,  961,  962,  1571, 2900, 2259, 1379, 967,  2910, 1027, 1408, 971,
         2035, 350,  1015, 1905, 1449, 3113, 978,  1295, 1425, 981,  2692, 3044,
         984,  3417, 2133, 840,  195,  1321, 660,  1851, 1893, 672,  1130, 2847,
         996,  1646, 2411, 3158, 1611, 2585, 2770, 2275, 3409, 1273, 2075, 1708,
         1407, 1669, 622,  1459, 275,  531,  1014, 974,  1016, 1017, 1018, 3177,
         1020, 1422, 3413, 1246, 3394, 1201, 1420, 969,  101,  1623, 3197, 1372,
         3402, 2943, 1034, 1498, 1568, 1789, 48,   1039, 3047, 1041, 1042, 1285,
         1044, 794,  1700, 2159, 2678, 1049, 1743, 880,  1297, 3157, 3200, 3327,
         2312, 3062, 1058, 2648, 3421, 28,   1882, 31,   2102, 1065, 1858, 1067,
         389,  3263, 76,   143,  1139, 1757, 2089, 2359, 1076, 3365, 1953, 2195,
         1080, 1081, 3226, 2669, 1816, 3098, 1086, 2614, 2345, 1792, 3265, 1142,
         1092, 2115, 2490, 1095, 600,  1097, 745,  3366, 2857, 242,  887,  897,
         1104, 1562, 2929, 1107, 445,  55,   1552, 2258, 2422, 1113, 2284, 2977,
         151,  3279, 1118, 673,  1445, 58,   2611, 2519, 2841, 1125, 469,  2043,
         679,  1129, 994,  437,  65,   3472, 2371, 519,  1136, 1689, 935,  1072,
         2896, 404,  1091, 2833, 849,  1247, 2280, 569,  1148, 1677, 478,  934,
         1207, 932,  2299, 1155, 1674, 1157, 1846, 2083, 2731, 498,  424,  790,
         3448, 1165, 213,  1167, 1168, 1169, 1170, 1171, 1208, 1812, 1517, 3420,
         1176, 148,  2476, 464,  1319, 2331, 2136, 857,  89,   1215, 2177, 907,
         2766, 1189, 2640, 3216, 3352, 2353, 1526, 3391, 104,  399,  2226, 1199,
         2098, 1025, 2047, 1320, 708,  1487, 1912, 1152, 1172, 2532, 3061, 1328,
         3499, 1213, 3437, 1185, 3048, 3168, 2155, 1656, 613,  2477, 751,  2434,
         2884, 465,  2791, 2852, 3330, 945,  592,  214,  430,  2621, 2733, 3155,
         1236, 1616, 1641, 2827, 3219, 1898, 1242, 2029, 1282, 235,  1023, 1145,
         2262, 3380, 2370, 641,  1547, 799,  1437, 3364, 1509, 2176, 784,  2326,
         499,  3116, 2850, 3481, 1402, 2576, 1266, 663,  3202, 2003, 2965, 2281,
         51,   1005, 1350, 56,   1627, 1277, 1310, 1960, 3072, 802,  1244, 1283,
         1284, 1043, 2291, 1639, 1288, 159,  3493, 3050, 2543, 1293, 2012, 979,
         2713, 1052, 1679, 2071, 3225, 1762, 340,  2650, 396,  3334, 1306, 2235,
         3447, 2073, 1278, 1652, 301,  1393, 1797, 1315, 2419, 2762, 2450, 1180,
         1203, 989,  274,  1894, 949,  782,  192,  1399, 1211, 1329, 1957, 2962,
         2034, 3228, 215,  2854, 3404, 3466, 3319, 3291, 1340, 842,  1342, 2347,
         3006, 2598, 534,  2712, 3115, 2704, 1274, 2292, 1916, 1353, 2287, 2862,
         2189, 2980, 682,  366,  2536, 343,  1362, 1363, 3024, 2559, 3485, 1367,
         2489, 2679, 66,   1371, 1031, 1414, 1644, 2182, 546,  1377, 3100, 966,
         763,  1906, 29,   3175, 255,  1385, 344,  901,  1388, 1389, 2768, 3132,
         1392, 1313, 1781, 3005, 1673, 811,  1398, 1327, 3503, 2685, 1264, 361,
         1404, 3114, 575,  1008, 970,  1409, 2595, 1411, 3085, 3020, 1373, 2387,
         1416, 1417, 1418, 2166, 1026, 1421, 1021, 671,  364,  980,  2334, 99,
         1428, 1429, 1672, 144,  2247, 1433, 1434, 2792, 2668, 1254, 1438, 3502,
         2019, 355,  1442, 1819, 187,  1120, 2879, 1447, 2303, 976,  2317, 379,
         2198, 128,  1454, 1485, 2753, 1457, 2463, 1011, 140,  3149, 3458, 1902,
         1464, 3429, 369,  1467, 1468, 149,  1556, 1601, 442,  2388, 1560, 2000,
         766,  565,  2894, 1970, 1480, 1481, 3026, 2005, 2809, 1455, 1486, 1205,
         1488, 209,  3347, 323,  2128, 1871, 37,   1495, 1496, 3302, 1035, 1499,
         2095, 3213, 3118, 25,   1504, 2491, 2959, 1806, 367,  1256, 3322, 2990,
         1512, 423,  2680, 1814, 900,  1174, 1518, 163,  1574, 311,  1522, 1995,
         3351, 2895, 1194, 3082, 938,  3058, 1775, 1636, 2815, 267,  3449, 1535,
         328,  820,  1538, 2835, 1540, 2918, 210,  3326, 3487, 1545, 3343, 1252,
         1887, 609,  656,  1551, 1110, 1589, 383,  895,  1470, 1631, 1852, 1559,
         1474, 1561, 1105, 1941, 666,  391,  372,  2351, 1036, 2406, 648,  963,
         1572, 222,  1520, 1608, 2629, 2193, 3361, 1952, 1596, 2637, 1582, 2732,
         1584, 1722, 1586, 1587, 1588, 1553, 67,   3262, 1650, 2695, 1594, 661,
         1580, 1834, 1598, 2749, 2218, 1471, 342,  1603, 2957, 734,  634,  1744,
         1575, 553,  1610, 1000, 1612, 2944, 1765, 1615, 1237, 2589, 1618, 3304,
         1620, 2772, 3001, 1029, 1624, 1625, 902,  1276, 2666, 2033, 1630, 1557,
         1632, 189,  1807, 2158, 1531, 3258, 1702, 1287, 38,   1238, 1642, 1647,
         1374, 1645, 997,  1643, 1648, 62,   1592, 2112, 1311, 2380, 3280, 2148,
         1219, 3443, 1885, 171,  1660, 878,  2446, 2813, 2309, 1950, 1666, 2781,
         1926, 1009, 2145, 331,  1430, 1396, 1156, 298,  1918, 1149, 2935, 1298,
         1680, 319,  2638, 2673, 1684, 946,  933,  2339, 2186, 1137, 2191, 2787,
         388,  780,  1741, 1695, 414,  1888, 2684, 1699, 1046, 1972, 1638, 956,
         926,  1705, 2617, 564,  1007, 528,  2855, 1711, 2266, 408,  3066, 3099,
         920,  1997, 3275, 1872, 2984, 1721, 1585, 1897, 916,  1725, 1726, 1727,
         1756, 1729, 3203, 1731, 2736, 3360, 1892, 606,  1736, 1737, 1738, 2945,
         3341, 1694, 1742, 1050, 1607, 262,  3288, 2222, 1748, 3232, 381,  2729,
         3141, 3128, 1754, 1755, 1728, 1073, 1758, 1759, 251,  547,  1301, 2170,
         81,   1614, 3426, 387,  3370, 1769, 3480, 1771, 692,  1773, 2175, 1530,
         2947, 238,  2171, 2119, 2304, 1394, 2928, 1783, 485,  2663, 1786, 743,
         610,  1037, 179,  2467, 1089, 2533, 1794, 1795, 1796, 1314, 2555, 948,
         691,  586,  2921, 1803, 1804, 1805, 1507, 1634, 2658, 3064, 2898, 1811,
         1173, 509,  1515, 3468, 1084, 604,  1818, 1443, 1820, 97,   2233, 643,
         1824, 1825, 2343, 2812, 405,  731,  1830, 276,  2683, 1833, 1597, 665,
         2703, 3060, 2234, 3034, 1840, 2153, 1842, 2567, 1844, 1845, 1158, 1847,
         1925, 2025, 1850, 991,  1558, 1853, 2720, 628,  2192, 1857, 1066, 288,
         3095, 697,  285,  152,  1864, 939,  548,  1867, 1868, 2756, 2571, 1493,
         1719, 2267, 2438, 3450, 184,  420,  957,  892,  2883, 1924, 1062, 3126,
         2042, 1658, 2142, 1548, 1697, 859,  416,  2163, 1734, 992,  1323, 1895,
         815,  1723, 1241, 2988, 61,   2562, 1463, 1969, 1904, 975,  1381, 2866,
         2544, 752,  3277, 2811, 1206, 395,  2783, 2487, 1352, 3431, 1676, 2644,
         1920, 896,  854,  3247, 1881, 1848, 1668, 2010, 1928, 3077, 2523, 706,
         2157, 3273, 2778, 321,  136,  2444, 3315, 1939, 1940, 1563, 675,  2439,
         788,  335,  2325, 3227, 1948, 196,  1665, 3332, 1579, 1078, 3408, 418,
         2933, 1330, 657,  3111, 1279, 3356, 46,   341,  244,  1965, 645,  161,
         1968, 1903, 1479, 489,  1701, 2856, 2989, 3266, 1976, 1977, 881,  2298,
         3331, 3397, 2028, 1983, 357,  1985, 333,  43,   1988, 1989, 2767, 2107,
         576,  3267, 552,  1523, 3131, 1717, 3400, 3088, 1475, 3017, 2527, 1269,
         3163, 1483, 480,  2007, 3278, 236,  1927, 2061, 1294, 2013, 247,  3476,
         786,  2017, 2018, 1440, 924,  390,  2569, 3101, 2255, 1849, 2026, 2046,
         1982, 1243, 2290, 2993, 246,  1629, 1332, 972,  295,  2037, 2038, 693,
         550,  506,  1884, 1127, 3459, 705,  2027, 1202, 3311, 2144, 3282, 2220,
         635,  862,  2566, 121,  726,  2057, 3482, 2169, 2348, 2011, 795,  2063,
         3033, 3296, 808,  2067, 3065, 2069, 2828, 1299, 92,   1309, 749,  1006,
         711,  2077, 2078, 2510, 2206, 2081, 176,  1159, 2139, 2085, 2700, 2224,
         2535, 1074, 2313, 2091, 2381, 927,  2094, 1500, 2096, 2826, 1200, 2099,
         2615, 317,  1064, 2538, 2552, 2597, 2106, 1991, 2763, 2202, 2360, 2111,
         1651, 336,  2114, 1093, 2116, 2890, 3381, 1779, 2645, 2714, 2122, 2123,
         915,  3140, 374,  868,  1492, 2129, 4,    26,   257,  986,  2134, 80,
         1182, 688,  2138, 2084, 334,  3193, 1886, 2655, 2049, 1670, 584,  2147,
         1655, 3474, 2150, 186,  2914, 1841, 74,   1218, 686,  1932, 1635, 1047,
         2160, 2795, 433,  1891, 2626, 2605, 1419, 2742, 3076, 2059, 1763, 1778,
         2172, 2188, 2174, 1774, 1257, 1186, 123,  2822, 2564, 2181, 1375, 133,
         2969, 2185, 1688, 792,  2173, 1356, 2190, 1690, 1856, 1577, 2357, 1079,
         24,   2197, 1452, 703,  2981, 2201, 2109, 2203, 3509, 870,  2080, 2207,
         3405, 3102, 3191, 733,  2212, 809,  605,  155,  115,  2468, 1600, 2219,
         2051, 2936, 1747, 2551, 2087, 3506, 1198, 2227, 2228, 2229, 2639, 847,
         2755, 1822, 1838, 1307, 2236, 2797, 2531, 2239, 2452, 2241, 2405, 2243,
         2244, 2599, 199,  1432, 2248, 2249, 2250, 2863, 590,  867,  2254, 2024,
         2256, 3411, 1111, 965,  240,  197,  1248, 2263, 3056, 2940, 1712, 1873,
         2268, 2591, 2911, 2271, 2272, 2273, 2764, 1003, 2329, 3371, 626,  2971,
         1146, 1271, 2282, 612,  1114, 27,   2861, 1354, 3117, 476,  2030, 1286,
         1351, 3488, 2488, 3196, 3038, 3342, 1979, 1154, 2300, 3084, 640,  1448,
         1780, 3105, 220,  3071, 502,  1664, 522,  2872, 1056, 2090, 59,   2654,
         2316, 1450, 2318, 2319, 951,  330,  2451, 539,  572,  1946, 1259, 3063,
         2786, 2276, 2330, 1181, 2332, 2579, 1426, 2335, 2484, 2337, 637,  1687,
         2340, 2341, 3079, 1826, 2344, 1088, 2346, 1343, 2060, 2349, 676,  1567,
         2352, 1193, 2354, 2355, 2563, 2194, 2358, 1075, 2110, 3372, 3425, 2363,
         850,  175,  2366, 701,  2368, 332,  1250, 1134, 800,  2423, 2374, 3281,
         2506, 879,  2378, 3104, 1653, 2092, 370,  2485, 2384, 2385, 639,  1415,
         1473, 853,  2390, 2697, 16,   73,   3246, 296,  3285, 440,  914,  2399,
         112,  193,  2902, 2466, 3053, 2242, 1569, 85,   3457, 2409, 595,  998,
         3368, 943,  2464, 2521, 2503, 412,  125,  1316, 2420, 2425, 1112, 2373,
         652,  2421, 472,  2472, 2428, 3325, 2952, 2460, 3008, 3300, 1223, 2541,
         2436, 3134, 1874, 1943, 2440, 2600, 921,  2922, 1937, 487,  1662, 772,
         2454, 3433, 1318, 2322, 2240, 2499, 2448, 2932, 3069, 2457, 573,  3339,
         2431, 2927, 455,  1458, 2414, 2832, 2403, 1791, 2217, 2469, 2470, 2471,
         2427, 518,  2474, 2475, 1178, 1221, 655,  2479, 580,  695,  2482, 2670,
         2336, 2383, 2486, 1915, 2294, 1368, 1094, 1505, 113,  385,  544,  2495,
         2887, 2497, 3207, 2453, 2500, 871,  3002, 2416, 17,   3043, 2376, 168,
         549,  3049, 2079, 3454, 2512, 903,  401,  2661, 3491, 2517, 157,  1123,
         2520, 2415, 2522, 1930, 3032, 758,  3355, 2002, 797,  636,  2985, 2238,
         1209, 1793, 2747, 2088, 1360, 841,  2103, 3153, 2540, 2435, 3029, 1292,
         1908, 2545, 2546, 2547, 3483, 2549, 3354, 2223, 2104, 2553, 2554, 1798,
         2556, 700,  2784, 1365, 475,  354,  1901, 2356, 2180, 2716, 2054, 1843,
         2901, 2022, 2570, 1870, 2572, 699,  2574, 2575, 1265, 3165, 2578, 2333,
         2707, 2581, 2582, 2602, 829,  1001, 2586, 2587, 3143, 1617, 2846, 2269,
         2592, 293,  2823, 1410, 2596, 2105, 1345, 2245, 2441, 2961, 2583, 228,
         2604, 2165, 271,  2607, 2608, 2609, 771,  1122, 3349, 2613, 1087, 2100,
         704,  1706, 603,  107,  281,  1233, 2622, 3455, 2624, 560,  2164, 2627,
         314,  1576, 2630, 647,  2632, 2633, 3230, 2635, 9,    1581, 1682, 2230,
         1190, 2667, 78,   729,  1919, 2120, 2646, 265,  1059, 225,  1303, 3130,
         2843, 3107, 2315, 2143, 2656, 2657, 1808, 2721, 304,  2515, 2662, 1785,
         2664, 443,  1628, 2641, 1436, 1083, 2483, 2853, 2672, 1683, 2674, 2978,
         2676, 3248, 1048, 1369, 1514, 3313, 3264, 1832, 1698, 1401, 2686, 402,
         845,  233,  95,   2691, 982,  2946, 2694, 1593, 3353, 2391, 2698, 322,
         2086, 3395, 875,  1836, 1349, 2705, 816,  2580, 3387, 2709, 2710, 3328,
         1347, 1296, 2121, 2715, 2565, 2999, 377,  353,  1854, 2659, 2722, 329,
         2724, 2725, 589,  591,  2728, 1751, 3145, 1160, 1583, 1234, 2734, 3469,
         1732, 2871, 3484, 2739, 3310, 2964, 2167, 517,  447,  2745, 925,  2534,
         87,   1599, 3379, 2796, 3106, 1456, 2760, 2232, 1869, 3414, 690,  674,
         2754, 2802, 1317, 2108, 2274, 2765, 1188, 1990, 1390, 40,   1002, 508,
         1621, 411,  185,  70,   2776, 2997, 1934, 2779, 2780, 1667, 2782, 1914,
         2558, 736,  2328, 1691, 2788, 2937, 2790, 1226, 1435, 2793, 574,  2161,
         2751, 2237, 2798, 2799, 3255, 3173, 2761, 2803, 100,  2805, 2897, 2807,
         191,  1484, 2810, 1911, 1827, 1663, 2814, 1532, 19,   434,  947,  441,
         2820, 864,  2179, 2594, 2824, 514,  2097, 1239, 2070, 201,  2830, 3256,
         2465, 1143, 277,  1539, 403,  2837, 3375, 3220, 2840, 1124, 349,  2652,
         863,  3486, 2590, 995,  2848, 269,  1262, 406,  1227, 2671, 1335, 1710,
         1973, 1100, 41,   2859, 2860, 2286, 1355, 2251, 719,  3501, 1907, 2867,
         607,  2963, 823,  2737, 2311, 102,  2975, 2875, 2876, 2877, 662,  1446,
         312,  2881, 3446, 1880, 1224, 3221, 2886, 2496, 2888, 804,  2117, 428,
         741,  2893, 1478, 1525, 1140, 2806, 1810, 3357, 964,  2568, 2402, 2996,
         2904, 488,  618,  2907, 483,  707,  968,  2270, 280,  3103, 2152, 3090,
         2916, 2917, 1541, 677,  380,  1802, 2443, 545,  2967, 2925, 761,  2461,
         1782, 1106, 869,  516,  2455, 1956, 513,  1678, 2221, 2789, 724,  3374,
         2265, 3467, 3307, 1033, 1613, 1739, 2693, 1776, 2948, 2949, 3270, 3316,
         2430, 2953, 2954, 3299, 320,  1604, 208,  1506, 109,  2601, 1331, 2869,
         2741, 1270, 2966, 2924, 69,   2184, 3234, 2279, 774,  410,  49,   2874,
         2976, 1115, 2675, 432,  1357, 2200, 559,  3383, 1720, 2530, 2986, 2987,
         1899, 1974, 1511, 376,  435,  2031, 2994, 2995, 2903, 2777, 3439, 2717,
         3000, 1622, 2502, 3286, 3004, 1395, 1344, 3007, 2432, 740,  77,   3011,
         750,  13,   183,  117,  538,  2001, 791,  3244, 1413, 913,  258,  625,
         1364, 39,   1482, 493,  182,  2542, 126,  466,  2524, 2064, 1839, 378,
         122,  940,  2296, 3039, 386,  3041, 3042, 2505, 983,  256,  678,  1040,
         1216, 2509, 1291, 260,  3052, 2404, 3054, 221,  2264, 33,   1529, 7,
         1837, 1210, 1057, 2327, 1809, 2068, 1714, 3067, 3068, 2456, 460,  2307,
         1280, 284,  3120, 3075, 2168, 1929, 3211, 2342, 177,  540,  1527, 3083,
         2301, 1412, 3086, 153,  1999, 53,   2915, 3091, 955,  3093, 3162, 1860,
         8,    865,  1085, 1715, 1378, 2023, 2209, 2913, 2379, 2305, 2752, 2653,
         106,  3164, 392,  1959, 448,  977,  1405, 1348, 1261, 2288, 1502, 253,
         3074, 3440, 3306, 3123, 620,  3125, 1883, 467,  1753, 3129, 2651, 1996,
         1391, 3133, 2437, 167,  462,  602,  3249, 627,  2125, 1752, 3142, 2588,
         3144, 2730, 3464, 3358, 206,  1461, 3150, 3154, 3152, 2539, 3151, 1235,
         419,  1053, 999,  890,  659,  3161, 3094, 2004, 3109, 2577, 3166, 3167,
         1217, 737,  3390, 598,  3495, 2801, 725,  1383, 554,  1019, 3178, 3345,
         3180, 454,  394,  94,   3385, 855,  558,  3407, 3188, 3445, 650,  2210,
         3192, 2141, 3194, 232,  2295, 1030, 300,  3199, 1054, 3201, 1268, 1730,
         3335, 3205, 720,  2498, 3208, 696,  165,  3078, 3212, 1501, 760,  227,
         1191, 3217, 684,  1240, 2839, 2885, 3222, 3223, 3224, 1300, 1082, 1947,
         1333, 3229, 2634, 3231, 1749, 3233, 2970, 555,  170,  3237, 303,  3239,
         422,  3241, 3401, 3243, 3019, 507,  2394, 1923, 2677, 3138, 3250, 959,
         146,  459,  3254, 2800, 2831, 264,  1637, 3259, 748,  3261, 1591, 1069,
         2682, 1090, 1975, 1993, 3268, 3269, 2950, 3271, 3317, 1933, 3274, 1718,
         3276, 1910, 2008, 1117, 1654, 2375, 2050, 131,  3284, 2396, 3003, 764,
         1746, 438,  3444, 1339, 243,  631,  400,  21,   2065, 292,  154,  2955,
         2433, 904,  1497, 668,  1619, 3305, 3122, 2942, 194,  3496, 2740, 2048,
         3312, 2681, 3314, 1938, 2951, 3272, 3318, 1338, 3320, 495,  1510, 3489,
         3324, 2429, 1543, 1055, 2711, 3329, 1228, 1980, 1951, 649,  1305, 3204,
         3336, 3337, 685,  2459, 398,  1740, 2297, 1546, 302,  3179, 3346, 1490,
         429,  2612, 906,  1524, 1192, 2696, 2550, 2526, 1961, 2899, 3147, 181,
         1733, 1578, 496,  535,  1255, 1077, 1099, 3367, 2412, 3369, 1768, 2277,
         2361, 363,  2939, 2838, 614,  88,   941,  2750, 1249, 2118, 3382, 2983,
         844,  3184, 3386, 2708, 520,  158,  3170, 1195, 3392, 47,   1024, 2701,
         861,  1981, 3398, 942,  1998, 3242, 1032, 172,  1336, 2208, 836,  3187,
         1954, 1004, 923,  2257, 3412, 1022, 2757, 596,  646,  985,  3418, 3419,
         1175, 1060, 805,  72,   86,   2362, 1766, 710,  3428, 1465, 3510, 1917,
         532,  2449, 241,  3435, 3478, 1214, 500,  2998, 3121, 3441, 461,  1657,
         3290, 3189, 2882, 1308, 1164, 1534, 1875, 120,  3452, 3453, 2511, 2623,
         599,  2408, 1462, 2044, 3460, 3461, 585,  3463, 3146, 3465, 1337, 2941,
         1815, 2735, 3470, 3471, 1133, 754,  2149, 3475, 2015, 3477, 3436, 446,
         1770, 1263, 2058, 2548, 2738, 1366, 2845, 1544, 2293, 3323, 308,  2516,
         3492, 1290, 3494, 3172, 3309, 644,  3498, 1212, 3500, 2865, 1439, 1400,
         3504, 337,  2225, 316,  3508, 2204, 3430}));

    S->add_generator(make<Perm>(
        {0,    556,  325,  67,   1579, 391,  2643, 1896, 2439, 2666, 3145, 820,
         2475, 3036, 1735, 2023, 1741, 3404, 1633, 363,  3334, 1163, 2404, 3217,
         230,  225,  3302, 2527, 1117, 2398, 2936, 2177, 3233, 2216, 472,  3428,
         2479, 7,    813,  2321, 376,  199,  1853, 1621, 2948, 2510, 1569, 838,
         2158, 521,  195,  1768, 2337, 3118, 1594, 863,  790,  1631, 2818, 3098,
         712,  2188, 2149, 254,  2574, 662,  72,   381,  580,  805,  3380, 517,
         1640, 414,  1366, 2998, 147,  539,  179,  385,  2902, 1816, 1100, 1951,
         519,  475,  2060, 3264, 3168, 1551, 3465, 2134, 537,  3149, 2739, 2029,
         1529, 2494, 1185, 2975, 2165, 963,  1049, 1337, 3095, 2489, 1749, 1428,
         162,  2323, 2701, 276,  764,  3122, 1459, 792,  917,  1176, 2298, 2349,
         1298, 1303, 1870, 2281, 3244, 2930, 1580, 701,  2525, 3068, 2225, 2577,
         1242, 1630, 681,  1077, 1372, 770,  1183, 263,  2063, 2085, 3164, 724,
         1383, 2740, 3214, 971,  1029, 2978, 346,  2347, 697,  100,  3017, 612,
         2032, 2529, 1507, 2956, 1773, 888,  954,  957,  2091, 280,  1571, 3496,
         153,  950,  3426, 1706, 1079, 3229, 3369, 190,  3381, 1933, 1881, 1849,
         2935, 2890, 3076, 2362, 3314, 2836, 3405, 1032, 258,  1419, 3097, 2248,
         2427, 1876, 2136, 598,  2721, 79,   2878, 1355, 1921, 3191, 1505, 731,
         2580, 352,  972,  2394, 3014, 473,  3286, 2410, 1423, 2880, 3278, 348,
         3101, 924,  2150, 1647, 196,  1022, 3331, 418,  779,  2456, 2954, 1394,
         224,  592,  1772, 149,  471,  3031, 510,  2436, 1479, 668,  165,  3431,
         2141, 3418, 2985, 3459, 2246, 1830, 2429, 751,  1648, 1244, 753,  1230,
         3117, 379,  2812, 2786, 847,  1090, 2653, 1265, 251,  3333, 2745, 14,
         2539, 489,  2670, 3134, 1747, 3305, 41,   3374, 616,  2720, 1239, 1555,
         1233, 2581, 1878, 487,  3040, 333,  450,  467,  1092, 2639, 1632, 2184,
         3272, 2760, 1657, 1786, 2118, 1015, 368,  309,  1364, 2911, 2966, 2684,
         35,   2449, 931,  2025, 2520, 2803, 197,  2810, 2926, 2987, 1144, 1862,
         809,  2832, 484,  1128, 2605, 1306, 465,  982,  219,  1578, 3258, 2901,
         3398, 1399, 1089, 1894, 928,  2842, 2689, 2499, 2830, 3326, 2395, 1908,
         2563, 1188, 2843, 2982, 3323, 1985, 89,   710,  366,  1626, 1696, 3094,
         1296, 2758, 480,  2516, 2846, 30,   3132, 1181, 1357, 406,  1488, 2715,
         1794, 68,   1638, 1374, 407,  2594, 2077, 3204, 1590, 1930, 3243, 2528,
         1251, 2162, 2426, 3169, 1024, 166,  1107, 979,  401,  2466, 3420, 2259,
         894,  1819, 40,   207,  1553, 1644, 624,  1361, 3282, 1755, 3375, 2900,
         514,  1234, 2089, 3315, 28,   3137, 744,  2218, 2127, 3277, 1545, 1453,
         2800, 597,  1543, 1434, 2081, 2062, 528,  2775, 506,  1487, 3373, 298,
         2537, 661,  759,  2488, 3127, 657,  1573, 2957, 987,  1070, 193,  2596,
         328,  1132, 551,  1174, 703,  1986, 481,  2864, 1668, 3056, 2899, 718,
         2100, 2887, 245,  1746, 1129, 2609, 815,  1481, 647,  1186, 3016, 2920,
         549,  3120, 3256, 2088, 1471, 846,  2033, 2211, 2641, 1599, 882,  844,
         1161, 1609, 2419, 1695, 2601, 428,  2752, 1886, 3173, 46,   1536, 2202,
         2271, 3065, 862,  2727, 1082, 1882, 855,  1528, 2490, 2686, 66,   520,
         919,  831,  2942, 1663, 3294, 1393, 1879, 2815, 2895, 2955, 370,  990,
         2631, 570,  500,  3113, 1898, 1651, 636,  2802, 2334, 1950, 257,  1760,
         3208, 1139, 1194, 1759, 761,  213,  3043, 3035, 2953, 3291, 1028, 1130,
         188,  2416, 980,  2546, 3207, 15,   1470, 2952, 2048, 2018, 2317, 1057,
         2498, 2433, 925,  2523, 672,  1734, 1023, 2460, 1327, 2674, 2283, 1386,
         344,  3506, 1719, 2729, 1702, 3450, 1007, 3276, 641,  1974, 1506, 3030,
         585,  1548, 1795, 1929, 2749, 3340, 1382, 2505, 1439, 1865, 2716, 148,
         999,  329,  497,  2458, 2254, 1381, 503,  3469, 3152, 2109, 1109, 1937,
         788,  802,  1263, 709,  553,  600,  3249, 1813, 2947, 2422, 1586, 2785,
         2233, 1510, 1652, 1456, 3355, 588,  3077, 3212, 2204, 2240, 2662, 3241,
         457,  1825, 554,  1272, 1591, 1325, 2759, 1087, 2129, 34,   2262, 1291,
         2229, 3013, 2521, 1600, 1869, 1859, 821,  3458, 229,  2756, 1671, 2147,
         1350, 2207, 1836, 2130, 2869, 2870, 3084, 1678, 1472, 2572, 2050, 659,
         2703, 2583, 873,  1422, 3187, 2264, 3432, 1063, 2001, 772,  541,  606,
         781,  604,  872,  2116, 3339, 2201, 1880, 3438, 617,  3395, 1452, 479,
         1259, 1465, 3254, 1673, 2356, 1440, 2951, 1227, 1524, 1550, 3170, 3111,
         2724, 903,  109,  3410, 3054, 2891, 1627, 969,  857,  1847, 2943, 654,
         1531, 3327, 1302, 3473, 23,   373,  2205, 3008, 1135, 1831, 3044, 1561,
         1797, 2455, 2293, 994,  2015, 508,  1566, 2371, 2789, 1368, 2808, 2557,
         743,  2181, 2894, 169,  183,  3041, 783,  817,  3102, 2157, 906,  2892,
         1119, 2389, 1576, 3061, 139,  1684, 3037, 776,  3146, 2235, 1872, 2630,
         1981, 71,   3228, 1448, 2984, 335,  2698, 2534, 1714, 3449, 3320, 1318,
         2330, 544,  656,  2620, 1909, 3154, 2872, 3042, 2968, 974,  2250, 92,
         2065, 2223, 2904, 2159, 2006, 1450, 1037, 1098, 575,  3003, 3344, 1468,
         3051, 3387, 2772, 161,  1924, 118,  2725, 87,   1069, 2567, 267,  1873,
         3226, 828,  1041, 3090, 97,   1313, 2299, 1775, 260,  2106, 246,  522,
         3163, 3005, 2669, 2096, 2690, 3236, 766,  1511, 1829, 2738, 2359, 1143,
         1467, 275,  774,  1277, 1311, 3171, 2889, 1305, 2989, 1949, 318,  3058,
         890,  729,  431,  3251, 1283, 593,  1165, 2055, 2324, 1988, 1515, 1681,
         2095, 2707, 1637, 867,  1756, 995,  2279, 1120, 1178, 3309, 1748, 2101,
         2308, 3049, 2011, 496,  1217, 829,  2251, 2030, 2156, 2042, 441,  2569,
         2076, 1431, 2092, 2980, 860,  1400, 3143, 3131, 3250, 301,  1522, 1977,
         2289, 3107, 3456, 2152, 849,  2604, 2370, 926,  2965, 1332, 981,  2179,
         558,  1517, 2859, 1064, 121,  1793, 2122, 2817, 1567, 2495, 464,  117,
         1570, 3025, 3429, 321,  1964, 2588, 1076, 2565, 1796, 2656, 1740, 762,
         914,  2450, 2350, 2163, 3248, 228,  2858, 945,  650,  694,  2592, 128,
         912,  1102, 1142, 1269, 3141, 607,  2909, 608,  3328, 2833, 3088, 78,
         2685, 1238, 2940, 2175, 2452, 2708, 2331, 733,  2501, 2417, 2391, 2781,
         1792, 2237, 2493, 3350, 3147, 707,  2514, 2914, 1834, 486,  3497, 3492,
         640,  1353, 1764, 2244, 1213, 722,  3280, 2228, 1054, 2663, 1257, 419,
         1841, 1935, 2491, 3177, 1762, 143,  57,   154,  2423, 2549, 1597, 1261,
         2671, 2265, 3160, 451,  1601, 3158, 3073, 320,  456,  2687, 1003, 3466,
         1941, 395,  1822, 1086, 3411, 1267, 953,  1389, 2160, 2369, 403,  3383,
         929,  2000, 2766, 3114, 1474, 1855, 2682, 63,   2420, 86,   962,  868,
         2515, 1616, 2658, 3481, 3507, 3112, 3099, 559,  270,  192,  2829, 3234,
         2937, 448,  3487, 1771, 2477, 1712, 758,  1699, 2634, 2699, 460,  1140,
         1589, 2480, 495,  2125, 2916, 996,  1915, 986,  527,  3010, 3384, 172,
         2428, 2632, 175,  2418, 2691, 1560, 2912, 1961, 1430, 2867, 905,  1343,
         3270, 2313, 2341, 763,  2069, 222,  2140, 2187, 2447, 262,  2616, 2322,
         2917, 1147, 3015, 1584, 3162, 1913, 1701, 1060, 84,   1846, 2387, 135,
         2044, 3424, 787,  2117, 2485, 702,  3048, 1154, 561,  1655, 1180, 212,
         2763, 3489, 704,  932,  609,  1823, 698,  2757, 2407, 2664, 728,  2260,
         1598, 1987, 2113, 3028, 2003, 1942, 564,  2075, 3480, 1222, 25,   2268,
         880,  65,   1919, 1451, 3494, 1266, 1593, 2288, 3303, 2793, 2950, 3299,
         2570, 303,  1953, 455,  1574, 1346, 2151, 2624, 2877, 1770, 1936, 1101,
         157,  2047, 2622, 1767, 3376, 1653, 502,  1669, 2470, 2137, 2028, 947,
         2777, 2131, 271,  3505, 3399, 483,  3096, 3190, 1820, 2608, 1606, 185,
         281,  1541, 840,  324,  1351, 443,  1779, 1532, 1732, 2991, 3001, 708,
         24,   64,   2502, 1160, 1072, 2226, 3128, 988,  423,  289,  3213, 1246,
         878,  664,  3060, 2326, 1776, 2933, 3237, 3219, 1379, 1608, 383,  1067,
         2209, 687,  9,    314,  433,  1656, 916,  3423, 1038, 1002, 1397, 970,
         1863, 1385, 1828, 2182, 2508, 1198, 534,  360,  3322, 1700, 2276, 799,
         2329, 3275, 2087, 2790, 1339, 3498, 1643, 2114, 151,  2052, 555,  1503,
         1056, 1916, 2166, 2478, 1088, 2469, 2301, 3316, 241,  531,  367,  3401,
         1206, 1,    992,  2929, 1193, 2782, 232,  130,  8,    2519, 3471, 55,
         2650, 1324, 2190, 2778, 2714, 2041, 1293, 634,  2688, 2734, 3508, 1861,
         1493, 327,  1612, 1031, 2451, 3440, 31,   653,  584,  1676, 1686, 3115,
         2355, 645,  2309, 3455, 1564, 1731, 2311, 1280, 2801, 2962, 1838, 1322,
         1983, 870,  2178, 3400, 264,  2067, 778,  504,  651,  209,  3441, 793,
         1352, 976,  3416, 2161, 1403, 3119, 1965, 871,  1310, 615,  3437, 769,
         342,  2421, 2963, 688,  791,  1934, 1288, 3109, 2411, 2614, 2090, 1218,
         752,  174,  1131, 2755, 3343, 1091, 171,  1697, 719,  3183, 3092, 1535,
         2425, 1677, 1408, 422,  2704, 3150, 678,  3503, 2277, 2970, 3142, 1094,
         1946, 832,  646,  1745, 2668, 2958, 3445, 3361, 294,  3468, 1294, 1226,
         1641, 138,  2340, 1483, 1066, 2949, 1525, 1808, 515,  103,  425,  627,
         876,  1437, 2771, 3349, 104,  2252, 3488, 775,  2432, 3,    1791, 1463,
         2373, 2545, 2886, 959,  1477, 74,   1494, 2120, 1016, 3103, 1464, 305,
         689,  3495, 438,  639,  1051, 2338, 2841, 907,  2788, 234,  1486, 2078,
         2086, 48,   133,  1809, 3246, 296,  3224, 748,  227,  738,  1979, 392,
         1126, 36,   3189, 2376, 396,  1093, 1957, 3133, 1708, 2651, 3446, 2811,
         818,  2071, 833,  705,  198,  2804, 3193, 44,   3439, 676,  200,  2339,
         3464, 1683, 543,  663,  2221, 1380, 509,  1806, 3403, 622,  105,  540,
         1211, 2659, 2600, 1208, 2054, 2526, 548,  2683, 3342, 1595, 3281, 2927,
         966,  341,  2552, 936,  3021, 1295, 691,  1052, 250,  2014, 1384, 814,
         1447, 3216, 2016, 898,  3079, 3257, 2133, 3290, 1857, 1190, 1247, 1944,
         2839, 1672, 2692, 3012, 3341, 442,  1720, 602,  3055, 1152, 3351, 429,
         2976, 239,  3370, 3397, 1162, 357,  2571, 2835, 536,  524,  693,  960,
         3195, 3385, 2995, 1014, 816,  2375, 2806, 2465, 563,  795,  1004, 2903,
         319,  2093, 315,  566,  777,  2748, 12,   453,  1396, 463,  1682, 1975,
         1807, 1729, 1134, 690,  1501, 1750, 454,  290,  2312, 1877, 1780, 2342,
         842,  1539, 3345, 187,  2486, 985,  1349, 1001, 3285, 1801, 3451, 317,
         2819, 3413, 2762, 1905, 1892, 3346, 3407, 1489, 3067, 2765, 2357, 1670,
         1556, 1818, 2316, 2124, 2173, 2598, 851,  2021, 1138, 2327, 1858, 416,
         843,  1982, 786,  2606, 1415, 1520, 735,  3148, 1301, 3034, 205,  322,
         240,  952,  739,  3288, 3287, 869,  323,  1629, 934,  1421, 201,  1219,
         2296, 1519, 194,  1084, 574,  268,  293,  3363, 22,   2773, 21,   412,
         1976, 2167, 411,  1885, 3414, 717,  1478, 2361, 2628, 884,  1484, 137,
         2827, 2750, 331,  2072, 2705, 2504, 347,  767,  726,  111,  2795, 1245,
         421,  3274, 3421, 3330, 2587, 951,  1237, 2068, 2913, 2981, 848,  3463,
         711,  2058, 1685, 69,   339,  1549, 1932, 3417, 1781, 3070, 3500, 42,
         1170, 27,   1718, 2481, 2415, 3313, 2706, 345,  1034, 1763, 413,  132,
         2040, 1664, 3394, 2013, 2462, 1997, 1614, 3106, 1375, 1080, 3198, 2754,
         2657, 677,  2145, 167,  1401, 466,  1966, 58,   3105, 398,  2798, 1938,
         2287, 3304, 2862, 1177, 3485, 2138, 1320, 1104, 2744, 2655, 389,  1508,
         159,  3130, 1223, 3412, 2414, 2295, 2884, 388,  220,  468,  2828, 1062,
         1262, 1914, 2363, 1694, 599,  2399, 338,  2195, 1895, 1713, 3289, 435,
         2320, 2105, 1654, 3448, 3032, 1639, 1711, 1011, 1509, 545,  1330, 2518,
         2575, 823,  16,   399,  1412, 3180, 2993, 2266, 3402, 20,   1071, 2589,
         1688, 1758, 261,  2532, 565,  2200, 911,  2144, 3462, 1215, 885,  3477,
         1722, 2629, 2257, 33,   1254, 649,  2702, 1996, 1166, 658,  458,  1241,
         3156, 3074, 2189, 2831, 2861, 1404, 2635, 184,  2183, 1842, 1738, 1742,
         3359, 2169, 1207, 2440, 3377, 2718, 145,  1457, 529,  2977, 1344, 1476,
         2307, 557,  2747, 1377, 3186, 2697, 2559, 1392, 2665, 208,  1111, 2192,
         29,   252,  1435, 765,  3482, 808,  2304, 1420, 1728, 516,  353,  2285,
         2352, 3223, 3353, 2358, 1201, 3461, 61,   2213, 2774, 377,  32,   1123,
         1546, 2944, 1784, 1596, 856,  955,  1179, 2258, 47,   1623, 243,  623,
         755,  2328, 1124, 426,  247,  3336, 1615, 2681, 2660, 2536, 249,  2906,
         1646, 2678, 1426, 2405, 287,  940,  3231, 3179, 918,  579,  369,  644,
         2132, 2733, 1660, 1018, 1212, 2857, 1906, 675,  2004, 1317, 152,  1158,
         3222, 113,  2263, 1040, 601,  1620, 382,  3009, 3319, 2234, 365,  633,
         11,   256,  238,  1592, 3062, 3011, 1993, 1243, 1200, 2712, 1947, 1821,
         1043, 2860, 1492, 291,  1777, 1149, 2735, 444,  2825, 2568, 302,  1514,
         1462, 1931, 2123, 686,  866,  1649, 2238, 2020, 2325, 3047, 1871, 1925,
         2102, 1943, 3029, 618,  3175, 807,  2112, 498,  1611, 923,  1390, 1495,
         2284, 1480, 1371, 1568, 1363, 1628, 2677, 3260, 2241, 1005, 3259, 1248,
         2314, 364,  2554, 1999, 2026, 2146, 3501, 1025, 1971, 1537, 437,  1911,
         393,  893,  218,  3201, 2578, 713,  2865, 430,  136,  1407, 2471, 2056,
         1053, 3306, 2343, 210,  1959, 1577, 2550, 1150, 680,  2066, 1661, 1336,
         2059, 2792, 4,    746,  3123, 1387, 292,  1499, 3367, 96,   1460, 3491,
         875,  756,  2918, 896,  2761, 3268, 1300, 1899, 1956, 2876, 2302, 858,
         2533, 83,   3182, 1753, 259,  1328, 1417, 3178, 94,   3002, 2103, 3318,
         1766, 3218, 19,   578,  106,  1897, 3253, 1926, 1153, 2517, 3225, 1710,
         538,  1726, 2472, 1187, 2780, 2807, 2531, 2497, 1666, 1675, 2390, 2142,
         282,  1693, 3486, 417,  91,   2769, 2783, 1096, 3378, 1282, 3504, 1019,
         334,  737,  3215, 217,  248,  2522, 1105, 1308, 1169, 660,  989,  1854,
         114,  796,  1727, 1680, 1444, 3347, 307,  1314, 2805, 2435, 1252, 2104,
         2994, 3184, 2560, 1115, 2824, 2474, 2813, 1402, 2199, 380,  3108, 967,
         278,  2164, 3202, 3022, 2007, 1075, 614,  2799, 942,  1497, 1733, 108,
         900,  745,  1737, 3181, 1354, 2496, 1255, 1017, 277,  155,  1196, 2667,
         1581, 968,  2627, 2905, 1491, 3185, 1991, 170,  3292, 311,  3238, 1890,
         2402, 2403, 62,   2064, 810,  2888, 1513, 3069, 427,  354,  362,  2743,
         1645, 3422, 469,  3075, 3338, 1837, 2111, 447,  854,  669,  2535, 1315,
         2919, 98,   937,  2008, 3389, 283,  899,  90,   3396, 511,  3443, 1835,
         1736, 2551, 1236, 2896, 2941, 1202, 1703, 3444, 176,  1184, 891,  146,
         1279, 1068, 2885, 2434, 523,  1047, 1662, 1866, 2206, 2468, 445,  2875,
         3493, 1427, 2882, 1572, 2722, 1073, 2584, 284,  3296, 804,  2170, 2646,
         1216, 1199, 825,  378,  2043, 1044, 3196, 1978, 2821, 2840, 2997, 2492,
         586,  2675, 211,  336,  1839, 1225, 2196, 1891, 3135, 834,  2261, 3194,
         2649, 1469, 1690, 2220, 1376, 5,    1845, 297,  836,  1155, 1010, 415,
         2354, 3063, 629,  39,   2483, 203,  3203, 1006, 797,  2585, 2139, 1761,
         1922, 1214, 2097, 552,  1338, 2617, 1810, 3057, 1475, 2879, 2400, 53,
         394,  3419, 1224, 2332, 3221, 2555, 1256, 3293, 1544, 2543, 304,  1157,
         99,   811,  3209, 1625, 494,  273,  60,   535,  3110, 1232, 1636, 1455,
         1260, 2378, 1583, 568,  269,  2680, 1604, 189,  1585, 1963, 3430, 789,
         1917, 2874, 2694, 1103, 2547, 3091, 3165, 1405, 2300, 984,  983,  2923,
         2736, 1658, 798,  596,  1785, 1319, 682,  2513, 1148, 3357, 1716, 3391,
         695,  101,  265,  3307, 3510, 569,  226,  1461, 3317, 1359, 3406, 1962,
         571,  2924, 887,  1108, 3245, 432,  51,   2080, 2297, 861,  1276, 1348,
         1709, 124,  1802, 595,  839,  3392, 2382, 2603, 2,    330,  1228, 2544,
         1429, 158,  3232, 1634, 37,   3064, 3144, 1370, 1500, 619,  3227, 125,
         2907, 964,  2197, 2845, 2171, 3078, 127,  1373, 1958, 1458, 1466, 2027,
         343,  3261, 2820, 2742, 235,  2612, 355,  2392, 45,   2599, 131,  3038,
         2726, 2019, 2847, 1026, 1035, 95,   3087, 1788, 1358, 2245, 2834, 2126,
         1167, 725,  2637, 2850, 2553, 2823, 1973, 1814, 93,   115,  122,  1538,
         2464, 3262, 1482, 991,  902,  242,  1164, 819,  439,  749,  1642, 1840,
         3337, 3379, 2597, 2463, 611,  2121, 620,  1078, 1021, 300,  2454, 1312,
         859,  1803, 1774, 1533, 1112, 2654, 589,  2210, 1603, 279,  2779, 3499,
         2348, 3027, 3467, 2652, 3435, 610,  1192, 938,  1945, 110,  85,   2119,
         372,  1083, 434,  2005, 1730, 374,  1033, 2070, 1715, 3200, 409,  841,
         715,  2232, 732,  1650, 1045, 1940, 505,  2396, 1446, 82,   877,  1912,
         2255, 3442, 2816, 356,  734,  1948, 2253, 625,  3255, 1362, 2049, 306,
         2476, 2611, 2290, 785,  2443, 2174, 2897, 2243, 3199, 993,  2203, 2045,
         440,  2647, 3046, 920,  3474, 1582, 1864, 2270, 1743, 1046, 977,  782,
         3019, 3081, 901,  3300, 491,  1347, 2278, 684,  1268, 107,  879,  1030,
         3151, 1013, 43,   683,  2524, 2272, 1137, 3080, 3509, 1851, 3475, 1843,
         3382, 2441, 1027, 2979, 1039, 3457, 1159, 3172, 1235, 2446, 181,  1928,
         1050, 824,  1156, 2484, 1960, 2856, 613,  2384, 3279, 2282, 1264, 1074,
         3023, 1285, 2615, 436,  1954, 2107, 628,  864,  2360, 2208, 182,  1443,
         2217, 2379, 1844, 3026, 845,  123,  408,  2191, 2219, 1171, 939,  2696,
         2128, 2180, 2945, 997,  2176, 126,  2227, 3354, 2383, 2564, 671,  1765,
         2057, 3329, 1175, 2051, 1805, 852,  358,  1085, 1127, 685,  2388, 476,
         274,  1333, 359,  1378, 2294, 2036, 874,  1287, 2619, 2541, 2556, 1331,
         73,   1195, 337,  1992, 2638, 3454, 2990, 1110, 1850, 141,  2898, 2377,
         134,  526,  477,  3476, 233,  2983, 313,  2098, 1512, 1518, 2961, 2303,
         546,  1059, 2784, 3033, 1883, 2713, 2661, 1789, 38,   1125, 2558, 140,
         1292, 299,  850,  2310, 2386, 853,  2365, 1984, 119,  1203, 2002, 1220,
         3167, 178,  567,  3265, 1367, 1304, 2873, 998,  2242, 2335, 1055, 621,
         800,  2482, 2960, 3335, 2012, 2457, 2711, 889,  3425, 1698, 2344, 3155,
         202,  1798, 1804, 1790, 3502, 2448, 3093, 1952, 3153, 1751, 2084, 2751,
         2079, 236,  2996, 2108, 1619, 2946, 1221, 3365, 102,  1998, 3140, 3086,
         2437, 1341, 2731, 3089, 1118, 206,  1284, 2215, 594,  512,  1281, 482,
         1433, 2764, 3452, 747,  2185, 3283, 1907, 3284, 253,  116,  2039, 1345,
         700,  935,  2186, 2648, 897,  2562, 54,   530,  2673, 2212, 1674, 1182,
         2693, 2037, 3472, 3220, 1365, 2249, 2061, 3121, 3197, 1146, 2881, 2695,
         2381, 2710, 3100, 1889, 1902, 2969, 2239, 1534, 160,  3433, 1707, 386,
         3348, 921,  1918, 1008, 2590, 1552, 1454, 237,  2626, 461,  754,  2700,
         2500, 773,  2863, 3066, 1724, 1972, 827,  390,  692,  3161, 1413, 1309,
         112,  485,  2315, 1097, 3427, 3386, 1498, 49,   1739, 1369, 1900, 2809,
         3479, 2397, 1081, 288,  1410, 626,  2430, 2168, 1289, 3083, 1411, 3071,
         801,  173,  1250, 2709, 948,  909,  2741, 76,   1744, 3484, 3297, 2593,
         452,  1617, 3311, 3298, 1563, 2922, 1923, 1271, 2723, 216,  881,  587,
         3409, 2837, 142,  2796, 1800, 958,  1783, 2385, 1990, 3039, 17,   806,
         462,  1360, 1020, 2412, 351,  312,  3308, 3470, 3273, 1209, 2610, 721,
         1605, 910,  2512, 2194, 2753, 822,  308,  723,  3239, 3390, 361,  3052,
         2351, 1485, 177,  2292, 1286, 70,   2442, 1827, 1521, 2511, 1191, 2928,
         2193, 2636, 1667, 1704, 1441, 3352, 2797, 3358, 1665, 2883, 1258, 1409,
         665,  2247, 255,  2467, 716,  1815, 3126, 1833, 771,  2305, 81,   1967,
         2621, 727,  1558, 933,  1887, 2506, 2959, 1523, 648,  2393, 1970, 560,
         3053, 2746, 2925, 3263, 3478, 449,  1530, 3006, 3310, 837,  2854, 150,
         1009, 2844, 1757, 2409, 3321, 1114, 2538, 1613, 2853, 1438, 3490, 2986,
         943,  946,  1326, 3415, 1113, 1416, 973,  2633, 1799, 2401, 2256, 590,
         56,   2623, 384,  3252, 492,  75,   2024, 865,  533,  1910, 2336, 2010,
         1240, 1106, 642,  1136, 2115, 3325, 1502, 1607, 1559, 2972, 750,  214,
         1725, 1307, 1141, 3192, 326,  670,  3000, 10,   168,  2582, 2083, 583,
         3242, 375,  387,  3434, 3332, 164,  2459, 1414, 1624, 1787, 1557, 191,
         1342, 2999, 1980, 1116, 2438, 2613, 1449, 2503, 2353, 1172, 2586, 295,
         2932, 1562, 478,  768,  930,  1769, 3362, 400,  1253, 459,  2269, 696,
         221,  2461, 2224, 1610, 2275, 2346, 3436, 1547, 2230, 1012, 2135, 1297,
         2974, 2372, 949,  1442, 1122, 1989, 3295, 2507, 310,  2286, 2672, 3408,
         2794, 1504, 581,  2280, 2866, 1432, 679,  1275, 1496, 499,  826,  3138,
         1418, 2768, 3267, 2291, 2770, 410,  2374, 2034, 2973, 2035, 77,   2368,
         3240, 2009, 652,  156,  638,  2453, 3157, 1635, 2408, 1205, 1299, 2967,
         883,  730,  18,   340,  2099, 794,  2431, 1000, 3364, 830,  956,  518,
         1274, 2908, 3059, 2053, 1955, 1231, 2444, 631,  3129, 2992, 180,  397,
         1278, 2730, 223,  26,   2607, 1516, 1249, 1901, 736,  1527, 1210, 1856,
         674,  3072, 1995, 2625, 50,   1920, 2871, 673,  3371, 2509, 1824, 470,
         532,  1659, 404,  3453, 3460, 886,  1874, 59,   1875, 2333, 2915, 1542,
         2855, 405,  2380, 3166, 2155, 2473, 1323, 3211, 490,  3372, 3324, 2154,
         2988, 3139, 2591, 2153, 2561, 2022, 550,  2530, 88,   1554, 2274, 576,
         163,  2017, 3018, 1927, 2868, 3085, 1692, 1968, 3269, 2849, 635,  2645,
         286,  2822, 3393, 285,  316,  741,  2964, 3483, 1058, 525,  2345, 3004,
         13,   3188, 266,  2791, 1151, 186,  3082, 1065, 1826, 740,  2236, 1884,
         3360, 1473, 2732, 961,  3125, 3007, 2231, 1398, 2413, 3266, 1273, 2364,
         2576, 1133, 655,  1388, 757,  965,  2602, 215,  742,  2406, 2719, 2318,
         1778, 1526, 488,  2548, 1189, 1832, 2367, 2814, 1587, 892,  2644, 144,
         1705, 2893, 605,  3368, 706,  2728, 1622, 630,  3206, 1290, 3159, 573,
         1994, 1969, 1329, 513,  1904, 3116, 978,  1903, 908,  1335, 2679, 1145,
         1717, 922,  2082, 2445, 780,  2094, 1229, 1540, 424,  2676, 760,  1168,
         1099, 895,  474,  493,  784,  2074, 1565, 1752, 3205, 1860, 975,  944,
         835,  2031, 3271, 1316, 1356, 1811, 1588, 2487, 2214, 632,  3388, 2306,
         2934, 2642, 3301, 2921, 1868, 2826, 2172, 2566, 402,  643,  2838, 1754,
         1888, 6,    1173, 2267, 1048, 2910, 591,  3312, 562,  507,  2767, 52,
         1817, 2222, 1391, 1893, 3045, 572,  2776, 714,  1691, 637,  2787, 3230,
         2540, 120,  3024, 2938, 1445, 542,  2148, 1197, 332,  812,  2424, 547,
         1939, 603,  3124, 350,  3366, 1436, 1424, 2038, 1321, 699,  1689, 2319,
         2542, 3050, 1812, 2971, 1490, 1095, 2046, 1061, 129,  3247, 1679, 244,
         3210, 2579, 582,  1036, 1042, 1204, 1575, 2737, 3104, 420,  2595, 3235,
         501,  1848, 2143, 3174, 2851, 1270, 3356, 1721, 1867, 1406, 1334, 2640,
         577,  2073, 3176, 904,  1723, 3136, 2273, 2848, 915,  2198, 1602, 666,
         2931, 927,  231,  2939, 371,  2110, 720,  941,  1782, 1618, 1121, 2852,
         2717, 1395, 1687, 204,  3020, 272,  1425, 2366, 913,  1852, 80,   803,
         2618, 3447, 349,  2573, 446,  1340, 667}));

    REQUIRE(S->size() == static_cast<uint64_t>(64'561'751'654'400));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "026",
                          "perm. Held group (order 4'030'387'200)",
                          "[no-valgrind][quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 2059;
    auto             S  = std::make_unique<SchreierSims<N>>();
    using Perm          = std::remove_reference_t<decltype(*S)>::element_type;

    S->add_generator(make<Perm>(
        {0,    374,  708,  1312, 1036, 1151, 1366, 1307, 1571, 1402, 236,  1438,
         363,  590,  288,  1372, 38,   1886, 816,  1953, 1582, 1109, 525,  124,
         24,   1218, 26,   1795, 499,  1574, 802,  31,   1040, 117,  1643, 35,
         1344, 1559, 16,   180,  1183, 979,  1819, 152,  44,   1161, 1500, 332,
         1131, 946,  1599, 51,   985,  1428, 54,   55,   401,  1661, 1351, 272,
         692,  120,  1545, 1221, 1211, 1423, 66,   318,  1536, 1722, 119,  790,
         407,  2024, 146,  2045, 937,  424,  339,  1395, 2056, 568,  1111, 444,
         746,  510,  1123, 2058, 1810, 550,  116,  103,  1009, 902,  632,  1799,
         1308, 1910, 1034, 1706, 693,  101,  922,  91,   104,  571,  2031, 714,
         1447, 1385, 1977, 409,  1220, 1456, 1094, 212,  90,   33,   1922, 70,
         61,   971,  769,  387,  23,   1060, 1057, 480,  1628, 398,  130,  1477,
         1326, 1166, 1974, 1272, 1324, 1295, 490,  291,  1216, 1389, 822,  306,
         552,  1301, 74,   1957, 1401, 149,  1407, 766,  43,   153,  1000, 656,
         638,  157,  158,  943,  677,  232,  162,  1031, 425,  195,  1403, 285,
         219,  311,  657,  1289, 852,  1518, 587,  1159, 1350, 1180, 1041, 923,
         39,   1334, 1735, 399,  1278, 1273, 1498, 488,  908,  1770, 537,  1941,
         595,  1404, 617,  165,  686,  1845, 1924, 939,  1112, 706,  579,  1262,
         749,  789,  775,  843,  1826, 1675, 1900, 1776, 115,  213,  966,  896,
         1673, 1331, 1437, 168,  1139, 1472, 494,  1640, 2008, 995,  982,  1086,
         1766, 800,  565,  231,  161,  853,  1551, 808,  10,   237,  1043, 1240,
         551,  553,  1305, 1069, 1641, 811,  246,  2051, 596,  249,  1349, 1408,
         1593, 1380, 1065, 451,  476,  1915, 1597, 259,  1113, 1225, 262,  457,
         709,  554,  1572, 1846, 447,  1748, 1300, 614,  59,   890,  1125, 1507,
         276,  1674, 1785, 1609, 308,  1635, 607,  815,  1682, 167,  427,  287,
         14,   2043, 1843, 139,  756,  810,  1345, 1794, 1524, 1587, 911,  1660,
         1358, 975,  1771, 1368, 304,  886,  143,  1008, 280,  705,  1194, 169,
         1379, 410,  1269, 757,  1037, 1454, 67,   1501, 702,  859,  731,  1642,
         813,  1752, 1064, 1557, 373,  778,  527,  539,  47,   1657, 2027, 1237,
         470,  1870, 1616, 78,   1023, 1178, 1624, 1014, 1081, 345,  1420, 1050,
         1063, 1699, 1035, 654,  406,  1242, 487,  1450, 2013, 1526, 700,  976,
         2041, 404,  1991, 12,   1634, 1669, 366,  683,  368,  369,  446,  1820,
         2023, 328,  1,    824,  1999, 473,  474,  676,  1306, 1774, 1769, 509,
         1714, 1589, 1595, 123,  1979, 840,  412,  1936, 1554, 393,  1523, 1731,
         695,  1598, 129,  183,  400,  56,   1867, 1981, 361,  1443, 352,  72,
         1662, 111,  313,  669,  390,  1659, 912,  415,  416,  1110, 1101, 534,
         1762, 1441, 628,  1383, 77,   164,  1778, 286,  1357, 1003, 434,  1683,
         989,  485,  430,  888,  1865, 1011, 849,  1916, 1478, 906,  2022, 1013,
         83,   1746, 370,  268,  1534, 1586, 1962, 255,  452,  1996, 1410, 1566,
         1458, 263,  1744, 1029, 1012, 897,  806,  1883, 1955, 1457, 466,  1836,
         1747, 1779, 336,  1837, 1839, 377,  378,  1116, 256,  477,  2020, 1266,
         127,  481,  1878, 1937, 1603, 433,  1824, 354,  187,  1784, 138,  1756,
         1627, 759,  222,  1146, 1189, 667,  1859, 28,   861,  1975, 660,  503,
         566,  1068, 1286, 1834, 1644, 383,  85,   729,  1583, 1448, 515,  514,
         1920, 581,  578,  1569, 1073, 1104, 522,  925,  1653, 22,   881,  330,
         1804, 1907, 1021, 1633, 689,  1758, 419,  872,  1701, 190,  1973, 331,
         592,  1816, 1128, 1088, 622,  1154, 1855, 580,  1892, 1103, 89,   240,
         144,  241,  265,  901,  557,  556,  558,  1963, 972,  629,  1096, 1198,
         1179, 230,  504,  1532, 81,   981,  1600, 105,  1995, 740,  1118, 1321,
         1475, 776,  518,  202,  547,  517,  1754, 1230, 904,  684,  600,  174,
         588,  589,  13,   1188, 540,  1719, 987,  192,  248,  1939, 704,  1209,
         586,  1783, 764,  603,  1315, 1611, 606,  282,  1905, 1788, 2029, 611,
         1540, 1390, 271,  619,  715,  194,  2036, 615,  1740, 621,  544,  623,
         1680, 625,  845,  1235, 422,  561,  1544, 1596, 94,   1020, 1303, 2046,
         1467, 1224, 156,  894,  1364, 1550, 1016, 1274, 1736, 919,  1543, 1718,
         648,  1330, 1253, 1002, 798,  1157, 351,  658,  155,  170,  655,  707,
         502,  661,  732,  809,  1208, 1347, 1044, 497,  1849, 411,  1181, 671,
         672,  1340, 851,  1832, 379,  160,  857,  774,  1797, 1019, 1909, 367,
         585,  685,  196,  1511, 1913, 532,  1369, 1562, 60,   100,  1429, 396,
         1793, 1455, 984,  826,  358,  1869, 320,  1860, 598,  309,  201,  659,
         2,    264,  1099, 1091, 1811, 818,  107,  616,  1153, 1879, 1847, 1292,
         1445, 1509, 1647, 948,  1809, 1415, 726,  1813, 1573, 511,  1434, 322,
         662,  959,  1048, 735,  1451, 1728, 1392, 739,  573,  1531, 1186, 1267,
         1932, 1926, 84,   1129, 1985, 204,  1397, 1055, 1333, 753,  1688, 755,
         292,  315,  1137, 493,  864,  1901, 1030, 1238, 602,  2050, 151,  1474,
         1945, 122,  1512, 1651, 1430, 773,  679,  206,  577,  1652, 329,  1614,
         910,  1329, 782,  1923, 784,  1033, 786,  1906, 788,  205,  71,   791,
         1142, 1084, 1986, 795,  834,  1233, 652,  1751, 229,  1182, 30,   1440,
         924,  1460, 462,  945,  235,  663,  293,  245,  812,  324,  1670, 283,
         18,   1650, 713,  1948, 991,  821,  142,  1414, 375,  1246, 699,  1763,
         828,  829,  973,  920,  1169, 854,  796,  933,  836,  1245, 1895, 1270,
         389,  1664, 1463, 207,  1689, 626,  929,  1960, 1537, 438,  1538, 674,
         172,  233,  833,  855,  1122, 678,  1772, 321,  1254, 500,  1723, 1298,
         760,  1730, 2030, 1519, 998,  1249, 1817, 1823, 535,  873,  1490, 2009,
         882,  934,  1320, 1742, 1409, 526,  876,  1339, 1965, 1265, 305,  1418,
         435,  1155, 273,  1812, 1684, 893,  639,  1726, 215,  461,  935,  1281,
         1177, 555,  93,   903,  584,  1782, 441,  1854, 188,  1231, 780,  298,
         414,  1453, 2052, 1844, 1446, 1127, 1332, 645,  831,  1681, 102,  179,
         804,  523,  1863, 1842, 928,  846,  1570, 1059, 2001, 835,  877,  898,
         1980, 76,   1958, 199,  1257, 1090, 1356, 159,  1929, 807,  49,   2034,
         723,  1287, 950,  1359, 1902, 2047, 1677, 1071, 1607, 1904, 1092, 733,
         1489, 1934, 988,  2016, 964,  1089, 214,  967,  1138, 1919, 1163, 121,
         560,  830,  1781, 301,  359,  977,  1696, 41,   1602, 569,  226,  2042,
         698,  52,   1175, 594,  962,  432,  2033, 820,  992,  1825, 994,  225,
         1360, 1737, 868,  1115, 154,  1581, 651,  429,  1004, 1005, 1061, 1535,
         307,  92,   2014, 437,  460,  443,  343,  1713, 642,  1017, 1051, 681,
         633,  530,  1285, 340,  1251, 2003, 1806, 1406, 1136, 459,  762,  163,
         1821, 785,  98,   350,  4,    316,  1579, 1992, 32,   178,  1695, 238,
         666,  1830, 1342, 1297, 734,  1851, 347,  1018, 1506, 1394, 1528, 751,
         1432, 126,  1687, 931,  125,  1006, 1712, 348,  326,  254,  1066, 1067,
         505,  243,  1172, 955,  1950, 520,  1196, 1283, 1631, 1427, 1575, 1911,
         1250, 344,  1705, 1818, 793,  1119, 227,  1590, 543,  965,  941,  711,
         958,  1093, 114,  1261, 562,  1882, 1120, 710,  2044, 418,  1102, 549,
         521,  1741, 1610, 1263, 1276, 21,   417,  82,   200,  260,  2057, 999,
         475,  1717, 574,  1085, 1098, 1124, 856,  86,   1121, 274,  1464, 917,
         542,  747,  1626, 48,   1367, 1725, 1656, 1876, 1028, 758,  968,  220,
         1140, 1792, 792,  1658, 1343, 1508, 495,  1984, 1439, 1887, 1828, 5,
         1775, 716,  545,  889,  1522, 653,  1555, 175,  1436, 45,   1621, 970,
         1993, 1807, 133,  1709, 1168, 832,  1654, 1697, 1070, 1327, 1502, 986,
         1176, 900,  341,  564,  177,  670,  801,  40,   1734, 1604, 742,  1314,
         591,  496,  1484, 1707, 1768, 2015, 310,  1229, 1074, 1790, 563,  1693,
         1773, 1966, 2048, 1203, 1493, 1205, 1206, 1341, 664,  599,  1558, 64,
         1874, 1354, 1938, 1215, 140,  1217, 25,   1750, 112,  63,   1852, 1223,
         637,  261,  1226, 1594, 2004, 1195, 583,  909,  1858, 797,  1258, 627,
         1236, 335,  763,  1239, 239,  1488, 353,  1243, 1884, 837,  825,  1749,
         1391, 869,  1080, 1024, 1252, 650,  860,  1400, 1517, 940,  1234, 1759,
         1260, 1095, 203,  1107, 1319, 885,  479,  743,  1492, 314,  839,  1944,
         135,  185,  643,  1275, 1108, 2005, 184,  1580, 1363, 899,  1989, 1075,
         1831, 1022, 506,  949,  1336, 171,  1577, 1291, 719,  1399, 1676, 137,
         1412, 1047, 863,  1648, 270,  145,  1462, 634,  2035, 242,  380,  7,
         96,   1309, 1310, 1564, 3,    1803, 1187, 604,  1373, 1850, 1796, 1264,
         878,  575,  1592, 1623, 136,  1724, 132,  1173, 1838, 781,  649,  217,
         918,  752,  181,  1335, 1288, 1703, 1925, 883,  673,  1207, 1046, 1144,
         36,   294,  1721, 665,  1361, 250,  176,  58,   2010, 1903, 1213, 1617,
         942,  428,  300,  951,  996,  1348, 1384, 1280, 640,  1994, 6,    1132,
         303,  690,  1370, 1767, 15,   1316, 1630, 1503, 1396, 1377, 1619, 312,
         253,  1546, 1388, 423,  1362, 109,  1618, 1829, 1382, 141,  613,  1248,
         738,  1416, 1053, 79,   1376, 750,  2006, 1293, 1255, 148,  9,    166,
         193,  1888, 1027, 150,  251,  880,  454,  1411, 1296, 1959, 823,  725,
         1393, 1868, 887,  1608, 346,  1997, 1422, 65,   1636, 1425, 1969, 1077,
         53,   694,  772,  1889, 1056, 1893, 730,  1872, 1160, 218,  11,   1148,
         803,  421,  1442, 405,  1576, 720,  916,  108,  513,  1615, 355,  736,
         2000, 913,  317,  697,  113,  465,  456,  1542, 805,  1466, 1302, 842,
         1126, 1520, 1461, 636,  1612, 1971, 1470, 1764, 221,  1961, 767,  576,
         1708, 131,  440,  1970, 1760, 1481, 1814, 1483, 1190, 2049, 1486, 1496,
         1241, 960,  874,  1491, 1268, 1204, 1632, 1848, 1487, 1857, 186,  2012,
         46,   319,  1174, 1375, 1533, 1505, 1052, 275,  1145, 721,  1983, 687,
         770,  1908, 1873, 1548, 1917, 1256, 173,  867,  1465, 1885, 1156, 394,
         296,  1525, 357,  1527, 1054, 1568, 2055, 741,  567,  1504, 448,  1007,
         68,   848,  850,  1930, 612,  1646, 1459, 646,  630,  62,   1381, 1553,
         1515, 1668, 641,  234,  1552, 1547, 392,  1158, 1556, 327,  1210, 37,
         1710, 1987, 691,  1791, 1311, 1565, 455,  1686, 1529, 519,  930,  8,
         266,  728,  29,   1078, 1444, 1290, 1700, 1038, 1279, 1001, 20,   512,
         1685, 1585, 449,  297,  1588, 385,  1087, 2018, 1322, 252,  1227, 386,
         631,  258,  397,  50,   570,  2032, 980,  484,  1185, 1745, 1606, 956,
         1419, 279,  1106, 605,  1468, 1921, 779,  1449, 338,  1355, 1386, 1378,
         2002, 1162, 2007, 1323, 342,  1732, 1130, 492,  128,  2053, 1374, 1076,
         1494, 531,  364,  281,  1424, 1912, 1733, 1639, 223,  244,  323,  34,
         508,  1645, 1541, 722,  1299, 1757, 817,  771,  777,  524,  1170, 1942,
         1134, 333,  1143, 413,  299,  57,   408,  2038, 841,  1765, 1729, 1667,
         1549, 365,  814,  2017, 1672, 216,  277,  209,  1294, 954,  1761, 1897,
         624,  921,  284,  431,  892,  1584, 1567, 1058, 754,  844,  1890, 1739,
         1952, 1199, 1877, 1042, 978,  1171, 1727, 349,  1578, 536,  2025, 1337,
         1949, 1082, 99,   1191, 1476, 1167, 1560, 1978, 1062, 1015, 384,  1720,
         1943, 1117, 647,  593,  1715, 1346, 69,   862,  1325, 1133, 895,  1698,
         737,  1666, 865,  395,  1625, 1638, 1184, 182,  644,  997,  1743, 1691,
         620,  1105, 879,  1738, 458,  1605, 445,  468,  269,  1247, 1219, 799,
         325,  2028, 582,  1755, 491,  1649, 533,  1259, 1480, 1678, 420,  827,
         1471, 1665, 228,  1371, 1192, 382,  189,  302,  858,  1200, 381,  1152,
         211,  1833, 426,  469,  1780, 974,  905,  601,  489,  278,  1786, 1802,
         609,  2011, 1197, 1563, 1141, 696,  295,  27,   1318, 680,  1798, 95,
         1808, 1951, 1787, 1313, 528,  2039, 1026, 1165, 1800, 724,  88,   712,
         891,  727,  1482, 1815, 541,  870,  1083, 42,   371,  1032, 1954, 871,
         486,  993,  208,  1827, 1150, 1387, 1045, 1284, 675,  1777, 507,  1881,
         467,  471,  1328, 472,  1968, 1998, 927,  290,  915,  197,  267,  718,
         1495, 668,  1317, 1049, 1222, 1862, 907,  546,  1856, 1497, 1232, 498,
         703,  1875, 1853, 926,  1927, 436,  2021, 402,  1417, 701,  337,  1871,
         1435, 1514, 1212, 1861, 1135, 1694, 482,  717,  1898, 1835, 1097, 463,
         1244, 1521, 17,   1149, 1405, 1431, 1690, 1946, 548,  1433, 1894, 838,
         1896, 1679, 1880, 1982, 210,  761,  952,  1353, 957,  608,  787,  529,
         1513, 682,  97,   1079, 1637, 688,  1914, 257,  439,  1516, 1918, 969,
         516,  1613, 118,  783,  198,  1338, 745,  1864, 1928, 944,  1539, 1990,
         744,  1964, 961,  1935, 391,  483,  1214, 597,  1988, 191,  1655, 1716,
         1271, 768,  1891, 1972, 819,  1704, 1072, 1801, 1692, 19,   1822, 464,
         1956, 147,  938,  1413, 847,  1473, 450,  559,  1933, 884,  1201, 1967,
         1840, 1426, 1479, 1469, 1947, 538,  134,  501,  2054, 110,  1711, 388,
         936,  403,  1899, 1510, 1147, 748,  794,  1561, 1940, 1282, 1931, 362,
         1039, 1164, 1365, 572,  453,  1421, 1841, 376,  1452, 932,  1620, 1025,
         1228, 1277, 1398, 1622, 224,  875,  1352, 1789, 1499, 356,  1010, 1193,
         963,  1671, 1591, 2019, 478,  1866, 442,  372,  73,   1702, 2026, 334,
         1753, 610,  866,  106,  1601, 990,  947,  1304, 618,  2037, 1663, 1805,
         2040, 360,  983,  289,  1100, 75,   635,  953,  1202, 1485, 765,  247,
         914,  1629, 1976, 1530, 80,   1114, 87}));

    S->add_generator(make<Perm>(
        {0,    502,  1182, 1390, 34,   2015, 1815, 1118, 395,  1677, 291,  1453,
         1711, 1737, 1432, 681,  1600, 339,  1556, 1001, 651,  532,  230,  1171,
         226,  1055, 1099, 1338, 1553, 191,  1906, 643,  698,  189,  436,  727,
         717,  1368, 697,  1560, 59,   1604, 1176, 1662, 420,  363,  1800, 1164,
         1047, 1921, 231,  2044, 1841, 39,   528,  2019, 1480, 394,  571,  434,
         2040, 1128, 258,  425,  1110, 1498, 1740, 892,  1594, 215,  1392, 468,
         2001, 1651, 929,  106,  855,  849,  1947, 1489, 1961, 1144, 1255, 1201,
         1165, 888,  1088, 1833, 771,  1232, 850,  1419, 721,  6,    1513, 2012,
         1380, 757,  1347, 1409, 265,  861,  846,  1431, 1598, 1321, 562,  1140,
         1003, 1660, 705,  1526, 346,  863,  1454, 599,  1706, 1297, 776,  1373,
         960,  1019, 1583, 1568, 1247, 992,  1388, 1673, 1832, 999,  554,  1658,
         334,  920,  1930, 1750, 1123, 1825, 637,  726,  682,  234,  1139, 2023,
         1713, 984,  983,  1886, 505,  2057, 412,  1446, 177,  2037, 1913, 827,
         485,  379,  331,  1458, 86,   1037, 793,  287,  1331, 1914, 1149, 1849,
         1644, 1326, 1316, 351,  1620, 269,  642,  222,  1127, 458,  736,  796,
         1228, 1922, 2025, 1442, 1354, 1224, 1163, 348,  433,  1138, 1854, 1335,
         56,   197,  712,  466,  401,  604,  972,  1716, 1124, 867,  810,  221,
         277,  1021, 445,  688,  364,  908,  375,  1985, 1211, 1734, 790,  145,
         560,  61,   877,  868,  512,  426,  661,  1772, 5,    1299, 883,  678,
         1801, 1156, 1580, 1945, 1426, 308,  349,  2032, 405,  1680, 327,  1812,
         989,  695,  1960, 210,  1393, 144,  151,  1756, 89,   1805, 782,  1397,
         283,  211,  1602, 1809, 438,  161,  501,  1100, 465,  19,   940,  223,
         887,  1076, 365,  722,  1532, 4,    748,  1507, 1052, 212,  60,   1018,
         889,  1046, 359,  525,  1864, 1314, 173,  309,  55,   510,  64,   2006,
         201,  1362, 1675, 2000, 1533, 97,   2033, 930,  1254, 1757, 116,  792,
         1577, 624,  980,  641,  1523, 170,  966,  655,  2014, 1208, 911,  324,
         1479, 1234, 1693, 1529, 1642, 1020, 1488, 988,  1610, 1850, 96,   1586,
         679,  1381, 129,  662,  1717, 1300, 1767, 977,  864,  648,  1764, 787,
         866,  2052, 1994, 167,  1951, 1878, 88,   1531, 437,  1481, 660,  1719,
         1510, 1048, 1796, 315,  2003, 766,  1436, 1179, 69,   987,  1206, 675,
         328,  1078, 1592, 1785, 1936, 1096, 1011, 928,  917,  1999, 1174, 1685,
         1573, 558,  175,  875,  107,  1962, 1636, 486,  805,  1646, 1273, 1525,
         1285, 1463, 1896, 559,  372,  94,   1274, 1227, 900,  8,    785,  914,
         761,  101,  1167, 480,  1315, 244,  768,  1226, 279,  22,   1325, 169,
         625,  352,  1293, 1996, 517,  853,  1916, 1681, 1933, 1039, 1589, 1474,
         1073, 1383, 946,  602,  707,  1406, 1909, 1345, 730,  452,  1279, 778,
         1495, 1026, 1146, 1890, 1016, 718,  1322, 652,  36,   90,   209,  1195,
         1015, 1821, 1484, 811,  936,  1771, 941,  186,  1923, 1748, 1967, 1198,
         1246, 1085, 1569, 408,  1998, 909,  1664, 1855, 2028, 589,  673,  1090,
         281,  2020, 497,  622,  527,  35,   148,  341,  1983, 842,  1218, 1672,
         820,  1007, 1305, 1931, 1449, 1537, 1749, 1455, 1976, 310,  1402, 2050,
         48,   567,  720,  1363, 1520, 20,   1248, 1486, 1473, 136,  696,  594,
         274,  1548, 52,   1301, 1584, 163,  373,  329,  640,  1708, 239,  1133,
         1013, 1829, 1281, 1000, 1710, 382,  1075, 1501, 692,  1384, 1425, 1289,
         303,  804,  2009, 1089, 1797, 1081, 1942, 1625, 1310, 1839, 1181, 896,
         1408, 700,  38,   511,  389,  2042, 1744, 1212, 1151, 1786, 353,  733,
         390,  1808, 98,   1389, 1576, 1816, 1667, 1822, 350,  1064, 667,  576,
         1527, 1837, 1077, 1278, 1282, 1574, 1170, 1670, 1205, 1521, 1676, 852,
         1956, 1440, 1542, 1776, 1563, 1465, 813,  561,  317,  2027, 66,   1623,
         784,  1445, 474,  1752, 1969, 503,  355,  122,  803,  278,  1701, 153,
         1851, 1056, 99,   1698, 422,  1260, 374,  522,  1334, 1342, 388,  423,
         838,  1250, 266,  1308, 1618, 2043, 1493, 1095, 653,  1319, 1550, 775,
         1443, 612,  268,  540,  548,  417,  1391, 461,  70,   15,   666,  552,
         737,  1491, 955,  1612, 2035, 1312, 10,   1210, 1894, 1230, 879,  854,
         1150, 49,   1369, 1413, 959,  1014, 1596, 2056, 1547, 1251, 1209, 1173,
         1367, 1652, 570,  860,  1337, 1462, 1826, 263,  333,  823,  1038, 683,
         1404, 839,  1333, 523,  658,  1223, 816,  781,  1558, 1692, 947,  257,
         1030, 577,  791,  1120, 956,  93,   740,  81,   1104, 993,  1236, 1044,
         1824, 126,  225,  756,  2022, 919,  62,   1153, 859,  699,  1330, 1633,
         1819, 459,  1059, 290,  1606, 524,  456,  1400, 1736, 157,  165,  1572,
         397,  102,  63,   618,  1485, 755,  1973, 672,  347,  1524, 245,  1060,
         1712, 1974, 1564, 1997, 454,  155,  220,  71,   1554, 716,  1323, 1733,
         788,  1196, 1901, 159,  1769, 489,  2030, 1460, 416,  168,  1810, 406,
         181,  537,  1634, 654,  899,  76,   1448, 1653, 467,  976,  2045, 635,
         969,  1512, 1788, 742,  115,  297,  890,  227,  694,  1605, 1907, 1948,
         1370, 472,  124,  1626, 1640, 1927, 392,  1101, 957,  1917, 1611, 809,
         25,   1694, 1497, 619,  134,  858,  1351, 442,  1478, 1134, 1517, 750,
         569,  738,  1561, 1420, 647,  1866, 1054, 710,  924,  996,  270,  1249,
         1496, 37,   1111, 84,   286,  1306, 1437, 342,  1641, 1804, 1058, 381,
         256,  362,  295,  1929, 817,  1063, 1794, 1022, 546,  325,  1043, 1217,
         1707, 1459, 553,  668,  252,  194,  1519, 638,  693,  1977, 1803, 610,
         418,  851,  304,  1450, 2016, 605,  1723, 1657, 79,   680,  753,  547,
         1659, 137,  188,  1545, 1588, 1467, 1530, 1551, 1650, 739,  1169, 847,
         1424, 1984, 255,  1121, 616,  1728, 1162, 774,  232,  1847, 318,  28,
         514,  183,  958,  1029, 118,  615,  1745, 2047, 476,  103,  3,    1503,
         419,  1259, 1132, 247,  288,  332,  1566, 1720, 1500, 366,  336,  903,
         57,   1755, 492,  1097, 905,  926,  1357, 1375, 184,  1079, 1057, 1966,
         1880, 1143, 1401, 541,  1200, 1830, 1957, 451,  873,  731,  1562, 1107,
         687,  764,  429,  1570, 259,  806,  1193, 253,  1353, 250,  961,  415,
         981,  789,  1028, 1470, 140,  54,   747,  243,  1619, 758,  1207, 1899,
         1702, 1970, 1242, 1240, 1214, 487,  1341, 1700, 1935, 1597, 441,  1950,
         176,  484,  130,  321,  1040, 550,  267,  1069, 199,  1336, 1082, 970,
         166,  1982, 87,   1061, 1438, 2031, 824,  1539, 586,  991,  1122, 1838,
         1350, 1687, 1941, 1688, 837,  587,  1978, 1051, 1853, 322,  818,  1035,
         583,  1202, 447,  1643, 1607, 262,  874,  1621, 282,  1964, 219,  725,
         865,  1760, 224,  1879, 1002, 1106, 1725, 109,  1466, 962,  1091, 1870,
         1565, 927,  398,  1843, 1609, 1313, 1943, 421,  754,  1924, 95,   709,
         1900, 1461, 1405, 393,  462,  1514, 734,  504,  152,  296,  1276, 1682,
         711,  1476, 85,   1256, 202,  1502, 592,  1231, 1070, 708,  1877, 11,
         1823, 1614, 403,  2024, 685,  1304, 198,  822,  591,  565,  614,  233,
         965,  1203, 1535, 719,  200,  1714, 728,  285,  1175, 1991, 114,  1791,
         500,  1421, 520,  963,  1427, 1585, 613,  1065, 1656, 1009, 1516, 2054,
         1593, 1903, 1630, 1355, 1789, 2048, 2018, 131,  190,  67,   407,  542,
         187,  1434, 1509, 893,  1166, 1186, 1852, 1483, 1639, 1902, 273,  80,
         1544, 1067, 1988, 400,  229,  1417, 1187, 367,  1204, 323,  596,  1032,
         314,  767,  1915, 402,  1131, 534,  1066, 1050, 154,  356,  280,  477,
         845,  293,  1045, 1986, 1145, 1552, 922,  23,   1845, 404,  912,  1284,
         1142, 1889, 119,  1414, 536,  43,   2026, 1754, 1487, 656,  765,  645,
         1587, 83,   1012, 455,  1875, 12,   1781, 1290, 1868, 1441, 1177, 1678,
         431,  1245, 1873, 1946, 915,  292,  1993, 471,  1086, 948,  593,  1940,
         1428, 646,  1908, 556,  1158, 358,  1989, 24,   1184, 1631, 112,  690,
         1715, 533,  572,  579,  217,  411,  312,  248,  1339, 1980, 490,  1937,
         783,  1735, 1008, 812,  530,  428,  1239, 598,  836,  913,  629,  1773,
         1113, 1356, 630,  160,  1221, 254,  1365, 1429, 597,  945,  1885, 1189,
         178,  300,  535,  872,  531,  378,  1328, 659,  1559, 1632, 1378, 639,
         1581, 1679, 870,  272,  1818, 1663, 1763, 82,   677,  862,  2039, 1192,
         1787, 1233, 1747, 470,  933,  582,  235,  469,  631,  1792, 147,  1366,
         1739, 1435, 724,  706,  891,  311,  1802, 368,  780,  299,  1456, 228,
         1098, 1590, 1115, 751,  1955, 1778, 482,  319,  1093, 1102, 1859, 240,
         246,  830,  954,  1541, 1528, 1469, 932,  1806, 1258, 515,  918,  1511,
         1920, 800,  330,  669,  825,  1540, 876,  338,  2010, 58,   1836, 275,
         1482, 1398, 1965, 384,  424,  607,  125,  772,  383,  1159, 1385, 1887,
         1746, 294,  901,  513,  1318, 623,  242,  986,  921,  1033, 1575, 1452,
         1844, 674,  544,  1266, 1793, 13,   1237, 519,  1271, 1954, 1697, 31,
         968,  551,  203,  1758, 498,  448,  1215, 1995, 1952, 1858, 1172, 16,
         446,  495,  1332, 313,  1949, 1418, 354,  1148, 1298, 601,  518,  1665,
         2005, 564,  1777, 1883, 1303, 1103, 1348, 934,  743,  807,  1307, 1817,
         545,  1624, 1477, 1092, 1669, 2036, 1023, 139,  938,  2007, 1774, 44,
         937,  1324, 1027, 2008, 1291, 1415, 1690, 1382, 2041, 626,  449,  361,
         127,  1848, 834,  316,  1302, 732,  1761, 1881, 46,   664,  1157, 573,
         41,   357,  74,   1084, 670,  1840, 1119, 994,  1857, 1329, 971,  770,
         1112, 621,  1416, 649,  884,  206,  1168, 1534, 1721, 1696, 1683, 113,
         575,  1464, 1807, 943,  606,  671,  453,  1287, 463,  773,  414,  50,
         2004, 978,  1241, 1691, 302,  1699, 2,    702,  1724, 65,   1925, 305,
         214,  475,  1616, 998,  7,    1374, 42,   345,  1349, 386,  746,  1882,
         1422, 1244, 1835, 1705, 665,  2013, 636,  526,  493,  284,  1703, 1129,
         1346, 2034, 1006, 326,  14,   432,  1444, 30,   439,  1394, 1229, 1617,
         478,  435,  1918, 1990, 953,  1191, 967,  1645, 271,  1360, 1268, 142,
         931,  427,  713,  779,  121,  387,  491,  840,  1674, 1895, 72,   1267,
         1615, 1340, 574,  1654, 1396, 91,   1126, 1072, 1518, 298,  494,  1010,
         1379, 608,  1579, 2021, 45,   982,  627,  808,  1183, 1263, 1031, 885,
         9,    413,  343,  995,  1622, 1738, 1668, 762,  2046, 488,  952,  676,
         1053, 1876, 205,  1295, 164,  581,  814,  794,  430,  1846, 172,  1934,
         644,  1872, 1981, 180,  1904, 174,  105,  997,  1555, 218,  1105, 213,
         869,  1262, 744,  1938, 769,  499,  1975, 1972, 376,  1567, 1433, 1430,
         132,  1504, 1213, 1371, 1125, 1726, 1261, 1549, 185,  1666, 1376, 595,
         1926, 1197, 320,  942,  241,  1471, 1557, 301,  2011, 1155, 1071, 1280,
         895,  529,  1506, 251,  1898, 684,  907,  543,  1468, 1790, 1919, 657,
         516,  538,  1049, 1704, 609,  2017, 620,  1578, 1765, 763,  1412, 2029,
         752,  1689, 973,  110,  714,  1135, 1784, 555,  443,  1130, 949,  1628,
         557,  1582, 410,  444,  1439, 123,  1411, 1387, 549,  906,  799,  729,
         111,  1490, 1343, 1358, 1062, 701,  162,  802,  1036, 1770, 1865, 1709,
         1732, 1968, 1595, 1505, 974,  1813, 715,  704,  2051, 1522, 1447, 385,
         192,  632,  1309, 1762, 1655, 32,   833,  856,  1005, 910,  1296, 1601,
         1108, 832,  1410, 1017, 916,  1094, 1939, 1871, 1613, 237,  1648, 1536,
         27,   236,  617,  1395, 1219, 650,  1649, 1141, 1472, 440,  798,  1891,
         143,  1892, 306,  1782, 749,  1407, 261,  857,  1992, 939,  745,  1959,
         68,   141,  1451, 2038, 26,   117,  1722, 1294, 75,   1932, 1499, 369,
         723,  1779, 578,  1684, 1768, 1423, 521,  1591, 1216, 1147, 1515, 1958,
         563,  2053, 1238, 1344, 2002, 1635, 207,  1252, 878,  1188, 1861, 1161,
         377,  1494, 371,  146,  1317, 1731, 691,  1831, 628,  216,  370,  1508,
         826,  1116, 689,  149,  1361, 1953, 138,  1780, 1199, 307,  1753, 760,
         104,  2049, 1190, 611,  40,   1320, 276,  483,  1194, 135,  1862, 1185,
         1888, 1775, 133,  703,  460,  871,  1270, 1403, 585,  391,  1538, 509,
         47,   1842, 539,  634,  457,  979,  935,  951,  238,  1399, 881,  1235,
         479,  835,  797,  496,  843,  1661, 1257, 821,  844,  150,  1117, 1359,
         925,  600,  1546, 1795, 1364, 1828, 663,  51,   1160, 29,   1034, 1327,
         120,  1860, 1742, 603,  1637, 77,   1627, 1686, 1311, 1783, 1987, 1971,
         1893, 1004, 815,  1863, 1253, 1286, 204,  1152, 53,   1272, 92,   21,
         1911, 1292, 1743, 335,  1178, 1603, 337,  1243, 786,  801,  633,  506,
         1869, 1080, 1671, 1629, 777,  741,  686,  882,  171,  580,  208,  950,
         1867, 1265, 1638, 1729, 179,  841,  886,  1912, 588,  1264, 902,  18,
         1730, 409,  1928, 1543, 848,  1799, 396,  1599, 735,  481,  1288, 73,
         1766, 2055, 1042, 1225, 1277, 1074, 1386, 1352, 182,  450,  1874, 1372,
         1068, 1154, 193,  904,  590,  1751, 894,  1897, 1759, 819,  128,  923,
         795,  1,    990,  975,  1041, 1608, 985,  2058, 1083, 964,  196,  1180,
         1137, 1024, 1910, 1283, 195,  1025, 260,  566,  1727, 1979, 898,  1475,
         1136, 360,  1571, 1275, 264,  340,  508,  1820, 1905, 78,   1963, 1798,
         1834, 1884, 399,  1492, 829,  1109, 944,  100,  897,  249,  1856, 289,
         1647, 1087, 33,   1222, 1811, 156,  1695, 380,  1827, 1220, 828,  17,
         108,  1114, 473,  568,  1944, 158,  1741, 1269, 831,  584,  507,  880,
         344,  1377, 464,  1718, 1457, 1814, 759}));

    REQUIRE(S->size() == static_cast<uint64_t>(4'030'387'200));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "027",
                          "perm. group C2 x C2 x C2 (order 8)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 8;
    using Perm          = libsemigroups::Perm<0, size_t>;
    auto S              = SchreierSims<N, size_t, Perm>();

    S.add_generator(make<Perm>({1, 0, 2, 3, 4, 5, 6, 7}));
    S.add_generator(make<Perm>({0, 1, 3, 2, 4, 5, 6, 7}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 5, 7, 6}));

    REQUIRE(S.size() == 8);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "028",
                          "perm. group C2 x C2 x C2 x C2 (order 16)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 8;
    using Perm          = Perm<>;
    auto S              = SchreierSims<N, Perm::point_type, Perm>();

    S.add_generator(make<Perm>({1, 0, 2, 3, 4, 5, 6, 7}));
    S.add_generator(make<Perm>({0, 1, 3, 2, 4, 5, 6, 7}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 5, 4, 6, 7}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 5, 7, 6}));

    REQUIRE(S.size() == 16);

    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(S.generator(1)));
    REQUIRE(S.contains(S.generator(2)));
    REQUIRE(S.contains(S.generator(3)));
    REQUIRE(S.contains(make<Perm>({1, 0, 2, 3, 5, 4, 6, 7})));
    REQUIRE(S.contains(make<Perm>({0, 1, 3, 2, 5, 4, 7, 6})));
    REQUIRE(!S.contains(make<Perm>({6, 7, 0, 1, 2, 3, 4, 5})));
    REQUIRE(!S.contains(make<Perm>({4, 1, 5, 6, 7, 0, 2, 3})));
    REQUIRE(!S.contains(make<Perm>({6, 4, 7, 0, 1, 2, 3, 5})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "029",
                          "perm. group (S5 x S5) : C2 (order 28'800)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 10;
    using Perm          = Perm<>;
    auto S              = SchreierSims<N, Perm::point_type, Perm>();

    S.add_generator(make<Perm>({0, 9, 2, 3, 4, 5, 6, 7, 8, 1}));
    S.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 0}));

    REQUIRE(S.size() == 28'800);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "030",
                          "perm. group C3 x D8 x A5 (order 1'440)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 12;
    using Perm          = Perm<>;
    auto S              = SchreierSims<N, Perm::point_type, Perm>();

    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 10, 9}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 6, 7, 5, 8, 9, 10, 11}));
    S.add_generator(make<Perm>({0, 1, 3, 4, 2, 5, 6, 7, 8, 9, 10, 11}));
    S.add_generator(make<Perm>({0, 2, 1, 4, 3, 5, 6, 7, 8, 9, 10, 11}));
    S.add_generator(make<Perm>({1, 0, 2, 4, 3, 5, 6, 7, 8, 9, 10, 11}));

    REQUIRE(S.size() == 1'440);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "031",
                          "perm. group GL(4, 3) (order 24'261'120)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 81;
    auto             S  = SchreierSims<N>();
    using Perm          = decltype(S)::element_type;

    S.add_generator(make<Perm>(
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 54, 55, 56, 57, 58, 59, 60,
         61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
         78, 79, 80, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53}));
    S.add_generator(make<Perm>(
        {0,  6,  3,  18, 24, 21, 9,  15, 12, 54, 60, 57, 72, 78, 75, 63, 69,
         66, 27, 33, 30, 45, 51, 48, 36, 42, 39, 55, 61, 58, 73, 79, 76, 64,
         70, 67, 28, 34, 31, 46, 52, 49, 37, 43, 40, 1,  7,  4,  19, 25, 22,
         10, 16, 13, 29, 35, 32, 47, 53, 50, 38, 44, 41, 2,  8,  5,  20, 26,
         23, 11, 17, 14, 56, 62, 59, 74, 80, 77, 65, 71, 68}));

    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(S.generator(1)));

    REQUIRE(S.contains(make<Perm>(
        {0,  65, 46, 7,  69, 53, 5,  67, 48, 21, 59, 40, 19, 54, 38, 26, 61,
         42, 15, 80, 34, 13, 75, 32, 11, 73, 27, 45, 2,  64, 52, 6,  71, 50,
         4,  66, 39, 23, 58, 37, 18, 56, 44, 25, 60, 33, 17, 79, 31, 12, 77,
         29, 10, 72, 63, 47, 1,  70, 51, 8,  68, 49, 3,  57, 41, 22, 55, 36,
         20, 62, 43, 24, 78, 35, 16, 76, 30, 14, 74, 28, 9})));

    REQUIRE(S.contains(make<Perm>(
        {0,  6,  3,  18, 24, 21, 9,  15, 12, 54, 60, 57, 72, 78, 75, 63, 69,
         66, 27, 33, 30, 45, 51, 48, 36, 42, 39, 55, 61, 58, 73, 79, 76, 64,
         70, 67, 28, 34, 31, 46, 52, 49, 37, 43, 40, 1,  7,  4,  19, 25, 22,
         10, 16, 13, 29, 35, 32, 47, 53, 50, 38, 44, 41, 2,  8,  5,  20, 26,
         23, 11, 17, 14, 56, 62, 59, 74, 80, 77, 65, 71, 68})));

    REQUIRE(S.contains(make<Perm>(
        {0,  13, 26, 39, 52, 29, 78, 55, 68, 65, 75, 61, 23, 6,  10, 35, 36,
         49, 46, 32, 42, 58, 71, 72, 16, 20, 3,  5,  15, 19, 44, 45, 31, 74,
         57, 70, 67, 80, 54, 25, 2,  12, 28, 41, 51, 48, 34, 38, 60, 64, 77,
         9,  22, 8,  7,  11, 21, 37, 50, 33, 76, 62, 63, 69, 73, 59, 18, 4,
         17, 30, 43, 47, 53, 27, 40, 56, 66, 79, 14, 24, 1})));

    REQUIRE(!S.contains(make<Perm>(
        {0,  13, 26, 39, 52, 29, 78, 55, 68, 65, 75, 61, 23, 4,  10, 35, 36,
         49, 46, 32, 42, 58, 71, 72, 16, 7,  3,  5,  15, 19, 44, 45, 31, 74,
         57, 70, 67, 80, 54, 25, 1,  12, 28, 41, 51, 48, 34, 38, 60, 64, 77,
         9,  63, 8,  22, 11, 21, 37, 50, 33, 76, 62, 53, 69, 73, 59, 18, 6,
         17, 30, 43, 47, 20, 27, 40, 56, 66, 79, 14, 24, 2})));

    REQUIRE(!S.contains(make<Perm>(
        {0,  29, 55, 35, 61, 4,  58, 3,  32, 20, 79, 24, 76, 21, 50, 18, 47,
         73, 67, 12, 41, 9,  38, 64, 44, 70, 15, 13, 39, 68, 36, 65, 10, 71,
         16, 42, 27, 56, 2,  62, 22, 33, 6,  30, 59, 80, 25, 51, 63, 48, 77,
         45, 74, 19, 26, 52, 78, 49, 75, 23, 72, 7,  46, 40, 66, 14, 53, 11,
         37, 17, 43, 69, 54, 1,  28, 8,  34, 60, 31, 57, 5})));
    REQUIRE(!S.contains(make<Perm>(
        {0,  13, 26, 39, 52, 29, 78, 55, 68, 65, 75, 61, 23, 4,  10, 35, 36,
         49, 46, 32, 42, 58, 71, 72, 16, 63, 3,  5,  15, 19, 44, 45, 31, 74,
         57, 70, 67, 80, 54, 25, 1,  12, 28, 41, 51, 48, 34, 38, 60, 64, 77,
         9,  20, 8,  53, 11, 21, 37, 50, 33, 76, 62, 7,  69, 73, 59, 18, 6,
         17, 30, 43, 47, 22, 27, 40, 56, 66, 79, 14, 24, 2})));
    REQUIRE(S.size() == 24'261'120);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "032",
                          "perm. group PSL(3, 7) (order 1'876'896)",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    constexpr size_t N  = 57;
    using Perm          = Perm<>;
    auto S = std::make_unique<SchreierSims<N, Perm::point_type, Perm>>();

    S->add_generator(
        make<Perm>({1,  8,  15, 22, 29, 36, 43, 50, 12, 19, 26, 33, 40, 47, 54,
                    10, 45, 52, 17, 24, 31, 38, 14, 35, 42, 49, 56, 21, 28, 11,
                    53, 18, 25, 32, 39, 46, 0,  5,  4,  3,  2,  7,  6,  13, 27,
                    34, 41, 48, 55, 20, 9,  37, 44, 51, 16, 23, 30}));
    S->add_generator(
        make<Perm>({0,  1,  3,  4,  5,  6,  7,  2,  8,  14, 9,  10, 11, 12, 13,
                    43, 49, 44, 45, 46, 47, 48, 50, 56, 51, 52, 53, 54, 55, 15,
                    21, 16, 17, 18, 19, 20, 22, 28, 23, 24, 25, 26, 27, 29, 35,
                    30, 31, 32, 33, 34, 36, 42, 37, 38, 39, 40, 41}));

    REQUIRE(S->contains(S->generator(0)));
    REQUIRE(S->contains(S->generator(1)));
    REQUIRE(S->size() == 1'876'896);

    REQUIRE(S->contains(
        make<Perm>({53, 51, 0,  54, 52, 56, 50, 55, 37, 35, 8,  27, 5,  19, 45,
                    1,  39, 25, 32, 46, 11, 18, 16, 43, 41, 4,  12, 31, 28, 44,
                    2,  33, 38, 21, 22, 13, 30, 26, 17, 14, 36, 48, 7,  9,  20,
                    3,  47, 24, 42, 29, 23, 10, 49, 15, 34, 6,  40})));

    REQUIRE(S->contains(
        make<Perm>({12, 40, 47, 19, 26, 54, 33, 1,  2,  25, 15, 55, 45, 42, 30,
                    48, 50, 23, 35, 18, 38, 3,  24, 5,  34, 46, 51, 36, 21, 56,
                    44, 6,  17, 29, 39, 27, 16, 31, 53, 4,  28, 41, 43, 32, 20,
                    49, 22, 7,  37, 52, 8,  14, 10, 9,  13, 0,  11})));

    REQUIRE(S->contains(
        make<Perm>({37, 5,  45, 27, 53, 19, 35, 8,  36, 39, 41, 40, 38, 0,  42,
                    24, 33, 18, 14, 55, 6,  43, 46, 15, 56, 31, 26, 3,  13, 12,
                    52, 22, 20, 49, 7,  32, 34, 28, 47, 50, 11, 2,  17, 21, 48,
                    10, 25, 29, 4,  54, 51, 9,  30, 44, 16, 1,  23})));

    REQUIRE(!S->contains(
        make<Perm>({37, 1,  19, 35, 8,  45, 27, 3,  36, 38, 2,  42, 39, 41, 40,
                    29, 55, 4,  43, 33, 18, 14, 46, 53, 24, 13, 15, 56, 31, 12,
                    49, 5,  32, 52, 22, 20, 34, 11, 0,  17, 28, 47, 50, 21, 26,
                    7,  54, 48, 10, 25, 51, 16, 6,  23, 9,  30, 44})));

    REQUIRE(!S->contains(
        make<Perm>({8,  12, 14, 11, 0,  13, 9,  10, 40, 16, 3,  56, 32, 2,  48,
                    47, 31, 21, 39, 4,  55, 23, 19, 26, 7,  27, 37, 45, 35, 29,
                    6,  34, 44, 52, 42, 18, 54, 28, 46, 1,  20, 30, 38, 33, 41,
                    51, 17, 49, 25, 53, 5,  43, 36, 24, 22, 15, 50})));

    REQUIRE(!S->contains(
        make<Perm>({51, 37, 4,  16, 44, 30, 9,  23, 6,  46, 12, 21, 36, 26, 34,
                    8,  29, 49, 25, 41, 33, 17, 45, 13, 1,  53, 40, 18, 28, 27,
                    15, 32, 5,  38, 14, 47, 24, 56, 52, 54, 2,  55, 50, 19, 31,
                    22, 48, 42, 7,  11, 35, 3,  20, 10, 39, 43, 0})));
  }

  // The following are comment out since the "action" struct requires the
  // compiler intrinsic lz_cnt, which we don't currently want to rely on. This
  // is a POC, and works when lz_cnt is available and actually counts leading
  // 0s.

  //     LIBSEMIGROUPS_TEST_CASE("SchreierSims", "033", "BMat8 cyclic group
  //     order 2",
  //               "[quick][schreier-sims]") {
  //       SchreierSims<8, size_t, BMat8> S;
  //       S.add_generator(BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
  //                              {1, 0, 0, 0, 0, 0, 0, 0},
  //                              {0, 0, 1, 0, 0, 0, 0, 0},
  //                              {0, 0, 0, 1, 0, 0, 0, 0},
  //                              {0, 0, 0, 0, 1, 0, 0, 0},
  //                              {0, 0, 0, 0, 0, 1, 0, 0},
  //                              {0, 0, 0, 0, 0, 0, 1, 0},
  //                              {0, 0, 0, 0, 0, 0, 0, 1}}));
  //       REQUIRE(S.size() == 2);
  //     }
  //
  //     LIBSEMIGROUPS_TEST_CASE("SchreierSims", "034", "BMat8 symmetric group
  //     8",
  //               "[quick][schreier-sims]") {
  //       SchreierSims<8, size_t, BMat8> S;
  //       S.add_generator(BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
  //                              {1, 0, 0, 0, 0, 0, 0, 0},
  //                              {0, 0, 1, 0, 0, 0, 0, 0},
  //                              {0, 0, 0, 1, 0, 0, 0, 0},
  //                              {0, 0, 0, 0, 1, 0, 0, 0},
  //                              {0, 0, 0, 0, 0, 1, 0, 0},
  //                              {0, 0, 0, 0, 0, 0, 1, 0},
  //                              {0, 0, 0, 0, 0, 0, 0, 1}}));
  //       S.add_generator(BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
  //                              {0, 0, 1, 0, 0, 0, 0, 0},
  //                              {0, 0, 0, 1, 0, 0, 0, 0},
  //                              {0, 0, 0, 0, 1, 0, 0, 0},
  //                              {0, 0, 0, 0, 0, 1, 0, 0},
  //                              {0, 0, 0, 0, 0, 0, 1, 0},
  //                              {0, 0, 0, 0, 0, 0, 0, 1},
  //                              {1, 0, 0, 0, 0, 0, 0, 0}}));
  //       REQUIRE(S.size() == 40'320);
  //     }
  //
  //
  //     LIBSEMIGROUPS_TEST_CASE("SchreierSims", "035", "BMat8 dihedral group
  //     D_16",
  //               "[quick][schreier-sims]") {
  //       SchreierSims<8, size_t, BMat8> S;
  //       S.add_generator(BMat8({{1, 0, 0, 0, 0, 0, 0, 0},
  //                              {0, 0, 0, 0, 0, 0, 0, 1},
  //                              {0, 0, 0, 0, 0, 0, 1, 0},
  //                              {0, 0, 0, 0, 0, 1, 0, 0},
  //                              {0, 0, 0, 0, 1, 0, 0, 0},
  //                              {0, 0, 0, 1, 0, 0, 0, 0},
  //                              {0, 0, 1, 0, 0, 0, 0, 0},
  //                              {0, 1, 0, 0, 0, 0, 0, 0}}));
  //       S.add_generator(BMat8({{0, 1, 0, 0, 0, 0, 0, 0},
  //                              {0, 0, 1, 0, 0, 0, 0, 0},
  //                              {0, 0, 0, 1, 0, 0, 0, 0},
  //                              {0, 0, 0, 0, 1, 0, 0, 0},
  //                              {0, 0, 0, 0, 0, 1, 0, 0},
  //                              {0, 0, 0, 0, 0, 0, 1, 0},
  //                              {0, 0, 0, 0, 0, 0, 0, 1},
  //                              {1, 0, 0, 0, 0, 0, 0, 0}}));
  //       REQUIRE(S.size() == 16);
  //     }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "033",
                          "symmetric perm. group (degree 5)",
                          "[quick][schreier-sims]") {
    auto rg    = ReportGuard(false);
    using Perm = Perm<>;
    SchreierSims<5, Perm::point_type, Perm> S;
    S.add_generator(make<Perm>({1, 2, 3, 4, 0}));
    S.add_generator(make<Perm>({1, 0, 2, 3, 4}));
    REQUIRE(S.size() == static_cast<uint64_t>(120));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "034",
                          "alternating perm. group (degree 17)",
                          "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    SchreierSims<17> S;
    S.init();
    using Perm = SchreierSims<17>::element_type;
    REQUIRE(S.size() == 1);
    S.add_generator(
        make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 14}));
    REQUIRE(S.size() == static_cast<uint64_t>(177'843'714'048'000));
    REQUIRE(S.base(0) == 0);
    REQUIRE(S.contains(make<Perm>(
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0})));
    REQUIRE(!S.contains(make<Perm>(
        {1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})));
    REQUIRE(S.contains(make<Perm>(
        {1, 0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})));

    S.init();
    REQUIRE(S.size() == 1);
    S.add_generator(
        make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 14}));

    S.init();
    REQUIRE_THROWS_AS(S.base(0), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.add_base_point(17), LibsemigroupsException);
    S.add_base_point(14);
    S.add_base_point(15);
    S.add_generator(
        make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0}));
    S.add_base_point(1);
    S.add_base_point(3);
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 14}));
    REQUIRE(S.base_size() == 4);
    REQUIRE(S.size() == static_cast<uint64_t>(177'843'714'048'000));
    REQUIRE(S.base(0) == 14);
    REQUIRE(S.base(1) == 15);
    REQUIRE(S.base(2) == 1);
    REQUIRE(S.base(3) == 3);
    REQUIRE(S.base_size() == 15);
    REQUIRE_THROWS_AS(S.add_base_point(1), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.base(15), LibsemigroupsException);
    REQUIRE(S.contains(make<Perm>(
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0})));
    REQUIRE(!S.contains(make<Perm>(
        {1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})));
    REQUIRE(S.contains(make<Perm>(
        {1, 0, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})));
    REQUIRE_THROWS_AS(S.add_base_point(1), LibsemigroupsException);

    S.init();
    S.add_generator(
        make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0}));
    REQUIRE(S.size() == 17);
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 14}));
    REQUIRE(S.size() == static_cast<uint64_t>(177'843'714'048'000));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "035",
                          "exceptions",
                          "[quick][schreier-sims]") {
    auto rg    = ReportGuard(false);
    using Perm = Perm<>;
    SchreierSims<5, Perm::point_type, Perm> S;
    S.add_generator(make<Perm>({1, 2, 3, 4, 0}));
    S.add_generator(make<Perm>({1, 0, 2, 3, 4}));
    REQUIRE(S.size() == static_cast<uint64_t>(120));
    REQUIRE_THROWS_AS(S.add_generator(make<Perm>({0, 1, 2})),
                      LibsemigroupsException);
    REQUIRE_THROWS_AS(S.add_generator(make<Perm>({2, 3, 0, 1, 4, 6})),
                      LibsemigroupsException);

    REQUIRE_THROWS_AS(S.generator(10), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.generator(2), LibsemigroupsException);
    REQUIRE_NOTHROW(S.generator(0));
    REQUIRE_NOTHROW(S.generator(1));

    REQUIRE_THROWS_AS(S.sift(make<Perm>({2, 3, 0, 1, 4, 6})),
                      LibsemigroupsException);
    REQUIRE_THROWS_AS(S.sift(make<Perm>({2, 0, 1})), LibsemigroupsException);
    REQUIRE_NOTHROW(S.sift(S.generator(1)));

    REQUIRE(S.contains(S.generator(1)));
    REQUIRE(S.contains(S.generator(0)));
    REQUIRE(S.contains(make<Perm>({0, 3, 1, 4, 2})));
    REQUIRE_THROWS_AS(make<Perm>({2, 3, 0, 1, 4, 6}), LibsemigroupsException);
    REQUIRE(!S.contains(make<Perm>({2, 0, 1})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "036",
                          "exceptions",
                          "[quick][schreier-sims]") {
    auto rg    = ReportGuard(false);
    using Perm = Perm<>;
    SchreierSims<5, Perm::point_type, Perm> S;
    S.add_generator(make<Perm>({1, 2, 3, 4, 0}));
    S.add_generator(make<Perm>({1, 0, 2, 3, 4}));
    REQUIRE_THROWS_AS(S.add_base_point(0), LibsemigroupsException);
    for (size_t i = 1; i < 5; ++i) {
      REQUIRE_NOTHROW(S.add_base_point(i));
    }
    REQUIRE_THROWS_AS(S.add_base_point(5), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.add_base_point(100), LibsemigroupsException);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "037",
                          "trivial group",
                          "[quick][schreier-sims]") {
    auto            rg = ReportGuard(false);
    SchreierSims<5> S;
    using Perm = typename decltype(S)::element_type;
    S.add_generator(make<Perm>({1, 2, 3, 4, 0}));
    S.add_generator(make<Perm>({1, 0, 2, 3, 4}));
    REQUIRE_THROWS_AS(S.add_base_point(0), LibsemigroupsException);
    for (size_t i = 1; i < 5; ++i) {
      REQUIRE_NOTHROW(S.add_base_point(i));
    }
    REQUIRE_THROWS_AS(S.add_base_point(6), LibsemigroupsException);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "038",
                          "A17 bug",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    SchreierSims<17> S, T;
    using Perm = SchreierSims<17>::element_type;
    S.add_generator(
        make<Perm>({0, 1, 2, 4, 16, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 3}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 4}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 6, 16, 7, 8, 9, 10, 11, 12, 13, 14, 15, 5}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 7, 16, 8, 9, 10, 11, 12, 13, 14, 15, 6}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 8, 16, 9, 10, 11, 12, 13, 14, 15, 7}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 9, 16, 10, 11, 12, 13, 14, 15, 8}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 16, 11, 12, 13, 14, 15, 9}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 16, 12, 13, 14, 15, 10}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 13, 14, 15, 11}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 16, 14, 15, 12}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 15, 13}));
    S.add_generator(
        make<Perm>({0, 1, 14, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 15, 2}));
    S.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 14, 15}));
    S.add_generator(
        make<Perm>({0, 15, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 1}));
    S.add_generator(
        make<Perm>({15, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 0}));

    T.add_generator(
        make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0}));
    T.add_generator(
        make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 14}));

    for (size_t i = 0; i < S.number_of_generators(); ++i) {
      REQUIRE(T.contains(S.generator(i)));
    }
    for (size_t i = 0; i < T.number_of_generators(); ++i) {
      REQUIRE(S.contains(T.generator(i)));
    }

    REQUIRE(T.size() == 177'843'714'048'000);
    REQUIRE(S.size() == 177'843'714'048'000);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "039",
                          "orbit_lookup test",
                          "[quick][schreier-sims]") {
    auto             rg = ReportGuard(false);
    SchreierSims<64> S, T;
    using Perm = typename decltype(S)::element_type;
    S.add_base_point(0);
    S.add_base_point(1);
    S.add_base_point(2);
    S.add_generator(make<Perm>(
        {0,  2,  59, 57, 16, 18, 43, 41, 36, 38, 31, 29, 52, 54, 15, 13,
         8,  10, 51, 49, 24, 26, 35, 33, 44, 46, 23, 21, 60, 62, 7,  5,
         32, 34, 27, 25, 48, 50, 11, 9,  4,  6,  63, 61, 20, 22, 47, 45,
         40, 42, 19, 17, 56, 58, 3,  1,  12, 14, 55, 53, 28, 30, 39, 37}));
    S.add_generator(make<Perm>(
        {0,  40, 51, 27, 1,  41, 50, 26, 2,  42, 49, 25, 3,  43, 48, 24,
         4,  44, 55, 31, 5,  45, 54, 30, 6,  46, 53, 29, 7,  47, 52, 28,
         16, 56, 35, 11, 17, 57, 34, 10, 18, 58, 33, 9,  19, 59, 32, 8,
         20, 60, 39, 15, 21, 61, 38, 14, 22, 62, 37, 13, 23, 63, 36, 12}));
    S.add_generator(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62}));
    S.run();

    detail::Array2<bool, 64> orbits_lookup_test;
    orbits_lookup_test[0]
        = {true, true, true, true, true, true, true, true, true, true, true,
           true, true, true, true, true, true, true, true, true, true, true,
           true, true, true, true, true, true, true, true, true, true, true,
           true, true, true, true, true, true, true, true, true, true, true,
           true, true, true, true, true, true, true, true, true, true, true,
           true, true, true, true, true, true, true, true, true};
    orbits_lookup_test[1]
        = {false, true, true, true, true, true, true, true, true, true, true,
           true,  true, true, true, true, true, true, true, true, true, true,
           true,  true, true, true, true, true, true, true, true, true, true,
           true,  true, true, true, true, true, true, true, true, true, true,
           true,  true, true, true, true, true, true, true, true, true, true,
           true,  true, true, true, true, true, true, true, true};
    orbits_lookup_test[2]
        = {false, false, true,  false, false, false, false, false, false, false,
           false, true,  false, false, false, false, false, false, false, false,
           false, false, false, false, false, false, false, false, false, false,
           false, false, true,  false, false, false, false, false, false, false,
           false, false, false, false, true,  false, false, false, false, false,
           false, false, false, false, false, false, true,  false, false, false,
           true,  false, false, false};
    REQUIRE_THROWS_AS(S.orbit_lookup(3, 0), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.orbit_lookup(4, 0), LibsemigroupsException);

    // GCC issues some warnings for the code below, but it's ok.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    REQUIRE_THROWS_AS(S.orbit_lookup(0, 64), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.orbit_lookup(0, 65), LibsemigroupsException);
#pragma GCC diagnostic pop
    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 64; ++j) {
        REQUIRE(S.orbit_lookup(i, j) == orbits_lookup_test[i][j]);
      }
    }

    T.add_base_point(0);
    T.add_base_point(1);
    T.add_base_point(4);
    T.add_generator(make<Perm>(
        {0,  30, 5,  27, 41, 55, 44, 50, 2,  28, 7,  25, 43, 53, 46, 48,
         4,  26, 1,  31, 45, 51, 40, 54, 6,  24, 3,  29, 47, 49, 42, 52,
         16, 14, 21, 11, 57, 39, 60, 34, 18, 12, 23, 9,  59, 37, 62, 32,
         20, 10, 17, 15, 61, 35, 56, 38, 22, 8,  19, 13, 63, 33, 58, 36}));
    T.add_generator(make<Perm>(
        {0,  10, 47, 37, 55, 61, 24, 18, 1,  11, 46, 36, 54, 60, 25, 19,
         32, 42, 15, 5,  23, 29, 56, 50, 33, 43, 14, 4,  22, 28, 57, 51,
         8,  2,  39, 45, 63, 53, 16, 26, 9,  3,  38, 44, 62, 52, 17, 27,
         40, 34, 7,  13, 31, 21, 48, 58, 41, 35, 6,  12, 30, 20, 49, 59}));
    T.add_generator(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62}));
    T.run();

    orbits_lookup_test[2]
        = {false, false, false, false, true,  true,  true,  true,  true,  true,
           true,  true,  false, false, false, false, false, false, false, false,
           false, false, false, false, false, false, false, false, false, false,
           false, false, false, false, false, false, false, false, false, false,
           false, false, false, false, false, false, false, false, false, false,
           false, false, false, false, false, false, false, false, false, false,
           false, false, false, false};
    REQUIRE_THROWS_AS(T.orbit_lookup(3, 0), LibsemigroupsException);
    REQUIRE_THROWS_AS(T.orbit_lookup(4, 0), LibsemigroupsException);
    // GCC issues some warnings for the code below, but it's ok.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    REQUIRE_THROWS_AS(T.orbit_lookup(0, 64), LibsemigroupsException);
    REQUIRE_THROWS_AS(T.orbit_lookup(0, 65), LibsemigroupsException);
#pragma GCC diagnostic pop
    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 64; ++j) {
        REQUIRE(T.orbit_lookup(i, j) == orbits_lookup_test[i][j]);
      }
    }
  }

  LIBSEMIGROUPS_TEST_CASE(
      "SchreierSims",
      "040",
      "transversal_element and inverse_transversal_element test",
      "[quick][schreier-sims][no-valgrind]") {
    auto             rg = ReportGuard(false);
    SchreierSims<64> S;
    using Perm   = typename decltype(S)::element_type;
    using Action = typename decltype(S)::Action;

    S.add_base_point(0);
    S.add_base_point(1);
    S.add_base_point(2);
    S.add_generator(make<Perm>(
        {0,  2,  59, 57, 16, 18, 43, 41, 36, 38, 31, 29, 52, 54, 15, 13,
         8,  10, 51, 49, 24, 26, 35, 33, 44, 46, 23, 21, 60, 62, 7,  5,
         32, 34, 27, 25, 48, 50, 11, 9,  4,  6,  63, 61, 20, 22, 47, 45,
         40, 42, 19, 17, 56, 58, 3,  1,  12, 14, 55, 53, 28, 30, 39, 37}));
    S.add_generator(make<Perm>(
        {0,  40, 51, 27, 1,  41, 50, 26, 2,  42, 49, 25, 3,  43, 48, 24,
         4,  44, 55, 31, 5,  45, 54, 30, 6,  46, 53, 29, 7,  47, 52, 28,
         16, 56, 35, 11, 17, 57, 34, 10, 18, 58, 33, 9,  19, 59, 32, 8,
         20, 60, 39, 15, 21, 61, 38, 14, 22, 62, 37, 13, 23, 63, 36, 12}));
    S.add_generator(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62}));
    S.run();

    REQUIRE_THROWS_AS(S.transversal_element(3, 0), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.transversal_element(4, 0), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.transversal_element(0, 64), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.transversal_element(0, 65), LibsemigroupsException);
    REQUIRE_THROWS_AS(S.inverse_transversal_element(3, 0),
                      LibsemigroupsException);
    REQUIRE_THROWS_AS(S.inverse_transversal_element(4, 0),
                      LibsemigroupsException);
    REQUIRE_THROWS_AS(S.inverse_transversal_element(0, 64),
                      LibsemigroupsException);
    REQUIRE_THROWS_AS(S.inverse_transversal_element(0, 65),
                      LibsemigroupsException);
    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 64; ++j) {
        if (S.orbit_lookup(i, j)) {
          REQUIRE(Action()(S.base(i), S.transversal_element(i, j)) == j);
          REQUIRE(Action()(j, S.inverse_transversal_element(i, j))
                  == S.base(i));
        } else {
          REQUIRE_THROWS_AS(S.transversal_element(i, j),
                            LibsemigroupsException);
          REQUIRE_THROWS_AS(S.inverse_transversal_element(i, j),
                            LibsemigroupsException);
        }
      }
    }
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "041",
                          "trivial perm. group intersection (degree 1)",
                          "[quick][schreier-sims][intersection]") {
    auto            rg = ReportGuard(false);
    SchreierSims<1> S, T, U;
    using Perm = decltype(S)::element_type;
    S.add_generator(make<Perm>({0}));
    T.add_generator(make<Perm>({0}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 1);
    REQUIRE(U.contains(make<Perm>({0})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "042",
                          "trivial perm. group intersection (degree 2)",
                          "[quick][schreier-sims][intersection]") {
    auto            rg = ReportGuard(false);
    SchreierSims<2> S, T, U;
    using Perm = SchreierSims<2>::element_type;
    S.add_generator(make<Perm>({0, 1}));
    T.add_generator(make<Perm>({1, 0}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 1);
    REQUIRE(U.sift(make<Perm>({1, 0})) == make<Perm>({1, 0}));
    REQUIRE(!U.contains(make<Perm>({1, 0})));
    REQUIRE(U.contains(make<Perm>({0, 1})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "043",
                          "cyclic group intersection (degree 13)",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<13> S, T, U;
    using Perm = SchreierSims<13>::element_type;
    // Adapted from:
    // https://math.stackexchange.com/q/4093199/152276
    // (0, 1, 2, 3, 4)(5, 9)(6, 10)(7, 11)(8, 12)
    S.add_generator(make<Perm>({1, 2, 3, 4, 0, 9, 10, 11, 12, 5, 6, 7, 8}));
    // (1, 4)(2, 3)(5, 6, 7, 8, 9, 10, 11, 12)
    T.add_generator(make<Perm>({0, 4, 3, 2, 1, 6, 7, 8, 9, 10, 11, 12, 5}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 2);
    REQUIRE(
        !U.contains(make<Perm>({1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12})));
    REQUIRE(U.contains(make<Perm>({0, 1, 2, 3, 4, 9, 10, 11, 12, 5, 6, 7, 8})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "044",
                          "D10 and Z5 intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<12> S, T, U;
    using Perm = SchreierSims<12>::element_type;
    // (0, 2, 4, 6, 8)(1, 3, 5, 7, 9)(10, 11)
    S.add_generator(make<Perm>({2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 11, 10}));
    // (0, 2, 4, 6, 8)(1, 3, 5, 7, 9)
    // (0, 1)(2, 9)(3, 8)(4, 7)(5, 6)
    T.add_generator(make<Perm>({2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 10, 11}));
    T.add_generator(make<Perm>({1, 0, 9, 8, 7, 6, 5, 4, 3, 2, 10, 11}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 5);
    // (0, 6, 2, 8, 4)(1, 7, 3, 9, 5)
    REQUIRE(U.contains(make<Perm>({6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 10, 11})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "045",
                          "D8 and Q8 intersection",
                          "[quick][schreier-sims][intersection]") {
    auto            rg = ReportGuard(false);
    SchreierSims<8> S, T, U;
    using Perm = SchreierSims<8>::element_type;
    S.add_generator(make<Perm>({1, 3, 7, 5, 2, 0, 4, 6}));
    S.add_generator(make<Perm>({2, 4, 3, 6, 5, 7, 0, 1}));
    S.add_generator(make<Perm>({3, 5, 6, 0, 7, 1, 2, 4}));
    T.add_generator(make<Perm>({1, 0, 7, 5, 6, 3, 4, 2}));
    T.add_generator(make<Perm>({2, 4, 3, 6, 5, 7, 0, 1}));
    T.add_generator(make<Perm>({3, 5, 6, 0, 7, 1, 2, 4}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 4);
    REQUIRE(U.contains(make<Perm>({2, 4, 3, 6, 5, 7, 0, 1})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "046",
                          "primitive on 8 points intersection",
                          "[quick][schreier-sims][intersection]") {
    auto            rg = ReportGuard(false);
    SchreierSims<8> S, T, U;
    using Perm = SchreierSims<8>::element_type;
    S.add_generator(make<Perm>({0, 2, 3, 4, 1, 5, 6, 7}));
    S.add_generator(make<Perm>({1, 2, 4, 0, 3, 5, 6, 7}));
    T.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 0, 7}));
    T.add_generator(make<Perm>({0, 1, 2, 3, 4, 6, 7, 5}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 10);
    REQUIRE(U.contains(make<Perm>({0, 3, 4, 1, 2, 5, 6, 7})));
    REQUIRE(U.contains(make<Perm>({1, 2, 4, 0, 3, 5, 6, 7})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "047",
                          "primitive on 8 points intersection (swap order)",
                          "[quick][schreier-sims][intersection]") {
    auto            rg = ReportGuard(false);
    SchreierSims<8> S, T, U;
    using Perm = SchreierSims<8>::element_type;
    S.add_generator(make<Perm>({0, 2, 3, 4, 1, 5, 6, 7}));
    S.add_generator(make<Perm>({1, 2, 4, 0, 3, 5, 6, 7}));
    T.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 0, 7}));
    T.add_generator(make<Perm>({0, 1, 2, 3, 4, 6, 7, 5}));
    schreier_sims::intersection(U, T, S);
    REQUIRE(U.size() == 10);
    REQUIRE(U.contains(make<Perm>({0, 3, 4, 1, 2, 5, 6, 7})));
    REQUIRE(U.contains(make<Perm>({1, 2, 4, 0, 3, 5, 6, 7})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "048",
                          "A13 and PGL(2, 11) intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<13> S, T, U;
    using Perm = SchreierSims<13>::element_type;
    S.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 10}));
    T.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 11, 12}));
    T.add_generator(make<Perm>({0, 2, 4, 6, 8, 10, 1, 3, 5, 7, 9, 11, 12}));
    T.add_generator(make<Perm>({11, 10, 5, 7, 8, 2, 9, 3, 4, 6, 1, 0, 12}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 660);
    REQUIRE(U.contains(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 11, 12})));
    REQUIRE(U.contains(make<Perm>({0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11, 12})));
    REQUIRE(U.contains(make<Perm>({11, 10, 5, 7, 8, 2, 9, 3, 4, 6, 1, 0, 12})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "049",
                          "A13 and PGL(2, 11) intersection (swap order)",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<13> S, T, U;
    using Perm = SchreierSims<13>::element_type;
    S.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0}));
    S.add_generator(make<Perm>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 10}));
    T.add_generator(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 11, 12}));
    T.add_generator(make<Perm>({0, 2, 4, 6, 8, 10, 1, 3, 5, 7, 9, 11, 12}));
    T.add_generator(make<Perm>({11, 10, 5, 7, 8, 2, 9, 3, 4, 6, 1, 0, 12}));
    schreier_sims::intersection(U, T, S);
    REQUIRE(U.size() == 660);
    REQUIRE(U.contains(make<Perm>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 11, 12})));
    REQUIRE(U.contains(make<Perm>({0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11, 12})));
    REQUIRE(U.contains(make<Perm>({11, 10, 5, 7, 8, 2, 9, 3, 4, 6, 1, 0, 12})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "050",
                          "S17 and A39 intersection",
                          "[quick][schreier-sims][no-valgrind][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<50> S, T, U;
    using Perm = SchreierSims<50>::element_type;
    S.add_generator(make<Perm>(
        {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 0,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49}));
    S.add_generator(make<Perm>(
        {1,  0,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49}));
    T.add_generator(make<Perm>(
        {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
         35, 36, 37, 38, 0,  39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49}));
    T.add_generator(make<Perm>(
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 37, 38, 36, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 177'843'714'048'000);
    REQUIRE(U.contains(make<Perm>(
        {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 0,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49})));
    REQUIRE(U.contains(make<Perm>(
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 15, 16, 14,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "051",
                          "A50 and PGL(2, 49) intersection",
                          "[standard][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<50> S, T, U;
    using Perm = SchreierSims<50>::element_type;
    S.add_generator(make<Perm>(
        {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
         18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
         35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 0,  49}));
    S.add_generator(make<Perm>(
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 48, 49, 47}));
    T.add_generator(make<Perm>(
        {1,  2,  3,  4,  5,  6,  0,  8,  9,  10, 11, 12, 13, 7,  15, 16, 17,
         18, 19, 20, 14, 22, 23, 24, 25, 26, 27, 21, 29, 30, 31, 32, 33, 34,
         28, 36, 37, 38, 39, 40, 41, 35, 43, 44, 45, 46, 47, 48, 42, 49}));
    T.add_generator(make<Perm>(
        {0,  2,  4,  6,  1,  3,  5,  14, 16, 18, 20, 15, 17, 19, 28, 30, 32,
         34, 29, 31, 33, 42, 44, 46, 48, 43, 45, 47, 7,  9,  11, 13, 8,  10,
         12, 21, 23, 25, 27, 22, 24, 26, 35, 37, 39, 41, 36, 38, 40, 49}));
    T.add_generator(make<Perm>(
        {0,  17, 34, 44, 12, 22, 39, 26, 36, 4,  14, 31, 48, 9,  45, 13, 23,
         40, 1,  18, 28, 15, 32, 42, 10, 27, 37, 5,  41, 2,  19, 29, 46, 7,
         24, 11, 21, 38, 6,  16, 33, 43, 30, 47, 8,  25, 35, 3,  20, 49}));
    T.add_generator(make<Perm>(
        {49, 6,  3,  2,  5,  4,  1,  7,  31, 22, 41, 36, 27, 32, 28, 46, 19,
         38, 39, 16, 45, 35, 9,  30, 43, 48, 33, 12, 14, 44, 23, 8,  13, 26,
         47, 21, 11, 40, 17, 18, 37, 10, 42, 24, 29, 20, 15, 34, 25, 0}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 58'800);
    REQUIRE(U.contains(make<Perm>(
        {1,  2,  3,  4,  5,  6,  0,  8,  9,  10, 11, 12, 13, 7,  15, 16, 17,
         18, 19, 20, 14, 22, 23, 24, 25, 26, 27, 21, 29, 30, 31, 32, 33, 34,
         28, 36, 37, 38, 39, 40, 41, 35, 43, 44, 45, 46, 47, 48, 42, 49})));
    REQUIRE(U.contains(make<Perm>(
        {0,  2,  4,  6,  1,  3,  5,  14, 16, 18, 20, 15, 17, 19, 28, 30, 32,
         34, 29, 31, 33, 42, 44, 46, 48, 43, 45, 47, 7,  9,  11, 13, 8,  10,
         12, 21, 23, 25, 27, 22, 24, 26, 35, 37, 39, 41, 36, 38, 40, 49})));
    REQUIRE(U.contains(make<Perm>(
        {0,  40, 24, 8,  48, 32, 16, 37, 21, 12, 45, 29, 20, 4,  25, 9,  42,
         33, 17, 1,  41, 13, 46, 30, 14, 5,  38, 22, 43, 34, 18, 2,  35, 26,
         10, 31, 15, 6,  39, 23, 7,  47, 19, 3,  36, 27, 11, 44, 28, 49})));
    REQUIRE(U.contains(make<Perm>(
        {49, 6,  3,  2,  5,  4,  1,  7,  31, 22, 41, 36, 27, 32, 28, 46, 19,
         38, 39, 16, 45, 35, 9,  30, 43, 48, 33, 12, 14, 44, 23, 8,  13, 26,
         47, 21, 11, 40, 17, 18, 37, 10, 42, 24, 29, 20, 15, 34, 25, 0})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "052",
                          "3^3:13 and 3^3.2.A(4) intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<27> S, T, U;
    using Perm = SchreierSims<27>::element_type;
    S.add_generator(
        make<Perm>({0,  17, 22, 1,  15, 23, 2,  16, 21, 3,  11, 25, 4, 9,
                    26, 5,  10, 24, 6,  14, 19, 7,  12, 20, 8,  13, 18}));
    S.add_generator(
        make<Perm>({1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14,
                    12, 16, 17, 15, 19, 20, 18, 22, 23, 21, 25, 26, 24}));
    T.add_generator(
        make<Perm>({0,  9, 18, 1,  10, 19, 2,  11, 20, 3,  12, 21, 4, 13,
                    22, 5, 14, 23, 6,  15, 24, 7,  16, 25, 8,  17, 26}));
    T.add_generator(
        make<Perm>({0,  2,  1,  6,  8,  7,  3,  5,  4,  9,  11, 10, 15, 17,
                    16, 12, 14, 13, 18, 20, 19, 24, 26, 25, 21, 23, 22}));
    T.add_generator(
        make<Perm>({0,  1,  2,  6,  7,  8,  3,  4,  5,  9,  10, 11, 15, 16,
                    17, 12, 13, 14, 18, 19, 20, 24, 25, 26, 21, 22, 23}));
    T.add_generator(
        make<Perm>({1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14,
                    12, 16, 17, 15, 19, 20, 18, 22, 23, 21, 25, 26, 24}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 27);
    REQUIRE(U.contains(
        make<Perm>({1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14,
                    12, 16, 17, 15, 19, 20, 18, 22, 23, 21, 25, 26, 24})));
    REQUIRE(U.contains(
        make<Perm>({3,  4, 5,  6,  7,  8,  0,  1,  2,  12, 13, 14, 15, 16,
                    17, 9, 10, 11, 21, 22, 23, 24, 25, 26, 18, 19, 20})));
    REQUIRE(U.contains(
        make<Perm>({9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                    23, 24, 25, 26, 0,  1,  2,  3,  4,  5,  6,  7,  8})));
  }

  LIBSEMIGROUPS_TEST_CASE(
      "SchreierSims",
      "053",
      "PGamma(2, 9) wreath Sym(2) and Alt(6)^2.D_8 intersection",
      "[quick][schreier-sims][no-valgrind][intersection]") {
    auto              rg = ReportGuard(false);
    SchreierSims<100> S, T, U;
    using Perm = SchreierSims<100>::element_type;
    S.add_generator(make<Perm>(
        {13, 17, 16, 19, 18, 12, 14, 10, 15, 11, 83, 87, 86, 89, 88, 82, 84,
         80, 85, 81, 63, 67, 66, 69, 68, 62, 64, 60, 65, 61, 53, 57, 56, 59,
         58, 52, 54, 50, 55, 51, 33, 37, 36, 39, 38, 32, 34, 30, 35, 31, 3,
         7,  6,  9,  8,  2,  4,  0,  5,  1,  43, 47, 46, 49, 48, 42, 44, 40,
         45, 41, 93, 97, 96, 99, 98, 92, 94, 90, 95, 91, 23, 27, 26, 29, 28,
         22, 24, 20, 25, 21, 73, 77, 76, 79, 78, 72, 74, 70, 75, 71}));
    S.add_generator(make<Perm>(
        {65, 25, 5,  55, 35, 95, 15, 85, 75, 45, 63, 23, 3,  53, 33, 93, 13,
         83, 73, 43, 64, 24, 4,  54, 34, 94, 14, 84, 74, 44, 68, 28, 8,  58,
         38, 98, 18, 88, 78, 48, 61, 21, 1,  51, 31, 91, 11, 81, 71, 41, 60,
         20, 0,  50, 30, 90, 10, 80, 70, 40, 69, 29, 9,  59, 39, 99, 19, 89,
         79, 49, 62, 22, 2,  52, 32, 92, 12, 82, 72, 42, 66, 26, 6,  56, 36,
         96, 16, 86, 76, 46, 67, 27, 7,  57, 37, 97, 17, 87, 77, 47}));
    S.add_generator(make<Perm>(
        {18, 68, 58, 98, 38, 28, 78, 88, 48, 8,  19, 69, 59, 99, 39, 29, 79,
         89, 49, 9,  16, 66, 56, 96, 36, 26, 76, 86, 46, 6,  17, 67, 57, 97,
         37, 27, 77, 87, 47, 7,  13, 63, 53, 93, 33, 23, 73, 83, 43, 3,  11,
         61, 51, 91, 31, 21, 71, 81, 41, 1,  15, 65, 55, 95, 35, 25, 75, 85,
         45, 5,  10, 60, 50, 90, 30, 20, 70, 80, 40, 0,  14, 64, 54, 94, 34,
         24, 74, 84, 44, 4,  12, 62, 52, 92, 32, 22, 72, 82, 42, 2}));
    T.add_generator(make<Perm>(
        {78, 76, 77, 79, 75, 74, 71, 72, 70, 73, 38, 36, 37, 39, 35, 34, 31,
         32, 30, 33, 28, 26, 27, 29, 25, 24, 21, 22, 20, 23, 88, 86, 87, 89,
         85, 84, 81, 82, 80, 83, 8,  6,  7,  9,  5,  4,  1,  2,  0,  3,  48,
         46, 47, 49, 45, 44, 41, 42, 40, 43, 68, 66, 67, 69, 65, 64, 61, 62,
         60, 63, 58, 56, 57, 59, 55, 54, 51, 52, 50, 53, 98, 96, 97, 99, 95,
         94, 91, 92, 90, 93, 18, 16, 17, 19, 15, 14, 11, 12, 10, 13}));
    T.add_generator(make<Perm>(
        {24, 74, 44, 64, 94, 4,  84, 34, 14, 54, 23, 73, 43, 63, 93, 3,  83,
         33, 13, 53, 28, 78, 48, 68, 98, 8,  88, 38, 18, 58, 29, 79, 49, 69,
         99, 9,  89, 39, 19, 59, 25, 75, 45, 65, 95, 5,  85, 35, 15, 55, 22,
         72, 42, 62, 92, 2,  82, 32, 12, 52, 20, 70, 40, 60, 90, 0,  80, 30,
         10, 50, 27, 77, 47, 67, 97, 7,  87, 37, 17, 57, 26, 76, 46, 66, 96,
         6,  86, 36, 16, 56, 21, 71, 41, 61, 91, 1,  81, 31, 11, 51}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 2);
    REQUIRE(U.contains(make<Perm>(
        {11, 1,  91, 81, 71, 61, 51, 41, 31, 21, 10, 0,  90, 80, 70, 60, 50,
         40, 30, 20, 19, 9,  99, 89, 79, 69, 59, 49, 39, 29, 18, 8,  98, 88,
         78, 68, 58, 48, 38, 28, 17, 7,  97, 87, 77, 67, 57, 47, 37, 27, 16,
         6,  96, 86, 76, 66, 56, 46, 36, 26, 15, 5,  95, 85, 75, 65, 55, 45,
         35, 25, 14, 4,  94, 84, 74, 64, 54, 44, 34, 24, 13, 3,  93, 83, 73,
         63, 53, 43, 33, 23, 12, 2,  92, 82, 72, 62, 52, 42, 32, 22})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "054",
                          "Alt(6)^2.2^2:4 and Alt(6)^2.4 intersection",
                          "[quick][schreier-sims][no-valgrind][intersection]") {
    auto              rg = ReportGuard(false);
    SchreierSims<100> S, T, U;
    using Perm = SchreierSims<100>::element_type;
    S.add_generator(make<Perm>(
        {68, 62, 69, 67, 66, 64, 61, 65, 60, 63, 98, 92, 99, 97, 96, 94, 91,
         95, 90, 93, 38, 32, 39, 37, 36, 34, 31, 35, 30, 33, 78, 72, 79, 77,
         76, 74, 71, 75, 70, 73, 8,  2,  9,  7,  6,  4,  1,  5,  0,  3,  28,
         22, 29, 27, 26, 24, 21, 25, 20, 23, 18, 12, 19, 17, 16, 14, 11, 15,
         10, 13, 88, 82, 89, 87, 86, 84, 81, 85, 80, 83, 58, 52, 59, 57, 56,
         54, 51, 55, 50, 53, 48, 42, 49, 47, 46, 44, 41, 45, 40, 43}));
    S.add_generator(make<Perm>(
        {72, 92, 12, 82, 32, 62, 42, 22, 52, 2,  70, 90, 10, 80, 30, 60, 40,
         20, 50, 0,  75, 95, 15, 85, 35, 65, 45, 25, 55, 5,  76, 96, 16, 86,
         36, 66, 46, 26, 56, 6,  71, 91, 11, 81, 31, 61, 41, 21, 51, 1,  79,
         99, 19, 89, 39, 69, 49, 29, 59, 9,  78, 98, 18, 88, 38, 68, 48, 28,
         58, 8,  74, 94, 14, 84, 34, 64, 44, 24, 54, 4,  77, 97, 17, 87, 37,
         67, 47, 27, 57, 7,  73, 93, 13, 83, 33, 63, 43, 23, 53, 3}));
    T.add_generator(make<Perm>(
        {20, 26, 23, 28, 24, 29, 25, 22, 27, 21, 40, 46, 43, 48, 44, 49, 45,
         42, 47, 41, 60, 66, 63, 68, 64, 69, 65, 62, 67, 61, 70, 76, 73, 78,
         74, 79, 75, 72, 77, 71, 50, 56, 53, 58, 54, 59, 55, 52, 57, 51, 30,
         36, 33, 38, 34, 39, 35, 32, 37, 31, 90, 96, 93, 98, 94, 99, 95, 92,
         97, 91, 10, 16, 13, 18, 14, 19, 15, 12, 17, 11, 0,  6,  3,  8,  4,
         9,  5,  2,  7,  1,  80, 86, 83, 88, 84, 89, 85, 82, 87, 81}));
    T.add_generator(make<Perm>(
        {21, 11, 71, 51, 41, 91, 1,  31, 61, 81, 24, 14, 74, 54, 44, 94, 4,
         34, 64, 84, 27, 17, 77, 57, 47, 97, 7,  37, 67, 87, 28, 18, 78, 58,
         48, 98, 8,  38, 68, 88, 25, 15, 75, 55, 45, 95, 5,  35, 65, 85, 20,
         10, 70, 50, 40, 90, 0,  30, 60, 80, 26, 16, 76, 56, 46, 96, 6,  36,
         66, 86, 23, 13, 73, 53, 43, 93, 3,  33, 63, 83, 22, 12, 72, 52, 42,
         92, 2,  32, 62, 82, 29, 19, 79, 59, 49, 99, 9,  39, 69, 89}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 4);
    REQUIRE(U.contains(make<Perm>(
        {1,  11, 21, 31, 41, 51, 61, 71, 81, 91, 0,  10, 20, 30, 40, 50, 60,
         70, 80, 90, 9,  19, 29, 39, 49, 59, 69, 79, 89, 99, 8,  18, 28, 38,
         48, 58, 68, 78, 88, 98, 7,  17, 27, 37, 47, 57, 67, 77, 87, 97, 6,
         16, 26, 36, 46, 56, 66, 76, 86, 96, 5,  15, 25, 35, 45, 55, 65, 75,
         85, 95, 4,  14, 24, 34, 44, 54, 64, 74, 84, 94, 3,  13, 23, 33, 43,
         53, 63, 73, 83, 93, 2,  12, 22, 32, 42, 52, 62, 72, 82, 92})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "055",
                          "3^3(S(4) x 2) and ASL(3, 3) intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<27> S, T, U;
    using Perm = SchreierSims<27>::element_type;
    S.add_generator(
        make<Perm>({0, 6,  3,  25, 22, 19, 14, 11, 17, 9,  15, 12, 7, 4,
                    1, 23, 20, 26, 18, 24, 21, 16, 13, 10, 5,  2,  8}));
    S.add_generator(
        make<Perm>({0, 18, 9, 25, 16, 7,  14, 5,  23, 3, 21, 12, 19, 10,
                    1, 17, 8, 26, 6,  24, 15, 22, 13, 4, 11, 2,  20}));
    S.add_generator(
        make<Perm>({0, 9,  18, 14, 23, 5,  25, 7,  16, 1, 10, 19, 12, 21,
                    3, 26, 8,  17, 2,  11, 20, 13, 22, 4, 24, 6,  15}));
    S.add_generator(
        make<Perm>({1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14,
                    12, 16, 17, 15, 19, 20, 18, 22, 23, 21, 25, 26, 24}));
    T.add_generator(
        make<Perm>({0,  26, 13, 1,  24, 14, 2, 25, 12, 9, 8,  22, 10, 6,
                    23, 11, 7,  21, 18, 17, 4, 19, 15, 5, 20, 16, 3}));
    T.add_generator(
        make<Perm>({0,  17, 22, 1,  15, 23, 2,  16, 21, 3,  11, 25, 4, 9,
                    26, 5,  10, 24, 6,  14, 19, 7,  12, 20, 8,  13, 18}));
    T.add_generator(
        make<Perm>({1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14,
                    12, 16, 17, 15, 19, 20, 18, 22, 23, 21, 25, 26, 24}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 648);
    REQUIRE(U.contains(
        make<Perm>({0,  9, 18, 6,  15, 24, 3,  12, 21, 1,  10, 19, 7, 16,
                    25, 4, 13, 22, 2,  11, 20, 8,  17, 26, 5,  14, 23})));
    REQUIRE(U.contains(
        make<Perm>({0,  1,  2,  25, 26, 24, 14, 12, 13, 3,  4,  5, 19, 20,
                    18, 17, 15, 16, 6,  7,  8,  22, 23, 21, 11, 9, 10})));
    REQUIRE(U.contains(
        make<Perm>({1,  2,  0,  26, 24, 25, 12, 13, 14, 4,  5, 3,  20, 18,
                    19, 15, 16, 17, 7,  8,  6,  23, 21, 22, 9, 10, 11})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "056",
                          "7^2:3 x Q(8) and 7^2:D(2*6) intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<49> S, T, U;
    using Perm = SchreierSims<49>::element_type;
    S.add_generator(make<Perm>(
        {0,  28, 35, 42, 7,  14, 21, 1,  29, 36, 43, 8,  15, 22, 2,  30, 37,
         44, 9,  16, 23, 3,  31, 38, 45, 10, 17, 24, 4,  32, 39, 46, 11, 18,
         25, 5,  33, 40, 47, 12, 19, 26, 6,  34, 41, 48, 13, 20, 27}));
    S.add_generator(make<Perm>(
        {0, 44, 10, 18, 26, 34, 36, 41, 22, 42, 5,  31, 16, 11, 43, 19, 30,
         7, 6,  39, 24, 9,  32, 27, 38, 14, 1,  47, 17, 13, 40, 29, 46, 21,
         2, 25, 3,  15, 48, 37, 12, 28, 33, 35, 4,  23, 8,  45, 20}));
    S.add_generator(make<Perm>(
        {0,  3,  4,  5,  6,  1,  2,  21, 24, 25, 26, 27, 22, 23, 28, 31, 32,
         33, 34, 29, 30, 35, 38, 39, 40, 41, 36, 37, 42, 45, 46, 47, 48, 43,
         44, 7,  10, 11, 12, 13, 8,  9,  14, 17, 18, 19, 20, 15, 16}));
    S.add_generator(make<Perm>(
        {1,  3,  5,  2,  0,  6,  4,  8,  10, 12, 9,  7,  13, 11, 15, 17, 19,
         16, 14, 20, 18, 22, 24, 26, 23, 21, 27, 25, 29, 31, 33, 30, 28, 34,
         32, 36, 38, 40, 37, 35, 41, 39, 43, 45, 47, 44, 42, 48, 46}));
    T.add_generator(make<Perm>(
        {0,  32, 40, 48, 8,  16, 24, 7,  4,  47, 34, 22, 37, 17, 14, 25, 5,
         13, 36, 30, 45, 21, 11, 33, 6,  15, 44, 38, 28, 46, 19, 41, 1,  23,
         10, 35, 18, 12, 27, 43, 2,  31, 42, 39, 26, 20, 29, 9,  3}));
    T.add_generator(make<Perm>(
        {0,  32, 40, 48, 8,  16, 24, 1,  28, 41, 46, 10, 19, 23, 2,  31, 35,
         43, 12, 18, 27, 3,  29, 39, 42, 9,  20, 26, 4,  34, 37, 47, 7,  17,
         22, 5,  30, 36, 45, 13, 14, 25, 6,  33, 38, 44, 11, 15, 21}));
    T.add_generator(make<Perm>(
        {0,  4,  5,  6,  1,  2,  3,  28, 32, 33, 34, 29, 30, 31, 35, 39, 40,
         41, 36, 37, 38, 42, 46, 47, 48, 43, 44, 45, 7,  11, 12, 13, 8,  9,
         10, 14, 18, 19, 20, 15, 16, 17, 21, 25, 26, 27, 22, 23, 24}));
    T.add_generator(make<Perm>(
        {1,  3,  5,  2,  0,  6,  4,  8,  10, 12, 9,  7,  13, 11, 15, 17, 19,
         16, 14, 20, 18, 22, 24, 26, 23, 21, 27, 25, 29, 31, 33, 30, 28, 34,
         32, 36, 38, 40, 37, 35, 41, 39, 43, 45, 47, 44, 42, 48, 46}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 98);
    REQUIRE(U.contains(make<Perm>(
        {0,  4,  5,  6,  1,  2,  3,  28, 32, 33, 34, 29, 30, 31, 35, 39, 40,
         41, 36, 37, 38, 42, 46, 47, 48, 43, 44, 45, 7,  11, 12, 13, 8,  9,
         10, 14, 18, 19, 20, 15, 16, 17, 21, 25, 26, 27, 22, 23, 24})));
    REQUIRE(U.contains(make<Perm>(
        {1,  0,  6,  4,  3,  5,  2,  29, 28, 34, 32, 31, 33, 30, 36, 35, 41,
         39, 38, 40, 37, 43, 42, 48, 46, 45, 47, 44, 8,  7,  13, 11, 10, 12,
         9,  15, 14, 20, 18, 17, 19, 16, 22, 21, 27, 25, 24, 26, 23})));
    REQUIRE(U.contains(make<Perm>(
        {7,  11, 12, 13, 8,  9,  10, 0,  4,  5,  6,  1,  2,  3,  42, 46, 47,
         48, 43, 44, 45, 28, 32, 33, 34, 29, 30, 31, 21, 25, 26, 27, 22, 23,
         24, 35, 39, 40, 41, 36, 37, 38, 14, 18, 19, 20, 15, 16, 17})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "057",
                          "D(2*53) and 53:13 intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<53> S, T, U;
    using Perm = SchreierSims<53>::element_type;
    S.add_generator(make<Perm>(
        {0,  27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
         44, 45, 46, 47, 48, 49, 50, 51, 52, 1,  2,  3,  4,  5,  6,  7,  8,  9,
         10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}));
    S.add_generator(make<Perm>(
        {1,  2,  18, 48, 35, 11, 24, 20, 40, 30, 37, 36, 10, 5,  34, 4,  13, 47,
         3,  15, 25, 43, 29, 9,  12, 16, 52, 0,  27, 44, 41, 39, 8,  23, 6,  49,
         38, 31, 50, 42, 21, 45, 51, 26, 28, 22, 33, 14, 19, 7,  32, 46, 17}));
    T.add_generator(make<Perm>(
        {0,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
         22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
         40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 1,  2,  3,  4}));
    T.add_generator(make<Perm>(
        {1,  2,  18, 48, 35, 11, 24, 20, 40, 30, 37, 36, 10, 5,  34, 4,  13, 47,
         3,  15, 25, 43, 29, 9,  12, 16, 52, 0,  27, 44, 41, 39, 8,  23, 6,  49,
         38, 31, 50, 42, 21, 45, 51, 26, 28, 22, 33, 14, 19, 7,  32, 46, 17}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 53);
    REQUIRE(U.contains(make<Perm>(
        {1,  2,  18, 48, 35, 11, 24, 20, 40, 30, 37, 36, 10, 5,  34, 4,  13, 47,
         3,  15, 25, 43, 29, 9,  12, 16, 52, 0,  27, 44, 41, 39, 8,  23, 6,  49,
         38, 31, 50, 42, 21, 45, 51, 26, 28, 22, 33, 14, 19, 7,  32, 46, 17})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "058",
                          "2^6:(7 x D_14) and PSL(2, 6)^2.4 intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<64> S, T, U;
    using Perm = SchreierSims<64>::element_type;
    S.add_generator(make<Perm>(
        {0,  32, 5,  37, 2,  34, 7,  39, 20, 52, 17, 49, 22, 54, 19, 51,
         8,  40, 13, 45, 10, 42, 15, 47, 28, 60, 25, 57, 30, 62, 27, 59,
         16, 48, 21, 53, 18, 50, 23, 55, 4,  36, 1,  33, 6,  38, 3,  35,
         24, 56, 29, 61, 26, 58, 31, 63, 12, 44, 9,  41, 14, 46, 11, 43}));
    S.add_generator(make<Perm>(
        {0,  17, 2,  19, 1,  16, 3,  18, 8,  25, 10, 27, 9,  24, 11, 26,
         4,  21, 6,  23, 5,  20, 7,  22, 12, 29, 14, 31, 13, 28, 15, 30,
         32, 49, 34, 51, 33, 48, 35, 50, 40, 57, 42, 59, 41, 56, 43, 58,
         36, 53, 38, 55, 37, 52, 39, 54, 44, 61, 46, 63, 45, 60, 47, 62}));
    S.add_generator(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62}));
    T.add_generator(make<Perm>(
        {10, 34, 58, 42, 50, 2, 18, 26, 9,  33, 57, 41, 49, 1, 17, 25,
         15, 39, 63, 47, 55, 7, 23, 31, 13, 37, 61, 45, 53, 5, 21, 29,
         12, 36, 60, 44, 52, 4, 20, 28, 14, 38, 62, 46, 54, 6, 22, 30,
         11, 35, 59, 43, 51, 3, 19, 27, 8,  32, 56, 40, 48, 0, 16, 24}));
    T.add_generator(make<Perm>(
        {39, 23, 63, 55, 31, 15, 47, 7, 37, 21, 61, 53, 29, 13, 45, 5,
         32, 16, 56, 48, 24, 8,  40, 0, 35, 19, 59, 51, 27, 11, 43, 3,
         33, 17, 57, 49, 25, 9,  41, 1, 34, 18, 58, 50, 26, 10, 42, 2,
         36, 20, 60, 52, 28, 12, 44, 4, 38, 22, 62, 54, 30, 14, 46, 6}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.contains(make<Perm>(
        {3,  2,  1,  0,  7,  6,  5,  4,  11, 10, 9,  8,  15, 14, 13, 12,
         19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28,
         35, 34, 33, 32, 39, 38, 37, 36, 43, 42, 41, 40, 47, 46, 45, 44,
         51, 50, 49, 48, 55, 54, 53, 52, 59, 58, 57, 56, 63, 62, 61, 60})));
    REQUIRE(U.contains(make<Perm>(
        {4,  5,  6,  7,  0,  1,  2,  3,  12, 13, 14, 15, 8,  9,  10, 11,
         20, 21, 22, 23, 16, 17, 18, 19, 28, 29, 30, 31, 24, 25, 26, 27,
         36, 37, 38, 39, 32, 33, 34, 35, 44, 45, 46, 47, 40, 41, 42, 43,
         52, 53, 54, 55, 48, 49, 50, 51, 60, 61, 62, 63, 56, 57, 58, 59})));
    REQUIRE(U.contains(make<Perm>(
        {24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23,
         8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,
         56, 57, 58, 59, 60, 61, 62, 63, 48, 49, 50, 51, 52, 53, 54, 55,
         40, 41, 42, 43, 44, 45, 46, 47, 32, 33, 34, 35, 36, 37, 38, 39})));
    REQUIRE(U.contains(make<Perm>(
        {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
         48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
         16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31})));
    REQUIRE(U.size() == 16);
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "059",
                          "2^6:(S_3 x GL(3, 2)) and 2.6:Alt(7) intersection",
                          "[quick][schreier-sims][intersection][no-valgrind]") {
    auto             rg = ReportGuard(false);
    SchreierSims<64> S, T, U;
    using Perm = SchreierSims<64>::element_type;
    S.add_generator(make<Perm>(
        {0,  45, 9,  36, 31, 50, 22, 59, 2,  47, 11, 38, 29, 48, 20, 57,
         8,  37, 1,  44, 23, 58, 30, 51, 10, 39, 3,  46, 21, 56, 28, 49,
         16, 61, 25, 52, 15, 34, 6,  43, 18, 63, 27, 54, 13, 32, 4,  41,
         24, 53, 17, 60, 7,  42, 14, 35, 26, 55, 19, 62, 5,  40, 12, 33}));
    S.add_generator(make<Perm>(
        {0,  9,  49, 56, 2,  11, 51, 58, 1,  8,  48, 57, 3,  10, 50, 59,
         4,  13, 53, 60, 6,  15, 55, 62, 5,  12, 52, 61, 7,  14, 54, 63,
         32, 41, 17, 24, 34, 43, 19, 26, 33, 40, 16, 25, 35, 42, 18, 27,
         36, 45, 21, 28, 38, 47, 23, 30, 37, 44, 20, 29, 39, 46, 22, 31}));
    S.add_generator(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62}));
    T.add_generator(make<Perm>(
        {0,  45, 58, 23, 28, 49, 38, 11, 2,  47, 56, 21, 30, 51, 36, 9,
         8,  37, 50, 31, 20, 57, 46, 3,  10, 39, 48, 29, 22, 59, 44, 1,
         32, 13, 26, 55, 60, 17, 6,  43, 34, 15, 24, 53, 62, 19, 4,  41,
         40, 5,  18, 63, 52, 25, 14, 35, 42, 7,  16, 61, 54, 27, 12, 33}));
    T.add_generator(make<Perm>(
        {0,  30, 53, 43, 38, 56, 19, 13, 1,  31, 52, 42, 39, 57, 18, 12,
         4,  26, 49, 47, 34, 60, 23, 9,  5,  27, 48, 46, 35, 61, 22, 8,
         16, 14, 37, 59, 54, 40, 3,  29, 17, 15, 36, 58, 55, 41, 2,  28,
         20, 10, 33, 63, 50, 44, 7,  25, 21, 11, 32, 62, 51, 45, 6,  24}));
    T.add_generator(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 64);
    REQUIRE(U.contains(make<Perm>(
        {1,  0,  3,  2,  5,  4,  7,  6,  9,  8,  11, 10, 13, 12, 15, 14,
         17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30,
         33, 32, 35, 34, 37, 36, 39, 38, 41, 40, 43, 42, 45, 44, 47, 46,
         49, 48, 51, 50, 53, 52, 55, 54, 57, 56, 59, 58, 61, 60, 63, 62})));
    REQUIRE(U.contains(make<Perm>(
        {2,  3,  0,  1,  6,  7,  4,  5,  10, 11, 8,  9,  14, 15, 12, 13,
         18, 19, 16, 17, 22, 23, 20, 21, 26, 27, 24, 25, 30, 31, 28, 29,
         34, 35, 32, 33, 38, 39, 36, 37, 42, 43, 40, 41, 46, 47, 44, 45,
         50, 51, 48, 49, 54, 55, 52, 53, 58, 59, 56, 57, 62, 63, 60, 61})));
    REQUIRE(U.contains(make<Perm>(
        {4,  5,  6,  7,  0,  1,  2,  3,  12, 13, 14, 15, 8,  9,  10, 11,
         20, 21, 22, 23, 16, 17, 18, 19, 28, 29, 30, 31, 24, 25, 26, 27,
         36, 37, 38, 39, 32, 33, 34, 35, 44, 45, 46, 47, 40, 41, 42, 43,
         52, 53, 54, 55, 48, 49, 50, 51, 60, 61, 62, 63, 56, 57, 58, 59})));
    REQUIRE(U.contains(make<Perm>(
        {8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,
         24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23,
         40, 41, 42, 43, 44, 45, 46, 47, 32, 33, 34, 35, 36, 37, 38, 39,
         56, 57, 58, 59, 60, 61, 62, 63, 48, 49, 50, 51, 52, 53, 54, 55})));
    REQUIRE(U.contains(make<Perm>(
        {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
         48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
         32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47})));
    REQUIRE(U.contains(make<Perm>(
        {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
         48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
         16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "060",
                          "AGL(7, 2) and PGL(2, 127) intersection",
                          "[standard][schreier-sims][intersection]") {
    auto              rg = ReportGuard(false);
    SchreierSims<128> S, T, U;
    using Perm = SchreierSims<128>::element_type;
    S.add_generator(make<Perm>(
        {0,   34,  31,  61,  1,   35,  30,  60,  96,  66, 127, 93,  97,
         67,  126, 92,  4,   38,  27,  57,  5,   39,  26, 56,  100, 70,
         123, 89,  101, 71,  122, 88,  16,  50,  15,  45, 17,  51,  14,
         44,  112, 82,  111, 77,  113, 83,  110, 76,  20, 54,  11,  41,
         21,  55,  10,  40,  116, 86,  107, 73,  117, 87, 106, 72,  32,
         2,   63,  29,  33,  3,   62,  28,  64,  98,  95, 125, 65,  99,
         94,  124, 36,  6,   59,  25,  37,  7,   58,  24, 68,  102, 91,
         121, 69,  103, 90,  120, 48,  18,  47,  13,  49, 19,  46,  12,
         80,  114, 79,  109, 81,  115, 78,  108, 52,  22, 43,  9,   53,
         23,  42,  8,   84,  118, 75,  105, 85,  119, 74, 104}));
    S.add_generator(make<Perm>(
        {0,   49,  78,  127, 13,  60,  67,  114, 2,   51,  76,  125, 15,
         62,  65,  112, 20,  37,  90,  107, 25,  40,  87,  102, 22,  39,
         88,  105, 27,  42,  85,  100, 8,   57,  70,  119, 5,   52,  75,
         122, 10,  59,  68,  117, 7,   54,  73,  120, 28,  45,  82,  99,
         17,  32,  95,  110, 30,  47,  80,  97,  19,  34,  93,  108, 64,
         113, 14,  63,  77,  124, 3,   50,  66,  115, 12,  61,  79,  126,
         1,   48,  84,  101, 26,  43,  89,  104, 23,  38,  86,  103, 24,
         41,  91,  106, 21,  36,  72,  121, 6,   55,  69,  116, 11,  58,
         74,  123, 4,   53,  71,  118, 9,   56,  92,  109, 18,  35,  81,
         96,  31,  46,  94,  111, 16,  33,  83,  98,  29,  44}));
    S.add_generator(make<Perm>(
        {1,   0,   3,   2,   5,   4,   7,   6,   9,   8,   11,  10,  13,
         12,  15,  14,  17,  16,  19,  18,  21,  20,  23,  22,  25,  24,
         27,  26,  29,  28,  31,  30,  33,  32,  35,  34,  37,  36,  39,
         38,  41,  40,  43,  42,  45,  44,  47,  46,  49,  48,  51,  50,
         53,  52,  55,  54,  57,  56,  59,  58,  61,  60,  63,  62,  65,
         64,  67,  66,  69,  68,  71,  70,  73,  72,  75,  74,  77,  76,
         79,  78,  81,  80,  83,  82,  85,  84,  87,  86,  89,  88,  91,
         90,  93,  92,  95,  94,  97,  96,  99,  98,  101, 100, 103, 102,
         105, 104, 107, 106, 109, 108, 111, 110, 113, 112, 115, 114, 117,
         116, 119, 118, 121, 120, 123, 122, 125, 124, 127, 126}));
    T.add_generator(make<Perm>(
        {0,   1,   8,   65,  42,  52,  11,  63,  35,  77,  38,  54,  70,
         31,  73,  45,  97,  56,  66,  43,  100, 107, 108, 88,  84,  33,
         12,  76,  61,  109, 21,  62,  113, 5,   68,  80,  93,  48,  39,
         22,  27,  106, 9,   18,  37,  105, 78,  67,  19,  94,  57,  83,
         120, 102, 4,   50,  115, 59,  23,  117, 17,  95,  82,  111, 126,
         53,  122, 98,  101, 16,  72,  34,  79,  112, 44,  118, 64,  124,
         29,  74,  36,  92,  7,   10,  114, 55,  110, 125, 26,  13,  91,
         119, 85,  49,  121, 58,  104, 6,   116, 25,  51,  14,  89,  41,
         30,  3,   71,  90,  99,  2,   81,  86,  69,  60,  15,  28,  40,
         103, 123, 46,  87,  127, 47,  32,  96,  75,  24,  20}));
    T.add_generator(make<Perm>(
        {82,  0,   72,  58,  33,  107, 21,  120, 46, 115, 77,  28,  16, 47,  14,
         61,  75,  48,  25,  8,   29,  31,  71,  20, 49,  73,  13,  22, 113, 23,
         112, 6,   40,  70,  76,  102, 78,  96,  34, 87,  110, 90,  79, 63,  10,
         15,  19,  26,  59,  69,  119, 97,  116, 64, 109, 121, 5,   50, 126, 17,
         123, 45,  55,  88,  95,  99,  104, 51,  35, 24,  4,   27,  80, 18,  36,
         12,  38,  44,  74,  108, 2,   91,  1,   92, 101, 41,  65,  94, 43,  83,
         85,  106, 89,  66,  39,  53,  100, 67,  98, 86,  37,  127, 68, 122, 93,
         7,   81,  56,  42,  114, 32,  30,  111, 11, 54,  118, 117, 52, 9,   57,
         105, 62,  125, 124, 60,  103, 3,   84}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 8);
    REQUIRE(U.contains(make<Perm>(
        {31,  30,  29,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,
         18,  17,  16,  15,  14,  13,  12,  11,  10,  9,   8,   7,   6,
         5,   4,   3,   2,   1,   0,   127, 126, 125, 124, 123, 122, 121,
         120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108,
         107, 106, 105, 104, 103, 102, 101, 100, 99,  98,  97,  96,  95,
         94,  93,  92,  91,  90,  89,  88,  87,  86,  85,  84,  83,  82,
         81,  80,  79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,
         68,  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,  57,  56,
         55,  54,  53,  52,  51,  50,  49,  48,  47,  46,  45,  44,  43,
         42,  41,  40,  39,  38,  37,  36,  35,  34,  33,  32})));
    REQUIRE(U.contains(make<Perm>(
        {32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
         45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,
         58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,
         71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
         84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,
         97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
         110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
         123, 124, 125, 126, 127, 0,   1,   2,   3,   4,   5,   6,   7,
         8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
         21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "061",
                          "PSL(2, 127) and AGL(1, 2^7) intersection",
                          "[quick][schreier-sims][intersection][no-valgrind]") {
    auto              rg = ReportGuard(false);
    SchreierSims<128> S, T, U;
    using Perm = SchreierSims<128>::element_type;
    S.add_generator(make<Perm>(
        {0,   1,   35,  53,  9,   120, 54,  111, 80, 124, 39, 4,  72,  62,  112,
         105, 6,   115, 122, 18,  51,  90,  99,  26, 114, 5,  70, 64,  95,  2,
         107, 82,  60,  52,  101, 36,  49,  19,  22, 108, 76, 71, 77,  66,  48,
         3,   29,  98,  43,  121, 59,  10,  87,  89, 42,  57, 28, 117, 88,  103,
         56,  58,  7,   86,  24,  102, 47,  116, 14, 97,  79, 68, 74,  69,  37,
         123, 126, 96,  109, 44,  93,  85,  63,  38, 15,  50, 81, 75,  12,  31,
         119, 46,  55,  94,  127, 23,  30,  11,  40, 33,  83, 73, 13,  106, 21,
         65,  34,  91,  25,  8,   92,  110, 16,  17, 45,  61, 27, 41,  32,  78,
         125, 20,  67,  113, 104, 118, 84,  100}));
    S.add_generator(make<Perm>(
        {82,  0,   72,  58,  33,  107, 21,  120, 46, 115, 77,  28,  16, 47,  14,
         61,  75,  48,  25,  8,   29,  31,  71,  20, 49,  73,  13,  22, 113, 23,
         112, 6,   40,  70,  76,  102, 78,  96,  34, 87,  110, 90,  79, 63,  10,
         15,  19,  26,  59,  69,  119, 97,  116, 64, 109, 121, 5,   50, 126, 17,
         123, 45,  55,  88,  95,  99,  104, 51,  35, 24,  4,   27,  80, 18,  36,
         12,  38,  44,  74,  108, 2,   91,  1,   92, 101, 41,  65,  94, 43,  83,
         85,  106, 89,  66,  39,  53,  100, 67,  98, 86,  37,  127, 68, 122, 93,
         7,   81,  56,  42,  114, 32,  30,  111, 11, 54,  118, 117, 52, 9,   57,
         105, 62,  125, 124, 60,  103, 3,   84}));
    T.add_generator(make<Perm>(
        {0,   34,  54,  20,  1,   35,  55,  21,  100, 70,  82, 112, 101,
         71,  83,  113, 4,   38,  50,  16,  5,   39,  51,  17, 96,  66,
         86,  116, 97,  67,  87,  117, 8,   42,  62,  28,  9,  43,  63,
         29,  108, 78,  90,  120, 109, 79,  91,  121, 12,  46, 58,  24,
         13,  47,  59,  25,  104, 74,  94,  124, 105, 75,  95, 125, 32,
         2,   22,  52,  33,  3,   23,  53,  68,  102, 114, 80, 69,  103,
         115, 81,  36,  6,   18,  48,  37,  7,   19,  49,  64, 98,  118,
         84,  65,  99,  119, 85,  40,  10,  30,  60,  41,  11, 31,  61,
         76,  110, 122, 88,  77,  111, 123, 89,  44,  14,  26, 56,  45,
         15,  27,  57,  72,  106, 126, 92,  73,  107, 127, 93}));
    T.add_generator(make<Perm>(
        {0,  59, 120, 67, 54, 13, 78, 117, 1,  58, 121, 66, 55, 12, 79, 116,
         2,  57, 122, 65, 52, 15, 76, 119, 3,  56, 123, 64, 53, 14, 77, 118,
         4,  63, 124, 71, 50, 9,  74, 113, 5,  62, 125, 70, 51, 8,  75, 112,
         6,  61, 126, 69, 48, 11, 72, 115, 7,  60, 127, 68, 49, 10, 73, 114,
         16, 43, 104, 83, 38, 29, 94, 101, 17, 42, 105, 82, 39, 28, 95, 100,
         18, 41, 106, 81, 36, 31, 92, 103, 19, 40, 107, 80, 37, 30, 93, 102,
         20, 47, 108, 87, 34, 25, 90, 97,  21, 46, 109, 86, 35, 24, 91, 96,
         22, 45, 110, 85, 32, 27, 88, 99,  23, 44, 111, 84, 33, 26, 89, 98}));
    T.add_generator(make<Perm>(
        {1,   0,   3,   2,   5,   4,   7,   6,   9,   8,   11,  10,  13,
         12,  15,  14,  17,  16,  19,  18,  21,  20,  23,  22,  25,  24,
         27,  26,  29,  28,  31,  30,  33,  32,  35,  34,  37,  36,  39,
         38,  41,  40,  43,  42,  45,  44,  47,  46,  49,  48,  51,  50,
         53,  52,  55,  54,  57,  56,  59,  58,  61,  60,  63,  62,  65,
         64,  67,  66,  69,  68,  71,  70,  73,  72,  75,  74,  77,  76,
         79,  78,  81,  80,  83,  82,  85,  84,  87,  86,  89,  88,  91,
         90,  93,  92,  95,  94,  97,  96,  99,  98,  101, 100, 103, 102,
         105, 104, 107, 106, 109, 108, 111, 110, 113, 112, 115, 114, 117,
         116, 119, 118, 121, 120, 123, 122, 125, 124, 127, 126}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 4);
    REQUIRE(U.contains(make<Perm>(
        {63,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,
         50,  49,  48,  47,  46,  45,  44,  43,  42,  41,  40,  39,  38,
         37,  36,  35,  34,  33,  32,  31,  30,  29,  28,  27,  26,  25,
         24,  23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,
         11,  10,  9,   8,   7,   6,   5,   4,   3,   2,   1,   0,   127,
         126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114,
         113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101,
         100, 99,  98,  97,  96,  95,  94,  93,  92,  91,  90,  89,  88,
         87,  86,  85,  84,  83,  82,  81,  80,  79,  78,  77,  76,  75,
         74,  73,  72,  71,  70,  69,  68,  67,  66,  65,  64})));
    REQUIRE(U.contains(make<Perm>(
        {64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,
         77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
         90,  91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102,
         103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
         116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0,
         1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
         14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
         27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,
         53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63})));
  }

  LIBSEMIGROUPS_TEST_CASE(
      "SchreierSims",
      "062",
      "PSL(3, 4).2 and PSL(3, 4).2 (nontrivial) intersection",
      "[quick][schreier-sims][intersection]") {
    auto              rg = ReportGuard(false);
    SchreierSims<120> S, T, U;
    using Perm = SchreierSims<120>::element_type;
    S.add_generator(make<Perm>(
        {35,  1,   66,  63,  98,  53,  117, 55,  20,  118, 56, 73,  93,  57,
         112, 15,  32,  119, 42,  19,  8,   25,  23,  22,  24, 21,  72,  75,
         64,  97,  38,  96,  16,  33,  88,  0,   52,  87,  30, 109, 108, 94,
         18,  43,  44,  100, 89,  47,  50,  116, 48,  51,  36, 5,   54,  7,
         10,  13,  76,  59,  85,  61,  62,  3,   28,  101, 2,  104, 80,  113,
         70,  106, 26,  11,  110, 27,  58,  77,  107, 92,  68, 95,  83,  82,
         84,  60,  86,  37,  34,  46,  90,  99,  79,  12,  41, 81,  31,  29,
         4,   91,  45,  65,  105, 103, 67,  102, 71,  78,  40, 39,  74,  111,
         14,  69,  114, 115, 49,  6,   9,   17}));
    S.add_generator(make<Perm>(
        {20,  95,  91,  53, 40,  21, 32,  118, 109, 2,   41,  74,  72, 102,
         115, 65,  3,   66, 43,  92, 7,   84,  1,   82,  104, 101, 24, 77,
         97,  57,  71,  35, 50,  56, 60,  42,  80,  85,  58,  22,  34, 88,
         117, 90,  49,  47, 112, 73, 10,  17,  111, 83,  33,  78,  61, 13,
         70,  28,  38,  94, 4,   69, 99,  87,  9,   14,  44,  54,  5,  67,
         52,  62,  116, 96, 51,  79, 105, 27,  16,  107, 108, 26,  19, 11,
         68,  103, 12,  63, 48,  18, 89,  64,  23,  55,  76,  39,  45, 29,
         37,  30,  113, 8,  93,  98, 81,  59,  36,  110, 106, 25,  75, 6,
         46,  114, 119, 15, 86,  31, 0,   100}));
    T.add_generator(make<Perm>(
        {22,  119, 110, 87, 4,   12,  16,  102, 98,  39, 113, 90,  5,  62,
         104, 116, 6,   93, 103, 48,  112, 38,  0,   28, 53,  94,  30, 51,
         23,  74,  26,  45, 118, 47,  115, 66,  101, 59, 21,  9,   49, 41,
         114, 96,  85,  31, 75,  33,  19,  40,  99,  27, 97,  24,  54, 57,
         56,  55,  89,  37, 108, 79,  13,  73,  65,  64, 35,  67,  69, 68,
         107, 88,  72,  63, 29,  46,  77,  76,  100, 61, 81,  80,  91, 86,
         105, 44,  83,  3,  71,  58,  11,  82,  106, 17, 25,  95,  43, 52,
         8,   50,  78,  36, 7,   18,  14,  84,  92,  70, 60,  109, 2,  111,
         20,  10,  42,  34, 15,  117, 32,  1}));
    T.add_generator(make<Perm>(
        {109, 40,  7,   74,  81,  3,   83,  113, 31, 53,  112, 10, 52,  19,
         32,  94,  107, 20,  13,  111, 88,  0,   9,  16,  41,  97, 80,  99,
         51,  17,  101, 77,  79,  26,  35,  117, 5,  1,   28,  33, 82,  4,
         76,  60,  38,  43,  67,  21,  34,  61,  96, 44,  73,  93, 116, 14,
         69,  86,  58,  25,  106, 49,  108, 66,  84, 65,  46,  63, 6,   92,
         103, 71,  8,   104, 36,  91,  57,  72,  2,  55,  39,  24, 37,  119,
         30,  100, 42,  59,  29,  56,  15,  105, 89, 22,  95,  90, 62,  87,
         98,  27,  115, 64,  75,  54,  12,  102, 45, 110, 50,  47, 23,  18,
         118, 78,  85,  114, 70,  48,  11,  68}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 20'160);
    REQUIRE(U.contains(make<Perm>(
        {0,   115, 24,  104, 82,  16, 88,  86,  106, 9,   39,  118, 40,  70,
         91,  77,  5,   111, 28,  57, 53,  87,  74,  58,  2,   112, 109, 72,
         18,  41,  44,  93,  71,  66, 64,  49,  46,  99,  54,  10,  12,  29,
         117, 78,  30,  68,  36,  48, 47,  35,  61,  119, 83,  20,  38,  90,
         75,  19,  23,  107, 63,  50, 103, 60,  34,  67,  33,  65,  45,  114,
         13,  32,  27,  73,  22,  56, 95,  15,  43,  92,  80,  110, 4,   52,
         113, 101, 7,   21,  6,   98, 55,  14,  79,  31,  94,  76,  102, 97,
         89,  37,  100, 85,  96,  62, 3,   108, 8,   59,  105, 26,  81,  17,
         25,  84,  69,  1,   116, 42, 11,  51})));
    REQUIRE(U.contains(make<Perm>(
        {0,   15,  83,  22, 70,  30, 48,  105, 54,  9,   81,  49,  69,  4,
         26,  115, 44,  64, 61,  46, 78,  75,  104, 27,  52,  60,  91,  58,
         50,  68,  16,  55, 107, 98, 17,  11,  19,  65,  8,   110, 114, 45,
         119, 20,  5,   29, 57,  6,  88,  118, 18,  42,  2,   43,  106, 93,
         21,  36,  72,  32, 112, 28, 79,  25,  111, 99,  89,  37,  41,  40,
         82,  59,  23,  73, 3,   87, 101, 1,   53,  103, 100, 39,  13,  24,
         102, 76,  108, 56, 47,  33, 31,  109, 62,  90,  97,  85,  84,  94,
         66,  67,  80,  95, 113, 92, 74,  86,  38,  71,  7,   14,  10,  34,
         63,  96,  12,  77, 116, 51, 35,  117})));
    REQUIRE(U.contains(make<Perm>(
        {0,   62,  33,  48,  82,  44,  22,  72, 38,  73,  55, 29, 25,  13,
         117, 92,  30,  56,  76,  19,  107, 34, 6,   108, 66, 12, 119, 86,
         95,  11,  16,  81,  78,  2,   21,  68, 46,  96,  8,  90, 112, 118,
         91,  71,  5,   49,  36,  104, 3,   45, 101, 109, 98, 59, 106, 10,
         17,  57,  105, 53,  114, 85,  1,   69, 87,  113, 24, 84, 35,  63,
         70,  43,  7,   9,   88,  111, 18,  79, 32,  77,  80, 31, 4,   89,
         67,  61,  27,  64,  74,  83,  39,  42, 15,  110, 97, 28, 37,  94,
         52,  102, 100, 50,  99,  115, 47,  58, 54,  20,  23, 51, 93,  75,
         40,  65,  60,  103, 116, 14,  41,  26})));
    REQUIRE(U.contains(make<Perm>(
        {0,   96,  62,  16,  21,  97,  48,  43,  31,  119, 117, 66,  60,  63,
         87,  68,  44,  73,  24,  46,  79,  8,   104, 11,  113, 69,  56,  41,
         89,  115, 22,  111, 18,  2,   112, 27,  5,   33,  75,  12,  82,  85,
         9,   28,  19,  99,  74,  94,  88,  71,  107, 40,  98,  105, 14,  39,
         70,  100, 32,  72,  17,  20,  78,  90,  55,  29,  50,  23,  58,  42,
         114, 53,  37,  64,  116, 26,  1,   101, 59,  84,  36,  93,  109, 61,
         86,  95,  92,  91,  80,  65,  54,  13,  83,  25,  30,  45,  103, 6,
         49,  35,  47,  76,  52,  108, 57,  102, 10,  118, 77,  106, 38,  34,
         4,   15,  110, 7,   3,   51,  67,  81})));
    REQUIRE(U.contains(make<Perm>(
        {1,   50,  80, 62,  114, 92,  57,  97,  6,   112, 7,   12,  58,  95,
         119, 54,  32, 106, 109, 56,  78,  15,  72,  44,  113, 25,  23,  29,
         75,  59,  13, 17,  88,  67,  87,  104, 103, 34,  76,  110, 117, 93,
         5,   105, 26, 69,  33,  115, 101, 68,  0,   22,  60,  39,  21,  28,
         64,  8,   11, 27,  84,  45,  116, 2,   19,  35,  4,   46,  98,  61,
         14,  118, 51, 81,  79,  55,  107, 18,  94,  111, 63,  108, 47,  83,
         52,  36,  43, 37,  16,  48,  71,  40,  42,  99,  20,  30,  9,   10,
         49,  41,  24, 89,  102, 85,  65,  86,  31,  38,  73,  77,  53,  74,
         96,  100, 66, 82,  3,   91,  90,  70})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "063",
                          "PSL(3, 4).Sym(3) and PSL(3, 4).2 intersection",
                          "[quick][schreier-sims][intersection]") {
    auto              rg = ReportGuard(false);
    SchreierSims<105> S, T, U;
    using Perm = SchreierSims<105>::element_type;
    S.add_generator(make<Perm>(
        {0,   32, 24, 16, 34,  10,  46, 43, 44, 33,  5,  93, 49, 13, 101,
         71,  3,  28, 20, 21,  18,  19, 67, 52, 2,   89, 42, 41, 17, 53,
         30,  36, 1,  9,  4,   63,  31, 77, 38, 76,  47, 27, 26, 7,  8,
         66,  6,  40, 72, 12,  59,  73, 23, 29, 90,  97, 78, 74, 82, 50,
         84,  85, 81, 35, 83,  69,  45, 22, 86, 65,  98, 15, 48, 51, 57,
         100, 39, 37, 56, 104, 103, 62, 58, 64, 60,  61, 68, 87, 91, 25,
         54,  88, 96, 11, 95,  94,  92, 55, 70, 102, 75, 14, 99, 80, 79}));
    S.add_generator(make<Perm>(
        {54, 18, 98, 67, 42,  22,  24, 35, 55, 8,  5,  104, 16,  15, 103,
         79, 99, 71, 63, 100, 34,  97, 10, 81, 94, 95, 32,  39,  65, 77,
         92, 90, 49, 31, 45,  58,  40, 2,  28, 48, 64, 30,  101, 3,  0,
         20, 78, 14, 27, 26,  102, 75, 73, 59, 44, 9,  52,  70,  7,  76,
         72, 91, 17, 1,  36,  38,  19, 43, 46, 89, 82, 62,  87,  56, 23,
         84, 53, 93, 68, 13,  11,  74, 57, 69, 51, 21, 50,  60,  25, 83,
         33, 96, 41, 29, 6,   88,  61, 85, 37, 12, 66, 4,   86,  47, 80}));
    T.add_generator(make<Perm>(
        {56, 87,  10, 6,  59, 95,  20, 2,  96, 17, 80,  85, 47,  22, 104,
         4,  99,  90, 18, 84, 61,  34, 79, 28, 25, 50,  97, 49,  83, 78,
         1,  48,  76, 69, 26, 58,  3,  63, 62, 27, 86,  44, 77,  40, 82,
         29, 73,  41, 94, 51, 102, 65, 11, 52, 81, 19,  67, 55,  33, 12,
         57, 30,  88, 98, 68, 13,  9,  21, 37, 14, 0,   91, 53,  89, 45,
         31, 35,  54, 75, 39, 66,  42, 15, 64, 46, 43,  72, 36,  71, 60,
         7,  103, 93, 24, 74, 8,   5,  70, 23, 38, 101, 92, 100, 16, 32}));
    T.add_generator(make<Perm>(
        {31, 49, 50,  8,  45, 99, 51,  5,  40,  54, 22,  75, 85, 66, 19,
         87, 21, 89,  84, 9,  44, 42,  38, 59,  24, 73,  63, 30, 71, 25,
         28, 14, 100, 3,  64, 16, 58,  90, 79,  56, 7,   12, 69, 47, 101,
         46, 98, 18,  29, 72, 11, 91,  13, 37,  86, 68,  88, 36, 65, 53,
         77, 34, 20,  6,  35, 95, 103, 43, 81,  61, 78,  27, 92, 62, 2,
         70, 74, 15,  76, 1,  32, 23,  0,  104, 80, 102, 82, 52, 41, 57,
         55, 83, 10,  17, 26, 93, 39,  33, 4,   97, 67,  48, 96, 60, 94}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 20'160);
    REQUIRE(U.contains(make<Perm>(
        {0,  1,   17, 47, 4,  5,  41, 21,  18,  9,   10, 51, 76, 38, 77,
         59, 40,  2,  8,  43, 44, 7,  94,  25,  28,  23, 90, 46, 24, 91,
         87, 65,  32, 33, 34, 81, 69, 101, 13,  49,  16, 6,  54, 19, 20,
         78, 27,  3,  57, 39, 71, 11, 89,  88,  42,  92, 66, 48, 82, 15,
         64, 104, 63, 62, 60, 31, 56, 95,  100, 36,  80, 50, 74, 93, 72,
         86, 12,  14, 45, 85, 70, 35, 58,  84,  83,  79, 75, 30, 53, 52,
         26, 29,  55, 73, 22, 67, 97, 96,  103, 102, 68, 37, 99, 98, 61})));
    REQUIRE(U.contains(make<Perm>(
        {0,   1,  6,  7,  4,  5,   2,   3,  54, 9,  10, 73, 63, 87,  45,
         48,  43, 41, 42, 40, 26,  47,  67, 29, 46, 91, 20, 28, 27,  23,
         38,  97, 32, 33, 34, 49,  55,  56, 30, 81, 19, 17, 18, 16,  90,
         14,  24, 21, 15, 35, 74,  93,  53, 52, 8,  36, 37, 59, 102, 57,
         104, 64, 76, 12, 61, 96,  101, 22, 70, 92, 68, 72, 71, 11,  50,
         103, 62, 78, 77, 84, 100, 39,  99, 85, 79, 83, 98, 13, 89,  88,
         44,  25, 69, 51, 95, 94,  65,  31, 86, 82, 80, 66, 58, 75,  60})));
    REQUIRE(U.contains(make<Perm>(
        {0,  1,  42, 7,  4,  34,  18, 3,   41, 9,  10, 85,  92,  14, 13,
         39, 53, 54, 6,  89, 90,  47, 56,  23, 28, 25, 44,  46,  24, 29,
         78, 80, 33, 32, 5,  57,  62, 67,  77, 15, 88, 8,   2,   52, 26,
         87, 27, 21, 81, 59, 98,  79, 43,  16, 17, 76, 22,  35,  60, 49,
         58, 99, 36, 69, 82, 70,  94, 37,  96, 63, 65, 103, 75,  83, 86,
         72, 55, 38, 30, 51, 31,  48, 64,  73, 93, 11, 74,  45,  40, 19,
         20, 91, 12, 84, 66, 101, 68, 100, 50, 61, 97, 95,  104, 71, 102})));
    REQUIRE(U.contains(make<Perm>(
        {0,   1,   88, 47, 4,  33,  89, 21, 43, 9,  10, 57, 101, 71, 103,
         93,  54,  53, 19, 18, 26,  7,  36, 23, 27, 25, 20, 24,  46, 29,
         74,  102, 34, 5,  32, 85,  22, 76, 50, 84, 42, 52, 40,  8,  90,
         75,  28,  3,  51, 83, 38,  48, 41, 17, 16, 67, 62, 11,  97, 73,
         100, 70,  56, 66, 68, 99,  63, 55, 64, 94, 61, 13, 87,  59, 30,
         45,  37,  98, 86, 81, 104, 79, 96, 49, 39, 35, 78, 72,  2,  6,
         44,  91,  95, 15, 69, 92,  82, 58, 77, 65, 60, 12, 31,  14, 80})));
    REQUIRE(U.contains(make<Perm>(
        {0,  9,  54,  20, 4,  33,  18,  90, 16, 10,  1,  60, 73, 78,  75,
         31, 17, 8,   43, 2,  23,  44,  79, 3,  24,  7,  91, 28, 46,  21,
         45, 92, 32,  34, 5,  102, 59,  39, 77, 99,  41, 42, 40, 6,   29,
         98, 27, 26,  96, 82, 72,  104, 52, 88, 19,  48, 35, 65, 101, 97,
         76, 12, 93,  51, 62, 69,  49,  83, 95, 57,  22, 71, 74, 61,  50,
         87, 11, 103, 86, 70, 67,  58,  66, 80, 100, 68, 13, 14, 89,  53,
         25, 47, 15,  64, 84, 85,  55,  36, 30, 37,  94, 81, 56, 38,  63})));
    REQUIRE(U.contains(make<Perm>(
        {0,  4,   54,  26, 1,  34, 8,   20,  41, 10, 9,  75, 99, 81,  57,
         45, 89,  42,  6,  53, 21, 44,  80,  24, 25, 28, 47, 29, 23,  46,
         49, 56,  33,  32, 5,  13, 64,  96,  35, 30, 52, 18, 2,  88,  7,
         59, 91,  90,  14, 87, 84, 86,  16,  43, 17, 61, 65, 77, 63,  78,
         36, 92,  58,  82, 69, 66, 31,  100, 67, 60, 22, 83, 85, 103, 79,
         51, 102, 48,  15, 72, 94, 38,  62,  50, 71, 74, 11, 39, 19,  40,
         3,  27,  104, 98, 70, 68, 101, 37,  73, 76, 95, 97, 12, 93,  55})));
    REQUIRE(U.contains(make<Perm>(
        {1,   4,  15,  7,  0,  25, 84,  47,  83, 9,  10, 13, 67, 88,  16,
         30,  51, 39,  73, 48, 44, 21,  56,  24, 34, 28, 20, 32, 5,   46,
         2,   80, 91,  29, 23, 75, 12,  63,  40, 45, 79, 93, 49, 81,  26,
         17,  33, 3,   86, 87, 8,  14,  57,  85, 59, 95, 94, 74, 96,  78,
         100, 60, 55,  76, 97, 31, 66,  36,  99, 92, 70, 41, 43, 103, 52,
         89,  37, 53,  54, 38, 65, 72,  68,  50, 98, 77, 19, 42, 11,  35,
         90,  27, 101, 71, 22, 62, 104, 102, 6,  82, 61, 69, 64, 18,  58})));
    REQUIRE(U.contains(make<Perm>(
        {2,   17,  84, 37, 6,  70,  93,  57, 58, 41,  34, 28, 19, 65, 55,
         69,  36,  23, 21, 67, 5,   100, 53, 56, 38,  59, 0,  68, 48, 80,
         86,  62,  30, 66, 15, 103, 16,  42, 63, 95,  90, 39, 96, 12, 32,
         72,  101, 13, 61, 73, 79,  82,  24, 45, 104, 89, 1,  78, 64, 25,
         27,  11,  54, 52, 92, 77,  40,  43, 74, 10,  99, 46, 22, 49, 60,
         102, 4,   47, 81, 97, 91,  7,   35, 83, 26,  50, 44, 29, 71, 98,
         33,  87,  8,  76, 18, 9,   3,   85, 14, 20,  94, 88, 75, 51, 31})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "064",
                          "3^4:5:4 and D_16:4 intersection",
                          "[quick][schreier-sims][intersection]") {
    auto             rg = ReportGuard(false);
    SchreierSims<81> S, T, U;
    using Perm = SchreierSims<81>::element_type;
    S.add_generator(make<Perm>(
        {0,  9,  18, 80, 62, 71, 40, 49, 31, 3,  12, 21, 74, 56, 65, 43, 52,
         34, 6,  15, 24, 77, 59, 68, 37, 46, 28, 27, 36, 45, 26, 8,  17, 67,
         76, 58, 30, 39, 48, 20, 2,  11, 70, 79, 61, 33, 42, 51, 23, 5,  14,
         64, 73, 55, 54, 63, 72, 53, 35, 44, 13, 22, 4,  57, 66, 75, 47, 29,
         38, 16, 25, 7,  60, 69, 78, 50, 32, 41, 10, 19, 1}));
    S.add_generator(make<Perm>(
        {0,  3,  6,  80, 74, 77, 40, 43, 37, 1,  4,  7,  78, 72, 75, 41, 44,
         38, 2,  5,  8,  79, 73, 76, 39, 42, 36, 9,  12, 15, 62, 56, 59, 49,
         52, 46, 10, 13, 16, 60, 54, 57, 50, 53, 47, 11, 14, 17, 61, 55, 58,
         48, 51, 45, 18, 21, 24, 71, 65, 68, 31, 34, 28, 19, 22, 25, 69, 63,
         66, 32, 35, 29, 20, 23, 26, 70, 64, 67, 30, 33, 27}));
    S.add_generator(make<Perm>(
        {1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14, 12, 16, 17,
         15, 19, 20, 18, 22, 23, 21, 25, 26, 24, 28, 29, 27, 31, 32, 30, 34,
         35, 33, 37, 38, 36, 40, 41, 39, 43, 44, 42, 46, 47, 45, 49, 50, 48,
         52, 53, 51, 55, 56, 54, 58, 59, 57, 61, 62, 60, 64, 65, 63, 67, 68,
         66, 70, 71, 69, 73, 74, 72, 76, 77, 75, 79, 80, 78}));
    T.add_generator(make<Perm>(
        {0,  6,  3,  5,  2,  8,  7,  4,  1,  36, 42, 39, 41, 38, 44, 43, 40,
         37, 72, 78, 75, 77, 74, 80, 79, 76, 73, 9,  15, 12, 14, 11, 17, 16,
         13, 10, 45, 51, 48, 50, 47, 53, 52, 49, 46, 54, 60, 57, 59, 56, 62,
         61, 58, 55, 18, 24, 21, 23, 20, 26, 25, 22, 19, 27, 33, 30, 32, 29,
         35, 34, 31, 28, 63, 69, 66, 68, 65, 71, 70, 67, 64}));
    T.add_generator(make<Perm>(
        {0,  2,  1,  6,  8,  7,  3,  5,  4,  9,  11, 10, 15, 17, 16, 12, 14,
         13, 18, 20, 19, 24, 26, 25, 21, 23, 22, 27, 29, 28, 33, 35, 34, 30,
         32, 31, 36, 38, 37, 42, 44, 43, 39, 41, 40, 45, 47, 46, 51, 53, 52,
         48, 50, 49, 54, 56, 55, 60, 62, 61, 57, 59, 58, 63, 65, 64, 69, 71,
         70, 66, 68, 67, 72, 74, 73, 78, 80, 79, 75, 77, 76}));
    T.add_generator(make<Perm>(
        {0,  9,  18, 27, 36, 45, 54, 63, 72, 1,  10, 19, 28, 37, 46, 55, 64,
         73, 2,  11, 20, 29, 38, 47, 56, 65, 74, 3,  12, 21, 30, 39, 48, 57,
         66, 75, 4,  13, 22, 31, 40, 49, 58, 67, 76, 5,  14, 23, 32, 41, 50,
         59, 68, 77, 6,  15, 24, 33, 42, 51, 60, 69, 78, 7,  16, 25, 34, 43,
         52, 61, 70, 79, 8,  17, 26, 35, 44, 53, 62, 71, 80}));
    T.add_generator(make<Perm>(
        {0,  2,  1,  4,  3,  5,  8,  7,  6,  45, 47, 46, 49, 48, 50, 53, 52,
         51, 63, 65, 64, 67, 66, 68, 71, 70, 69, 27, 29, 28, 31, 30, 32, 35,
         34, 33, 72, 74, 73, 76, 75, 77, 80, 79, 78, 9,  11, 10, 13, 12, 14,
         17, 16, 15, 54, 56, 55, 58, 57, 59, 62, 61, 60, 18, 20, 19, 22, 21,
         23, 26, 25, 24, 36, 38, 37, 40, 39, 41, 44, 43, 42}));
    T.add_generator(make<Perm>(
        {1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14, 12, 16, 17,
         15, 19, 20, 18, 22, 23, 21, 25, 26, 24, 28, 29, 27, 31, 32, 30, 34,
         35, 33, 37, 38, 36, 40, 41, 39, 43, 44, 42, 46, 47, 45, 49, 50, 48,
         52, 53, 51, 55, 56, 54, 58, 59, 57, 61, 62, 60, 64, 65, 63, 67, 68,
         66, 70, 71, 69, 73, 74, 72, 76, 77, 75, 79, 80, 78}));
    schreier_sims::intersection(U, S, T);
    REQUIRE(U.size() == 162);
    REQUIRE(U.contains(make<Perm>(
        {0,  9,  18, 27, 36, 45, 54, 63, 72, 1,  10, 19, 28, 37, 46, 55, 64,
         73, 2,  11, 20, 29, 38, 47, 56, 65, 74, 3,  12, 21, 30, 39, 48, 57,
         66, 75, 4,  13, 22, 31, 40, 49, 58, 67, 76, 5,  14, 23, 32, 41, 50,
         59, 68, 77, 6,  15, 24, 33, 42, 51, 60, 69, 78, 7,  16, 25, 34, 43,
         52, 61, 70, 79, 8,  17, 26, 35, 44, 53, 62, 71, 80})));
    REQUIRE(U.contains(make<Perm>(
        {1,  2,  0,  4,  5,  3,  7,  8,  6,  10, 11, 9,  13, 14, 12, 16, 17,
         15, 19, 20, 18, 22, 23, 21, 25, 26, 24, 28, 29, 27, 31, 32, 30, 34,
         35, 33, 37, 38, 36, 40, 41, 39, 43, 44, 42, 46, 47, 45, 49, 50, 48,
         52, 53, 51, 55, 56, 54, 58, 59, 57, 61, 62, 60, 64, 65, 63, 67, 68,
         66, 70, 71, 69, 73, 74, 72, 76, 77, 75, 79, 80, 78})));
    REQUIRE(U.contains(make<Perm>(
        {3,  4,  5,  6,  7,  8,  0,  1,  2,  12, 13, 14, 15, 16, 17, 9,  10,
         11, 21, 22, 23, 24, 25, 26, 18, 19, 20, 30, 31, 32, 33, 34, 35, 27,
         28, 29, 39, 40, 41, 42, 43, 44, 36, 37, 38, 48, 49, 50, 51, 52, 53,
         45, 46, 47, 57, 58, 59, 60, 61, 62, 54, 55, 56, 66, 67, 68, 69, 70,
         71, 63, 64, 65, 75, 76, 77, 78, 79, 80, 72, 73, 74})));
  }

  LIBSEMIGROUPS_TEST_CASE("SchreierSims",
                          "065",
                          "uint8_t Perm",
                          "[quick][schreier-sims][copy constructor]") {
    auto rg    = ReportGuard(false);
    using Perm = Perm<0, uint8_t>;
    auto S1    = std::make_unique<SchreierSims<255, uint8_t, Perm>>();
    S1->add_generator(make<Perm>(
        {1,   0,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
         14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
         28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
         42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
         70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
         84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,
         98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
         112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
         126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
         140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
         154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
         168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
         182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195,
         196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
         210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
         224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
         238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
         252, 253, 254}));
    S1->add_generator(make<Perm>(
        {1,   2,   3,   4,   0,   5,   6,   7,   8,   9,   10,  11,  12,  13,
         14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
         28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
         42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
         70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
         84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,
         98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
         112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
         126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
         140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
         154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
         168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
         182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195,
         196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
         210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
         224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
         238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
         252, 253, 254}));

    auto S2
        = std::make_unique<SchreierSims<255, uint8_t, Perm>>(std::move(*S1));
    REQUIRE(S2->size() == 120);

    auto S3 = std::make_unique<SchreierSims<255, uint8_t, Perm>>();
    *S3     = std::move(*S2);
    REQUIRE(S3->size() == 120);
  }

}  // namespace libsemigroups
