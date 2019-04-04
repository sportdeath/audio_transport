#include <iostream>
#include <audiorw.hpp>

#include <sample_info/spectral_point.hpp>

#include "audio_transport/audio_transport.hpp"

double sample_rate = 44100; // samples per second
double total_time = 10; // seconds
double window_size = 0.05; // seconds
int padding = 5000; // samples
bool synthesis_window = false;

int main(int argc, char ** argv) {

  if (argc != 2) {
    std::cout <<
      "Usage: " << argv[0] << " output_file"
      << std::endl;
    return 1;
  }

  // Construct sines at 440 and 880 hz
  std::vector<std::vector<double>> audio_left (1, std::vector<double>(sample_rate * total_time));
  std::vector<std::vector<double>> audio_right(1, std::vector<double>(sample_rate * total_time));
  for (size_t i = 0; i < sample_rate * total_time; i++) {
    double t = i/sample_rate_left;
    audio_left[0][i] = std::sin(2 * M_PI * 440 * t);
    audio_right[0][i] = std::sin(2 * M_PI * 880 * t);
  }

  // Initialize the output audio
  size_t num_channels = std::min(audio_left.size(), audio_right.size());
  std::vector<std::vector<double>> audio_interpolated(num_channels);

  // Initialize phases
  std::map<std::pair<size_t, size_t>, double> phases;

  // Iterate over the channels
  for (size_t c = 0; c < num_channels; c++) {

    std::cout << "Processing channel " << c << std::endl;

    std::cout << "Converting left input to the spectral domain" << std::endl;
    std::vector<std::vector<sample_info::spectral_point>> spectral_points_left =
      sample_info::spectral_analysis(audio_left[c], sample_rate_left, window_size, padding, synthesis_window);
    std::cout << "Converting right input to the spectral domain" << std::endl;
    std::vector<std::vector<sample_info::spectral_point>> spectral_points_right =
      sample_info::spectral_analysis(audio_right[c], sample_rate_right, window_size, padding, synthesis_window);

    std::cout << "Performing optimal transport based interpolation" << std::endl;
    size_t num_windows = std::min(spectral_points_left.size(), spectral_points_right.size());
    std::vector<std::vector<sample_info::spectral_point>> spectral_points_interpolated(num_windows);
    for (size_t w = 0; w < num_windows; w++) {
      double interpolation_factor = w/(double) num_windows;

      spectral_points_interpolated[w] = 
        audio_transport::interpolate(
          spectral_points_left[w],
          spectral_points_right[w],
          phases,
          window_size,
          interpolation_factor);
    }

    std::cout << "Converting the interpolation to the time domain" << std::endl;
    audio_interpolated[c] = 
      sample_info::spectral_synthesis(spectral_points_interpolated, padding, synthesis_window);
  }

  // Write the file
  std::cout << "Writing to file " << argv[1] << std::endl;
  audiorw::write(audio_interpolated, argv[1], sample_rate_left);
}
