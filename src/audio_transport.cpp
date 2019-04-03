#include <cmath>
#include <vector>
#include <tuple>

#include <sample_info/spectral_point.hpp>

#include "audio_transport/audio_transport.hpp"

std::vector<sample_info::spectral_point> audio_transport::interpolate(
    const std::vector<sample_info::spectral_point> & left,
    const std::vector<sample_info::spectral_point> & right,
    double interpolation_factor) {

  // Group the left and right spectra
  std::vector<spectral_mass> left_masses = group_spectrum(left);
  std::vector<spectral_mass> right_masses = group_spectrum(right);

  // Get the transport matrix
  std::vector<std::tuple<size_t, size_t, double>> T =
    transport_matrix(left_masses, right_masses);

  // Initialize the output spectral masses
  std::vector<sample_info::spectral_point> interpolated(left.size());

  // Perform the interpolation
  for (auto t : T) {
    spectral_mass left_mass  =  left_masses[std::get<0>(t)];
    spectral_mass right_mass = right_masses[std::get<1>(t)];

    // Calculate the new bin
    double interpolated_bin = 
          (1 - interpolation_factor) * left_mass.center_bin +
          interpolation_factor * right_mass.center_bin;

    // Place the left and right masses
    place_mass(
        left_mass, 
        interpolated_bin, 
        (1 - interpolation_factor) * std::get<2>(t)/left_mass.mass,
        left,
        interpolated
        );
    place_mass(
        right_mass, 
        interpolated_bin, 
        interpolation_factor * std::get<2>(t)/right_mass.mass,
        right,
        interpolated
        );
  }

  return interpolated;
}

void audio_transport::place_mass(
    const spectral_mass & mass,
    double center_bin,
    double scale,
    const std::vector<sample_info::spectral_point> & input,
    std::vector<sample_info::spectral_point> & output) {

  for (size_t i = mass.left_bin; i < mass.right_bin; i++) {
    // Compute the location in the new array
    int new_i = i + std::round(center_bin - mass.center_bin);
    if (new_i < 0) continue;
    if (new_i >= (int) output.size()) continue;

    output[new_i].value += scale * input[i].value;
  }
}

std::vector<std::tuple<size_t, size_t, double>> audio_transport::transport_matrix(
    const std::vector<audio_transport::spectral_mass> & left,
    const std::vector<audio_transport::spectral_mass> & right) {

  // Initialize the algorithm
  std::vector<std::tuple<size_t, size_t, double>> T;
  size_t left_index = 0, right_index = 0;
  double left_mass  = left[0].mass;
  double right_mass = right[0].mass;

  while (true) {
    if (left_mass < right_mass) {
      T.emplace_back(
          left_index,
          right_index,
          left_mass);

      right_mass -= left_mass;

      left_index += 1;
      if (left_index >= left.size()) break;
      left_mass = left[left_index].mass;
    } else {
      T.emplace_back(
          left_index,
          right_index,
          right_mass);

      left_mass -= right_mass;

      right_index += 1;
      if (right_index >= right.size()) break;
      right_mass = right[right_index].mass;
    }
  }

  return T;
}

std::vector<audio_transport::spectral_mass> audio_transport::group_spectrum(
   const std::vector<sample_info::spectral_point> & spectrum
   ) {

  // Initialize
  std::vector<spectral_mass> masses;
  audio_transport::spectral_mass initial_mass;
  initial_mass.left_bin = 0;
  initial_mass.center_bin = 0;
  masses.push_back(initial_mass);
  double mass_sum = 0;

  bool sign = (spectrum[0].freq_reassigned > spectrum[0].freq);
  for (size_t i = 1; i < spectrum.size(); i++) {
    bool current_sign = (spectrum[i].freq_reassigned > spectrum[i].freq);

    if (current_sign == sign) continue;
    if (sign) {
      // We are falling 
      // This is the center bin
      // Choose the one closest to the right

      // These should both be positive
      double left_dist = spectrum[i - 1].freq_reassigned - spectrum[i - 1].freq;
      double right_dist = spectrum[i].freq - spectrum[i].freq_reassigned;

      // Go to the closer side
      if (left_dist < right_dist) {
        masses[masses.size() - 1].center_bin = i - 1;
      } else {
        masses[masses.size() - 1].center_bin = i;
      }
    } else {
      // We are rising
      // This is the end

      // Set the end of the mass
      masses[masses.size() - 1].right_bin = i;

      // Compute the actual mass
      masses[masses.size() - 1].mass = 0;
      for (size_t j = masses[masses.size() - 1].left_bin; j < i; j++) {
        masses[masses.size() - 1].mass += std::abs(spectrum[j].value);
      }
      mass_sum += masses[masses.size() - 1].mass;

      // Construct a new mass
      spectral_mass mass;
      mass.left_bin = i;
      mass.center_bin = i;
      masses.push_back(mass);
    }
    sign = current_sign;
  }

  // Finish the last mass
  masses[masses.size() - 1].right_bin = spectrum.size();
  masses[masses.size() - 1].mass = 0;
  for (size_t j = masses[masses.size() - 1].left_bin; j < spectrum.size(); j++) {
    masses[masses.size() - 1].mass += std::abs(spectrum[j].value);
  }
  mass_sum += masses[masses.size() - 1].mass;

  // Normalize masses so that they sum to 1
  for (size_t i = 0; i < masses.size(); i++) {
    masses[i].mass /= mass_sum;
  }

  return masses;
} 
