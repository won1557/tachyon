#ifndef TACHYON_MATH_POLYNOMIALS_MULTIVARIATE_MULTIVARIATE_POLYNOMIAL_H_
#define TACHYON_MATH_POLYNOMIALS_MULTIVARIATE_MULTIVARIATE_POLYNOMIAL_H_

#include <stddef.h>

#include <type_traits>

#include "tachyon/math/polynomials/multivariate/sparse_coefficients.h"
#include "tachyon/math/polynomials/polynomial.h"

namespace tachyon::math {

template <typename Coefficients>
class MultivariatePolynomial
    : public Polynomial<MultivariatePolynomial<Coefficients>> {
 public:
  constexpr static const size_t kMaxDegree = Coefficients::kMaxDegree;

  using Field = typename Coefficients::Field;
  using Literal = typename Coefficients::Literal;

  constexpr MultivariatePolynomial() = default;
  constexpr MultivariatePolynomial(const Coefficients& coefficients)
      : coefficients_(coefficients) {}
  constexpr MultivariatePolynomial(Coefficients&& coefficients)
      : coefficients_(std::move(coefficients)) {}

  constexpr Field Evaluate(const std::vector<Field>& points) const {
    return DoEvaluate(points);
  }

  constexpr static MultivariatePolynomial Zero() {
    return MultivariatePolynomial(Coefficients::Zero());
  }

  constexpr static MultivariatePolynomial One() {
    return MultivariatePolynomial(Coefficients::One());
  }

  constexpr static MultivariatePolynomial Random(size_t arity, size_t degree) {
    return MultivariatePolynomial(Coefficients::Random(arity, degree));
  }

  constexpr bool IsZero() const { return coefficients_.IsZero(); }

  constexpr bool IsOne() const { return coefficients_.IsOne(); }

  constexpr const Coefficients& coefficients() const { return coefficients_; }

  constexpr bool operator==(const MultivariatePolynomial& other) const {
    return coefficients_ == other.coefficients_;
  }

  constexpr bool operator!=(const MultivariatePolynomial& other) const {
    return !operator==(other);
  }

  constexpr Field* operator[](const Literal& literal) {
    return coefficients_.Get(literal);
  }

  constexpr const Field* operator[](const Literal& literal) const {
    return coefficients_.Get(literal);
  }

  constexpr const Field* GetLeadingCoefficient() const {
    return coefficients_.GetLeadingCoefficient();
  }

  std::string ToString() const { return coefficients_.ToString(); }

#define OPERATION_METHOD(Name)                                              \
  template <typename Coefficients2,                                         \
            std::enable_if_t<internal::SupportsPoly##Name<                  \
                Coefficients, MultivariatePolynomial<Coefficients>,         \
                MultivariatePolynomial<Coefficients2>>::value>* = nullptr>  \
  constexpr auto Name(const MultivariatePolynomial<Coefficients2>& other)   \
      const {                                                               \
    return internal::MultivariatePolynomialOp<Coefficients>::Name(*this,    \
                                                                  other);   \
  }                                                                         \
                                                                            \
  template <typename Coefficients2,                                         \
            std::enable_if_t<internal::SupportsPoly##Name##InPlace<         \
                Coefficients, MultivariatePolynomial<Coefficients>,         \
                MultivariatePolynomial<Coefficients2>>::value>* = nullptr>  \
  constexpr auto& Name##InPlace(                                            \
      const MultivariatePolynomial<Coefficients2>& other) {                 \
    return internal::MultivariatePolynomialOp<Coefficients>::Name##InPlace( \
        *this, other);                                                      \
  }

  // AdditiveSemigroup methods
  OPERATION_METHOD(Add)

  // AdditiveGroup methods
  OPERATION_METHOD(Sub)

  MultivariatePolynomial& NegInPlace() {
    return internal::MultivariatePolynomialOp<Coefficients>::NegInPlace(*this);
  }
#undef OPERATION_METHOD

 private:
  friend class Polynomial<MultivariatePolynomial<Coefficients>>;
  friend class internal::MultivariatePolynomialOp<Coefficients>;

  // Polynomial methods
  constexpr size_t DoDegree() const { return coefficients_.Degree(); }

  constexpr Field DoEvaluate(const std::vector<Field>& points) const {
    return coefficients_.Evaluate(points);
  }

  Coefficients coefficients_;
};

template <typename Coefficients>
std::ostream& operator<<(std::ostream& os,
                         const MultivariatePolynomial<Coefficients>& p) {
  return os << p.ToString();
}

template <typename Coefficients>
class CoefficientsTraits<MultivariatePolynomial<Coefficients>> {
 public:
  using Field = typename Coefficients::Field;
};

template <typename F, size_t MaxDegree>
using SparseMultivariatePolynomial =
    MultivariatePolynomial<SparseCoefficients<F, MaxDegree>>;

}  // namespace tachyon::math

#include "tachyon/math/polynomials/multivariate/multivariate_polynomial_ops.h"

#endif  // TACHYON_MATH_POLYNOMIALS_MULTIVARIATE_MULTIVARIATE_POLYNOMIAL_H_