#include <iostream>
#include <cmath>
#include "heat.h"

using namespace std;

//initial condition profile f0(x)
//the root of it all
double initialCondition(double x, double xmin, double length) {
    double s = (x - xmin) / length;
    //scaling position to (0,1)
    return exp(-200 * (s - 0.5) * (s - 0.5));
}

// trapezoid rule total heat: integral of u over [xmin, xmax]
// exempts periodic bcs due to ring behavior
double totalHeat(const vector<double>& u, double dx, bool periodic) {
    int n = u.size();
    double sum = 0.0;
    //sum of "heat" of each entry in each cell
    for (int i= 0; i < n; i++) sum += u[i];
    //subtracts average of start and end points when not wrapping around
    if (!periodic) sum -= 0.5 * (u[0] + u[n - 1]);
    //sum times space in x dimension will give us total heat
    return sum * dx;
}

int main() {
    //just for sight in terminal no actual purpose
    cout << "1D heat equation  u_t - k u_xx = 0  (forward Euler)\n";
    cout << "Press Enter to accept the value in [brackets].\n\n";

    //reads what the xmin is based on user entry has a default val of -1
    double xmin = readDouble("Left end of domain, xmin", -1.0, -1e6, 1e6);

    // xmax depends on xmin, so it gets its own check
    double xmax;
    do {
        //same read as xmin for the xmax now and holds default value 0f 1 making default interval [-1,1]
        xmax = readDouble("Right end of domain, xmax", 1.0, -1e6, 1e6);
        //check to make sure that xmax > xmin
        if (xmax <= xmin) cout << "  xmax must be greater than xmin (" << xmin << ").\n";
    } while (xmax <= xmin);

    //final minus initial idk pretty self explanatory
    double length = xmax - xmin;

    //diffusion coefficient + intervales + total time measured + CFL condition value (r < 0.5) user inputs
    double k = readDouble("Diffusion coefficient, k", 1.0, 1e-9, 1e6);
    int intervals = readInt("Number of intervals, N", 160, 2, 2000);
    double finalTime = readDouble("Final time, T", 0.1, 1e-9, 1e3);
    double targetR = readDouble("Target CFL condition r = k*dt/dx^2 (needs to be <= 0.5)",0.45, 0.01, 1.0);
    cout << "\n";

    //number of snapshots, i held it constant but i GUESS u could make it user input i just didnt want to
    int nFrames = 200;
    string outFile = "heat.csv";

    //BC filled by readbc by reference
    BCType leftType = BCType::Dirichlet, rightType = BCType::Dirichlet;

    //initializing value variables for either derivative values (neumann) or constants (dirichlet) for BC
    double leftValue = 0.0;
    double rightValue = 0.0;

    //just makes sure if one end is period then the other is to otherwise it just calls to read bc based on user input
    readBC("left", xmin, true, leftType, leftValue);
    if (leftType == BCType::Periodic) {
        rightType = BCType::Periodic; // periodic pairs to periodic
        rightValue = 0.0;
        cout << "  Right end is periodic as well" << endl;
    } else {
        //gives options for otherside if not periodic not allowing period on this side if not on other side
        readBC("right", xmax, false, rightType, rightValue);
    }
    //just a space
    cout << endl;

  

    // makes sure periodic BC is set for both sides
    // returns 1 (standard for bug)
    bool periodic = (leftType == BCType::Periodic);
    if ((leftType == BCType::Periodic) != (rightType == BCType::Periodic)) {
        cerr << "Error: Periodic BC must be set for both ends" << endl;
        return 1;
    }

    //spacing on the grid
    //reflected in total heat
    double dx = length / intervals;

    //intervals + 1 is gridpoints so that it measures as x0, x1, x2, etc.
    //takes one writes to another and copies it
    int gridpoints = periodic ? intervals : intervals + 1;
    vector<double> x(gridpoints);
    vector<double> u(gridpoints, 0.0);
    vector<double> unew(gridpoints, 0.0);

    //filling x
    //for each value it is dependent on dx, i and the xmin value
    for (int i = 0; i < gridpoints; ++i) {
        x[i] = xmin + i * dx;
    }

    //filling u bsed on ic
    // uses each x from before into a u
    for (int i = 0; i < gridpoints; ++i) {
        u[i] = initialCondition(x[i], xmin, length);
    }

    // apply Dirichlet BCs at t = 0 (Neumann endpoints keep f(x))
    // differs from derivative
    if (leftType == BCType::Dirichlet) {
        u[0] = leftValue;
    } else if (rightType == BCType::Dirichlet) {
        u[gridpoints - 1] = rightValue;
    }


    // calculation of the CFL condition
    // (dx)^2/2dk
    double dtMax = dx * dx / (2.0 * k);
    double dt = targetR * dx * dx / k;

    //check to make sure it doesnt explode and make heat go to 10e14 or something crazy
    if (dt > dtMax) {
        cout << "[stability] dt = " << dt << " exceeds dtMax = " << dtMax << ", clamping" << endl;
        dt = 0.99 * dtMax;
    }

    //calculates how many steps to reach final time, then rounding up and shrinking dt by a bit
    //lands on T exactly eventually then only makes r smaller thus safer
    int nSteps = static_cast<int>(ceil(finalTime/ dt));
    dt = finalTime / nSteps;

    //sets r to kdt/(dx)^2
    double r = k * dt / (dx * dx);
    //just display output of each value
    cout << "dx = " << dx << "  dt = " << dt << "   dt max = " << dtMax << "    r = " << r << "    steps = " << nSteps << endl;

    // time steps

    //5 step snapshot of progress
    double time = 0.0;

    int printEvery = nSteps / 5;
    if (printEvery < 1) printEvery = 1;

    cout << "heat at t = 0: " << totalHeat(u, dx, periodic) << endl;

    //open file and write header
    ofstream csv(outFile);
    //just makes sure file is readable and not some random stuff
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

    //makes sure atleast a frame is kept
    int saveEvery = nSteps / nFrames;
    if (saveEvery < 1) saveEvery = 1;

    // time loop
    //main body of calculations
    for (int n = 1; n<= nSteps; ++n) {
    // read from u old and write to u new
        for (int i = 1; i < gridpoints -1; i++) {
            unew[i] = u[i] + r * (u[i + 1] - 2.0 * u[i] + u[i - 1]);
        }

        int last = gridpoints - 1;

        // same formula as u new
        // left boundary (i = 0)
        if (leftType == BCType::Dirichlet) {
            unew[0] = leftValue;
        } else if (leftType == BCType::Neumann){
            double ghostL = u[1] - 2.0 * dx * leftValue;
            unew[0] = u[0] + r * (u[1] - 2.0 * u[0] + ghostL);
        } else {
            unew[0] = u[0] + r * (u[1] - 2.0 * u[0] + u[last]);
        }

        // same formula as u new
        // right boundary (i = N)
        if (rightType == BCType::Dirichlet) {
            unew[last] = rightValue;
        } else if (rightType == BCType::Neumann){
            double ghostR = u[last - 1] + 2.0 * dx * rightValue;
            unew[last] = u[last] + r * (ghostR - 2.0 * u[last] + u[last - 1]);
        } else { //periodic
            unew[last] = u[last] + r * (u[0] - 2.0 * u[last] + u[last - 1]);
        }

        //turns u into the new unew that was calculated for the next step
        u.swap(unew);
        time += dt;

        //write frame to csv
        //writes the newly updated u to it
        if (n % saveEvery == 0 || n == nSteps) {
            writeRow(csv, time, u);
            frames.push_back(u);
        }

        //just small check for making sure points are existing
        if (n % printEvery == 0 || n == nSteps) {
            double umax = u[0];
            for (int i = 1; i < gridpoints; i++) {
                if (u[i] > umax) umax = u[i];
            }
            cout << "step " << n << "   time = " << time << "   max u = " << umax << endl;
        }

    }

    cout << "heat at t = " << time << ": " << totalHeat(u, dx, periodic) << endl;

    writePPM("heat.ppm", frames, 600, 400);
    return 0;
}
//thank you for grading :)