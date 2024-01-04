// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#include "tachyon/zk/plonk/prover/synthesizer.h"

#include <memory>

#include "gtest/gtest.h"

#include "tachyon/zk/plonk/circuit/examples/simple_circuit.h"
#include "tachyon/zk/plonk/circuit/floor_planner/simple_floor_planner.h"
#include "tachyon/zk/plonk/halo2/pinned_verifying_key.h"
#include "tachyon/zk/plonk/halo2/prover_test.h"

namespace tachyon::zk {
namespace {

class SynthesizerTest : public halo2::ProverTest {
 public:
  using F = typename PCS::Field;

  void SetUp() override {
    halo2::ProverTest::SetUp();

    circuits_ = {SimpleCircuit<F, SimpleFloorPlanner>(),
                 SimpleCircuit<F, SimpleFloorPlanner>()};

    CHECK(verifying_key_.Load(prover_.get(), circuits_[0]));

    synthesizer_ =
        Synthesizer<PCS>(circuits_.size(), &verifying_key_.constraint_system());
  }

 protected:
  std::vector<SimpleCircuit<F, SimpleFloorPlanner>> circuits_;
  VerifyingKey<PCS> verifying_key_;
  Synthesizer<PCS> synthesizer_;
};

}  // namespace

// TODO(dongchangYoo): it should be verified if it produces the expected values.
TEST_F(SynthesizerTest, GenerateAdviceColumns) {
  std::vector<std::vector<Evals>> instance_columns_vec =
      base::CreateVector(circuits_.size(), [this]() {
        return base::CreateVector(1, prover_->domain()->Random<Evals>());
      });
  synthesizer_.GenerateAdviceColumns(prover_.get(), circuits_,
                                     instance_columns_vec);

  std::vector<F> challenges = synthesizer_.ExportChallenges();
}

}  // namespace tachyon::zk
