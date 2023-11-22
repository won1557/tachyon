// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#include "tachyon/zk/plonk/plonk_prover/witness_collection.h"

#include <memory>

#include "gtest/gtest.h"

#include "tachyon/zk/base/halo2_prover_test.h"
#include "tachyon/zk/plonk/circuit/examples/simple_circuit.h"

namespace tachyon::zk {
namespace {

class WitnessCollectionTest : public Halo2ProverTest {
 public:
  using F = typename PCS::Field;

  void SetUp() override {
    Halo2ProverTest::SetUp();

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
};

}  // namespace

TEST_F(WitnessCollectionTest, QueryInstance) {
  // Query a value in specific instance column.
  Value<F> instance_value;
  witness_collection_.QueryInstance(InstanceColumn(0), 10, &instance_value);
  EXPECT_EQ(instance_value, Value<F>::Known(expected_instance_[0][10]));

  // Out of bound case.
  EXPECT_EQ(
      witness_collection_.QueryInstance(InstanceColumn(1), 10, &instance_value),
      Error::kBoundsFailure);
}

TEST_F(WitnessCollectionTest, AssignAdvice) {
  math::RationalField<F> expected_value(F::Random());
  size_t target_column_index = 0;
  size_t target_row = 10;

  // Assign a random value to specific cell.
  Error result = witness_collection_.AssignAdvice(
      "", AdviceColumn(target_column_index), target_row,
      [&expected_value]() { return expected_value; });
  EXPECT_EQ(result, Error::kNone);
  EXPECT_EQ(expected_value,
            *witness_collection_.advices()[target_column_index][target_row]);

  // Out of bound case.
  EXPECT_EQ(witness_collection_.AssignAdvice(
                "", AdviceColumn(0), kUsableRows,
                [&expected_value]() { return expected_value; }),
            Error::kNotEnoughRowsAvailable);
}

TEST_F(WitnessCollectionTest, GetChallenge) {
  // Get challenge value.
  size_t target_idx = 0;
  Value<F> challenge =
      witness_collection_.GetChallenge(Challenge(target_idx, kFirstPhase));
  EXPECT_EQ(Value<F>::Known(expected_challenge_), challenge);
}

// TODO(dongchangYoo): complete test after |Synthesize()| implemented.
TEST_F(WitnessCollectionTest, Synthesized) {
  ConstraintSystem<F> constraint_system;

  Phase current_phase = Phase(0);

  std::vector<std::vector<F>> instance_vector = base::CreateVector(3, []() {
    return std::vector<F>({F::Random(), F::Random(), F::Random()});
  });

  SimpleCircuit<F> circuit;
  // FieldConfig<F> config = SimpleCircuit<F>::Configure(constraint_system);

  std::unique_ptr<WitnessCollection<PCS>> witness =
      WitnessCollection<PCS>::Synthesized<SimpleCircuit<F>>(
          current_phase, circuit, constraint_system,
          Ref<absl::flat_hash_map<size_t, F>>(&challenges_), instance_vector);
}

}  // namespace tachyon::zk
