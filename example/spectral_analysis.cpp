#include <iostream>
#include <fstream>
#include <cmath>
#include <audiorw.hpp>

#include <audio_transport/spectral.hpp>
#include <audio_transport/equal_loudness.hpp>

double window_size = 0.05; // seconds
unsigned int padding = 2; // multiplies window size

int main(int argc, char ** argv) {

  if (argc != 2) {
    std::cout <<
      "Usage: " << argv[0] << " input_audio"
      << std::endl;
    return 1;
  }

  // Read the file
  std::cout << "Reading from file " << argv[1] << std::endl;
  double sample_rate;
  std::vector<std::vector<double>> audio =
    audiorw::read(argv[1], sample_rate);
  std::cout << "audio size: " << audio.size() << ", " << audio[0].size() << std::endl;

  // Computing frequency reassignment
  std::cout << "Computing the frequency reassignment" << std::endl;
  std::vector<std::vector<audio_transport::spectral::point>> points =
    audio_transport::spectral::analysis(audio[0], sample_rate, window_size, padding);

  // Equal loudness
  std::cout << "Applying equal loudness filter" << std::endl;
  audio_transport::equal_loudness::apply(points);

  // Write the file
  std::cout << "Writing to file " << argv[1] << ".txt" << std::endl;
  std::ofstream output;
  output.open(std::string(argv[1]) + ".txt");
  for (auto p_ : points) {
    for (audio_transport::spectral::point p : p_) {
      output << p.time_reassigned  << " " << p.freq_reassigned << " " << std::abs(p.value) << std::endl;
    }
  }
  output.close();
  
  return 0;
}
