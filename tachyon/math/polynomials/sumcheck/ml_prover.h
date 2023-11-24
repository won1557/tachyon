#ifndef TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_PROVER_H_
#define TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_PROVER_H_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "tachyon/math/polynomials/multivariate/multilinear_extension.h"
#include "tachyon/math/polynomials/sumcheck/ml_verifier.h"

namespace tachyon::math {

template <typename F>
struct ProverMsg {
  std::vector<F> evaluations;
};

template <typename F, size_t kMaxDegree>
struct ProverState {
  // TODO(woony): is it right type?
  using denseMLE = MultilinearDenseEvaluations<F, kMaxDegree>;

  std::vector<F> randomness;
  std::vector<std::pair<F, std::vector<size_t>>> list_of_products;
  std::vector<denseMLE> flattened_ml_extensions;
  size_t num_vars;
  size_t max_multiplicands;
  size_t round;
  bool error_flag;
};

template <typename F, size_t kMaxDegree>
class IPForMLSumcheck {
 public:
  using DenseMultilinearExtension = MultilinearDenseEvaluations<F, kMaxDegree>;

  static ProverState<F, kMaxDegree> ProverInit(
      const ListOfProductsOfPolynomials<F, kMaxDegree>& polynomial) {
    ProverState<F, kMaxDegree> prover_state{
        std::vector<F>(),
        polynomial.products(),
        std::vector<MultilinearDenseEvaluations<F, kMaxDegree>>(),
        polynomial.num_variables(),
        polynomial.max_multiplicands(),
        0,
        false};

    if (polynomial.num_variables() == 0) {
      prover_state.error_flag = true;
      return prover_state;
    }

    for (const auto& extension : polynomial.flattened_ml_extensions()) {
      prover_state.flattened_ml_extensions.push_back(*extension);
    }

    return prover_state;
  }

  static ProverMsg<F> ProveRound(ProverState<F, kMaxDegree>& prover_state,
                                  const std::optional<VerifierMsg<F>>& v_msg) {
    ProverMsg<F> empty_msg;

    if (prover_state.error_flag) {
      return empty_msg;
    }

    if (v_msg.has_value()) {
      if (prover_state.round == 0) {
        prover_state.error_flag = true;
        return empty_msg;
      }
      prover_state.randomness.push_back(v_msg->randomness);

      size_t i = prover_state.round;
      F r = prover_state.randomness[i - 1];
      for (auto& multiplicand : prover_state.flattened_ml_extensions) {
        multiplicand.FixVariables({r});
      }
    } else if (prover_state.round > 0) {
      prover_state.error_flag = true;
      return empty_msg;
    }

    prover_state.round += 1;

    if (prover_state.round > prover_state.num_vars) {
      prover_state.error_flag = true;
      return empty_msg;
    }

    size_t i = prover_state.round;
    size_t nv = prover_state.num_vars;
    size_t degree = prover_state.max_multiplicands;

    std::vector<F> products_sum(degree + 1, F::Zero());

    for (size_t b = 0; b < (1ull << (nv - i)); ++b) {
      for (const auto& [coefficient, products] :
           prover_state.list_of_products) {
        std::vector<F> product(degree + 1, F::Zero());
        for (size_t jth_product : products) {
          const DenseMultilinearExtension& table =
              prover_state.flattened_ml_extensions[jth_product];
          F start = *table.Get(b << 1);
          F step = *table.Get((b << 1) + 1) - start;
          for (F& p : product) {
            p *= start;
            start += step;
          }
        }
        for (size_t t = 0; t < degree + 1; ++t) {
          products_sum[t] += product[t];
        }
      }
    }

    return ProverMsg<F>{products_sum};
  }
};

}  // namespace tachyon::math

#endif  // TACHYON_MATH_POLYNOMIALS_SUMCHECK_ML_PROVER_H_
