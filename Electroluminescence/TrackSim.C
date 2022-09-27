// This script generates electron tracks and gets their energy depositions 
// as it propagates
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TF2.h>
#include <TROOT.h>
#include <TGraph.h>
#include <TAxis.h>


#include "Garfield/ComponentComsol.hh"
#include "Garfield/ComponentConstant.hh"
#include "Garfield/MediumGas.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/Plotting.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/Random.hh"
#include "Garfield/SolidBox.hh"
#include "Garfield/GeometrySimple.hh"

/*
Run info:
Compile by making a build directory
$ cd build
$ cmake ..
make;

To run:
# evt id, num e-, seed, grid, jobid, rotated
./build/TrackSim.C
*/


using namespace Garfield;

int main(int argc, char * argv[]) {

    // Gas Physics
    double temperature = 293.15; // Kelvin
    double torr = 750.062;
    double pressure = 13.5*torr; // Give pressure in bar and convert it to torr


    randomEngine.Seed(123456);
    TApplication app("app", &argc, argv);
    SetDefaultStyle();

    // Histograms
    TH1::StatOverflows(true); 
    TH1F hElectrons("hElectrons", "Number of electrons", 200, 1e5, 3e5);
    TH1F hEdep("hEdep", "Energy Loss", 100, 0., 5.);
    TH1F hClusterSize("hClusterSize", "Cluster size", 100, 0.5, 100.5);

    // Make a medium
    MediumMagboltz gas("xe");
    gas.SetTemperature(temperature);
    gas.SetPressure(pressure);
    gas.Initialise(true);  

    // Thickness of the gas gap [cm]
    constexpr double width = 1000.;

    // Gas gap [cm].
    SolidBox box(0.5 * width, 0, 0, 0.5 * width, width, width);
    GeometrySimple geo;
    geo.AddSolid(&box, &gas);

    // Make a component
    ComponentConstant cmp;
    cmp.SetGeometry(&geo);
    cmp.SetMedium(&gas);
    cmp.SetElectricField(0., 0., 0.);

    // Make a sensor
    Sensor sensor;
    sensor.AddComponent(&cmp);

    // Track class
    TrackHeed track;
    track.SetSensor(&sensor);
    track.SetParticle("e-");
    track.SetEnergy(2.5e6); // Set the energy of the particle
    constexpr bool verbose = true;
    // track.EnableCoulombScattering(true);
    // track.EnableDeltaElectronTransport();
    track.Initialise(&gas, verbose); 

    std::cout << "Starting Energy: " << track.GetEnergy()<< std::endl;
    
    // Total events to simulate
    const int nEvents = 100;
    
    for (int i = 0; i < nEvents; ++i) {
        if (i % 100 == 0) std::cout << i << "/" << nEvents << "\n";
        
        // Initial position and direction 
        double x0 = 0., y0 = 0., z0 = 0., t0 = 0.;
        double dx0 = 1., dy0 = 0., dz0 = 0.; 
        track.NewTrack(x0, y0, z0, t0, dx0, dy0, dz0);
        
        // Cluster coordinates
        double xc = 0., yc = 0., zc = 0., tc = 0.;
        // Number of electrons produced in a collision
        int nc = 0;
        // Energy loss in a collision
        double ec = 0.;
        // Dummy variable (not used at present)
        double extra = 0.;
        // Total energy loss along the track
        double esum = 0.;
        // Total number of electrons produced along the track
        int nsum = 0;
        
        // Loop over the clusters.
        while (track.GetCluster(xc, yc, zc, tc, nc, ec, extra)) {

            // std::cout << ec << ", " << nc << ", " << xc << ", " << yc << ", " << zc  << ", " << tc<<  std::endl;

            esum += ec;
            nsum += nc;
            hClusterSize.Fill(nc);
        }
        
        hElectrons.Fill(nsum);
        hEdep.Fill(esum/1e6);
        
        if (i % 100 == 0){
            std::cout << esum << std::endl;
            std::cout << nsum << std::endl;
        }
    }

    TCanvas c1;
    hElectrons.GetXaxis()->SetTitle("number of electrons"); 
    hElectrons.Draw();
    c1.SaveAs("ne.pdf");

    TCanvas c2;
    hEdep.GetXaxis()->SetTitle("energy loss [MeV]");
    hEdep.Draw();
    c2.SaveAs("edep.pdf");

    TCanvas c3;
    hClusterSize.GetXaxis()->SetTitle("electrons / cluster");
    hClusterSize.Draw();
    c3.SetLogy();
    c3.SaveAs("clusterSizeDistribution.pdf");

    app.Run(true); 

}