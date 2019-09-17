#include <iostream>
#include <algorithm>
#include <audiorw.hpp>

#include "audio_transport/spectral.hpp"
#include "audio_transport/audio_transport.hpp"

double window_size = 0.05; // seconds
unsigned int padding = 7; // multiplies window size
double interpolation_factor = 0.9; // in [0,1]

int main(int argc, char ** argv) {

  if (argc != 3) {
    std::cout <<
      "Usage: " << argv[0] << " input_file output_file"
      << std::endl;
    return 1;
  }

  // Open the audio files
  double sample_rate;
  std::vector<std::vector<double>> audio =
    audiorw::read(argv[1], sample_rate);

  // Initialize the output audio
  size_t num_channels = audio.size();
  std::vector<std::vector<double>> audio_interpolated(num_channels);

  // Iterate over the channels
  for (size_t c = 0; c < num_channels; c++) {

    std::cout << "Processing channel " << c << std::endl;

    std::cout << "Converting input to the spectral domain" << std::endl;
    std::vector<std::vector<audio_transport::spectral::point>> points =
      audio_transport::spectral::analysis(audio[c], sample_rate, window_size, padding);

    std::vector<audio_transport::spectral::point> points_prev(points[0].size());
    for (unsigned int i = 0; i < points_prev.size(); i++) {
      points_prev[i] = points[0][i];
    }

    // Initialize phases
    std::vector<double> phases(points[0].size(), 0);

    std::cout << "Performing optimal transport based interpolation" << std::endl;
    size_t num_windows = points.size();
    std::vector<std::vector<audio_transport::spectral::point>> points_interpolated(num_windows);
    for (size_t w = 0; w < num_windows; w++) {
      points_interpolated[w] = 
        audio_transport::interpolate(
          points_prev,
          points[w],
          phases,
          window_size,
          interpolation_factor);

      // Copy over the previous
      points_prev.resize(points_interpolated[w].size());
      for (unsigned int i = 0; i < points_prev.size(); i++) {
        points_prev[i] = points_interpolated[w][i];
      }
    }

    std::cout << "Converting the interpolation to the time domain" << std::endl;
    audio_interpolated[c] = 
      audio_transport::spectral::synthesis(points_interpolated, padding);
  }

  // Write the file
  std::cout << "Writing to file " << argv[2] << std::endl;
  audiorw::write(audio_interpolated, argv[2], sample_rate);
}
