#ifndef TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_VERIFIER_H_
#define TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_VERIFIER_H_

#include <algorithm>
#include <cstdint>
#include <vector>

// #include "tachyon/math/polynomials/multivariate/multilinear_extension.h"
#include "tachyon/math/polynomials/sumcheck/ml_prover.h"

namespace tachyon::math {

template <typename F>
struct VerifierMsg {
  F randomness;
};

template <typename F>
struct VerifierState {
  size_t round;
  size_t num_vars;
  size_t max_multiplicands;
  std::vector<std::vector<F>> polynomials_received;
  std::vector<F> randomness;
  bool finished;
  bool error_flag;
};

template <typename F>
struct SubClaim {
  std::vector<F> point;
  F expected_evaluation;
};

template <typename F, size_t kMaxDegree>
class IPForMLSumcheck {
 public:
  static VerifierState<F> verifier_init(const PolynomialInfo& index_info) {
    VerifierState<F> verifier_state{1,
                                    index_info.num_vars(),
                                    index_info.max_multiplicands(),
                                    index_info.max_multiplicands(),
                                    std::vector<F>(),
                                    std::vector<F>(),
                                    0,
                                    false};

    return prover_state;
  }

  static std::optional<VerifierMsg<F>> verify_round(
      const ProverMsg<F>&, VerifierState<F>& verifier_state) {
    if (verifier_state.finished) {
      return std::nullopt;
    }

    if (verifier_state.round == verifier_state.nv) {
      verifier_state.finished = true;
    } else {
      verifier_state.round += 1;
    }

    return sample_round();
  }

  static Result<SubClaim<F>, std::string> check_and_generate_subclaim(
      VerifierState<F> verifier_state, F asserted_sum) {
    return SubClaim<F>{/*...*/};
  }

};  // namespace tachyon::math
}  // namespace tachyon::math
#endif  // TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_VERIFIER_H_
