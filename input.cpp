#include "input.h"
#include <iostream>
#include <sstream>

using namespace std;

double readDouble(const string& prompt, double defaultValue,
                  double minValue, double maxValue) {
    while (true) {                                   // repeat until we get a valid answer
        cout << prompt << " [" << defaultValue << "]: ";

        string line;
        if (!getline(cin, line)) return defaultValue;   // input ended: use the default
        if (line.empty()) return defaultValue;          // user pressed Enter

        istringstream in(line);       // read the number out of the line
        double value;
        char extra;
        // fails if there's no number, or if anything follows it (like "12abc")
        if (!(in >> value) || (in >> extra)) {
            cout << "  Please enter a number.\n";
            continue;
        }
        if (value < minValue || value > maxValue) {
            cout << "  Must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }
        return value;
    }
}

int readInt(const string& prompt, int defaultValue, int minValue, int maxValue) {
    while (true) {
        cout << prompt << " [" << defaultValue << "]: ";

        string line;
        if (!getline(cin, line)) return defaultValue;
        if (line.empty()) return defaultValue;

        istringstream in(line);
        int value;
        char extra;
        if (!(in >> value) || (in >> extra)) {
            cout << "  Please enter a whole number.\n";
            continue;
        }
        if (value < minValue || value > maxValue) {
            cout << "  Must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }
        return value;
    }
}