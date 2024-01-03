// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_FLOOR_PLANNER_CONSTANT_H_
#define TACHYON_ZK_PLONK_CIRCUIT_FLOOR_PLANNER_CONSTANT_H_

#include "tachyon/math/base/rational_field.h"
#include "tachyon/zk/plonk/circuit/cell.h"

namespace tachyon::zk {

template <typename F>
struct Constant {
  Constant(math::RationalField<F> value, Cell cell)
      : value(value), cell(cell) {}

  math::RationalField<F> value;
  Cell cell;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_FLOOR_PLANNER_CONSTANT_H_
