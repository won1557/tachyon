// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#include "tachyon/zk/plonk/plonk_prover/instance_single.h"

#include <memory>

#include "gtest/gtest.h"

#include "tachyon/base/containers/container_util.h"
#include "tachyon/zk/base/halo2_prover_test.h"

namespace tachyon::zk {
namespace {

class InstanceSingleTest : public Halo2ProverTest {};

}  // namespace

TEST_F(InstanceSingleTest, Generate) {
  // Prepare instance for a circuit
  std::vector<std::vector<F>> instance_vector = base::CreateVector(3, []() {
    return std::vector<F>({F::Random(), F::Random(), F::Random()});
  });

  InstanceSingle<PCS> instance_singles;
  Error result = InstanceSingle<PCS>::Generate(prover_.get(), instance_vector,
                                               &instance_singles, true);
  EXPECT_EQ(result, Error::kNone);

  InstanceSingle<PCS> instance_singles2;
  Error result2 = InstanceSingle<PCS>::Generate(prover_.get(), instance_vector,
                                                &instance_singles2, false);
  EXPECT_EQ(result2, Error::kNone);
}

}  // namespace tachyon::zk
