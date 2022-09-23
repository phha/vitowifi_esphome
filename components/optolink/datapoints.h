/*
 * Custom datapoints for optolink
 */

#pragma once

#include "DPTypes.hpp"
#include "Datapoint.hpp"

// 32 bit float
class conv4_1_F : public DPType {
 public:
  void encode(uint8_t* out, DPValue in) {  // encode can be left empty when the DP is only used for reading.
    int32_t tmp = in.getFloat();
    out[3] = tmp >> 24;
    out[2] = tmp >> 16;
    out[1] = tmp >> 8;
    out[0] = tmp & 0xFF;
  }

  DPValue decode(const uint8_t* in) {
    int32_t tmp = in[3] << 24 | in[2] << 16 | in[1] << 8 | in[0];  // keep endianess in mind! input is LSB first
    DPValue out(tmp * 1.0f);
    return out;
  }

  const size_t getLength() const {
    return 4;
  }
};
typedef Datapoint<conv4_1_F> dp_float32;

// 16 bit float
class conv2_1_F : public DPType {
  public:
    void encode(uint8_t* out, DPValue in) {
      int16_t tmp = in.getFloat();
      out[1] = tmp >> 8;
      out[0] = tmp & 0xFF;
    }

    DPValue decode(const uint8_t* in) {
      int16_t tmp = in[1] << 8 | in[0];
      DPValue out(tmp * 1.0f);
      return out;
    }

    const size_t getLength() const {
      return 2;
    }
};
typedef Datapoint<conv2_1_F> dp_float16;

typedef Datapoint<conv1_1_US> dp_uint8;
