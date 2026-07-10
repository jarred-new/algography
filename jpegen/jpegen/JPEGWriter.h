#pragma once

#include <string>

bool WriteJPEG(
    const std::string& filename,
    int width,
    int height,
    int quality,
    const unsigned char* rgbData);