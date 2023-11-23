// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_PLONK_PROVER_INSTANCE_SINGLE_H_
#define TACHYON_ZK_PLONK_PLONK_PROVER_INSTANCE_SINGLE_H_

#include <utility>
#include <vector>

#include "tachyon/base/containers/container_util.h"
#include "tachyon/zk/base/prover.h"
#include "tachyon/zk/plonk/error.h"
#include "tachyon/zk/transcript/transcript.h"

namespace tachyon::zk {

// Instance for a circuit.
template <typename PCSTy>
class InstanceSingle {
 public:
  using F = typename PCSTy::Field;
  using Evals = typename PCSTy::Evals;
  using Poly = typename PCSTy::Poly;
  using Domain = typename PCSTy::Domain;

  constexpr static size_t kDomainSize = PCSTy::kMaxDegree + 1;

  std::vector<Evals>& instance_value() const { return instance_values_; }
  std::vector<Poly>& instance_polys() const { return instance_polys_; }

  InstanceSingle() = default;

  static Error Generate(Prover<PCSTy>* prover,
                        std::vector<std::vector<F>> instance_vector,
                        InstanceSingle<PCSTy>* instance_singles,
                        bool query_instance) {
    std::vector<Evals> instance_values;
    instance_values.reserve(instance_vector.size());
    std::vector<Poly> instance_polys;
    instance_polys.reserve(instance_vector.size());

    for (const std::vector<F> instance : instance_vector) {
      if (instance.size() >
          kDomainSize - prover->blinder().blinding_factors()) {
        return Error::kInstanceTooLarge;
      }

      // Generate instance evaluations with leading zeros.
      std::vector<F> values =
          base::CreateVector(kDomainSize - instance.size(), F::Zero());
      values.insert(values.begin(), instance.begin(), instance.end());
      Evals instance_evals(values);

      // Generate instance polynomial.
      Poly instance_poly = prover->domain()->IFFT(instance_evals);

      // Write evals to the transcript.
      if (query_instance) {
        prover->CommitLagrange(instance_evals);
      } else {
        for (const F& eval : instance) {
          prover->writer()->WriteToTranscript(eval);
        }
      }

      instance_values.push_back(std::move(instance_evals));
      instance_polys.push_back(std::move(instance_poly));
    }

    *instance_singles =
        InstanceSingle(std::move(instance_values), std::move(instance_polys));

    return Error::kNone;
  }

 private:
  InstanceSingle(std::vector<Evals> instance_values,
                 std::vector<Poly> instance_polys)
      : instance_values_(std::move(instance_values)),
        instance_polys_(std::move(instance_polys)) {}
  std::vector<Evals> instance_values_;
  std::vector<Poly> instance_polys_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PLONK_PROVER_INSTANCE_SINGLE_H_
