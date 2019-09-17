#pragma once

#include <vector>
#include <tuple>
#include <map>

#include "spectral.hpp"

namespace audio_transport {

struct spectral_mass {
  size_t left_bin;
  size_t right_bin;
  size_t center_bin;
  double mass;
};

std::vector<audio_transport::spectral::point> interpolate(
    const std::vector<audio_transport::spectral::point> & left,
    const std::vector<audio_transport::spectral::point> & right,
    std::vector<double> & phases,
    double window_size,
    double interpolation_factor);

std::vector<std::tuple<size_t, size_t, double>> transport_matrix(
    const std::vector<spectral_mass> & left,
    const std::vector<spectral_mass> & right);

std::vector<spectral_mass> group_spectrum(
    const std::vector<audio_transport::spectral::point> & spectrum);

void place_mass(
    const spectral_mass & mass,
    int center_bin,
    double scale,
    double interpolated_freq,
    double center_phase,
    const std::vector<audio_transport::spectral::point> & input,
    std::vector<audio_transport::spectral::point> & output,
    double next_phase,
    std::vector<double> & phases,
    std::vector<double> & amplitudes);

}
