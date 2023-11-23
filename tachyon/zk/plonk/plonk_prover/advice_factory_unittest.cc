// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#include "tachyon/zk/plonk/plonk_prover/advice_factory.h"

#include <memory>

#include "gtest/gtest.h"

#include "tachyon/zk/base/halo2_prover_test.h"

namespace tachyon::zk {
namespace {

class AdviceSingleFactoryTest : public Halo2ProverTest {
 public:
  constexpr static size_t kMaxDegree = PCS::kMaxDegree;
  constexpr static size_t kDomainSize = kMaxDegree + 1;

  using F = typename PCS::Field;
  using Evals = typename PCS::Evals;
  using RationalEvals =
      typename math::UnivariateEvaluations<math::RationalField<F>, kMaxDegree>;

  void SetUp() override {
    // There is a single challenge in |challenges_|.
    expected_challenge_ = F::Random();
    challenges_[0] = expected_challenge_;

    // There is a single instance column in |expected_instance_|.
    expected_instance_ = {base::CreateVector(kDomainSize, F::Random())};

    Phase current_phase(0);
    witness_collection_ = WitnessCollection<PCS>(
        5, 3, kUsableRows, current_phase,
        Ref<absl::flat_hash_map<size_t, F>>(&challenges_), expected_instance_);
  }

 protected:
  F expected_challenge_;
  absl::flat_hash_map<size_t, F> challenges_;
  std::vector<std::vector<F>> expected_instance_;
  WitnessCollection<PCS> witness_collection_;

  size_t num_advice_columns_ = 6;
};

}  // namespace

TEST_F(AdviceSingleFactoryTest, SelectiveBatchEvaluate) {
  std::vector<Phase> phases = {
      Phase(0), Phase(1), Phase(0), Phase(1), Phase(0), Phase(1),
  };

  std::vector<Ref<const RationalEvals>> random_evals_ptrs;
  std::vector<RationalEvals> random_evals = base::CreateVector(
      num_advice_columns_, []() { return RationalEvals::Random(); });
  for (const RationalEvals& evals : random_evals) {
    random_evals_ptrs.push_back(Ref(&evals));
  }

  // Ensure that only the indices of phase 0 column are stored in |indices0|.
  std::vector<size_t> indices0;
  AdviceSingleFactory<PCS>::ParseIndices(Phase(0), phases, &indices0);

  // Collect only the |RationalEvals| with phase 0, then evaluate all at once.
  std::vector<Evals> results;
  AdviceSingleFactory<PCS>::SelectiveBatchEvaluate(indices0, random_evals,
                                                   &results);
  EXPECT_EQ(results.size(), 3);

  // Check if the results of evaluating each |Evals| individually are the same
  // as the result of evaluating all at once.
  std::vector<F> eval;
  eval.resize(kDomainSize);
  math::RationalField<F>::BatchEvaluate(random_evals[0].evaluations(), &eval);
  EXPECT_EQ(eval, results[0].evaluations());

  math::RationalField<F>::BatchEvaluate(random_evals[2].evaluations(), &eval);
  EXPECT_EQ(eval, results[1].evaluations());

  math::RationalField<F>::BatchEvaluate(random_evals[4].evaluations(), &eval);
  EXPECT_EQ(eval, results[2].evaluations());
}

TEST_F(AdviceSingleFactoryTest, Generate) {
  Phase current_phase(0);
  ConstraintSystem<F> constraint_system;

  AdviceSingleFactory<PCS> factory(1, num_advice_columns_, 1);
  AdviceSingleFactory<PCS>::DoStep(prover_.get(), 0, current_phase,
                                   witness_collection_, meta);

}  // namespace tachyon::zk
