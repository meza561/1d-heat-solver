#ifndef HEAT_H
#define HEAT_H
//could do 2-3 different header files but i dont care enough to do that so just one "heat" one
#include <fstream>
#include <string>
#include <vector>

// -- output -- //

// write one CSV snapshot: time, then every u value
void writeRow(std::ofstream& file, double t, const std::vector<double>& u);

// space-time heat map as a binary PPM image: x across, t down
void writePPM(const std::string& path, const std::vector<std::vector<double>>& frames, int width, int height);

// -- input -- //

// Prompt until the user enters a whole number in [minValue, maxValue].
// Pressing Enter returns defaultValue.
int readInt(const std::string& prompt, int defaultValue, int minValue, int maxValue);

// Same, for decimal numbers
double readDouble(const std::string& prompt, double defaultValue, double minValue, double maxValue);

// -- BCs -- //

//BC types
enum class BCType{Dirichlet, Neumann, Periodic};
//reads which bc type is input by user
void readBC(const std::string& sideName, double position, bool allowPeriodic, BCType& type, double& value);

#endif // HEAT_H
