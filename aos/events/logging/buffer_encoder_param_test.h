#ifndef AOS_EVENTS_LOGGING_BUFFER_ENCODER_PARAM_TEST_H_
#define AOS_EVENTS_LOGGING_BUFFER_ENCODER_PARAM_TEST_H_

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "aos/events/logging/logfile_utils.h"
#include "aos/events/logging/logger_generated.h"
#include "aos/testing/random_seed.h"

namespace aos::logger::testing {

// Contains some helpful infrastructure for testing DetachedBufferEncoder
// implementations.
class BufferEncoderBaseTest : public ::testing::Test {
 public:
  // Creates and encodes n random buffers, returning a copy of the encoded data.
  std::vector<std::vector<uint8_t>> CreateAndEncode(
      int n, DetachedBufferEncoder *encoder) {
    std::vector<std::vector<uint8_t>> result;
    for (int i = 0; i < n; i++) {
      flatbuffers::DetachedBuffer buffer = CreateRandomBuffer();
      result.emplace_back(buffer.data(), buffer.data() + buffer.size());
      encoder->Encode(std::move(buffer));
    }
    return result;
  }

  // Returns the total size of a vector full of objects with a size() method.
  template <typename T>
  static size_t TotalSize(const std::vector<T> &buffers) {
    size_t result = 0;
    for (const auto &v : buffers) {
      result += v.size();
    }
    return result;
  }

  // Returns a flattened copy of a vector of sequences.
  template <typename T>
  static std::vector<typename T::value_type> Flatten(
      const std::vector<T> &buffers) {
    std::vector<typename T::value_type> result;
    for (const auto &buffer : buffers) {
      result.insert(result.end(), buffer.begin(), buffer.end());
    }
    return result;
  }

  void Reseed(int seed) {
    // This algorithm allows a reasonable --runs_per_test to cover new seeds.
    random_number_generator_ =
        std::mt19937(::aos::testing::RandomSeed() + 1000 * seed);
  }

  std::mt19937 *random_number_generator() { return &random_number_generator_; }

 private:
  flatbuffers::DetachedBuffer CreateRandomBuffer() {
    std::uniform_int_distribution<int> length_distribution(2800, 3000);
    const size_t length = length_distribution(random_number_generator_);

    flatbuffers::FlatBufferBuilder fbb;
    uint8_t *data;
    const flatbuffers::Offset<flatbuffers::Vector<uint8_t>> data_offset =
        fbb.CreateUninitializedVector(length, &data);
    {
      std::independent_bits_engine<std::mt19937, CHAR_BIT, uint8_t> engine(
          std::ref(random_number_generator_));
      const uint8_t byte = engine();
      std::fill(data, data + length, byte);
    }

    MessageHeader::Builder builder(fbb);
    builder.add_data(data_offset);
    fbb.FinishSizePrefixed(builder.Finish());

    return fbb.Release();
  }

  std::mt19937 random_number_generator_{
      std::mt19937(::aos::testing::RandomSeed())};
};

// Tests some generic properties for any DetachedBufferEncoder+DataDecoder
// implementation pair.
//
// First and second test parameters are methods to create instances of the
// encoder and decoder. Third test parameter is a random seed, to allow testing
// many different data patterns.
class BufferEncoderTest
    : public BufferEncoderBaseTest,
      public ::testing::WithParamInterface<std::tuple<
          std::function<std::unique_ptr<DetachedBufferEncoder>()>,
          std::function<std::unique_ptr<DataDecoder>(std::string_view)>, int>> {
 public:
  BufferEncoderTest() { Reseed(std::get<2>(GetParam())); }

  std::unique_ptr<DetachedBufferEncoder> MakeEncoder() const {
    return std::get<0>(GetParam())();
  }
  std::unique_ptr<DataDecoder> MakeDecoder(std::string_view filename) const {
    return std::get<1>(GetParam())(filename);
  }
};

}  // namespace aos::logger::testing

#endif  // AOS_EVENTS_LOGGING_BUFFER_ENCODER_PARAM_TEST_H_
