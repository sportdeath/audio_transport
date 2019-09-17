#include <vector>
#include <cmath>
#include <complex>
#include <ciso646>
#include <cassert>

#include <fftw3.h>

#include "audio_transport/spectral.hpp"

using namespace audio_transport;

std::vector<double> audio_transport::spectral::synthesis(
    const std::vector<std::vector<spectral::point>> & points,
    unsigned int padding,
    unsigned int overlap) {

  // Initialize the window
  std::vector<double> window_padded(2 * (points[0].size() - 1));
  size_t window_size = window_padded.size()/(1 + padding);
  size_t padding_samples = (window_padded.size() - window_size)/2;

  // Initialize the audio
  // Accounting for an overlap factor of 2 * overlap
  size_t hop_size = window_size/(2 * overlap);
  size_t num_hops = points.size() + 2 * overlap - 1;
  std::vector<double> audio(num_hops * hop_size, 0);

  // Initialize FFT
  fftw_complex * fft;
  fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * points[0].size());
  fftw_plan fft_plan = fftw_plan_dft_c2r_1d(
      window_padded.size(),
      fft,
      window_padded.data(),
      FFTW_MEASURE);

  // Iterate over the windows
  for (size_t w = 0; w < points.size(); w++) {

    // Fill the FFT
    for (size_t i = 0; i < points[w].size(); i++) {
      fft[i][0] = std::real(points[w][i].value);
      fft[i][1] = std::imag(points[w][i].value);
    }
    
    // Execute the plans
    fftw_execute(fft_plan);

    // Apply the weighted overlap add
    for (size_t i = 0; i < window_size; i++) {
      // Scale down to correct for FFT and overlap sizes
      double value = window_padded[i + padding_samples]/(overlap * window_padded.size());

      // Add it to the overlapped signal
      audio[i + w * window_size/(2 * overlap)] += value;
    }
  }

  // Cleanup
  fftw_destroy_plan(fft_plan);
  fftw_free(fft);

  return audio;
}

std::vector<std::vector<audio_transport::spectral::point>> audio_transport::spectral::analysis(
    const std::vector<double> & audio,
    double sample_rate,
    double window_size,
    unsigned int padding,
    unsigned int overlap) {

  // Make sure inputs are positive
  assert(sample_rate > 0);
  assert(window_size > 0);

  // Convert the window size to samples
  size_t N = std::round(window_size * sample_rate);
  // Make sure it is even for symmetry
  while (N % (2 * overlap) != 0) N += 1;
  size_t N_padded = N * (1 + padding);
  // Initialize the windows
  std::vector<double> window(N_padded, 0), window_t(N_padded, 0), window_d(N_padded, 0);

  // Determine samples used for padding
  size_t padding_samples = (N_padded - N)/2;

  // Compute the number of windows
  // Accounting for an overlap factor of 2 * overlap
  size_t num_hops = std::floor(audio.size()/(N/(2 * overlap)));
  size_t num_windows = num_hops - (2 * overlap - 1);

  // Initialize FFT
  size_t fft_size = N_padded/2 + 1;
  fftw_complex * fft, * fft_t, * fft_d;
  fft   = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_size);
  fft_t = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_size);
  fft_d = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_size);
  fftw_plan fft_plan    = fftw_plan_dft_r2c_1d(
      window.size(),
      window.data(),
      fft,
      FFTW_MEASURE);
  fftw_plan fft_plan_t  = fftw_plan_dft_r2c_1d(
      window_t.size(),
      window_t.data(),
      fft_t,
      FFTW_MEASURE);
  fftw_plan fft_plan_d  = fftw_plan_dft_r2c_1d(
      window_d.size(),
      window_d.data(),
      fft_d,
      FFTW_MEASURE);

  // Initialize the spectral points
  std::vector<std::vector<spectral::point>> points(num_windows);

  // Iterate over the windows
  for (size_t w = 0; w < num_windows; w++) {

    // Reserve space for each spectral point in each channel
    points[w].reserve(fft_size);

    // Apply the various windows
    for (size_t i = 0; i < N; i++) {
      // The sample index of with window
      // if the center of the window has n = 0
      double n = i - (N - 1)/2.;

      // The audio sample to window accounting for overlap of 2 * overlap
      double a = audio[i + w * N/(2 * overlap)];

      // Apply the various windows
      window  [i + padding_samples] = a * hann  (n, N);
      window_t[i + padding_samples] = a * hann_t(n, N, sample_rate);
      window_d[i + padding_samples] = a * hann_d(n, N, sample_rate);
    }

    // Execute the plans
    fftw_execute(fft_plan);
    fftw_execute(fft_plan_t);
    fftw_execute(fft_plan_d);

    // Compute the center time
    double t = ((N - 1)/2. + w * N/(2 * overlap))/sample_rate;

    for (size_t i = 0; i < fft_size; i++) {
      // Convert to C++ complex
      std::complex<double> X   (fft   [i][0], fft   [i][1]);
      std::complex<double> X_t (fft_t [i][0], fft_t [i][1]);
      std::complex<double> X_d (fft_d [i][0], fft_d [i][1]);

      // Begin to construct a spectral point
      spectral::point p;
      p.value = X;
      p.time = t;
      p.freq = (2 * M_PI * i * sample_rate)/(double) N_padded;

      // Compute how the frequency and time changed
      std::complex<double> conj_over_norm = std::conj(X)/std::norm(X);
      double dphase_domega =  std::real(X_t * conj_over_norm);
      double dphase_dt     = -std::imag(X_d * conj_over_norm);

      // Compute the reassigned time and frequency
      p.time_reassigned = p.time + dphase_domega;
      p.freq_reassigned = p.freq + dphase_dt;

      // Add the point
      points[w].push_back(p);
    }
  }

  // Cleanup
  fftw_destroy_plan(fft_plan);
  fftw_destroy_plan(fft_plan_t);
  fftw_destroy_plan(fft_plan_d);
  fftw_free(fft);
  fftw_free(fft_t);
  fftw_free(fft_d);

  return points;
}

double audio_transport::spectral::hann(
    double n,
    double N) {
  return 0.5 + 0.5 * std::cos(2 * M_PI * n/(N - 1));
}

double audio_transport::spectral::hann_t(
    double n,
    double N,
    double sample_rate) {
  return (n/sample_rate) * hann(n, N);
}

double audio_transport::spectral::hann_d(
    double n,
    double N,
    double sample_rate) {
  return - (M_PI * sample_rate)/(N - 1) * std::sin(2 * M_PI * n/(N - 1));
}
