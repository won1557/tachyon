// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_TRANSCRIPT_SHA256_TRANSCRIPT_H_
#define TACHYON_ZK_TRANSCRIPT_SHA256_TRANSCRIPT_H_

#include <array>
#include <utility>

#include "openssl/sha.h"

#include "tachyon/base/buffer/vector_buffer.h"
#include "tachyon/base/ranges/algorithm.h"
#include "tachyon/base/types/always_false.h"
#include "tachyon/math/base/big_int.h"
#include "tachyon/math/elliptic_curves/affine_point.h"
#include "tachyon/zk/transcript/transcript.h"

namespace tachyon::zk {

// Dummy zeros that come before prefix to a prover's message
constexpr uint8_t kShaPrefixZeros[31] = {0};

// Prefix to a prover's message soliciting a challenge
constexpr uint8_t kShaPrefixChallenge[1] = {0};

// Prefix to a prover's message containing a curve point
constexpr uint8_t kShaPrefixPoint[1] = {1};

// Prefix to a prover's message containing a scalar
constexpr uint8_t kShaPrefixScalar[1] = {2};

template <typename Curve>
class Sha256Reader : public TranscriptReader<math::AffinePoint<Curve>> {
 public:
  using AffinePointTy = typename Curve::AffinePointTy;
  using BaseField = typename Curve::BaseField;
  using ScalarField = typename Curve::ScalarField;

  Sha256Reader() = default;
  // Initialize a transcript given an input buffer.
  explicit Sha256Reader(base::Buffer read_buf) : buffer_(std::move(read_buf)) {
    SHA256_Init(&state_);
  }

  base::Buffer& buffer() { return buffer_; }
  const base::Buffer& buffer() const { return buffer_; }

  // Transcript methods
  Challenge255<AffinePointTy> SqueezeChallenge() override {
    SHA256_Update(&state_, kShaPrefixChallenge, 1);
    SHA256_CTX hasher = state_;
    uint8_t result[32] = {0};
    SHA256_Final(result, &hasher);

    SHA256_Init(&state_);
    SHA256_Update(&state_, result, 32);

    if constexpr (ScalarField::N <= 4) {
      return Challenge255<AffinePointTy>(ScalarField::FromAnySizedBigInt(
          math::BigInt<4>::FromBytesLE(result)));
    } else {
      base::AlwaysFalse<Curve>();
    }
  }

  bool WriteToTranscript(const AffinePointTy& point) override {
    SHA256_Update(&state_, kShaPrefixZeros, 31);
    SHA256_Update(&state_, kShaPrefixPoint, 1);
    SHA256_Update(&state_, point.x().ToBigInt().ToBytesBE().data(),
                  BaseField::BigIntTy::kByteNums);
    SHA256_Update(&state_, point.y().ToBigInt().ToBytesBE().data(),
                  BaseField::BigIntTy::kByteNums);
    return true;
  }

  bool WriteToTranscript(const ScalarField& scalar) override {
    SHA256_Update(&state_, kShaPrefixZeros, 31);
    SHA256_Update(&state_, kShaPrefixScalar, 1);
    SHA256_Update(&state_, scalar.ToBigInt().ToBytesBE().data(),
                  ScalarField::BigIntTy::kByteNums);
    return true;
  }

  // TranscriptRead methods
  bool ReadPoint(AffinePointTy* point) override {
    return buffer_.Read(point) && WriteToTranscript(*point);
  }

  bool ReadScalar(ScalarField* scalar) override {
    return buffer_.Read(scalar) && WriteToTranscript(*scalar);
  }

 private:
  SHA256_CTX state_;
  base::Buffer buffer_;
};

template <typename Curve>
class Sha256Writer : public TranscriptWriter<math::AffinePoint<Curve>> {
 public:
  using AffinePointTy = typename Curve::AffinePointTy;
  using BaseField = typename Curve::BaseField;
  using ScalarField = typename Curve::ScalarField;

  Sha256Writer() = default;
  // Initialize a transcript given an output buffer.
  explicit Sha256Writer(base::VectorBuffer write_buf)
      : buffer_(std::move(write_buf)) {
    SHA256_Init(&state_);
  }

  base::VectorBuffer& buffer() { return buffer_; }
  const base::VectorBuffer& buffer() const { return buffer_; }

  // Transcript methods
  Challenge255<AffinePointTy> SqueezeChallenge() override {
    SHA256_Update(&state_, kShaPrefixChallenge, 1);
    SHA256_CTX hasher = state_;
    uint8_t result[32] = {0};
    SHA256_Final(result, &hasher);

    SHA256_Init(&state_);
    SHA256_Update(&state_, result, 32);

    if constexpr (ScalarField::N <= 4) {
      return Challenge255<AffinePointTy>(ScalarField::FromAnySizedBigInt(
          math::BigInt<4>::FromBytesLE(result)));
    } else {
      base::AlwaysFalse<Curve>();
    }
  }

  bool WriteToTranscript(const AffinePointTy& point) override {
    SHA256_Update(&state_, kShaPrefixZeros, 31);
    SHA256_Update(&state_, kShaPrefixPoint, 1);
    SHA256_Update(&state_, point.x().ToBigInt().ToBytesBE().data(),
                  BaseField::BigIntTy::kByteNums);
    SHA256_Update(&state_, point.y().ToBigInt().ToBytesBE().data(),
                  BaseField::BigIntTy::kByteNums);
    return true;
  }

  bool WriteToTranscript(const ScalarField& scalar) override {
    SHA256_Update(&state_, kShaPrefixZeros, 31);
    SHA256_Update(&state_, kShaPrefixScalar, 1);
    SHA256_Update(&state_, scalar.ToBigInt().ToBytesBE().data(),
                  ScalarField::BigIntTy::kByteNums);
    return true;
  }

  // TranscriptWrite methods
  bool WriteToProof(const AffinePointTy& point) override {
    return WriteToTranscript(point) && buffer_.Write(point);
  }

  bool WriteToProof(const ScalarField& scalar) override {
    return WriteToTranscript(scalar) && buffer_.Write(scalar);
  }

 private:
  SHA256_CTX state_;
  base::VectorBuffer buffer_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_TRANSCRIPT_SHA256_TRANSCRIPT_H_
