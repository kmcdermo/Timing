/******************************************************************************** 
*   generateb.cc by Flip Tanedo (flip.tanedo@uci.edu)                           *
*   Generates a pair of b quarks and lets Pythia shower.                        * 
*   31 July 2014                                                                *
*	based on generateg.cc														*
********************************************************************************/

#include <time.h>                           // for random number seed
//#include <fastjet/ClusterSequence.hh>       // fastjet clustering
#include <sstream>                  // for string stream
#include <fstream>                  // for file in/out

#include "Pythia8/Pythia.h" 
using namespace Pythia8; 
using namespace std;


// int main(int argc, char *argv[]) { 
int main() { 
	

// 	Output filename
	string outfile = "output_HVphoton.dat";
	ofstream outstream;
	outstream.open(outfile.c_str(), ios::out); // append to end of file, overwirte the file each time ::out
	outstream.precision(6);
	outstream.setf(ios::fixed);
	outstream.setf(ios::showpoint);

// 	Generator
	Pythia pythia;
	Event& event = pythia.event; // Shorthand: pythia event record
    ParticleData& pdt = pythia.particleData; // define pdt

	pythia.readFile("HVphoton.cmnd");
	int nEvent = pythia.mode("Main:numberOfEvents");
	int nAbort = pythia.mode("Main:timesAllowErrors");

	pythia.init();
    
//List of decay channels for v-hadrons
    pdt.readString("4900111:addChannel 1 1. 0 22 22");//make v-pion decay into photons with Br = 1
    
//v-pion lifetime 
    
    pythia.readString("4900111:tau0 = 200"); //lifetime in mm



//	Begin event loop.
	int iAbort = 0;
	for (int iEvent = 0; iEvent < nEvent; ++iEvent) {
		if (!pythia.next()) {
			if (++iAbort < nAbort) continue;
			cout << " Event generation aborted prematurely, owing to error!\n";
			break;
		}

		for (int iPart = 0; iPart < event.size(); iPart++){
        	if (event[iPart].status()==-23 ||
                event[iPart].idAbs()==4900111 || (event[iPart].status()== 91 && event[iPart].idAbs()==22 && (event[iPart].xProd() > 0.01 || event[iPart].yProd() > 0.01) ) ) 
                {
                outstream <<  event[iPart].id() << " " << event[iPart].px() << " " << event[iPart].py() << " " << event[iPart].pz() << " " << event[iPart].e() << " " << event[iPart].m0() << " " << event[iPart].xProd() << " " << event[iPart].yProd() << " " << event[iPart].zProd() << "\n";
			}
			}
	outstream << "#\n";} // End of event loop 

	outstream.close();
	return 0;     
}
   
