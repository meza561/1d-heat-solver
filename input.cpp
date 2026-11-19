#include "heat.h"
#include <iostream>
#include <sstream>

using namespace std;
//read int but w/ doubles
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
//reads input ints from user or sets them to default values
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

// asks for one end of the BC
// value and type are passed by reference so the function can fill them in
void readBC(const string& sideName, double position, bool allowPeriodic, BCType& type, double& value) {
    cout << "\nBoundary Condition at the " << sideName << " end (x = " << position << "):" << endl;

    cout << "   1) Dirichlet   - Fixed temperature" << endl;
    cout << "   2) Neumann     - Fixed derivative" << endl;
    if (allowPeriodic) {
        cout << "   3) Periodic    - Ends wrap around" << endl;
    }
    int maxChoice = allowPeriodic ? 3 : 2;
    int defaultChoice = allowPeriodic ? 3 : 1;
    int choice = readInt("Choice", defaultChoice, 1 , maxChoice);

    switch (choice) {
        case 1:
            type = BCType::Dirichlet;
            value = readDouble("   Temperature at the " + sideName + " end", 0.0, -1e6, 1e6);
            break;
        case 2:
            type = BCType::Neumann;
            value = readDouble("   Derivative at the " + sideName + " end", 0.0, -1e6, 1e6);
            break;
        case 3:
            type = BCType::Periodic;
            value = 0.0;
            break;
    }
}
