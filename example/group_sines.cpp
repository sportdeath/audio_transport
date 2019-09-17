#include <iostream>
#include <fstream>
#include <cmath>
#include <audiorw.hpp>

#include "spectral.hpp"
#include "audio_transport.hpp"

double window_size = 1;
int window_index = 0;

int main(int argc, char ** argv) {

  if (argc != 2) {
    std::cout <<
      "Usage: " << argv[0] << " output_file"
      << std::endl;
    return 1;
  }

  // Construct some sines
  double sample_rate = 44100;
  std::vector<double> audio(sample_rate * 10);
  for (size_t i = 0; i < audio.size(); i++) {
    double t = i/sample_rate;
    audio[i] = 
      std::sin(2 * M_PI * 440 * t) + 
      std::sin(2 * M_PI * 550 * t) + 
      std::sin(2 * M_PI * 660 * t) +
      std::sin(2 * M_PI * 880 * t) +
      0.5 * std::sin(2 * M_PI * 350 * t) +
      0.5 * std::sin(2 * M_PI * 1000 * t);
  }

  std::cout << "Converting to the spectral domain" << std::endl;
  std::vector<std::vector<audio_transport::spectral::point>> points =
    audio_transport::spectral::analysis(audio, sample_rate, window_size);

  std::cout << "Grouping window #" << window_index << std::endl;
  std::vector<audio_transport::spectral_mass> masses = 
    audio_transport::group_spectrum(points[window_index]);

  std::cout << "Writing to output" << std::endl;
  std::ofstream output;
  output.open(argv[1]);
  for (auto mass :  masses) {
    for (size_t i = mass.left_bin; i < mass.right_bin; i++) {
      audio_transport::spectral::point point = points[window_index][i];
      output << 
        point.freq << " " << 
        point.freq_reassigned << " " << 
        std::abs(point.value) << " " << 
        (i == mass.left_bin) << " " <<
        (i == mass.center_bin) << std::endl;
    }
  }

  output.close();
}
