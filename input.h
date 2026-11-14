#pragma once

#include <string>

// Prompt until the user enters a whole number in [minValue, maxValue].
// Pressing Enter returns defaultValue.
int readInt(const std::string& prompt, int defaultValue, int minValue, int maxValue);

// Same, for decimal numbers
double readDouble(const std::string& prompt, double defaultValue, double minValue, double maxValue);