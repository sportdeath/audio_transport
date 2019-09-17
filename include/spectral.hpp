#pragma once

#include <vector>
#include <complex>

namespace audio_transport {
namespace spectral {

struct point {
  std::complex<double> value;

  double time;
  double freq;
  
  double time_reassigned;
  double freq_reassigned;
};

/**
 * Analyze an audio signal to produce an array of spectral points.
 * Points are reduced to mono.
 */
std::vector<std::vector<point>> analysis(
    const std::vector<double> & audio,
    double sample_rate,
    double window_size = 0.05, // seconds
    unsigned int padding = 0,
    unsigned int overlap = 1
    );

/**
 * Synthesize an audio signal from an array of spectral points.
 */
std::vector<double> synthesis(
    const std::vector<std::vector<point>> & points,
    unsigned int padding = 0,
    unsigned int overlap = 1
    );

/**
 * A Hamming window, chosen because it is COLA
 * and easy to compute
 *
 * -(N + 1)/2 < n < (N + 1)/2
 */
double hann(double n, double N);

// Derivatives and time-weightings of the hamming window
double hann_t (double n, double N, double sample_rate);
double hann_d (double n, double N, double sample_rate);

}}
