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
  std::vector<F> randomness;
};

template <typename F>
struct VerifierState {
  size_t round;
  size_t nv;
  size_t max_multiplicands;
  bool finished;
  std::vector<std::vector<F>> polynomials_received;
  std::vector<F> randomness;
};

template <typename F>
struct SubClaim {
  std::vector<F> point;
  F expected_evaluation;
};

template <typename F>
class IPForMLSumcheck {
 public:
  VerifierState<F> VerifierInit(const PolynomialInfo& index_info) {
    return VerifierState<F>{
        .round = 1,
        .nv = index_info.num_variables_,
        .max_multiplicands = index_info.max_multiplicands_,
        .finished = false,
        .polynomials_received = {},
        .randomness = {},
    };
  }

  static VerifierMsg<F> VerifyRound(
      const std::vector<F>& prover_msg_evaluations,
      VerifierState<F>& verifier_state) {
    if (verifier_state.finished) {
      throw std::runtime_error(
          "Incorrect verifier state: Verifier is already finished.");
    }

    VerifierMsg<F> msg = SampleRound();
    verifier_state.randomness.push_back(msg.randomness);
    verifier_state.polynomials_received.push_back(prover_msg_evaluations);

    if (verifier_state.round == verifier_state.nv) {
      verifier_state.finished = true;
    } else {
      verifier_state.round += 1;
    }
    return msg;
  }

  SubClaim<F> CheckAndGenerateSubclaim(
      const VerifierState<F>& verifier_state, F asserted_sum) {
    if (!verifier_state.finished) {
      throw std::runtime_error("Verifier has not finished.");
    }

    F expected = asserted_sum;
    if (verifier_state.polynomials_received.size() != verifier_state.nv) {
      throw std::runtime_error("Insufficient rounds");
    }

    for (size_t i = 0; i < verifier_state.nv; ++i) {
      const auto& evaluations = verifier_state.polynomials_received[i];
      if (evaluations.size() != verifier_state.max_multiplicands + 1) {
        throw std::runtime_error("Incorrect number of evaluations");
      }

      F p0 = evaluations[0];
      F p1 = evaluations[1];
      if (p0 + p1 != expected) {
        throw std::runtime_error(
            "Prover message is not consistent with the claim.");
      }

      expected =
         InterpolateUniPoly(evaluations, verifier_state.randomness[i]);
    }

    return SubClaim<F>{
        .point = verifier_state.randomness,
        .expected_evaluation = expected,
    };
  }

  static VerifierMsg<F> SampleRound() {
    return VerifierMsg<F>{
        .randomness = F::rand(),
    };
  }
};
}  // namespace tachyon::math

#endif  // TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_VERIFIER_H_
