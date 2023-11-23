// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#include "tachyon/zk/plonk/plonk_prover/advice_single.h"

#include "gtest/gtest.h"

#include "tachyon/base/containers/container_util.h"
#include "tachyon/zk/base/halo2_prover_test.h"

namespace tachyon::zk {
namespace {

class AdviceSingleTest : public Halo2ProverTest {
 public:
  using F = typename PCS::Field;
  using Poly = typename PCS::Poly;
};

}  // namespace

TEST_F(AdviceSingleTest, SetBlindedPolynomial) {
  size_t num_advice_columns = 3;

  AdviceSingle<PCS> advice_single(num_advice_columns);
  EXPECT_EQ(advice_single.advice_polys().size(), num_advice_columns);
  EXPECT_EQ(advice_single.advice_blinds().size(), num_advice_columns);

  EXPECT_FALSE(advice_single.IsAssigned(0));
  EXPECT_FALSE(advice_single.IsAssigned(1));
  EXPECT_FALSE(advice_single.IsAssigned(2));

  Poly poly = Poly::Random(kMaxDegree);
  F blind = F::Random();
  BlindedPolynomial<Poly> blinded_polynomial(poly, blind);
  advice_single.SetBlindedPolynomial(1, blinded_polynomial);
  EXPECT_FALSE(advice_single.IsAssigned(0));

  EXPECT_TRUE(advice_single.IsAssigned(1));
  EXPECT_EQ(advice_single.advice_polys()[1], poly);
  EXPECT_EQ(advice_single.advice_blinds()[1], blind);

  EXPECT_FALSE(advice_single.IsAssigned(2));
}

}  // namespace tachyon::zk
