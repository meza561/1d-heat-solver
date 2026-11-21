1D heat equation solver
solves u_t = k u_xx on [xmin, xmax] with forward euler
pick dirichlet, neumann, or periodic BCs for the ends

compile (from this folder):
    g++ main.cpp input.cpp output.cpp -o 1dheat

run:
    ./1dheat

it asks for each setting, just press enter to take the default in [brackets]

output:
    heat.csv  - time in the first column then u at every grid point, one row per snapshot
    heat.ppm  - space-time heat map picture (x across, t down)
