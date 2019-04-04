#include <cmath>
#include <vector>
#include <tuple>
#include <map>

#include <sample_info/spectral_point.hpp>

#include "audio_transport/audio_transport.hpp"

std::vector<sample_info::spectral_point> audio_transport::interpolate(
    const std::vector<sample_info::spectral_point> & left,
    const std::vector<sample_info::spectral_point> & right,
    std::map<std::pair<size_t, size_t>, double> & phases,
    double window_size,
    double interpolation) {

  // Group the left and right spectra
  std::vector<spectral_mass> left_masses = group_spectrum(left);
  std::vector<spectral_mass> right_masses = group_spectrum(right);

  // Get the transport matrix
  std::vector<std::tuple<size_t, size_t, double>> T =
    transport_matrix(left_masses, right_masses);

  // Initialize the output spectral masses
  std::vector<sample_info::spectral_point> interpolated(left.size());

  // Initialize new phases
  std::map<std::pair<size_t, size_t>, double> new_phases;

  // Perform the interpolation
  for (auto t : T) {
    //if (std::get<2>(t) < 0.1) continue;
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

    // Check to see if these masses were interpolated last time
    double center_phase;
    std::pair<size_t, size_t> phase_query(left_mass.center_bin, right_mass.center_bin);
    if (phases.find(phase_query) != phases.end()) {
      // If so update the phase using the reassigned frequency
      // Division by 2 comes from window overlap
      //center_phase = phases[phase_query] + (interpolated_freq * window_size/2.)/2.;
      //center_phase = phases[phase_query] + interpolated_freq * window_size/2.;
      center_phase = phases[phase_query];
    } else {
      // Otherwise set the initial value to be an interpolation of the actual phases
      center_phase =
        (1 - interpolation) * std::arg(left[left_mass.center_bin].value) +
        interpolation * std::arg(right[right_mass.center_bin].value);
    }

    // Add the phase to the new phases
    //new_phases[phase_query] = center_phase + (interpolated_freq * window_size/2.)/2.;
    //new_phases[phase_query] = center_phase;
    new_phases[phase_query] = center_phase + interpolated_freq * window_size/2.;

    // Place the left and right masses
    place_mass(
        left_mass, 
        interpolated_bin, 
        (1 - interpolation) * std::get<2>(t)/left_mass.mass,
        center_phase,
        left,
        interpolated
        );
    place_mass(
        right_mass, 
        interpolated_bin, 
        interpolation * std::get<2>(t)/right_mass.mass,
        center_phase,
        right,
        interpolated
        );
  }

  // Clear the map
  phases.clear();
  // Fill it with new phases
  for (const auto & new_phase : new_phases) {
    phases[new_phase.first] = new_phase.second;
  }

  return interpolated;
}

void audio_transport::place_mass(
    const spectral_mass & mass,
    int center_bin,
    double scale,
    double center_phase,
    const std::vector<sample_info::spectral_point> & input,
    std::vector<sample_info::spectral_point> & output) {

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
    std::complex<double> shifted_value =
      std::polar(std::abs(input[i].value), phase);

    output[new_i].value += scale * shifted_value;
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

  // Initialize the first mass
  std::vector<spectral_mass> masses;
  audio_transport::spectral_mass initial_mass;
  initial_mass.left_bin = 0;
  initial_mass.center_bin = 0;
  masses.push_back(initial_mass);

  // Keep track of the total mass
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
