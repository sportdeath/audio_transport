#pragma once

#include <vector>
#include <tuple>
#include <sample_info/spectral_point.hpp>

namespace audio_transport {

struct spectral_mass {
  size_t left_bin;
  size_t right_bin;
  size_t center_bin;
  double mass;
};

std::vector<sample_info::spectral_point> interpolate(
    const std::vector<sample_info::spectral_point> & left,
    const std::vector<sample_info::spectral_point> & right,
    double interpolation_factor);

std::vector<std::tuple<size_t, size_t, double>> transport_matrix(
    const std::vector<spectral_mass> & left,
    const std::vector<spectral_mass> & right);

std::vector<spectral_mass> group_spectrum(
    const std::vector<sample_info::spectral_point> & spectrum);

void place_mass(
    const spectral_mass & mass,
    double center_bin,
    double scale,
    const std::vector<sample_info::spectral_point> & input,
    std::vector<sample_info::spectral_point> & output);

}
