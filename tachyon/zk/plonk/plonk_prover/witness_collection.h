// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_PLONK_PROVER_WITNESS_COLLECTION_H_
#define TACHYON_ZK_PLONK_PLONK_PROVER_WITNESS_COLLECTION_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/memory/memory.h"
#include "absl/numeric/bits.h"

#include "tachyon/base/containers/container_util.h"
#include "tachyon/base/range.h"
#include "tachyon/math/polynomials/univariate/univariate_evaluations.h"
#include "tachyon/zk/base/ref.h"
#include "tachyon/zk/plonk/circuit/assignment.h"
#include "tachyon/zk/plonk/circuit/phase.h"
#include "tachyon/zk/plonk/constraint_system.h"

namespace tachyon::zk {

// Instance for a circuit.
template <typename PCSTy>
class WitnessCollection : public Assignment<typename PCSTy::Field> {
 public:
  constexpr static size_t kMaxDegree = PCSTy::kMaxDegree;
  constexpr static size_t kDomainSize = kMaxDegree + 1;

  using F = typename PCSTy::Field;
  using Evals = typename PCSTy::Evals;
  using Poly = typename PCSTy::Poly;
  using Domain = typename PCSTy::Domain;
  using RationalEvals =
      typename math::UnivariateEvaluations<math::RationalField<F>, kMaxDegree>;
  using AssignCallback = typename Assignment<F>::AssignCallback;

  WitnessCollection() = default;
  WitnessCollection(size_t k, size_t num_advice_columns,
                    size_t unusable_row_start, const Phase& current_phase,
                    Ref<absl::flat_hash_map<size_t, F>> challenges,
                    const std::vector<std::vector<F>>& instances)
      : k_(k),
        advices_(base::CreateVector(num_advice_columns,
                                    RationalEvals::UnsafeZero(kMaxDegree))),
        usable_rows_(base::Range<size_t>::Until(unusable_row_start)),
        current_phase_(current_phase),
        challenges_(challenges),
        instances_(instances) {}

  // TODO(dongchangYoo): add |CircuitTy::Config config| to input argument.
  template <typename CircuitTy>
  static std::unique_ptr<WitnessCollection> Synthesized(
      Phase current_phase, CircuitTy& circuit, const ConstraintSystem<F>& meta,
      Ref<absl::flat_hash_map<size_t, F>> challenge,
      const std::vector<std::vector<F>>& instances) {
    size_t k = absl::bit_ceil(kDomainSize);
    size_t num_advice_columns = meta.num_advice_columns();

    // The prover will not be allowed to assign values to advice
    // cells that exist within inactive rows, which include some
    // number of blinding factors and an extra row for use in the
    // permutation argument.
    size_t unusable_rows_start = kMaxDegree - meta.ComputeBlindingFactors();

    std::unique_ptr<WitnessCollection> witness = absl::WrapUnique(
        new WitnessCollection(k, num_advice_columns, unusable_rows_start,
                              current_phase, challenge, instances));

    // TODO(dongchangYoo): uncomment this line.
    // CircuitTy::Synthesize(witness, circuit, config, meta.constants());

    return std::move(witness);
  }

  // Do nothing; we don't care about regions in this context.
  void EnterRegion(std::string_view) {}

  // Do nothing
  void NameColumn(std::string_view, const AnyColumn&) {}

  // Do nothing; we don't care about regions in this context.
  void ExitRegion() {}

  // We only care about advice columns here
  Error EnableSelector(std::string_view, const Selector&, size_t) {
    return Error::kNone;
  }

  Error QueryInstance(const InstanceColumn& column, size_t row,
                      Value<F>* instance) {
    if (!usable_rows_.Contains(row)) {
      return Error::kNotEnoughRowsAvailable;
    }
    if (column.index() >= instances_.size()) return Error::kBoundsFailure;

    *instance = Value<F>::Known(instances_[column.index()][row]);

    return Error::kNone;
  }

  Error AssignAdvice(std::string_view, const AdviceColumn& column, size_t row,
                     AssignCallback assign) {
    // Ignore assignment of advice column in different phase than current one.
    if (current_phase_.value() < column.phase().value()) return Error::kNone;

    if (!usable_rows_.Contains(row)) return Error::kNotEnoughRowsAvailable;
    if (column.index() >= advices_.size()) return Error::kBoundsFailure;

    *advices_[column.index()][row] = std::move(assign).Run();

    return Error::kNone;
  }

  // We only care about advice columns here
  Error AssignFixed(std::string_view, const FixedColumn&, size_t,
                    AssignCallback) {
    return Error::kNone;
  }

  // We only care about advice columns here
  Error Copy(const AnyColumn&, size_t, const AnyColumn&, size_t) {
    return Error::kNone;
  }

  Error FillFromRow(const FixedColumn&, size_t, AssignCallback) {
    return Error::kNone;
  }

  Value<F> GetChallenge(const Challenge& challenge) {
    if (challenge.index() >= challenges_->size()) return Value<F>::Unknown();
    return Value<F>::Known((*challenges_)[challenge.index()]);
  }

  // Do nothing; we don't care about namespaces in this context.
  void PushNamespace(std::string_view name) {}

  // Do nothing; we don't care about namespaces in this context.
  void PopNamespace(const std::optional<std::string>& gadget_name) {}

  size_t k() const { return k_; }
  const std::vector<RationalEvals>& advices() const { return advices_; }
  const Phase current_phase() const { return current_phase_; }
  const base::Range<size_t>& usable_rows() const { return usable_rows_; }
  const absl::flat_hash_map<size_t, F>& challenges() const {
    return challenges_;
  }
  const std::vector<std::vector<F>>& instances() const { return instances_; }

 private:
  size_t k_ = 0;
  std::vector<RationalEvals> advices_;
  base::Range<size_t> usable_rows_;
  Phase current_phase_;
  Ref<absl::flat_hash_map<size_t, F>> challenges_;
  std::vector<std::vector<F>> instances_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PLONK_PROVER_WITNESS_COLLECTION_H_
