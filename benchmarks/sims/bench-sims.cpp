//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2022-2025 James D. Mitchell
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

#include <cmath>     // for pow
#include <cstddef>   // for size_t
#include <cstdint>   // for uint64_t
#include <iostream>  // for operator<<, cout, ostream
#include <string>    // for operator+, basic_string
#include <thread>    // for thread
#include <vector>    // for vector, operator==

#include "Catch2-3.8.0/catch_amalgamated.hpp"  // for REQUIRE, REQUIRE_NOTHROW, REQUIRE_THROWS_AS

#include "libsemigroups/bipart.hpp"                 // for Bipartition
#include "libsemigroups/bmat-fastest.hpp"           // for BMatFastest
#include "libsemigroups/bmat8.hpp"                  // for BMat8
#include "libsemigroups/detail/report.hpp"          // for ReportGuard
#include "libsemigroups/froidure-pin.hpp"           // for FroidurePinBase
#include "libsemigroups/froidure-pin.hpp"           // for FroidurePin
#include "libsemigroups/knuth-bendix.hpp"           // for redundant_rule
#include "libsemigroups/pbr.hpp"                    // for PBR
#include "libsemigroups/presentation-examples.hpp"  // for presentation::examples
#include "libsemigroups/runner.hpp"                 // for Runner
#include "libsemigroups/sims.hpp"                   // for Sims
#include "libsemigroups/to-presentation.hpp"        // for to<Presentation>
#include "libsemigroups/transf.hpp"                 // for Transf
#include "libsemigroups/types.hpp"                  // for word_type etc

namespace libsemigroups {
  namespace {
    std::string invert(std::string const& g) {
      auto invert = [](char c) {
        if (std::isupper(c)) {
          return std::tolower(c);
        } else {
          return std::toupper(c);
        }
      };
      auto G = g;
      std::reverse(G.begin(), G.end());
      std::transform(G.begin(), G.end(), G.begin(), invert);
      return G;
    }

    std::string comm(std::string const& g, std::string const& h) {
      return invert(g) + invert(h) + g + h;
    }

    bool conj_pruner(WordGraph<uint32_t> const& wg) {
      using node_type = uint32_t;
      static thread_local std::vector<node_type> new_old(
          wg.number_of_nodes(), static_cast<node_type>(UNDEFINED));
      static thread_local std::vector<node_type> old_new(
          wg.number_of_nodes(), static_cast<node_type>(UNDEFINED));

      for (uint32_t root = 1; root < wg.number_of_active_nodes(); ++root) {
        node_type    next = 0;
        size_t const n    = wg.out_degree();
        std::fill(
            old_new.begin(), old_new.end(), static_cast<node_type>(UNDEFINED));

        new_old[0]    = root;
        old_new[root] = 0;

        for (node_type s = 0; s <= next; ++s) {
          for (letter_type a = 0; a < n; ++a) {
            node_type t_old = wg.target_no_checks(new_old[s], a);
            node_type sa    = wg.target_no_checks(s, a);
            if (t_old == UNDEFINED || sa == UNDEFINED) {
              goto end;
            }
            node_type t_new = old_new[t_old];
            if (t_new == UNDEFINED) {
              old_new[t_old] = ++next;
              new_old[next]  = t_old;
              t_new          = next;
            }
            if (sa < t_new) {
              goto end;
            } else if (sa != UNDEFINED && sa > t_new) {
              // fmt::print("{}\n", old_new);
              // fmt::print("{}\n\n", detail::to_string(wg));
              return false;
            }
          }
        }
      end:
        (void) 0;
      }
      return true;
    }

    template <typename S, typename T>
    void xml_tag(S name, T val) {
      std::cout << fmt::format("<{0} value=\"{1}\"></{0}>", name, val);
    }
  }  // namespace

