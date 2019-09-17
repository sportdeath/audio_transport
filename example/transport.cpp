#include <iostream>
#include <algorithm>
#include <audiorw.hpp>

#include "audio_transport/spectral.hpp"
#include "audio_transport/audio_transport.hpp"

double window_size = 0.05; // seconds
unsigned int padding = 7; // multiplies window size

int main(int argc, char ** argv) {

  if (argc != 4) {
    std::cout <<
      "Usage: " << argv[0] << " left_file right_file output_file"
      << std::endl;
    return 1;
  }

  // Open the audio files
  double sample_rate_left;
  std::vector<std::vector<double>> audio_left =
    audiorw::read(argv[1], sample_rate_left);
  double sample_rate_right;
  std::vector<std::vector<double>> audio_right =
    audiorw::read(argv[2], sample_rate_right);

  if (sample_rate_left != sample_rate_right) {
    std::cout << "Sample rates are different! " << sample_rate_left << " != " << sample_rate_right << std::endl;
  }
  double sample_rate = sample_rate_left;

  // Initialize the output audio
  size_t num_channels = std::min(audio_left.size(), audio_right.size());
  std::vector<std::vector<double>> audio_interpolated(num_channels);


  // Iterate over the channels
  for (size_t c = 0; c < num_channels; c++) {

    std::cout << "Processing channel " << c << std::endl;

    std::cout << "Converting left input to the spectral domain" << std::endl;
    std::vector<std::vector<audio_transport::spectral::point>> points_left =
      audio_transport::spectral::analysis(audio_left[c], sample_rate, window_size, padding);
    std::cout << "Converting right input to the spectral domain" << std::endl;
    std::vector<std::vector<audio_transport::spectral::point>> points_right =
      audio_transport::spectral::analysis(audio_right[c], sample_rate, window_size, padding);

    // Initialize phases
    std::vector<double> phases(points_left[0].size(), 0);

    std::cout << "Performing optimal transport based interpolation" << std::endl;
    size_t num_windows = std::min(points_left.size(), points_right.size());
    std::vector<std::vector<audio_transport::spectral::point>> points_interpolated(num_windows);
    for (size_t w = 0; w < num_windows; w++) {
      double interpolation_factor = w/(double) num_windows;
      interpolation_factor = 4 * interpolation_factor - 1;
      interpolation_factor = std::min(1.,std::max(0.,interpolation_factor));

      points_interpolated[w] = 
        audio_transport::interpolate(
          points_left[w],
          points_right[w],
          phases,
          window_size,
          interpolation_factor);
    }

    std::cout << "Converting the interpolation to the time domain" << std::endl;
    audio_interpolated[c] = 
      audio_transport::spectral::synthesis(points_interpolated, padding);
  }

  // Write the file
  std::cout << "Writing to file " << argv[3] << std::endl;
  audiorw::write(audio_interpolated, argv[3], sample_rate);
}
