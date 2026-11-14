#pragma once

#include <fstream>
#include <string>
#include <vector>

// write one CSV snapshot: time, then every u value
void writeRow(std::ofstream& file, double t, const std::vector<double>& u);

// space-time heat map as a binary PPM image: x across, t down
void writePPM(const std::string& path, const std::vector<std::vector<double>>& frames, int width, int height);