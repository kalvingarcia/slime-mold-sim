/*
	Author: Kalvin Garcia
	Inspiration: Sebastian Lague
	Resources: Adomas Alimas, Sage Jenson
	
	Description:
		This is the driver code for the slime mold simulation. Most of the program loop occurs in the Simulation class.
		Eventually, I may try to add different colored slimes in 1 sim, or even running multiple slimes in a "petri dish" concurrently.
*/
#include "simulation.h"

int main() {
	Simulation sim; // creating the sim object
	sim.run(); // running the simulation
    return 0;
}