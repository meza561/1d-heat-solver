#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include "output.h"
#include "input.h"

using namespace std;

//initial condition profile f0(x)
double initialCondition(double x, double xmin, double length) {
    double s = (x - xmin) / length;
    //scaling position to (0,1)
    return exp(-200 * (s - 0.5) * (s - 0.5));
}

// Trapezoid-rule total heat: integral of u over [0, L]
double totalHeat(const vector<double>& u, double dx, bool periodic) {
    int n = u.size();
    double sum = 0.0;
    for (int i= 0; i < n; i++) sum += u[i];
    if (!periodic) sum -= 0.5 * (u[0] + u[n - 1]);

    // 0.5 * u[0] + 0.5 * u[n - 1];
    // for (int i = 1; i < n - 1; ++i) sum += u[i];
    return sum * dx;
}

//BC types
enum class BCType{Dirichlet, Neumann, Periodic};

// asks for one end of the BC
// value and type are based by reference so the function can fill them in
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
            value = readDouble("   Temperature at the " + sideName + " end", 0.0, 1e-6, 1e6);
            break;
        case 2:
            type = BCType::Neumann;
            value = readDouble("   Derivative at the " + sideName + "end", 0.0, 1e-6, 1e6);
            break;
        case 3:
            type = BCType::Periodic;
            value = 0.0;
            break;
    }
}

