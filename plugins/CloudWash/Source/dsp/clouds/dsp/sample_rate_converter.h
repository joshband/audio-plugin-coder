// Copyright 2014 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Sample rate converter

#ifndef CLOUDS_DSP_SAMPLE_RATE_CONVERTER_H_
#define CLOUDS_DSP_SAMPLE_RATE_CONVERTER_H_

#include "stmlib/stmlib.h"

#include "clouds/dsp/frame.h"

namespace clouds {

template<int32_t ratio, int32_t filter_size, const float* coefficients>
class SampleRateConverter {
 public:
  SampleRateConverter() { }
  ~SampleRateConverter() { }
 
  void Init() {
    for (int32_t i = 0; i < filter_size * 2; ++i) {
      history_[i].l = history_[i].r = 0.0f;
    }
    std::copy(&coefficients[0], &coefficients[filter_size], &coefficients_[0]);
    history_ptr_ = filter_size - 1;
  };

  void Process(const FloatFrame* in, FloatFrame* out, size_t input_size) {
    // SAFETY CHECK: Validate input pointer
    if (in == nullptr || out == nullptr) {
      return;
    }
    
    // SAFETY CHECK: Validate input_size is reasonable
    if (input_size == 0 || input_size > 65536) {
      return;
    }

    int32_t history_ptr = history_ptr_;
    FloatFrame* history = history_;
    const float scale = ratio < 0 ? 1.0f : float(ratio);
    
    // CRITICAL FIX: Use local copies of input/output to avoid pointer issues
    const FloatFrame* input_ptr = in;
    FloatFrame* output_ptr = out;
    size_t remaining = input_size;
    
    while (remaining > 0) {
      int32_t consumed = ratio < 0 ? -ratio : 1;
      
      // SAFETY: Ensure we don't read past input buffer
      if (consumed > static_cast<int32_t>(remaining)) {
        consumed = static_cast<int32_t>(remaining);
      }
      
      for (int32_t i = 0; i < consumed; ++i) {
        // Clamp history_ptr to valid range before using as index
        if (history_ptr < 0 || history_ptr >= filter_size) {
          history_ptr = filter_size - 1;
        }
        // SAFETY: Read from local pointer with bounds check
        FloatFrame input_sample = *input_ptr++;
        history[history_ptr + filter_size] = history[history_ptr] = input_sample;
        --remaining;
        --history_ptr;
        if (history_ptr < 0) {
          history_ptr += filter_size;
        }
      }
    
      int32_t produced = ratio > 0 ? ratio : 1;
      for (int32_t i = 0; i < produced; ++i) {
        float y_l = 0.0f;
        float y_r = 0.0f;
        // SAFETY FIX: Ensure history_ptr + 1 is within valid bounds
        int32_t x_index = history_ptr + 1;
        if (x_index < 0) x_index += filter_size;
        if (x_index >= filter_size) x_index -= filter_size;
        const FloatFrame* x = &history[x_index];
        
        // SAFETY: Use pointer arithmetic with explicit bounds checking
        for (int32_t j = i; j < filter_size; j += produced) {
          const float h = coefficients_[j];
          // CRITICAL FIX: Ensure we don't read past history buffer
          int32_t read_index = x_index + (j - i) / produced;
          if (read_index < 0) read_index += filter_size;
          if (read_index >= filter_size) read_index -= filter_size;
          const FloatFrame* sample_ptr = &history[read_index];
          y_l += sample_ptr->l * h;
          y_r += sample_ptr->r * h;
        }
        output_ptr->l = y_l * scale;
        output_ptr->r = y_r * scale;
        ++output_ptr;
      }
    }
    history_ptr_ = history_ptr;
  }
 
 private:
  float coefficients_[filter_size];
  FloatFrame history_[filter_size * 2];
  int32_t history_ptr_;

  DISALLOW_COPY_AND_ASSIGN(SampleRateConverter);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_SAMPLE_RATE_CONVERTER_H_
