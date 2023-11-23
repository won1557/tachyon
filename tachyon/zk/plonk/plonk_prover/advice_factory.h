#ifndef TACHYON_ZK_PLONK_PLONK_PROVER_ADVICE_FACTORY_H_
#define TACHYON_ZK_PLONK_PLONK_PROVER_ADVICE_FACTORY_H_

#include "tachyon/zk/base/prover.h"
#include "tachyon/zk/plonk/plonk_prover/advice_single.h"
#include "tachyon/zk/plonk/plonk_prover/witness_collection.h"

namespace tachyon::zk {

template <typename PCSTy>
class AdviceFactory {
 public:
  constexpr static size_t kMaxDegree = PCSTy::kMaxDegree;
  constexpr static size_t kDomainSize = kMaxDegree + 1;

  using F = typename PCSTy::Field;
  using Poly = typename PCSTy::Poly;
  using Evals = typename PCSTy::Evals;
  using RationalEvals =
      typename math::UnivariateEvaluations<math::RationalField<F>, kMaxDegree>;

  AdviceFactory() = default;
  AdviceFactory(size_t num_circuits, size_t num_advice_columns,
                size_t num_challenges) {
    advice_singles_.resize(num_circuits);
    challenges_.reserve(num_challenges);
  }

  // NOTE(dongchangYoo): instances related to each circuit.
  template <typename CircuitTy>
  Error DoStep(Prover<PCSTy>* prover, size_t circuit_idx, Phase current_phase,
               WitnessCollection<PCSTy> witness,
               const ConstraintSystem<F> meta) {
    CHECK_LT(circuit_idx, advice_singles_.size());

    std::vector<size_t> selected_advice_column_indices;
    ParseIndices(current_phase, meta.advice_column_phases(),
                 &selected_advice_column_indices);

    std::vector<Evals> advice_evals_vec;
    SelectiveBatchEvaluate(selected_advice_column_indices, witness.advices(),
                           &advice_evals_vec);

    for (size_t i = 0; i < advice_evals_vec.size(); ++i) {
      // Add blinding factors to advice columns
      *advice_evals_vec[i][kMaxDegree] = F(1);

      BlindedPolynomial<Poly> blinded_poly;
      prover->CommitEvalsWithBlind(advice_evals_vec[i], &blinded_poly);

      size_t effective_idx = selected_advice_column_indices[i];
      advice_singles_[circuit_idx].SetBlindedPolynomial(effective_idx,
                                                        blinded_poly);
    }

    std::vector<size_t> selected_challenge_indices;
    ParseIndices(current_phase, meta.challenge_phases(),
                 &selected_challenge_indices);
    for (size_t i = 0; i < selected_challenge_indices.size(); ++i) {
      CHECK(challenges_.contains(selected_challenge_indices[i]));
      challenges_[i] = prover->writer()->SqueezeChallenge();
    }
  }

  static void ParseIndices(Phase current_phase, std::vector<Phase>& phases,
                           std::vector<size_t>* indices) {
    std::vector<size_t> ret;
    for (size_t i = 0; i < phases.size(); ++i) {
      if (phases[i] == current_phase) {
        ret.push_back(i);
      }
    }
    *indices = std::move(ret);
  }

  const std::vector<AdviceSingle<PCSTy>>& advice_singles() const {
    return advice_singles_;
  }
  const std::vector<F>& challenges() const { return challenges_; }

 private:
  // Extract the |RationalEvals| corresponding to the indices contained in
  // |column_indices| from |rational_evals_vector|.
  // Then returns result of evaluating them all at once, as a vector of |Evals|.
  static bool SelectiveBatchEvaluate(
      std::vector<size_t>& column_indices,
      std::vector<RationalEvals>& rational_evals_vector,
      std::vector<Evals>* results) {
    std::vector<Ref<const RationalEvals>> selective_rational_evals;
    selective_rational_evals.reserve(column_indices.size());

    for (size_t i = 0; i < column_indices.size(); ++i) {
      size_t selective_index = column_indices[i];
      selective_rational_evals.emplace_back(
          &rational_evals_vector[selective_index]);
    }

    return BatchEvaluate(selective_rational_evals, results);
  }

  // Returns result of evaluating |rational_evals_vector| all at once,
  // as a vector of |Evals|.
  static bool BatchEvaluate(
      std::vector<Ref<const RationalEvals>>& rational_evals_vector,
      std::vector<Evals>* results, const F& coeff = F::One()) {
    // Merge rational evaluations.
    std::vector<math::RationalField<F>> batch;
    for (const Ref<const RationalEvals>& rationals_evals :
         rational_evals_vector) {
      const std::vector<math::RationalField<F>>& evals =
          rationals_evals->evaluations();
      batch.insert(batch.end(), evals.begin(), evals.end());
    }

    // Evaluate every rational field elements in the merged vector.
    std::vector<F> evaluated;
    evaluated.resize(rational_evals_vector.size() * kDomainSize);
    if (!math::RationalField<F>::BatchEvaluate(batch, &evaluated, coeff))
      return false;

    // Split evaluated values to each |Evals|.
    std::vector<Evals> results_tmp;
    results_tmp.reserve(rational_evals_vector.size());
    for (size_t i = 0; i < rational_evals_vector.size(); ++i) {
      std::vector<F> evals;
      evals.insert(evals.end(), evaluated.begin() + i * kDomainSize,
                   evaluated.begin() + (i + 1) * kDomainSize);
      results_tmp.emplace_back(std::move(evals));
    }
    *results = std::move(results_tmp);
    return true;
  }

  std::vector<AdviceSingle<PCSTy>> advice_singles_;
  absl::flat_hash_map<size_t, F> challenges_;
};
}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PLONK_PROVER_ADVICE_FACTORY_H_
