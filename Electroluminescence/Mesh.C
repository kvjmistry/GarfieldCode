// This script illustrates the simulation of VUV electroluminescence 
// and its properties in pure Xe between two hexagonal meshes).
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>

#include "Garfield/ComponentComsol.hh"
#include "Garfield/MediumGas.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/Random.hh"

using namespace Garfield;

std::ofstream myfile;
int event = 0;

void userHandle(double x, double y, double z, double t,
                int type, int level, Garfield::Medium* /*m*/) {

    // Skip inelastic collisions that are not excitations.
    if (type != 4) return;

    // Write information to a histogram
    myfile << event << "," << x << "," << y << "," << z << ","<< t << "\n";
    // std::cout << event << "," << x << "," << y << "," << z << ","<< t << "\n";
}




int main(int argc, char * argv[]) {
    
    std::cout << "The event number is: " << argv[1] << std::endl;
    event = std::stoi(argv[1]);

    TApplication app("app", &argc, argv);

    // Initialize the csv file
    myfile.open("Data.csv");

    // Simulation parameters

    // Number of primary electrons (avalanches) to simulate
    constexpr unsigned int npe = 2;
    
    // Choose whether to plot the field maps
    bool plotmaps = true;

    // Width of the parallel gap [cm]
    constexpr double yGap = 1.0;

    // Verbose Print
    constexpr bool print = true;

    // Mesh Boundary Zone
    double MeshBoundary = 0.65; // cm

    // Start Z
    double z0 = 0.65; //cm

    // ----- 


    // Setup the gas.
    MediumMagboltz gas("xe");
    gas.SetTemperature(293.15);
    gas.SetPressure(7500.62);
    gas.Initialise(true);  
    
    // Setup the electric potential map
    ComponentComsol* fm = new ComponentComsol(); // Field Map
    fm->Initialise("Files/Aligned_Mesh.mphtxt","Files/AlignedMesh_MaterialProperties.txt", "Files/Aligned_Mesh_Data", "mm");
    
    // Print some information about the cell dimensions.
    fm->PrintRange();

    // Associate the gas with the corresponding field map material.
    fm->SetGas(&gas); 
    fm->PrintMaterials();
    fm->Check();

    // Plot the fieldmaps
    if (plotmaps){
        ViewField fieldView;
        ViewField fieldViewXZ;
        fieldView.SetComponent(fm);
        // Plot the potential along the central axis.
        fieldView.SetPlane(0, -1, 0, 0, 0, 0);
        fieldView.SetNumberOfContours(100);

        // Plot the contour of the potential in the x-y plane
        TCanvas* cf = new TCanvas("cf", "", 600, 600);
        cf->SetLeftMargin(0.16);
        fieldView.SetCanvas(cf);
        fieldView.PlotContour();

        TCanvas* cf2 = new TCanvas("cf2", "", 600, 600);
        cf2->SetLeftMargin(0.16);
        cf2->SetRightMargin(0.25);
        fieldViewXZ.SetArea(-0.7, -0.8, 0.7, 0.7); 
        fieldViewXZ.SetCanvas(cf2);
        fieldViewXZ.SetComponent(fm);
        fieldViewXZ.SetNumberOfContours(100);
        fieldViewXZ.SetPlane(0, -1, 0, 0, 0, 0);
        fieldViewXZ.Plot("emag", "colz");
    }


    // Crate the sensor
    Sensor sensor;
    sensor.AddComponent(fm);
    sensor.SetArea();

    // Make a microscopic tracking class for electron transport.
    AvalancheMicroscopic aval;
    aval.SetSensor(&sensor);
   
    // Make a histogram of the electron energy distribution.
    TH1D hEn("hEn","energy distribution", 1000, 0., 100.);
    aval.EnableElectronEnergyHistogramming(&hEn);

    // Initialise object to plot the drift paths
    ViewDrift driftView;
    aval.EnablePlotting(&driftView);

    // Enable handle to retrieve all the inelastic (VUV gamma production) collisions
    aval.SetUserHandleInelastic(userHandle);

    
    std::vector<unsigned int> nVUV;

    TRandom3 rng; // Random number generators for x and y positions
    int seed = 1;
    rng.SetSeed(seed);
    
    // Calculate a few avalanches.
    for (unsigned int i = 0; i < npe; ++i) {

        event++;
        
        // Release the primary electron near the top mesh.
        bool sample_pos = true;

        double x0 = rng.Uniform(-1*MeshBoundary, MeshBoundary);
        double y0 = rng.Uniform(-1*MeshBoundary, MeshBoundary);

        while(sample_pos){
            if (std::sqrt(x0*x0 + y0*y0) < 0.7){
                std::cout << "Sampled position is valid!" << std::endl;
                sample_pos = false;
            }
            else{
                x0 = rng.Uniform(-1*MeshBoundary, MeshBoundary);
                y0 = rng.Uniform(-1*MeshBoundary, MeshBoundary);
            }
        }
        
        const double t0 = 0.;
        double r = std::sqrt(x0*x0 + y0*y0);

        // Draw the initial energy [eV] from the energy distribution.
        const double e0 = i == 0 ? 1. : hEn.GetRandom();
        std::cout << "Avalanche "<< i + 1 << " of " << npe << ".\n";
        
        if (print) {
            std::cout << "  Primary electron starts at (x, y, z) = ("
                        << x0 << ", " << y0 << ", " << z0 
                        << ") with an energy of " << e0 << " eV.\n";
        }
        
        // Simulate the avalanche
        aval.AvalancheElectron(x0, y0, z0, t0, e0, 0, 0, 0);
        
        // Get the number of electrons and ions.
        int ne = 0, ni = 0;
        aval.GetAvalancheSize(ne, ni);
        
        // Get information about all the electrons produced in the avalanche.
        unsigned int nBottomPlane = 0;
        unsigned int nTopPlane = 0;
        const int np = aval.GetNumberOfElectronEndpoints();

        std::cout << "Number of electrons produced in avalanche: " << np << std::endl;
        
        // Loop over the electrons produced [should be only one!]
        for (int ie = 0; ie < np; ie++) {
            
            double x1, y1, z1, t1, e1;
            double x2, y2, z2, t2, e2;
            int status;
            aval.GetElectronEndpoint(ie, x1, y1, z1, t1, e1, x2, y2, z2, t2, e2, status);

            if (print) {
            std::cout << "  Primary electron ends at (x, y, z) = ("
                        << x2 << ", " << y2 << ", " << z2
                        << ") with an energy of " << e2 << " eV.\n";
        }
            
            if (status == -5) {
                
                // The electron left the drift medium.
                
                // Landed on the bottom electrode
                if (z2 < -0.4) {
                    ++nBottomPlane;
                }
                else if (z2 > 0.4) {
                    ++nTopPlane;
                }
            }
            else {
                std::cout << "\nElectron " << ie << " of avalanche " << i   
                        << " ended with a strange status (" << status << "):\n"
                        << "(x1, y1, z1) = (" << x1 << ", " << y1 << ", " << z1 
                        << "), t1 = " << t1 << ", e1 = " << e1 << "\n"
                        << "(x2, y2, z2) = (" << x2 << ", " << y2 << ", " << z2 
                        << "), t2 = " << t2 << ", e2 = " << e2 << "\n";
            }
        
        
        }
        
        unsigned int nEl = 0;
        unsigned int nIon = 0;
        unsigned int nAtt = 0;
        unsigned int nInel = 0;
        unsigned int nExc = 0;
        unsigned int nSup = 0;
        gas.GetNumberOfElectronCollisions(nEl, nIon, nAtt, nInel, nExc, nSup);
        gas.ResetCollisionCounters();
        nVUV.push_back(nExc + ni);
        
        if (!print) continue;
        
        std::cout << "  Number of electrons: " << ne << " (" << nTopPlane 
                << " of them ended on the top electrode and " << nBottomPlane 
                << " on the bottom electrode)\n"
                << "  Number of ions: " << ni << "\n"
                << "  Number of excitations: " << nExc << "\n";
    }

    // Print the num of VUV photons
    std::cout << "Printing number of VUV photons per event" << std::endl;
    for (const auto& n : nVUV){
        std::cout << n << std::endl;
    }

    if (plotmaps){
        TCanvas* cf3 = new TCanvas("cf3", "", 600, 600);
        ViewFEMesh* meshView = new ViewFEMesh();
        meshView->SetCanvas(cf3);
        meshView->SetComponent(fm);
        // x-z projection
        meshView->SetPlane(0, -1, 0, 0, 0, 0);
        meshView->SetFillMeshWithBorders(); 
        // Set the color of the kapton.
        meshView->SetColor(2, kYellow + 3);
        meshView->EnableAxes();
        meshView->SetViewDrift(&driftView);
        meshView->Plot();
    }

    myfile.close();

    app.Run(true);

}