  TEST_CASE("POI(3) from FroidurePin", "[POI3][Sim1][quick][talk]") {
    auto                  rg = ReportGuard(false);
    FroidurePin<PPerm<3>> S;
    S.add_generator(make<PPerm<3>>({0, 1, 2}, {0, 1, 2}, 3));
    S.add_generator(make<PPerm<3>>({1, 2}, {0, 1}, 3));
    S.add_generator(make<PPerm<3>>({0, 1}, {0, 2}, 3));
    S.add_generator(make<PPerm<3>>({0, 2}, {1, 2}, 3));
    S.add_generator(make<PPerm<3>>({0, 1}, {1, 2}, 3));
    S.add_generator(make<PPerm<3>>({0, 2}, {0, 1}, 3));
    S.add_generator(make<PPerm<3>>({1, 2}, {0, 2}, 3));
    REQUIRE(S.size() == 20);

    auto p = to<Presentation<word_type>>(S);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(20) == 99);
    };
    BENCHMARK("Left congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(20) == 99);
    };
  }

  TEST_CASE("POI(4) from FroidurePin", "[POI4][Sim1][standard]") {
    auto                  rg = ReportGuard(false);
    FroidurePin<PPerm<4>> S;
    S.add_generator(make<PPerm<4>>({0, 1, 2, 3}, {0, 1, 2, 3}, 4));
    S.add_generator(make<PPerm<4>>({1, 2, 3}, {0, 1, 2}, 4));
    S.add_generator(make<PPerm<4>>({0, 1, 2}, {0, 1, 3}, 4));
    S.add_generator(make<PPerm<4>>({0, 1, 3}, {0, 2, 3}, 4));
    S.add_generator(make<PPerm<4>>({0, 2, 3}, {1, 2, 3}, 4));
    S.add_generator(make<PPerm<4>>({0, 1, 2}, {1, 2, 3}, 4));
    S.add_generator(make<PPerm<4>>({0, 1, 3}, {0, 1, 2}, 4));
    S.add_generator(make<PPerm<4>>({0, 2, 3}, {0, 1, 3}, 4));
    S.add_generator(make<PPerm<4>>({1, 2, 3}, {0, 2, 3}, 4));
    REQUIRE(S.size() == 70);

    auto p = to<Presentation<word_type>>(S);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(70) == 8'146);
    };
  }

  TEST_CASE("ReflexiveBooleanMatMonoid(3) from FroidurePin",
            "[ReflexiveBooleanMatMonoid3][Sim1][fail]") {
    auto                        rg = ReportGuard(true);
    FroidurePin<BMatFastest<3>> S;
    S.add_generator(One<BMatFastest<3>>()());
    S.add_generator(BMatFastest<3>({{1, 1, 0}, {0, 1, 1}, {1, 0, 1}}));
    S.add_generator(BMatFastest<3>({{1, 1, 0}, {0, 1, 0}, {0, 0, 1}}));
    S.add_generator(BMatFastest<3>({{1, 0, 1}, {1, 1, 0}, {0, 1, 1}}));
    S.add_generator(BMatFastest<3>({{1, 0, 1}, {0, 1, 0}, {0, 0, 1}}));
    S.add_generator(BMatFastest<3>({{1, 0, 0}, {1, 1, 0}, {0, 0, 1}}));
    S.add_generator(BMatFastest<3>({{1, 0, 0}, {0, 1, 1}, {0, 0, 1}}));
    S.add_generator(BMatFastest<3>({{1, 0, 0}, {0, 1, 0}, {1, 0, 1}}));
    S.add_generator(BMatFastest<3>({{1, 0, 0}, {0, 1, 0}, {0, 1, 1}}));
    REQUIRE(S.size() == 64);

    auto p = to<Presentation<word_type>>(S);
    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(S.size()) == 7);
    };
    BENCHMARK("Left congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(S.size()) == 7);
    };
  }

  TEST_CASE("singular_brauer_monoid(3) (Maltcev-Mazorchuk)",
            "[talk][singular_brauer_monoid3]") {
    auto                    rg = ReportGuard(false);
    Presentation<word_type> p
        = presentation::examples::singular_brauer_monoid_MM07(3);
    REQUIRE(p.rules.size() == 48);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(9) == 205);
    };
    BENCHMARK("Left congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(9) == 205);
    };
  }

  TEST_CASE("singular_brauer_monoid(4) (Maltcev-Mazorchuk)",
            "[talk][singular_brauer_monoid4]") {
    auto                    rg = ReportGuard(true);
    Presentation<word_type> p
        = presentation::examples::singular_brauer_monoid_MM07(4);
    REQUIRE(presentation::length(p) == 660);
    presentation::remove_duplicate_rules(p);
    REQUIRE(presentation::length(p) == 600);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);
    REQUIRE(p.rules.size() == 252);
    // presentation::remove_redundant(p);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      C.cbegin_long_rules((252 - 64) / 2);
      REQUIRE(C.number_of_congruences(81) == 601'265);
    };
  }

  TEST_CASE("symmetric_inverse_monoid(2) (Hivert)",
            "[talk][symmetric_inverse_monoid2]") {
    auto                    rg = ReportGuard(true);
    Presentation<word_type> p
        = presentation::examples::symmetric_inverse_monoid_Sol04(2);
    presentation::remove_duplicate_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(7) == 10);
    };
  }

  TEST_CASE("symmetric_inverse_monoid(3) (Hivert)",
            "[talk][symmetric_inverse_monoid3]") {
    auto                    rg = ReportGuard(true);
    Presentation<word_type> p
        = presentation::examples::symmetric_inverse_monoid_Sol04(3);
    presentation::remove_duplicate_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_congruences(34) == 274);
    };
  }

  TEST_CASE("symmetric_inverse_monoid(4) (Hivert)",
            "[talk][symmetric_inverse_monoid4]") {
    auto                    rg = ReportGuard(false);
    Presentation<word_type> p
        = presentation::examples::symmetric_inverse_monoid_Sol04(4);
    presentation::remove_duplicate_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);

    BENCHMARK("Right congruences") {
      Sims1 C;
      C.presentation(p);
      REQUIRE(C.number_of_threads(std::thread::hardware_concurrency())
                  .number_of_congruences(209)
              == 195'709);
    };
  }

  namespace {

    std::vector<std::pair<Presentation<word_type>, size_t>>
    generate_random_sample(size_t sample_size,
                           size_t num_letters,
                           size_t word_len) {
      std::vector<std::pair<Presentation<word_type>, size_t>> sample;
      sample.reserve(sample_size);

      Presentation<word_type> p;
      p.alphabet(num_letters);

      for (size_t i = 0; i < sample_size; ++i) {
        p.rules = {random_word(word_len, num_letters),
                   random_word(word_len, num_letters)};
        sample.emplace_back(p, i);
      }

      std::sort(sample.begin(), sample.end(), [](auto const& x, auto const& y) {
        return x.second < y.second;
      });
      return sample;
    }

    //    std::vector<std::pair<Presentation<word_type>, size_t>>
    //    generate_sample(size_t num_letters, size_t word_len, size_t
    //    num_classes) {
    //      std::vector<std::pair<Presentation<word_type>, size_t>> sample;
    //
    //      Presentation<word_type> p;
    //      p.alphabet(num_letters);
    //      Sims1 C;
    //
    //      WordRange lhs;
    //      lhs.alphabet_size(num_letters).min(1).max(word_len);
    //      WordRange rhs;
    //      rhs.alphabet_size(num_letters);
    //
    //      for (auto const& l : lhs) {
    //        rhs.first(l).max(word_len);
    //        for (auto const& r : rhs | rx::skip_n(1)) {
    //          p.rules = {l, r};
    //          C.presentation(p);
    //          size_t const m
    //              = C.number_of_threads(1).number_of_congruences(num_classes);
    //          sample.emplace_back(p, m);
    //        }
    //      }
    //
    //      std::sort(sample.begin(), sample.end(), [](auto const& x, auto
    //      const& y) {
    //        return x.second < y.second;
    //      });
    //      return sample;
    //    }

    void bench_parallel(
        std::vector<std::pair<Presentation<word_type>, size_t>> const& sample,
        size_t num_threads,
        size_t num_classes) {
      xml_tag(
          "Title",
          fmt::format("Algorithm 4 for {} randomly chosen presentations "
                      "with $|A| = {}$, "
                      "$|R| = {}$, $|\\langle A\\mid R \\rangle| = {}$, and "
                      "number of classes = {}",
                      sample.size(),
                      sample[0].first.alphabet().size(),
                      sample[0].first.rules.size() / 2,
                      presentation::length(sample[0].first),
                      num_classes));
      xml_tag("XLabel", "Test case");

      Sims1    C;
      uint64_t num_congs = 0;
      for (auto const& s : sample) {
        C.presentation(s.first);
        num_congs += C.number_of_threads(num_threads)
                         .number_of_congruences(num_classes);
        BENCHMARK(fmt::format("{}", s.second)) {
          C.presentation(s.first);
          C.number_of_threads(num_threads).number_of_congruences(num_classes);
        };
      }
      std::cout << "Mean number of congruences per monoid "
                << num_congs / sample.size() << std::endl;
    }

  }  // namespace

  TEST_CASE("Parallel version", "[parallel]") {
    auto rg = ReportGuard(false);

    size_t const sample_size = 128;
    size_t const num_letters = 2;
    size_t const word_len    = 10;
    size_t const num_classes = 5;

    auto sample = generate_random_sample(sample_size, num_letters, word_len);

    for (size_t i = 1; i <= std::thread::hardware_concurrency(); i *= 2) {
      bench_parallel(sample, i, num_classes);
    }
  }

  namespace {
    void bench_length(Presentation<word_type>& p,
                      size_t                   max_classes = 16,
                      size_t                   expected    = 134) {
      auto rg = ReportGuard(false);
      xml_tag("XLabel", "Length");

      std::vector<std::pair<Presentation<word_type>, size_t>> ps;
      ps.emplace_back(p, presentation::length(p));

      auto it  = knuth_bendix::redundant_rule(p, std::chrono::milliseconds(10));
      size_t i = 0;

      while (it != p.rules.cend()) {
        i++;
        p.rules.erase(it, it + 2);
        it = knuth_bendix::redundant_rule(p, std::chrono::milliseconds(10));
        if (i % 5 == 0) {
          ps.emplace_back(p, presentation::length(p));
        }
      }

      for (auto& q : ps) {
        // FIXME something bad goes wrong here if long_rule_length is called
        // repeatedly, same for presentation.
        Sims1 C;
        C.presentation(q.first).number_of_threads(1);
        BENCHMARK(fmt::format("{}", q.second)) {
          REQUIRE(C.number_of_congruences(max_classes) == expected);
        };
      }
    }

  }  // namespace

  TEST_CASE("Presentation length Iwahori T_n",
            "[full_transf_monoid][length][000]") {
    auto p = presentation::examples::full_transformation_monoid_II74(4);
    bench_length(p);
  }

  // Doesn't run, or is so slow that it's useless
  TEST_CASE("Presentation length Aizenstat",
            "[full_transf_monoid][length][001]") {
    auto  p = presentation::examples::full_transformation_monoid_Aiz58(4);
    Sims1 C;
    C.presentation(p).number_of_threads(1);
    REQUIRE(presentation::length(p) == 0);
    BENCHMARK(fmt::format("{}", presentation::length(p))) {
      REQUIRE(C.number_of_congruences(16) == 134);
    };
  }

  TEST_CASE("Presentation length machine",
            "[full_transf_monoid][length][002]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<4>> S;
    S.add_generator(make<Transf<4>>({1, 2, 3, 0}));
    S.add_generator(make<Transf<4>>({1, 0, 2, 3}));
    S.add_generator(make<Transf<4>>({0, 1, 2, 0}));
    REQUIRE(S.size() == 256);
    auto p = to<Presentation<word_type>>(S);
    bench_length(p);
  }

  TEST_CASE("Presentation length Burnside+Miller S_n",
            "[symmetric_group][length][003]") {
    auto p = presentation::examples::symmetric_group_Bur12(5);
    bench_length(p, 120, 156);
  }

  TEST_CASE("Presentation length Fernandes cyclic inverse monoid 1st",
            "[cyclic_inverse][length][000]") {
    size_t n = 10;
    auto   p = presentation::examples::cyclic_inverse_monoid_Fer22_a(n);
    bench_length(p, 4, 6);
  }

  TEST_CASE("Presentation length Fernandes cyclic inverse monoid 2nd",
            "[cyclic_inverse][length][001]") {
    size_t n = 10;
    auto   p = presentation::examples::cyclic_inverse_monoid_Fer22_b(n);
    bench_length(p, 4, 6);
  }

  TEST_CASE("Presentation length machine cyclic inverse monoid 1st",
            "[cyclic_inverse][length][002]") {
    auto rg = ReportGuard(false);

    FroidurePin<PPerm<10>> S;
    S.add_generator(make<PPerm<10>>({1, 2, 3, 4, 5, 6, 7, 8, 9, 0}));
    S.add_generator(make<PPerm<10>>(
        {1, 2, 3, 4, 5, 6, 7, 8, 9}, {1, 2, 3, 4, 5, 6, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 2, 3, 4, 5, 6, 7, 8, 9}, {0, 2, 3, 4, 5, 6, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 3, 4, 5, 6, 7, 8, 9}, {0, 1, 3, 4, 5, 6, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 4, 5, 6, 7, 8, 9}, {0, 1, 2, 4, 5, 6, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 3, 5, 6, 7, 8, 9}, {0, 1, 2, 3, 5, 6, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 3, 4, 6, 7, 8, 9}, {0, 1, 2, 3, 4, 6, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 3, 4, 5, 7, 8, 9}, {0, 1, 2, 3, 4, 5, 7, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 3, 4, 5, 6, 8, 9}, {0, 1, 2, 3, 4, 5, 6, 8, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 3, 4, 5, 6, 7, 9}, {0, 1, 2, 3, 4, 5, 6, 7, 9}, 10));
    S.add_generator(make<PPerm<10>>(
        {0, 1, 2, 3, 4, 5, 6, 7, 8}, {0, 1, 2, 3, 4, 5, 6, 7, 8}, 10));

    size_t n = 10;
    REQUIRE(S.size() == n * std::pow(2, n) - n + 1);
    auto p = to<Presentation<word_type>>(S);
    bench_length(p, 4, 6);
  }

  TEST_CASE("Presentation length machine cyclic inverse monoid 2nd",
            "[cyclic_inverse][length][003]") {
    auto rg = ReportGuard(false);

    FroidurePin<PPerm<10>> S;
    S.add_generator(make<PPerm<10>>({1, 2, 3, 4, 5, 6, 7, 8, 9, 0}));
    S.add_generator(make<PPerm<10>>(
        {1, 2, 3, 4, 5, 6, 7, 8, 9}, {1, 2, 3, 4, 5, 6, 7, 8, 9}, 10));

    size_t n = 10;
    REQUIRE(S.size() == n * std::pow(2, n) - n + 1);
    auto p = to<Presentation<word_type>>(S);
    bench_length(p, 4, 6);
  }

  // TEST_CASE("Presentation length S_5 (GAP presentation)",
  //           "[symmetric_group][length][004]") {
  //   using presentation::pow;
  //   Presentation<std::string> p;
  //   p.alphabet("abcd");
  //   p.contains_empty_word(true);
  //   presentation::add_rule_no_checks(p, "aa", "");
  //   presentation::add_rule_no_checks(p, "bb", "");
  //   presentation::add_rule_no_checks(p, "cc", "");
  //   presentation::add_rule_no_checks(p, "dd", "");
  //   presentation::add_rule_no_checks(p, pow("ab", 3), "");
  //   presentation::add_rule_no_checks(p, pow("ac", 2), "");
  //   presentation::add_rule_no_checks(p, pow("ad", 2), "");
  //   presentation::add_rule_no_checks(p, pow("bc", 3), "");
  //   presentation::add_rule_no_checks(p, pow("bd", 2), "");
  //   presentation::add_rule_no_checks(p, pow("cd", 3), "");
  //   auto pp = to<Presentation<word_type>>(p);
  //   bench_length(pp, 120, 156);
  // }

  // // Too slow
  // TEST_CASE("Presentation length S_6 (GAP presentation)",
  //           "[symmetric_group][length][005]") {
  //   auto rg = ReportGuard(true);

  //   using presentation::pow;
  //   Presentation<std::string> p;
  //   p.alphabet("abcde");
  //   p.contains_empty_word(true);
  //   presentation::add_rule_no_checks(p, "aa", "");
  //   presentation::add_rule_no_checks(p, "bb", "");
  //   presentation::add_rule_no_checks(p, "cc", "");
  //   presentation::add_rule_no_checks(p, "dd", "");
  //   presentation::add_rule_no_checks(p, "ee", "");
  //   presentation::add_rule_no_checks(p, pow("ab", 3), "");
  //   presentation::add_rule_no_checks(p, pow("ac", 2), "");
  //   presentation::add_rule_no_checks(p, pow("ad", 2), "");
  //   presentation::add_rule_no_checks(p, pow("ae", 2), "");
  //   presentation::add_rule_no_checks(p, pow("bc", 3), "");
  //   presentation::add_rule_no_checks(p, pow("bd", 2), "");
  //   presentation::add_rule_no_checks(p, pow("be", 2), "");
  //   presentation::add_rule_no_checks(p, pow("cd", 3), "");
  //   presentation::add_rule_no_checks(p, pow("ce", 2), "");
  //   presentation::add_rule_no_checks(p, pow("de", 3), "");
  //   auto pp = to<Presentation<word_type>>(p);
  //   bench_length(pp, 720, 1'455);
  // }

  ////////////////////////////////////////////////////////////////////////
  // Low index subgroups of groups
  ////////////////////////////////////////////////////////////////////////

  TEST_CASE("(2, 3, 7)-triangle group - 1-sided - index 50",
            "[triangle-group-2-3-7-50]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("xy");
    presentation::add_rule(p, "xx", "");
    presentation::add_rule(p, "yyy", "");
    presentation::add_rule(p, "xyxyxyxyxyxyxy", "");
    presentation::add_rule(p, "yxyxyxyxyxyxyx", "");

    // REQUIRE(presentation::to_gap_string(p, "G") == "");
    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p)).add_pruner(conj_pruner);

    BENCHMARK("libsemigroups (1 thread)") {
      REQUIRE(S.number_of_threads(1).number_of_congruences(50) == 1'747);
    };
    BENCHMARK("libsemigroups (2 threads)") {
      REQUIRE(S.number_of_threads(2).number_of_congruences(50) == 1'747);
    };
    BENCHMARK("libsemigroups (4 threads)") {
      REQUIRE(S.number_of_threads(4).number_of_congruences(50) == 1'747);
    };
    BENCHMARK("libsemigroups (8 threads)") {
      REQUIRE(S.number_of_threads(8).number_of_congruences(50) == 1'747);
    };
  }

  TEST_CASE("Braid group 5 - index 12 - 1-sided", "[braid-group-5-12]") {
    auto rg = ReportGuard(false);
    auto p  = presentation::examples::braid_group(5);
    presentation::reduce_complements(p);
    REQUIRE(presentation::length(p) == 76);
    presentation::remove_trivial_rules(p);
    presentation::remove_duplicate_rules(p);
    presentation::remove_redundant_generators(p);
    REQUIRE(presentation::length(p) == 76);

    // REQUIRE(presentation::to_gap_string(p, "G") == "");
    Sims1 S;
    S.presentation(p).add_pruner(conj_pruner);
    for (auto nr_threads : {1, 2, 4, 8}) {
      BENCHMARK(fmt::format("libsemigroups ({} thread{})",
                            nr_threads,
                            nr_threads == 1 ? "" : "s")
                    .c_str()) {
        REQUIRE(S.number_of_threads(nr_threads).number_of_congruences(12)
                == 21);
      };
    }
  }

  TEST_CASE("modular group - index 23 - 1-sided", "[modular-group-23]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("STt");
    presentation::add_rule(p, "SS", "");
    presentation::add_rule(p, "Tt", "");
    presentation::add_rule(p, "tT", "");
    presentation::add_rule(p, "STSTST", "");
    presentation::add_rule(p, "TSTSTS", "");

    // REQUIRE(presentation::to_gap_string(p, "G") == "");
    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p)).add_pruner(conj_pruner);

    for (auto nr_threads : {1, 2, 4, 8}) {
      BENCHMARK(fmt::format("libsemigroups ({} thread{})",
                            nr_threads,
                            nr_threads == 1 ? "" : "s")
                    .c_str()) {
        REQUIRE(S.number_of_threads(nr_threads).number_of_congruences(23)
                == 109'859);
      };
    }
  }

  TEST_CASE("fundamental group of K11n34 - index 7 - 1-sided",
            "[fundamental-group-of-K11n34-7]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("abcABC");
    presentation::add_inverse_rules(p, "ABCabc");
    presentation::add_cyclic_conjugates_no_checks(p, "aaBcbbcAc");
    presentation::add_rule(p, "aacAbCBBaCAAbbcBc", "");

    presentation::reduce_complements(p);
    presentation::remove_duplicate_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);

    // REQUIRE(presentation::to_gap_string(p, "G") == "");
    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p))
        .add_pruner(conj_pruner)
        .long_rule_length(17);
    // REQUIRE(S.presentation().rules == std::vector<word_type>());

    REQUIRE(S.number_of_long_rules() == 1);

    for (auto nr_threads : {1, 2, 4, 8}) {
      BENCHMARK(fmt::format("libsemigroups ({} thread{})",
                            nr_threads,
                            nr_threads == 1 ? "" : "s")
                    .c_str()) {
        REQUIRE(S.number_of_threads(nr_threads).number_of_congruences(7) == 52);
      };
    }
  }

  TEST_CASE("Heineken group - index 8 - 1-sided", "[heineken-group-8]") {
    using knuth_bendix::reduce;

    auto rg = ReportGuard(false);

    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("xXyYzZ");
    presentation::add_inverse_rules(p, "XxYyZz");

    KnuthBendix kb(congruence_kind::twosided, p);

    //  < x,y,z | [x,[x,y]]=z, [y,[y,z]]=x, [z,[z,x]]=y >
    auto w = reduce(kb, comm("x", comm("x", "y")) + "Z");
    presentation::add_cyclic_conjugates_no_checks(p, w);
    w = invert(w);
    presentation::add_cyclic_conjugates_no_checks(p, w);

    w = reduce(kb, comm("y", comm("y", "z")) + "X");
    presentation::add_cyclic_conjugates_no_checks(p, w);
    w = invert(w);
    presentation::add_cyclic_conjugates_no_checks(p, w);

    w = reduce(kb, comm("z", comm("z", "x")) + "Y");
    presentation::add_cyclic_conjugates_no_checks(p, w);
    w = invert(w);
    presentation::add_cyclic_conjugates_no_checks(p, w);

    presentation::remove_trivial_rules(p);
    presentation::remove_duplicate_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);

    // REQUIRE(p.rules == std::vector<std::string>());
    REQUIRE(presentation::to_gap_string(p, "G") == "");
    Sims1 S;

    S.presentation(to<Presentation<word_type>>(p))
        .add_pruner(conj_pruner)
        .idle_thread_restarts(256);

    for (auto nr_threads : {1, 2, 4, 8}) {
      BENCHMARK(fmt::format("libsemigroups ({} thread{})",
                            nr_threads,
                            nr_threads == 1 ? "" : "s")
                    .c_str()) {
        REQUIRE(S.number_of_threads(nr_threads).number_of_congruences(8) == 3);
      };
    }
  }

  TEST_CASE("fundamental group of K15n12345 - index 7 - 1-sided",
            "[fundamental-group-of-K15n12345-7]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("abcABC");
    presentation::add_inverse_rules(p, "ABCabc");

    // Short rules
    presentation::add_cyclic_conjugates_no_checks(p, "aBcACAcb");

    // Long rules
    presentation::add_rule(
        p,
        "aBaCacBAcAbaBabaCAcAbaBaCacBAcAbaBabCAcAbABaCabABAbABaCabCAcAb",
        "");

    presentation::reduce_complements(p);
    presentation::remove_duplicate_rules(p);
    presentation::remove_trivial_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);
    // REQUIRE(S.presentation().rules == std::vector<std::string>());
    // REQUIRE(presentation::to_gap_string(p, "G") == "");

    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p))
        .add_pruner(conj_pruner)
        .long_rule_length(62);

    for (auto nr_threads : {1, 2, 4, 8}) {
      BENCHMARK(fmt::format("libsemigroups ({} thread{})",
                            nr_threads,
                            nr_threads == 1 ? "" : "s")
                    .c_str()) {
        REQUIRE(S.number_of_threads(nr_threads).number_of_congruences(7) == 40);
      };
    }
  }

  TEST_CASE("fundamental group of o9_15405 - index 9 - 1-sided",
            "[fundamental-group-of-o9-15405-9]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("abAB");
    presentation::add_inverse_rules(p, "ABab");

    // Short rules
    presentation::add_cyclic_conjugates_no_checks(
        p, "aaaaabbbaabbbaaaaabbbaabbbaaaaaBBBBBBBB");

    presentation::reduce_complements(p);
    presentation::remove_duplicate_rules(p);
    presentation::remove_trivial_rules(p);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);

    // REQUIRE(S.presentation().rules == std::vector<std::string>());
    // REQUIRE(presentation::to_gap_string(p, "G") == "");

    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p)).add_pruner(conj_pruner);

    for (auto nr_threads : {1, 2, 4, 8}) {
      BENCHMARK(fmt::format("libsemigroups ({} thread{})",
                            nr_threads,
                            nr_threads == 1 ? "" : "s")
                    .c_str()) {
        REQUIRE(S.number_of_threads(nr_threads).number_of_congruences(9) == 38);
      };
    }
  }

  // NOTE: this presentation was found to be wrong, use test case marked
  // [heineken] for actual one
  TEST_CASE("Not Heineken group - index 10", "[notheineken][fail]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("xXyY");
    presentation::add_inverse_rules(p, "XxYy");
    presentation::add_rule(p, "yXYYxyYYxyyXYYxyyXyXYYxy", "x");
    presentation::add_rule(p, "YxyyXXYYxyxYxyyXYXyXYYxxyyXYXyXYYxyx", "y");

    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p));
    BENCHMARK("1 thread") {
      REQUIRE(S.number_of_threads(1).number_of_congruences(10) == 1);
    };
    BENCHMARK("2 threads") {
      REQUIRE(S.number_of_threads(2).number_of_congruences(10) == 1);
    };
    BENCHMARK("4 threads") {
      REQUIRE(S.number_of_threads(4).number_of_congruences(10) == 1);
    };
    REQUIRE(S.number_of_congruences(10) == 1);
  }

  TEST_CASE("Catalan monoid n = 1 - all", "[catalan][n=1]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<1>> S;
    S.add_generator(make<Transf<1>>({0}));
    REQUIRE(S.size() == 1);
    auto p = to<Presentation<word_type>>(S);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(1) == 1);
    };
  }

  TEST_CASE("Catalan monoid n = 2 - all", "[catalan][n=2]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<2>> S;
    S.add_generator(make<Transf<2>>({0, 1}));
    S.add_generator(make<Transf<2>>({0, 0}));
    REQUIRE(S.size() == 2);
    auto p = to<Presentation<word_type>>(S);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(S.size()) == 2);
    };
  }

  TEST_CASE("Catalan monoid n = 3 - all", "[catalan][n=3]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<3>> S;
    S.add_generator(make<Transf<3>>({0, 1, 2}));
    S.add_generator(make<Transf<3>>({0, 0, 2}));
    S.add_generator(make<Transf<3>>({0, 1, 1}));
    REQUIRE(S.size() == 5);
    auto p = to<Presentation<word_type>>(S);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(S.size()) == 11);
    };
  }

  TEST_CASE("Catalan monoid n = 4 - all", "[catalan][n=4]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<4>> S;
    S.add_generator(make<Transf<4>>({0, 1, 2, 3}));
    S.add_generator(make<Transf<4>>({0, 0, 2, 3}));
    S.add_generator(make<Transf<4>>({0, 1, 1, 3}));
    S.add_generator(make<Transf<4>>({0, 1, 2, 2}));
    REQUIRE(S.size() == 14);
    auto p = to<Presentation<word_type>>(S);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(S.size()) == 575);
    };
  }

  TEST_CASE("Catalan monoid n = 5 - all", "[catalan][n=5]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<5>> S;
    S.add_generator(make<Transf<5>>({0, 1, 2, 3, 4}));
    S.add_generator(make<Transf<5>>({0, 0, 2, 3, 4}));
    S.add_generator(make<Transf<5>>({0, 1, 1, 3, 4}));
    S.add_generator(make<Transf<5>>({0, 1, 2, 2, 4}));
    S.add_generator(make<Transf<5>>({0, 1, 2, 3, 3}));
    REQUIRE(S.size() == 42);
    auto p = to<Presentation<word_type>>(S);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(S.size())
              == 5'295'135);
    };
  }

  // NOTE: this presentation was found to be wrong, use test case marked
  // [heineken] instead
  TEST_CASE("Not Heineken monoid", "[notheineken][001][fail]") {
    Presentation<std::string> p;
    p.contains_empty_word(true);
    p.alphabet("xyXY");
    presentation::add_rule(p, "yXYYxyYYxyyXYYxyyXyXYYxyX", "");
    presentation::add_rule(p, "YxyyXXYYxyxYxyyXYXyXYYxxyyXYXyXYYxyxY", "");
    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p));
    REQUIRE(S.number_of_threads(4).number_of_congruences(2) == 4);
  }

  TEST_CASE("Order endomorphisms n = 2 - all", "[order_endos][n=2]") {
    auto                   rg = ReportGuard(false);
    FroidurePin<Transf<2>> S;
    S.add_generator(make<Transf<2>>({0, 1}));
    S.add_generator(make<Transf<2>>({0, 0}));
    S.add_generator(make<Transf<2>>({1, 1}));
    REQUIRE(S.size() == 3);

    Sims1 C;
    C.presentation(to<Presentation<word_type>>(S));
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(3) == 5);
    };
  }

  TEST_CASE("Order endomorphisms n = 3 - all", "[order_endos][n=3]") {
    auto rg = ReportGuard(false);
    auto p  = presentation::examples::order_preserving_monoid(3);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(10) == 25);
    };
  }

  TEST_CASE("Order endomorphisms n = 4 - all", "[order_endos][n=4]") {
    auto rg = ReportGuard(false);
    auto p  = presentation::examples::order_preserving_monoid(4);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(35) == 385);
    };
  }

  TEST_CASE("Order endomorphisms n = 5 - all", "[order_endos][n=5]") {
    auto rg = ReportGuard(false);
    auto p  = presentation::examples::order_preserving_monoid(5);
    presentation::sort_each_rule(p);
    presentation::sort_rules(p);
    presentation::remove_duplicate_rules(p);
    presentation::reduce_complements(p);
    presentation::remove_trivial_rules(p);

    Sims1 C;
    C.presentation(p);
    BENCHMARK("1 thread") {
      REQUIRE(C.number_of_threads(1).number_of_congruences(126) == 37'951);
    };
  }

  TEST_CASE("Fibonacci(2, 9) - index 12", "[fibonacci]") {
    auto                      rg = ReportGuard(false);
    Presentation<std::string> p;
    p.alphabet("abAB");
    p.contains_empty_word(true);
    presentation::add_inverse_rules(p, "ABab");
    presentation::add_rule(p, "Abababbab", "aBaaBaB");
    presentation::add_rule(p, "babbabbAb", "ABaaBaa");
    presentation::add_rule(p, "abbabbAbA", "BABaaBa");
    presentation::add_rule(p, "bbabbAbAA", "ABABaaB");
    presentation::add_rule(p, "babbAbAAb", "BABABaa");
    presentation::add_rule(p, "abbAbAAbA", "BBABABa");
    presentation::add_rule(p, "bbAbAAbAA", "ABBABAB");
    presentation::add_rule(p, "bAbAAbAAb", "BABBABA");
    presentation::add_rule(p, "AbAAbAAba", "BBABBAB");
    presentation::add_rule(p, "bAAbAAbab", "aBBABBA");
    presentation::add_rule(p, "AAbAAbaba", "BaBBABB");

    presentation::add_rule(p, "AAbababb", "BaaBaBBA");
    presentation::add_rule(p, "Abababba", "aBaaBaBB");
    presentation::add_rule(p, "abbabaaBaaB", "bAbAAbA");
    presentation::add_rule(p, "babaaBaaBaB", "BAbAbAA");

    Sims1 S;
    S.presentation(to<Presentation<word_type>>(p));

    BENCHMARK("1 thread") {
      REQUIRE(S.number_of_threads(1).number_of_congruences(12) == 6);
    };
    BENCHMARK("2 threads") {
      REQUIRE(S.number_of_threads(2).number_of_congruences(12) == 6);
    };
    BENCHMARK("4 threads") {
      REQUIRE(S.number_of_threads(4).number_of_congruences(12) == 6);
    };
  }

  void bench_sims_refiner_faithful(std::string const&             name,
                                   Presentation<word_type> const& p,
                                   std::vector<word_type> const&  forbid,
                                   size_t                         target_size) {
    SimsRefinerFaithful pruno(forbid);
    Sims1               sims;
    sims.presentation(p).add_pruner(pruno);

    size_t offset      = p.contains_empty_word() ? 0 : 1;
    auto   return_true = [](auto const&) { return true; };

    for (size_t i = 1; i <= std::thread::hardware_concurrency(); i *= 2) {
      BENCHMARK(fmt::format(name + " - SimsRefinerFaithful - {} / {} threads",
                            i,
                            std::thread::hardware_concurrency())) {
        Sims1::word_graph_type wg;
        size_t                 N = target_size + offset;
        do {
          wg = sims.number_of_threads(i).find_if(N, return_true);
          // fmt::print("{}!\n", wg.number_of_active_nodes());
          REQUIRE(wg.number_of_active_nodes() < N);
          N = wg.number_of_active_nodes() - 1 - offset;
        } while (wg.number_of_active_nodes() != 0);
      };
    }
  }

  void bench_filter(std::string const&             name,
                    Presentation<word_type> const& p,
                    std::vector<word_type> const&  forbid,
                    size_t                         target_size) {
    auto filter = [&forbid](auto const& wg) {
      auto first = forbid.cbegin();
      auto last  = forbid.cend();
      for (auto it = first; it != last; it += 2) {
        bool this_rule_compatible = true;
        for (auto n : wg.nodes()) {
          auto l = word_graph::follow_path_no_checks(wg, n, *it);
          auto r = word_graph::follow_path_no_checks(wg, n, *(it + 1));
          if (l != r) {
            this_rule_compatible = false;
            break;
          }
        }
        if (this_rule_compatible) {
          return false;
        }
      }
      return true;
    };

    Sims1 sims;
    sims.presentation(p);

    size_t offset = p.contains_empty_word() ? 0 : 1;
    for (size_t i = 1; i <= std::thread::hardware_concurrency(); i *= 2) {
      BENCHMARK(fmt::format(name + " - Filter - {} / {} threads",
                            i,
                            std::thread::hardware_concurrency())) {
        Sims1::word_graph_type wg;
        size_t                 N = target_size + offset;
        do {
          wg = sims.number_of_threads(i).find_if(N, filter);
          REQUIRE(wg.number_of_active_nodes() < N);
          N = wg.number_of_active_nodes() - 1 - offset;
        } while (wg.number_of_active_nodes() != 0);
      };
    }
  }

  void bench_orc(std::string const&             name,
                 Presentation<word_type> const& p,
                 size_t                         target_size) {
    Sims1 sims;
    auto  q = p;
    q.contains_empty_word(true);

    for (size_t i = 1; i <= std::thread::hardware_concurrency(); i *= 2) {
      MinimalRepOrc orc;
      BENCHMARK(fmt::format(name + " - Orc - {} / {} threads",
                            i,
                            std::thread::hardware_concurrency())) {
        auto wg = orc.presentation(q)
                      .number_of_threads(i)
                      .target_size(target_size + 1)
                      .word_graph();
      };
    }
  }

  TEST_CASE("SimsRefinerFaithful", "[SimsRefinerFaithful]") {
    auto rg = ReportGuard(false);

    FroidurePin<Bipartition> S;
    S.add_generator(Bipartition({{1, 2}, {3, -1}, {4, -2}, {-3, -4}}));
    S.add_generator(Bipartition({{1, 2}, {3, -1}, {4, -4}, {-2, -3}}));
    S.add_generator(Bipartition({{1, 2}, {3, -3}, {4, -1}, {-2, -4}}));
    S.add_generator(Bipartition({{1, 2}, {3, -2}, {4, -3}, {-1, -4}}));
    S.add_generator(Bipartition({{1, 2}, {3, -2}, {4, -4}, {-1, -3}}));
    S.add_generator(Bipartition({{1, 3}, {2, -4}, {4, -3}, {-1, -2}}));
    S.add_generator(Bipartition({{1, -4}, {2, 3}, {4, -3}, {-1, -2}}));
    S.add_generator(Bipartition({{1, 4}, {2, -3}, {3, -4}, {-1, -2}}));
    S.add_generator(Bipartition({{1, -3}, {2, 4}, {3, -4}, {-1, -2}}));
    S.add_generator(Bipartition({{1, -3}, {2, -4}, {3, 4}, {-1, -2}}));
    REQUIRE(S.size() == 81);

    auto p = to<Presentation<word_type>>(S);
    p.throw_if_bad_alphabet_or_rules();
    REQUIRE(p.alphabet().size() == 10);
    REQUIRE(presentation::length(p) == 719);

    std::vector<word_type> forbid = {{0},
                                     {3, 0},
                                     {0, 0},
                                     {0, 1},
                                     {0, 0},
                                     {0, 2},
                                     {0, 2},
                                     {0, 1},
                                     {0, 0},
                                     {5, 9},
                                     {0, 0},
                                     {6, 9},
                                     {5, 9},
                                     {6, 9}};

    bench_sims_refiner_faithful("Singular Brauer", p, forbid, S.size());
    bench_filter("Singular Brauer", p, forbid, S.size());
    bench_orc("Singular Brauer", p, S.size());

    // BENCHMARK("Singular Brauer Monoid - MinimalRepOrc - 1 thread") {
    //   orc.stats().stats_zero();
    //   p.contains_empty_word(true);
    //   auto const& wg = orc.presentation(p)
    //                        .number_of_threads(1)
    //                        .target_size(S.size() + 1)
    //                        .word_graph();

    //   REQUIRE(orc.stats().total_pending_now == 4'685'894);
    //   REQUIRE(wg.number_of_active_nodes() == 18);
    // };
  }

  TEST_CASE("Gossip monoid 2-sided", "[gossip_monoid_2_sided]") {
    auto rg = ReportGuard(false);
    auto S  = make<FroidurePin>({BMat8({{1, 1, 0}, {1, 1, 0}, {0, 0, 1}}),
                                 BMat8({{1, 0, 1}, {0, 1, 0}, {1, 0, 1}}),
                                 BMat8({{1, 0, 0}, {0, 1, 1}, {0, 1, 1}})});
    Presentation<word_type> p(to<Presentation<word_type>>(S));
    p.contains_empty_word(true);

    Sims2 T;
    BENCHMARK("n = 3, index 11, threads 1") {
      T.init(p);
      REQUIRE(T.number_of_threads(1).number_of_congruences(11) == 84);
    };
  }

  TEST_CASE("Jones monoid 2-sided", "[jones_monoid_2_sided]") {
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {0, 0, 0, 0, 14, 42, 132, 429};
    std::vector<size_t> num   = {0, 0, 0, 0, 9, 6, 10, 7};

    for (size_t n = 4; n < 8; ++n) {
      Sims2 S;
      auto  p = presentation::examples::temperley_lieb_monoid(n);
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        S.init(p);
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("Brauer monoid 2-sided", "[brauer_monoid_2_sided]") {
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {0, 0, 3, 15, 105};
    std::vector<size_t> num   = {0, 0, 3, 7, 19};

    for (size_t n = 2; n < 5; n += 2) {
      Sims2 S;
      auto  p = presentation::examples::brauer_monoid(n);
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        S.init(p);
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("Partition monoid 2-sided", "[partition_monoid_2_sided]") {
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {0, 0, 15, 203};
    std::vector<size_t> num   = {0, 0, 13, 16};

    {
      size_t n = 2;
      auto   S = make<FroidurePin>({Bipartition({{1, -2}, {2, -1}}),
                                    Bipartition({{1}, {2, -2}, {-1}}),
                                    Bipartition({{1, 2, -1, -2}})});
      REQUIRE(S.size() == sizes[2]);
      auto  p = to<Presentation<word_type>>(S);
      Sims2 T(p);
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        REQUIRE(T.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
    {
      size_t n = 3;
      auto   S = make<FroidurePin>({Bipartition({{1, -2}, {2, -3}, {3, -1}}),
                                    Bipartition({{1, -2}, {2, -1}, {3, -3}}),
                                    Bipartition({{1}, {2, -2}, {3, -3}, {-1}}),
                                    Bipartition({{1, 2, -1, -2}, {3, -3}})});
      REQUIRE(S.size() == sizes[n]);
      auto  p = to<Presentation<word_type>>(S);
      Sims2 T(p);
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        REQUIRE(T.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("full pbr monoid", "[full_pbr_2_sided]") {
    auto rg = ReportGuard(false);
    auto S  = make<FroidurePin>({PBR({{}}, {{1}}),
                                 PBR({{-1, 1}}, {{1}}),
                                 PBR({{-1}}, {{}}),
                                 PBR({{-1}}, {{1}}),
                                 PBR({{-1}}, {{-1, 1}})});

    size_t n = 1;
    REQUIRE(S.size() == 16);
    auto  p = to<Presentation<word_type>>(S);
    Sims2 T(p);
    BENCHMARK(fmt::format("n = {}, index {}, threads 1", n, 16).c_str()) {
      REQUIRE(T.number_of_threads(1).number_of_congruences(16) == 167);
    };
  }

  TEST_CASE("symmetric group", "[symmetric_group_2_sided]") {
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {1, 1, 2, 6, 24, 120};
    std::vector<size_t> num   = {0, 1, 2, 3, 4, 3};

    for (size_t n = 4; n < 6; ++n) {
      Sims2 S;
      auto  p = presentation::examples::symmetric_group_Car56(n);
      BENCHMARK(
          fmt::format("n = {} | index {} | threads 1", n, sizes[n]).c_str()) {
        S.init(p);
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("full transf. monoid 2-sided", "[full_transf_2_sided]") {
    using literals::    operator""_p;
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {0, 0, 4, 27, 256, 3125};
    std::vector<size_t> num   = {0, 0, 4, 7, 11, 14};

    // {
    //   size_t                    n = 2;
    //   Presentation<std::string> p;
    //   p.alphabet("ab");
    //   p.contains_empty_word(true);
    //   presentation::add_rule(p, "aa", "");
    //   presentation::add_rule(p, "ab", "b");
    //   presentation::add_rule(p, "bb", "b");
    //   Sims2 S;
    //   BENCHMARK(
    //       fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
    //     S.init(p);
    //     REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
    //             == num[n]);
    //   };
    // }

    {
      size_t                    n = 3;
      Presentation<std::string> p;
      p.alphabet("abc");
      p.contains_empty_word(true);
      presentation::add_rule(p, "b^2"_p, ""_p);
      presentation::add_rule(p, "bc"_p, "ac"_p);
      presentation::add_rule(p, "c^2"_p, "c"_p);
      presentation::add_rule(p, "a^3"_p, ""_p);
      presentation::add_rule(p, "a^2b"_p, "ba"_p);
      presentation::add_rule(p, "aba"_p, "b"_p);
      presentation::add_rule(p, "baa"_p, "ab"_p);
      presentation::add_rule(p, "bab"_p, "aa"_p);
      presentation::add_rule(p, "bac"_p, "c"_p);
      presentation::add_rule(p, "cac"_p, "cb"_p);
      presentation::add_rule(p, "aca^2c"_p, "ca^2c"_p);
      presentation::add_rule(p, "ca^2cb"_p, "ca^2ca"_p);
      presentation::add_rule(p, "ca^2cab"_p, "ca^2c"_p);
      Sims2 S;
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        S.init(to<Presentation<word_type>>(p));
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
    {
      size_t n = 4;
      auto   p = presentation::examples::full_transformation_monoid_II74(n);
      Sims2  S;
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        S.init(to<Presentation<word_type>>(p));
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("rook monoid 2-sided", "[rook_2_sided]") {
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {0, 2, 7, 34, 209};
    std::vector<size_t> num   = {0, 2, 4, 7, 11};

    for (size_t n = 3; n < 5; ++n) {
      auto  p = presentation::examples::symmetric_inverse_monoid_Sol04(n);
      Sims2 S;
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        S.init(p);
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("motzkin monoid 2-sided", "[motzkin_2_sided]") {
    auto                rg    = ReportGuard(false);
    std::vector<size_t> sizes = {0, 2, 9, 51, 323};
    std::vector<size_t> num   = {0, 2, 9, 10, 11};

    for (size_t n = 3; n < 5; ++n) {
      auto  p = presentation::examples::motzkin_monoid(n);
      Sims2 S;
      BENCHMARK(
          fmt::format("n = {}, index {}, threads 1", n, sizes[n]).c_str()) {
        S.init(p);
        REQUIRE(S.number_of_threads(1).number_of_congruences(sizes[n])
                == num[n]);
      };
    }
  }

  TEST_CASE("partial transf. monoid 2-sided", "[part_transf_2_sided]") {
    auto rg = ReportGuard(false);

    auto T = make<FroidurePin>({Transf<4>({1, 0, 2, 3}),
                                Transf<4>({2, 0, 1, 3}),
                                Transf<4>({3, 1, 2, 3}),
                                Transf<4>({1, 1, 2, 3})});
    REQUIRE(T.size() == 64);
    auto  p = to<Presentation<word_type>>(T);
    Sims2 S;
    BENCHMARK(fmt::format("n = {}, index {}, threads 1", 3, 64).c_str()) {
      S.init(p);
      REQUIRE(S.number_of_threads(1).number_of_congruences(64) == 7);
    };
  }

  TEST_CASE("partial brauer monoid 2-sided", "[part_brauer_2_sided]") {
    auto rg = ReportGuard(false);

    auto  p = presentation::examples::partial_brauer_monoid(3);
    Sims2 S;
    BENCHMARK(fmt::format("n = {}, index {}, threads 1", 3, 76).c_str()) {
      S.init(p);
      REQUIRE(S.number_of_threads(1).number_of_congruences(76) == 16);
    };
  }

}  // namespace libsemigroups
