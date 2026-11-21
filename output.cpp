//everything that writes results out to files
//writeRow puts one snapshot into the csv
//colormap turns a value into a color
//writePPM makes the heat map picture out of all the snapshots

#include "heat.h"
#include <iostream>

using namespace std;

//snapshot as well as helper function for main
//writes one csv line with the time first then every u value after it all split by commas
//gets called every saved step so the file ends up one row per snapshot and one column per grid point
void writeRow(ofstream& file, double t, const vector <double>& u) {
    //first column is the time
    file << t;
    //rest of the columns are u at each grid point
    for (double v : u) file << "," << v;
    //ends the row
    file << "\n";
}

//one pixel's color, 1 byte each for red green blue (0-255)
//this is exactly how a binary ppm wants each pixel laid out
struct RGB { unsigned char r, g, b; };

//turns s in [0,1] into a color going black -> purple -> red -> orange -> pale yellow
//5 stops roughly copying the "inferno" palette and anything between stops gets blended
//static just keeps this helper private to this file
static RGB colormap(double s) {
    //clamps s so rounding error or bad data cant go outside the table
    if (s < 0.0) s = 0.0;
    if (s > 1.0) s = 1.0;

    //color table, each row is an {R, G, B} color evenly spaced over [0,1]
    const double stops[5][3] = {
        {  0,   0,   4},    //s = 0.00  black
        { 87,  16, 110},    //s = 0.25  purple
        {188,  55,  84},    //s = 0.50  red
        {249, 142,   9},    //s = 0.75  orange
        {252, 255, 164}     //s = 1.00  pale yellow
    };

    //figures out which two stops s is between
    double pos = s * 4.0;
    int i = static_cast<int>(pos);
    //s = 1 would give i = 4 which is past the last pair so knock it back to 3
    if (i > 3) i = 3;
    //how far s is between stop i and stop i+1
    double f = pos - i;

    //blends each channel starting at stop i and moving f of the way to stop i+1
    //the cast chops it down to a 0-255 byte
    RGB c;
    c.r =static_cast<unsigned char>(stops[i][0] + f * (stops[i + 1][0] - stops[i][0]));
    c.g = static_cast<unsigned char>(stops[i][1] + f * (stops[i + 1][1] - stops[i][1]));
    c.b = static_cast<unsigned char>(stops[i][2] + f * (stops[i + 1][2] - stops[i][2]));
    return c;
}

//space-time heat map with x going across and t going down (t = 0 at the top)
//frames[k] is the whole u at the k-th saved snapshot
//each pixel just grabs the closest snapshot and grid point so the picture can be any size
//output is a binary ppm which is a super simple image format most viewers can open
//returns false if the file wont open so main can tell something went wrong
bool writePPM(const string& path, const vector<vector<double>>& frames, int width, int height) {
    //color range over the whole run
    //finds the min and max of u across every snapshot so a color means the same value everywhere
    double umin = frames[0][0], umax = frames[0][0];
    for (const auto& f : frames)
        for (double v : f) {
            if (v < umin) umin = v;
            if (v > umax) umax = v;
        }
    //size of the value range, uses 1 if u is constant so we dont divide by zero
    double span = (umax > umin) ? (umax - umin) : 1.0;

    //number of snapshots (time going down)
    int nFr = frames.size();
    //number of grid points (space going across)
    int nPts = frames[0].size();

    //opens in binary so the pixel bytes get written exactly with no newline messing
    ofstream img(path, ios::binary);
    //same check as the csv in main, bails if the file cant be opened
    if (!img) {
        cerr << "\nError: Couldn't open file " << path << endl;
        return false;
    }
    //ppm header: "P6", the image size, then 255 as the max value for each color
    img << "P6\n" << width << " " << height << "\n255\n";

    //pixel data goes rows top to bottom and pixels left to right, 3 bytes each
    //each pixel maps back to a data point then gets scaled to [0,1] and colored
    for (int py = 0; py < height; ++py) {
        //which snapshot this row of pixels shows
        int fr = py * nFr / height;
        for (int px = 0; px < width; ++px) {
            //which grid point this column of pixels shows
            int i = px * nPts / width;
            RGB c = colormap((frames[fr][i] - umin) / span);
            //writes the pixel as raw R, G, B bytes
            img.put(c.r);
            img.put(c.g);
            img.put(c.b);
        }
    }

    //says where the image went and the range the colors cover (black = umin, pale yellow = umax)
    //file closes by itself when img goes out of scope
    cout << "wrote " << path << "  (" << width << " x " << height << ", u from " << umin << " to " << umax << ")" << endl;
    return true;
}
