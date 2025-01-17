#ifndef TACHYON_CRYPTO_RANDOM_XOR_SHIFT_XOR_SHIFT_RNG_H_
#define TACHYON_CRYPTO_RANDOM_XOR_SHIFT_XOR_SHIFT_RNG_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "tachyon/base/random.h"
#include "tachyon/crypto/random/rng.h"

namespace tachyon::crypto {

// XORShiftRNG stands for "XOR Shift Random Number Generator".
// Please use |base::Uniform()| mostly for production. This is being used in
// Halo2 and we need a seed stability to ensure the same random number to be
// generated from a given seed.
//
// See
// [Seed Stability]
// (https://abseil.io/docs/cpp/guides/random#classes-of-generator-stability)
// and [Xorshift RNGs](https://www.jstatsoft.org/v08/i14/paper).
class XORShiftRNG : public RNG<XORShiftRNG> {
 public:
  XORShiftRNG() = default;

  static XORShiftRNG FromSeed(const uint8_t seed[16]) {
    XORShiftRNG ret;
    memcpy(&ret.x_, &seed[0], sizeof(uint32_t));
    memcpy(&ret.y_, &seed[4], sizeof(uint32_t));
    memcpy(&ret.z_, &seed[8], sizeof(uint32_t));
    memcpy(&ret.w_, &seed[12], sizeof(uint32_t));
    return ret;
  }

  static XORShiftRNG FromRandomSeed() {
    uint8_t seed[16];
    uint64_t lo = base::Uniform(base::Range<uint64_t>::All());
    uint64_t hi = base::Uniform(base::Range<uint64_t>::All());
    memcpy(&seed[0], &lo, sizeof(uint64_t));
    memcpy(&seed[8], &hi, sizeof(uint64_t));
    return FromSeed(seed);
  }

  uint32_t NextUint32() {
    uint32_t t = x_ ^ (x_ << 11);
    x_ = y_;
    y_ = z_;
    z_ = w_;
    w_ = w_ ^ (w_ >> 19) ^ (t ^ (t >> 8));
    return w_;
  }

 private:
  uint32_t x_ = 195911405;  // 0xBAD_5EED
  uint32_t y_ = 195911405;  // 0xBAD_5EED
  uint32_t z_ = 195911405;  // 0xBAD_5EED
  uint32_t w_ = 195911405;  // 0xBAD_5EED
};

}  // namespace tachyon::crypto

#endif  // TACHYON_CRYPTO_RANDOM_XOR_SHIFT_XOR_SHIFT_RNG_H_
