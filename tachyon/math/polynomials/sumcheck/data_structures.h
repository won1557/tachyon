#ifndef TACHYON_MATH_POLYNOMIALS_SUMCHECK_DATA_STRUCTURES_H_
#define TACHYON_MATH_POLYNOMIALS_SUMCHECK_DATA_STRUCTURES_H_

#include <iostream>
#include <vector>

#include "absl/container/flat_hash_map.h"

#include "tachyon/math/polynomials/multivariate/multilinear_extension.h"

namespace tachyon::math {

// TODO: ETHAN COMMENT
// Convention
// - 변수명을 snake case로 변경하기
// - Constructor와 일반 함수 사이에 enter하나 필요
// - 함수 이름은 CamelCase, 시작을 대문자로 (Pascal Case?)
// - 인자가 하나인 Constructor 앞에는 "explicit" 키워드를 붙인다.
//
// Rc 대신에 shared_ptr을 쓰는게 맞는지?
// HashMap 대신에 flat_hash_map을 쓰는게 맞는지?
//  - 아닐것 같음.
//  - flat_hash_map: [(key, value), (key, value)...]
//  - HashMap: {key: value, key: value, ...}

struct PolynomialInfo {
  size_t max_multiplicands_ = 0;
  size_t num_variables_ = 0;
};

template <typename F, size_t kMaxDegree>
class ListOfProductsOfPolynomials {
 public:
  // TODO(woony): check type.
  // MultilinearExtension<MultilinearDenseEvaluations<F, kMaxDegree>>;
  using DenseMultilinearExtension = MultilinearDenseEvaluations<F, kMaxDegree>;

  ListOfProductsOfPolynomials() = default;
  explicit ListOfProductsOfPolynomials(size_t num_variables)
      : max_multiplicands_(0), num_variables_(num_variables) {}

  void AddProduct(
      const std::vector<std::shared_ptr<DenseMultilinearExtension>>& product,
      F coefficient) {
    std::vector<size_t> indexed_product;
    indexed_product.reserve(product.size());

    for (const auto& m : product) {
      assert(m->numVars == num_variables_ &&
             "Product has a multiplicand with the wrong number of variables");

      auto m_ptr = m.get();
      auto it = raw_pointers_lookup_table_.find(m_ptr);

      if (it != raw_pointers_lookup_table_.end()) {
        indexed_product.push_back(it->second);
      } else {
        size_t current_index = flattened_ml_extensions_.size();
        flattened_ml_extensions_.push_back(m);
        raw_pointers_lookup_table_[m_ptr] = current_index;
        indexed_product.push_back(current_index);
      }
    }

    products_.emplace_back(coefficient, indexed_product);
    max_multiplicands_ = std::max(max_multiplicands_, product.size());
  }

  F Evaluate(const std::vector<F>& point) const {
    F result = 0;

    for (const auto& product : products_) {
      F term = product.first;

      for (size_t index : product.second) {
        term *= flattened_ml_extensions_[index]->evaluate(point);
      }

      result += term;
    }

    return result;
  }

  size_t max_multiplicands() const { return max_multiplicands_; }
  size_t num_variables() const { return num_variables_; }
  std::vector<std::pair<F, std::vector<size_t>>> products() const {
    return products_;
  }
  std::vector<std::shared_ptr<DenseMultilinearExtension>>
  flattened_ml_extensions() const {
    return flattened_ml_extensions_;
  };

 private:
  size_t max_multiplicands_;
  size_t num_variables_ = 0;
  std::vector<std::pair<F, std::vector<size_t>>> products_;
  std::vector<std::shared_ptr<DenseMultilinearExtension>>
      flattened_ml_extensions_;
  absl::flat_hash_map<const DenseMultilinearExtension*, size_t>
      raw_pointers_lookup_table_;
};

}  // namespace tachyon::math

#endif  // TACHYON_MATH_POLYNOMIALS_SUMCHECK_DATA_STRUCTURES_H_