int main() {

    cout << "1D heat equation  u_t - k u_xx = 0  (forward Euler)\n";
    cout << "Press Enter to accept the value in [brackets].\n\n";

    double xmin = readDouble("Left end of domain, xmin", -1.0, -1e6, 1e6);

    // xmax depends on xmin, so it gets its own check
    double xmax;
    do {
        xmax = readDouble("Right end of domain, xmax", 1.0, -1e6, 1e6);
        if (xmax <= xmin) cout << "  xmax must be greater than xmin (" << xmin << ").\n";
    } while (xmax <= xmin);
    double length = xmax - xmin;

    double k = readDouble("Diffusion coefficient, k", 1.0, 1e-9, 1e6);
    int intervals = readInt("Number of intervals, N", 160, 2, 2000);
    double finalTime = readDouble("Final time, T", 0.1, 1e-9, 1e3);
    double targetR = readDouble("Target CFL condition r = k*dt/dx^2 (needs to be <= 0.5)",0.45, 0.01, 1.0);
    cout << "\n";

    //number of snapshots
    int nFrames = 200;
    string outFile = "heat.csv";

    //BC filled by readbc by reference
    BCType leftType = BCType::Dirichlet, rightType = BCType::Dirichlet;
    double leftValue = 0.0;
    double rightValue = 0.0;

    readBC("left", xmin, true, leftType, leftValue);
    if (leftType == BCType::Periodic) {
        rightType = BCType::Periodic; // periodic pairs to periodic
        rightValue = 0.0;
        cout << "  Right end is periodic as well" << endl;
    } else {
        readBC("right", xmax, false, rightType, rightValue);
    }
    cout << endl;

    // //u(0, t) / u_x(0,t)
    // BCType leftType = BCType::Periodic;
    // double leftValue = 0.0;
    // //u(L, t) / u_x(L,t)
    // BCType rightType = BCType::Periodic;
    // double rightValue = 0.0;

    // makes sure periodic BC is set for both sides
    bool periodic = (leftType == BCType::Periodic);
    if ((leftType == BCType::Periodic) != (rightType == BCType::Periodic)) {
        cerr << "Error: Periodic BC must be set for both ends" << endl;
        return 1;
    }

    //spacing on the grid
    double dx = length / intervals;

    //intervals + 1 is gridpoints so that it measures as x0, x1, x2, etc.
    int gridpoints = periodic ? intervals : intervals + 1;
    vector<double> x(gridpoints);
    vector<double> u(gridpoints, 0.0);
    vector<double> unew(gridpoints, 0.0);

    //filling x
    for (int i = 0; i < gridpoints; ++i) {
        x[i] = xmin + i * dx;
    }

    //filling u
    for (int i = 0; i < gridpoints; ++i) {
        u[i] =initialCondition(x[i], xmin, length);
    }

    // Apply Dirichlet BCs at t = 0 (Neumann endpoints keep f(x))
    if (leftType == BCType::Dirichlet)  u[0] = leftValue;
    if (rightType == BCType::Dirichlet) u[gridpoints - 1] = rightValue;

    // calculation of the CFL condition
    double dtMax = dx * dx / (2.0 * k);
    double dt = targetR * dx * dx / k;

    if (dt > dtMax) {
        cout << "[stability] dt = " << dt << " exceeds dtMax = " << dtMax << ", clamping" << endl;
        dt = 0.99 * dtMax;
    }

    //calculates how many steps to reach final time, then rounding up and shrinking dt by a bit
    //lands on T exactly eventually then only makes r smaller thus safer
    int nSteps = static_cast<int>(ceil(finalTime/ dt));
    dt = finalTime / nSteps;

    double r = k * dt / (dx * dx);

    cout << "dx = " << dx << "  dt = " << dt << "   dt max = " << dtMax << "    r = " << r << "    steps = " << nSteps << endl;

    // time steps

    //5 step snapshot
    double time = 0.0;

    int printEvery = nSteps / 5;
    if (printEvery < 1) printEvery = 1;

    cout << "heat at t = 0: " << totalHeat(u, dx, periodic) << endl;

    //open file and write header
    ofstream csv(outFile);
    if (!csv) {
        cerr << "Error: Could not open " << outFile << endl;
        return 1;
    }

    // header for x positions
    csv << "t";
    for (double xi : x) csv << "," << xi;
    csv << "\n";

    // IC snapshot
    writeRow(csv, time, u);

    //making image
    vector<vector<double>> frames;
    frames.push_back(u);

    int saveEvery = nSteps / nFrames;
    if (saveEvery < 1) saveEvery = 1;

    // time loop
    for (int n = 1; n<= nSteps; ++n) {
    // read from u old and write to u new
        for (int i = 1; i < gridpoints -1; i++) {
            unew[i] = u[i] + r * (u[i + 1] - 2.0 * u[i] + u[i - 1]);
        }

        int last = gridpoints - 1;

        // left boundary (i = 0)
        if (leftType == BCType::Dirichlet) {
            unew[0] = leftValue;
        } else if (leftType == BCType::Neumann){
            double ghostL = u[1] - 2.0 * dx * leftValue;
            unew[0] = u[0] + r * (u[1] - 2.0 * u[0] + ghostL);
        } else {
            unew[0] = u[0] + r * (u[1] - 2.0 * u[0] + u[last]);
        }

        // right boundary (i = N)
        if (rightType == BCType::Dirichlet) {
            unew[last] = rightValue;
        } else if (rightType == BCType::Neumann){
            double ghostR = u[last - 1] + 2.0 * dx * rightValue;
            unew[last] = u[last] + r * (ghostR - 2.0 * u[last] + u[last - 1]);
        } else { //periodic
            unew[last] = u[last] + r * (u[0] - 2.0 * u[last] + u[last - 1]);
        }

        u.swap(unew);
        time += dt;

        //write frame to csv
        if (n % saveEvery == 0 || n == nSteps) {
            writeRow(csv, time, u);
            frames.push_back(u);
        }

        if (n % printEvery == 0 || n == nSteps) {
            double umax = u[0];
            for (int i =1; i < gridpoints; i++) {
                if (u[i] > umax) umax = u[i];
            }
            cout << "step " << n << "   time = " << time << "   max u = " << umax << endl;
        }

    }

    //max error calculation
    double maxError = 0.0;
    for (int i = 0; i < gridpoints; i++) {
        double exact = exp(-4.0 * M_PI * M_PI * k * time / (length * length)) * sin(2.0 * M_PI * (x[i] - xmin) / length);
        maxError = max(maxError, fabs(u[i] - exact));
    }
    cout << "Max error VS. Exact: " << maxError << endl;

    cout << "heat at t = " << time << ": " << totalHeat(u, dx, periodic) << endl;

    cout << u[gridpoints - 1] << endl;

    //print the grid in x[i] = c1 u[i] = c2 form
    // for (int i = 0; i < gridpoints; ++i) {
    //     cout << "x[" << i << "] = " << x[i];
    //     cout << "   u[" << i << "] = " << u[i] << endl;
    // }
    writePPM("heat.ppm", frames, 600, 400);
    return 0;
}
