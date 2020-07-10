#include <vector>
#include <cmath>
#include <ciso646>
#include <iostream>
#include <fftw3.h>

#include "audio_transport/equal_loudness.hpp"

using namespace audio_transport;

double audio_transport::equal_loudness::a_weighting_amp(double freq) {
  // Convert to hertz
  freq /= 2 * M_PI;
  double freq_squared = freq * freq;
  double top  = 12194 * 12194 * freq_squared * freq_squared;
  double bot1 =  20.6 *  20.6 + freq_squared;
  double bot2 = 107.7 * 107.7 + freq_squared;
  double bot3 = 737.9 * 737.9 + freq_squared;
  double bot4 = 12194 * 12194 + freq_squared;
  return top/(bot1 * std::sqrt(bot2 * bot3) * bot4);
}

void audio_transport::equal_loudness::apply(
    std::vector<std::vector<spectral::point>> & points) {
  for (size_t w = 0; w < points.size(); w++) {
    for (size_t i = 0; i < points[w].size(); i++) {
      points[w][i].value *= equal_loudness::a_weighting_amp(points[w][i].freq);
    }
  }
}

void audio_transport::equal_loudness::remove(
    std::vector<std::vector<spectral::point>> & points) {
  for (size_t w = 0; w < points.size(); w++) {
    for (size_t i = 0; i < points[w].size(); i++) {
      double value = equal_loudness::a_weighting_amp(points[w][i].freq);
      if (value > 0) {
        points[w][i].value /= value;
      }
    }
  }
}
