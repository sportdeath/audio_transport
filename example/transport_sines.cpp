#include <iostream>
#include <audiorw.hpp>

#include "spectral.hpp"
#include "audio_transport.hpp"

double sample_rate = 44100; // samples per second
double total_time = 5; // seconds
double window_size = 0.05; // seconds
unsigned int padding = 7; // multiplies window size

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
    double t = i/sample_rate;
    audio_left[0][i] =  0.7 * std::sin(2 * M_PI * 220 * t);
    //audio_right[0][i] = std::sin(2 * M_PI * 440 * t);
    audio_right[0][i] = 0.7 * (2 * std::fmod(220 * t, 1) - 1);
  }

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

    // Zero out every other window
    //for (unsigned int i = 0; i < points_left.size(); i++)
      //if (i % 2 != 0)
        //for (unsigned int j = 0; j < points_left[i].size(); j++)
          //points_left[i][j].value = 0.00000001;
    //for (unsigned int i = 0; i < points_right.size(); i++)
      //if (i % 2 != 0)
        //for (unsigned int j = 0; j < points_right[i].size(); j++)
          //points_right[i][j].value = 0.00000001;

    // Initialize phases
    std::vector<double> phases(points_left[0].size(), 0);

    std::cout << "Performing optimal transport based interpolation" << std::endl;
    size_t num_windows = std::min(points_left.size(), points_right.size());
    std::vector<std::vector<audio_transport::spectral::point>> points_interpolated(num_windows);
    for (size_t w = 0; w < num_windows; w++) {
      double interpolation_factor = w/(double) num_windows;
      interpolation_factor = interpolation_factor * 2 - 0.5;
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
  std::cout << "Writing to file " << argv[1] << std::endl;
  audiorw::write(audio_interpolated, argv[1], sample_rate);
}
