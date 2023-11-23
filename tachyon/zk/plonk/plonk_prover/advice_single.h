// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_PLONK_PROVER_ADVICE_SINGLE_H_
#define TACHYON_ZK_PLONK_PLONK_PROVER_ADVICE_SINGLE_H_

#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"

#include "tachyon/base/logging.h"
#include "tachyon/zk/base/blinded_polynomial.h"

namespace tachyon::zk {

// Instance for a circuit.
template <typename PCSTy>
class AdviceSingle {
 public:
  constexpr static size_t kMaxDegree = PCSTy::kMaxDegree;
  constexpr static size_t kDomainSize = kMaxDegree + 1;

  using F = typename PCSTy::Field;
  using Evals = typename PCSTy::Evals;
  using Poly = typename PCSTy::Poly;

  AdviceSingle() = default;
  explicit AdviceSingle(size_t num_advice_columns) {
    advice_polys_.resize(num_advice_columns);
    advice_blinds_.resize(num_advice_columns);
    assigned_.reserve(num_advice_columns);
  }

  void SetBlindedPolynomial(size_t column_idx,
                            BlindedPolynomial<Poly>& blinded_polynomial) {
    CHECK_LT(column_idx, advice_polys_.size());
    CHECK(!assigned_.contains(column_idx));
    advice_polys_[column_idx] = std::move(blinded_polynomial.poly());
    advice_blinds_[column_idx] = std::move(blinded_polynomial.blind());
    assigned_[column_idx] = true;
  }

  bool IsAssigned(size_t column_idx) const {
    CHECK_LT(column_idx, advice_polys_.size());
    return assigned_.contains(column_idx);
  }

  const std::vector<Poly>& advice_polys() const {
    return const_cast<const std::vector<Poly>&>(advice_polys_);
  }
  const std::vector<F>& advice_blinds() const {
    return const_cast<const std::vector<F>&>(advice_blinds_);
  }

 private:
  std::vector<Poly> advice_polys_;
  std::vector<F> advice_blinds_;
  absl::flat_hash_map<size_t, bool> assigned_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PLONK_PROVER_ADVICE_SINGLE_H_
