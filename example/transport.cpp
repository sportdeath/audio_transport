#include <iostream>
#include <audiorw.hpp>

#include <sample_info/spectral_point.hpp>

#include "audio_transport/audio_transport.hpp"

int padding = 0;
double window_size = 0.05;
bool synthesis_window = false;

int main(int argc, char ** argv) {

  if (argc != 2) {
    std::cout <<
      "Usage: " << argv[0] << " output_file"
      << std::endl;
    return 1;
  }

  // Read the files
  //std::cout << "Reading left from file " << argv[1] << std::endl;
  //double sample_rate_left;
  //std::vector<std::vector<double>> audio_left =
    //audiorw::read(argv[1], sample_rate_left);
  //std::cout << "Reading right from file " << argv[2] << std::endl;
  //double sample_rate_right;
  //std::vector<std::vector<double>> audio_right =
    //audiorw::read(argv[2], sample_rate_right);

  // Construct some sines
  double sample_rate_left = 44100;
  double sample_rate_right = 44100;
  std::vector<std::vector<double>> audio_left(1, std::vector<double>(sample_rate_left * 10));
  std::vector<std::vector<double>> audio_right(1, std::vector<double>(sample_rate_right * 10));
  for (size_t i = 0; i < audio_left[0].size(); i++) {
    double t = i/sample_rate_left;
    audio_left[0][i] = std::sin(2 * M_PI * 440 * t);
  }
  for (size_t i = 0; i < audio_left[0].size(); i++) {
    double t = i/sample_rate_right;
    //audio_right[0][i] = 0.5 * std::sin(2 * M_PI * 880 * t) + 0.5 * std::sin(2 * M_PI * 220 * t);
    audio_right[0][i] = std::sin(2 * M_PI * 880 * t);
  }

  if (sample_rate_left != sample_rate_right) {
    std::cout << "Samples rates are not equal! " << sample_rate_left << " != " << sample_rate_right << std::endl;
    return 1;
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

      //if (w % 2 == 0) {
        //for (int i = 0; i < spectral_points_interpolated[w].size(); i++) {
          //spectral_points_interpolated[w][i].value = 0;
        //}
      //}
    }

    std::cout << "Converting the interpolation to the time domain" << std::endl;
    audio_interpolated[c] = 
      sample_info::spectral_synthesis(spectral_points_interpolated, padding, synthesis_window);
  }

  // Write the file
  std::cout << "Writing to file " << argv[1] << std::endl;
  audiorw::write(audio_interpolated, argv[1], sample_rate_left);
}
