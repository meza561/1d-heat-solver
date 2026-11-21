#include "heat.h"
//for cout and cin
#include <iostream>
//for istringstream so we can pull a number out of a line of text
#include <sstream>

using namespace std;


//read int but w/ doubles
//prints the prompt with the default in brackets then keeps asking until it gets something usable
double readDouble(const string& prompt, double defaultValue,
                  double minValue, double maxValue) {
    while (true) {                                   // repeat until we get a valid answer
        cout << prompt << " [" << defaultValue << "]: ";

        //grabs the whole line at once instead of cin >> so junk doesnt get stuck in the buffer
        string line;
        if (!getline(cin, line)) return defaultValue;   // input ended: use the default
        if (line.empty()) return defaultValue;          // user pressed Enter

        istringstream in(line);       // read the number out of the line
        double value;
        //extra is just a catch for anything left over after the number
        char extra;
        // fails if there's no number, or if anything follows it (like "12abc")
        if (!(in >> value) || (in >> extra)) {
            cout << "  Please enter a number.\n";
            continue;
        }
        //number is real but outside the allowed range so ask again
        if (value < minValue || value > maxValue) {
            cout << "  Must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }
        //passed every check so hand it back
        return value;
    }
}
//reads input ints from user or sets them to default values
//pretty much a copy of readDouble just with ints
int readInt(const string& prompt, int defaultValue, int minValue, int maxValue) {
    //same loop as readDouble keeps going until valid
    while (true) {
        cout << prompt << " [" << defaultValue << "]: ";

        //end of input or just hitting enter both give back the default
        string line;
        if (!getline(cin, line)) return defaultValue;
        if (line.empty()) return defaultValue;

        //turns the line into a stream to read the int out of
        istringstream in(line);
        int value;
        char extra;
        //no number or stuff after it like "3.5" or "4x" fails here
        if (!(in >> value) || (in >> extra)) {
            cout << "  Please enter a whole number.\n";
            continue;
        }
        //range check same as before
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
    //header telling the user which end theyre setting
    cout << "\nBoundary Condition at the " << sideName << " end (x = " << position << "):" << endl;

    //menu of BC options
    cout << "   1) Dirichlet   - Fixed temperature" << endl;
    cout << "   2) Neumann     - Fixed derivative" << endl;
    //only shows periodic on the left end since right just copies it if picked
    if (allowPeriodic) {
        cout << "   3) Periodic    - Ends wrap around" << endl;
    }
    //limits the choices to what was actually shown
    //default is periodic when allowed, otherwise dirichlet
    int maxChoice = allowPeriodic ? 3 : 2;
    int defaultChoice = allowPeriodic ? 3 : 1;
    int choice = readInt("Choice", defaultChoice, 1 , maxChoice);

    //sets the type and asks for the value that goes with it
    //no default case needed since readInt already keeps choice in range
    switch (choice) {
        //fixed temp so value is the temperature at that end
        case 1:
            type = BCType::Dirichlet;
            value = readDouble("   Temperature at the " + sideName + " end", 0.0, -1e6, 1e6);
            break;
        //fixed derivative so value is the slope du/dx at that end
        case 2:
            type = BCType::Neumann;
            value = readDouble("   Derivative at the " + sideName + " end", 0.0, -1e6, 1e6);
            break;
        //periodic doesnt need a value since the ends just connect
        case 3:
            type = BCType::Periodic;
            value = 0.0;
            break;
    }
}
