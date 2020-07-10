#pragma once

#include <vector>

#include "audio_transport/spectral.hpp"

namespace audio_transport {
namespace equal_loudness {

double a_weighting_amp(double freq);

void apply(
    std::vector<std::vector<spectral::point>> & points);
void remove(
    std::vector<std::vector<spectral::point>> & points);

}}
