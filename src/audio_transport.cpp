#include <cmath>
#include <vector>
#include <tuple>
#include <map>

#include <sample_info/spectral.hpp>

#include "audio_transport.hpp"

std::vector<sample_info::spectral::point> audio_transport::interpolate(
    const std::vector<sample_info::spectral::point> & left,
    const std::vector<sample_info::spectral::point> & right,
    std::vector<double> & phases,
    double window_size,
    double interpolation) {

  // Group the left and right spectra
  std::vector<spectral_mass> left_masses = group_spectrum(left);
  std::vector<spectral_mass> right_masses = group_spectrum(right);

  // Get the transport matrix
  std::vector<std::tuple<size_t, size_t, double>> T =
    transport_matrix(left_masses, right_masses);

  // Initialize the output spectral masses
  std::vector<sample_info::spectral::point> interpolated(left.size());

  // Initialize new phases
  std::vector<double> new_amplitudes(phases.size(), 0);
  std::vector<double> new_phases(phases.size(), 0);

  // Perform the interpolation
  for (auto t : T) {
    spectral_mass left_mass  =  left_masses[std::get<0>(t)];
    spectral_mass right_mass = right_masses[std::get<1>(t)];

    // Calculate the new bin and frequency
    int interpolated_bin = std::round(
      (1 - interpolation) * left_mass.center_bin +
      interpolation * right_mass.center_bin
      );

    // Compute the actual interpolation factor given the new bin
    double interpolation_rounded = interpolation;
    if (left_mass.center_bin != right_mass.center_bin) {
      interpolation_rounded = 
        (interpolated_bin - left_mass.center_bin)/
        ((double)(right_mass.center_bin - left_mass.center_bin));
    }
    // Interpolate the frequency appropriately
    double interpolated_freq = 
      (1 - interpolation_rounded) * left[left_mass.center_bin].freq_reassigned +
      interpolation_rounded * right[right_mass.center_bin].freq_reassigned;

    double center_phase =
      phases[interpolated_bin] + (interpolated_freq * window_size/2.)/2. - (M_PI * interpolated_bin);
    double new_phase = 
      center_phase + (interpolated_freq * window_size/2.)/2. + (M_PI * interpolated_bin);

    // Place the left and right masses
    place_mass(
        left_mass, 
        interpolated_bin, 
        (1 - interpolation) * std::get<2>(t)/left_mass.mass,
        center_phase,
        left,
        interpolated,
        new_phase,
        new_phases,
        new_amplitudes
        );
    place_mass(
        right_mass, 
        interpolated_bin, 
        interpolation * std::get<2>(t)/right_mass.mass,
        center_phase,
        right,
        interpolated,
        new_phase,
        new_phases,
        new_amplitudes
        );
  }

  // Fill the phases with the new phases
  for (size_t i = 0; i < phases.size(); i++) {
    phases[i] = new_phases[i];
  }

  return interpolated;
}

void audio_transport::place_mass(
    const spectral_mass & mass,
    int center_bin,
    double scale,
    double center_phase,
    const std::vector<sample_info::spectral::point> & input,
    std::vector<sample_info::spectral::point> & output,
    double next_phase,
    std::vector<double> & phases,
    std::vector<double> & amplitudes) {

  // Compute how the phase changes in each bin
  double phase_shift = center_phase - std::arg(input[mass.center_bin].value);

  for (size_t i = mass.left_bin; i < mass.right_bin; i++) {
    // Compute the location in the new array
    int new_i = i + center_bin - mass.center_bin;
    if (new_i < 0) continue;
    if (new_i >= (int) output.size()) continue;

    // Rotate the output by the phase offset
    // plus the frequency 
    double phase = phase_shift + std::arg(input[i].value);
    double mag = scale * std::abs(input[i].value);
    std::complex<double> shifted_value =
      std::polar(mag, phase);

    output[new_i].value += shifted_value;

    if (mag > amplitudes[new_i]) {
      amplitudes[new_i] = mag;
      phases[new_i] = next_phase;
    }
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
   const std::vector<sample_info::spectral::point> & spectrum
   ) {

  // Keep track of the total mass
  double mass_sum = 0;
  for (size_t i = 0; i < spectrum.size(); i++) {
    mass_sum += std::abs(spectrum[i].value);
  }

  // Initialize the first mass
  std::vector<spectral_mass> masses;
  audio_transport::spectral_mass initial_mass;
  initial_mass.left_bin = 0;
  initial_mass.center_bin = 0;
  masses.push_back(initial_mass);

  bool sign;
  bool first = true;
  for (size_t i = 0; i < spectrum.size(); i++) {
    bool current_sign = (spectrum[i].freq_reassigned > spectrum[i].freq);

    if (first) {
      first = false;
      sign = current_sign;
      continue;
    }

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

      // Compute the actual mass
      masses[masses.size() - 1].mass = 0;
      for (size_t j = masses[masses.size() - 1].left_bin; j < i; j++) {
        masses[masses.size() - 1].mass += std::abs(spectrum[j].value);
      }
      // Normalize
      masses[masses.size() - 1].mass /= mass_sum;

      // Set the end of the mass
      masses[masses.size() - 1].right_bin = i;

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
  masses[masses.size() - 1].mass /= mass_sum;

  return masses;
} 
