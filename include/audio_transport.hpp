#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <sample_info/spectral.hpp>

namespace audio_transport {

struct spectral_mass {
  size_t left_bin;
  size_t right_bin;
  size_t center_bin;
  double mass;
};

std::vector<sample_info::spectral::point> interpolate(
    const std::vector<sample_info::spectral::point> & left,
    const std::vector<sample_info::spectral::point> & right,
    std::vector<double> & phases,
    double window_size,
    double interpolation_factor);

std::vector<std::tuple<size_t, size_t, double>> transport_matrix(
    const std::vector<spectral_mass> & left,
    const std::vector<spectral_mass> & right);

std::vector<spectral_mass> group_spectrum(
    const std::vector<sample_info::spectral::point> & spectrum);

void place_mass(
    const spectral_mass & mass,
    int center_bin,
    double scale,
    double interpolated_freq,
    double center_phase,
    const std::vector<sample_info::spectral::point> & input,
    std::vector<sample_info::spectral::point> & output,
    double next_phase,
    std::vector<double> & phases,
    std::vector<double> & amplitudes);

}
