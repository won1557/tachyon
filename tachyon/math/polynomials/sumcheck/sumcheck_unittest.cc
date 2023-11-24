#include "gtest/gtest.h"

#include "tachyon/math/finite_fields/test/gf7.h"
#include "tachyon/math/polynomials/multivariate/multilinear_dense_evaluations.h"
#include "tachyon/math/polynomials/multivariate/multilinear_extension.h"
#include "tachyon/math/polynomials/sumcheck/data_structures.h"
#include "tachyon/math/polynomials/sumcheck/ml_prover.h"
// #include "tachyon/math/polynomials/sumcheck/ml_sumcheck.h"

namespace tachyon::math {
namespace {

class SumcheckMLProverTest : public ::testing::Test {
 public:
  static constexpr size_t kMaxDegree = 4;

  static void SetUpTestSuite() { GF7Config::Init(); }

  void SetUp() override {}

 protected:
  void TearDown() override {}
};

TEST_F(SumcheckMLProverTest, ProverInitTest) {
  ListOfProductsOfPolynomials<GF7, kMaxDegree> polynomial(kMaxDegree);

  ProverState<GF7, kMaxDegree> prover_state =
      IPForMLSumcheck<GF7, kMaxDegree>::ProverInit(polynomial);

  ASSERT_FALSE(prover_state.error_flag);
  ASSERT_EQ(prover_state.num_vars, polynomial.num_variables());
}

// TEST_F(SumcheckMLProverTest, ProveRoundTest) {
//   ListOfProductsOfPolynomials<GF7, kMaxDegree> polynomial(kMaxDegree);

//   VerifierMsg<GF7> v_msg;

//   ProverState<GF7, kMaxDegree> prover_state =
//       IPForMLSumcheck<GF7, kMaxDegree>::prover_init(polynomial);

//   ProverMsg<GF7> prover_msg = IPForMLSumcheck<GF7, kMaxDegree>::prove_round(
//       prover_state, std::optional<VerifierMsg<GF7>>(v_msg));

//   ASSERT_FALSE(prover_state.error_flag);
// }


}  // namespace
}  // namespace tachyon::math
