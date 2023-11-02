// Copyright 2022 arkworks contributors
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.arkworks and the LICENCE-APACHE.arkworks
// file.

#ifndef TACHYON_MATH_POLYNOMIALS_MULTIVARIATE_MULTILINEAR_SPARSE_EVALUATIONS_H_
#define TACHYON_MATH_POLYNOMIALS_MULTIVARIATE_MULTILINEAR_SPARSE_EVALUATIONS_H_

#include <stddef.h>

#include <string>
#include <utility>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/flat_hash_map.h"
#include "absl/random/random.h"

#include "tachyon/base/bits.h"
#include "tachyon/base/containers/container_util.h"
#include "tachyon/base/logging.h"
#include "tachyon/base/strings/string_util.h"
#include "tachyon/math/polynomials/multivariate/support_poly_operators.h"

namespace tachyon::math {

template <typename F, std::size_t MaxDegree, std::size_t NumVars>
class MultilinearSparseEvaluations {
 public:
  static constexpr std::size_t kMaxDegree = MaxDegree;
  static constexpr std::size_t kNumVars = NumVars;

  using Field = F;

  MultilinearSparseEvaluations() : zero_(F::Zero()) {
    evaluations_ = {{0, F::Zero()}};
  }

  // MultilinearSparseEvaluations() : zero_(num_vars_(NumVars), F::Zero()), {
  //   evaluations_ = {{0, F::Zero()}};
  // }

  MultilinearSparseEvaluations(
      const std::vector<std::pair<std::size_t, F>>& evaluations)
      : evaluations_(evaluations.begin(), evaluations.end()) {
    CHECK_LE(NumVars, MaxDegree);
  }

  MultilinearSparseEvaluations(
      std::vector<std::pair<std::size_t, F>>&& evaluations)
      : evaluations_(std::move(evaluations.begin()),
                     std::move(evaluations.end())) {
    CHECK_LE(NumVars, MaxDegree);
  }

  static MultilinearSparseEvaluations Zero(std::size_t degree) {
    std::vector<std::pair<std::size_t, F>> evaluations;
    for (std::size_t i = 0; i < (static_cast<std::size_t>(1) << degree); ++i) {
      evaluations.push_back({i, F::Zero()});
    }
    return MultilinearSparseEvaluations(evaluations);
  }

  static MultilinearSparseEvaluations One(std::size_t degree) {
    std::vector<std::pair<std::size_t, F>> evaluations;
    for (std::size_t i = 0; i < (static_cast<std::size_t>(1) << degree); ++i) {
      evaluations.push_back({i, F::One()});
    }
    return MultilinearSparseEvaluations(evaluations);
  }

  static MultilinearSparseEvaluations Random(std::size_t num_vars,
                                             absl::BitGen& bitgen) {
    return RandWithConfig(
        num_vars, static_cast<std::size_t>(1) << (num_vars / 2), bitgen);
  }

  bool operator==(const MultilinearSparseEvaluations& other) const {
    return evaluations_ == other.evaluations_;
  }

  bool operator!=(const MultilinearSparseEvaluations& other) const {
    return !operator==(other);
  }

  const F* Get(std::size_t i) const {
    auto it = evaluations_.find(i);
    if (it != evaluations_.end()) {
      return &(it->second);
    }
    return nullptr;
  }

  bool IsZero() const {
    return std::all_of(evaluations_.begin(), evaluations_.end(),
                       [](const std::pair<std::size_t, F>& pair) {
                         return pair.second.IsZero();
                       });
  }

  bool IsOne() const {
    return std::all_of(evaluations_.begin(), evaluations_.end(),
                       [](const std::pair<std::size_t, F>& pair) {
                         return pair.second.IsOne();
                       });
  }

  std::size_t Degree() const {
    if (evaluations_.empty()) {
      return 0;
    }

    std::size_t maxKey = 0;
    for (const auto& pair : evaluations_) {
      maxKey = std::max(maxKey, pair.first);
    }

    return base::bits::SafeLog2Ceiling(maxKey);
  }

  absl::btree_map<std::size_t, F> tuples_to_treemap(
      const std::vector<std::pair<std::size_t, F>>& tuples) {
    absl::btree_map<std::size_t, F> result;
    for (const auto& entry : tuples) {
      result[entry.first] = entry.second;
    }
    return result;
  }

  static MultilinearSparseEvaluations RandWithConfig(
      std::size_t num_vars, std::size_t num_nonzero_entries,
      absl::BitGen& bitgen) {
    assert(num_nonzero_entries <= (static_cast<std::size_t>(1) << num_vars));

    absl::flat_hash_map<std::size_t, F> map;

    for (std::size_t i = 0; i < num_nonzero_entries; ++i) {
      std::size_t index;
      do {
        index = absl::Uniform(bitgen, static_cast<std::size_t>(0),
                              static_cast<std::size_t>(1) << num_vars);
      } while (map.find(index) != map.end());
      map[index] = F::Random();
    }

    MultilinearSparseEvaluations result(
        std::vector<std::pair<std::size_t, F>>(map.begin(), map.end()));

    return result;
  }

  F Evaluate(const std::vector<F>& point) const {
    CHECK_EQ(point.size(), num_vars_);

    MultilinearSparseEvaluations fixed = FixVariables(point);

    if (fixed.IsZero()) return F::Zero();
    return fixed.evaluations_[0];
  }

  // std::string ToString() const {
  //   if (evaluations_.empty()) return "Empty Map";

  //   std::stringstream ss;
  //   bool has_entry = false;
  //   for (const auto& entry : evaluations_) {
  //     if (has_entry) ss << ", ";
  //     has_entry = true;
  //     ss << "Key: " << entry.first << ", Value: " << entry.second;
  //   }
  //   return ss.str();
  // }

  std::string ToString() const {
    std::string result = "[";
    bool firstEntry = true;  // 첫 번째 항목인지 여부를 추적

    for (const auto& entry : evaluations_) {
      if (!firstEntry) {
        result += ", ";  // 첫 번째 항목이 아닌 경우에 쉼표를 추가
      }
      result += "(" + std::to_string(entry.first) + ", " +
                entry.second.ToString() + ")";
      firstEntry = false;  // 첫 번째 항목 처리 후에는 false로 설정
    }

    result += "]";
    return result;
  }

 private:
  friend class internal::MultilinearExtensionOp<
      MultilinearSparseEvaluations<F, MaxDegree, NumVars>>;

  absl::btree_map<std::size_t, F> evaluations_;
  std::size_t num_vars_;
  F zero_ = F::Zero();
};

}  // namespace tachyon::math
#endif  // TACHYON_MATH_POLYNOMIALS_MULTIVARIATE_MULTILINEAR_SPARSE_EVALUATIONS_H_
