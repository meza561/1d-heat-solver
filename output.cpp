//   - writeRow:  appends one time snapshot to a CSV file
//   - colormap:  turns a normalized value into a color
//   - writePPM:  renders all snapshots as a space-time heat map image

#include "heat.h"   // declarations for writeRow / writePPM (and fstream, vector, string)
#include <iostream>

using namespace std;

// snapshot as well as helper function for main
// Writes one CSV line: the time t first, then every grid value u[0..n-1],
// all comma-separated. Called once per saved time step, so the file ends up
// with one row per snapshot and one column per grid point (plus the time column).
void writeRow(ofstream& file, double t, const vector <double>& u) {
    file << t;                                  // first column: simulation time
    for (double v : u) file << "," << v;        // remaining columns: solution at each grid point
    file << "\n";                               // end of this snapshot's row
}

// One pixel's color: 8 bits each for red, green, blue (0-255).
// This is exactly the byte layout a binary PPM (P6) file expects per pixel.
struct RGB { unsigned char r, g, b; };

// maps s in [0, 1] to a color: black -> purple -> red -> orange -> pale yellow
// The 5 stops approximate the "inferno" palette; colors between stops are
// found by linear interpolation. `static` keeps this helper private to this file.
static RGB colormap(double s) {
    // Clamp the input so rounding error or bad data can't index outside the table
    if (s < 0.0) s = 0.0;
    if (s > 1.0) s = 1.0;

    // Color table: each row is an {R, G, B} anchor color, evenly spaced over [0, 1]
    const double stops[5][3] = {
        {  0,   0,   4},    // s = 0.00  black
        { 87,  16, 110},    // s = 0.25  purple
        {188,  55,  84},    // s = 0.50  red
        {249, 142,   9},    // s = 0.75  orange
        {252, 255, 164}     // s = 1.00  pale yellow
    };

    double pos = s * 4.0;              // which pair of stops s falls between
    int i = static_cast<int>(pos);
    if (i > 3) i = 3;                  // s = 1 would give i = 4, past the last pair
    double f = pos - i;                // how far between stop i and stop i+1

    // Blend each channel: start at stop i and move fraction f toward stop i+1.
    // The cast truncates the result to a 0-255 byte.
    RGB c;
    c.r =static_cast<unsigned char>(stops[i][0] + f * (stops[i + 1][0] - stops[i][0]));
    c.g = static_cast<unsigned char>(stops[i][1] + f * (stops[i + 1][1] - stops[i][1]));
    c.b = static_cast<unsigned char>(stops[i][2] + f * (stops[i + 1][2] - stops[i][2]));
    return c;
}

// Space-time heat map: x across, t down (t = 0 at the top)
// frames[k] is the full solution u at the k-th saved snapshot. The image is
// width x height pixels; each pixel samples the nearest snapshot (row) and
// grid point (column), so the picture can be larger or smaller than the data.
// Output is a binary PPM (P6), a minimal image format most viewers can open.
void writePPM(const string& path, const vector<vector<double>>& frames,
              int width, int height) {
    // color range over the whole run
    // Find the global min and max of u across every snapshot, so the same
    // color means the same value everywhere in the image.
    double umin = frames[0][0], umax = frames[0][0];
    for (const auto& f : frames)
        for (double v : f) {
            if (v < umin) umin = v;
            if (v > umax) umax = v;
        }
    // Width of the value range; falls back to 1 if u is constant to avoid dividing by zero
    double span = (umax > umin) ? (umax - umin) : 1.0;

    int nFr = frames.size();      // number of snapshots (time axis, vertical)
    int nPts = frames[0].size();  // number of grid points (space axis, horizontal)

    // Open in binary mode so pixel bytes are written exactly, with no newline translation.
    // PPM header: magic "P6", image size, then max channel value (255 = one byte per channel).
    ofstream img(path, ios::binary);
    img << "P6\n" << width << " " << height << "\n255\n";

    // Pixel data follows the header: rows top to bottom, pixels left to right, 3 bytes each.
    // Each pixel maps back to a data cell by integer scaling (nearest-neighbor sampling),
    // then that value is normalized to [0, 1] and colored.
    for (int py = 0; py < height; ++py) {
        int fr = py * nFr / height;              // which snapshot this pixel row shows
        for (int px = 0; px < width; ++px) {
            int i = px * nPts / width;           // which grid point this pixel column shows
            RGB c = colormap((frames[fr][i] - umin) / span);
            img.put(c.r);                        // write the pixel as raw R, G, B bytes
            img.put(c.g);
            img.put(c.b);
        }
    }

    // Report where the image went and the value range the colors span
    // (black = umin, pale yellow = umax). The file closes when img goes out of scope.
    cout << "wrote " << path << "  (" << width << " x " << height << ", u from " << umin << " to " << umax << ")" << endl;
}
