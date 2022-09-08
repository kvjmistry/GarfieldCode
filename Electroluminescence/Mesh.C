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

/*
Run info:
Compile by making a build directory
$ cd build
$ cmake ..
make;

To run:
# evt id, num e-, seed, grid, jobid, mode [align, rot, shift]
./build/Mesh 0 1 1 0 0 0
*/


using namespace Garfield;

int event = 0;

std::vector<std::vector<float>> evtInfo;
std::vector<std::string> metadata;

// enums for config variables
enum eVars {
    k_evt,
    k_x,
    k_y,
    k_z,
    k_t,
    k_config_MAX
    };

// Round to three decimal places to save on space in the file
float roundDP(float var)
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
    float value = (int)(var * 1000 + .5);
    return (float)value / 1000;
}

void userHandle(double x, double y, double z, double t,
                int type, int level, Garfield::Medium* /*m*/) {

    // Skip inelastic collisions that are not excitations.
    if (type != 4) return;

    std::vector<float> temp;

    // Save information in a vector
    temp.push_back(event);
    temp.push_back(roundDP(x));
    temp.push_back(roundDP(y));
    temp.push_back(roundDP(z));
    temp.push_back(roundDP(t));

    evtInfo.push_back(temp);
}




int main(int argc, char * argv[]) {
    
    std::cout << "The event number is: " << argv[1] << std::endl;
    std::cout << "Simulating a total of " << argv[2] << " electrons" << std::endl;
    std::cout << "The seed number is: " << argv[3] << std::endl;
    std::cout << "Using the grid? " << argv[4] << std::endl;
    std::cout << "JobID " << argv[5] << std::endl;
    std::cout << "Mode of simulation?: " << argv[6] << std::endl;
    std::cout << "\n" << std::endl;

    // Set the event number
    event = std::stoi(argv[1]);

    TApplication app("app", &argc, argv);

    // Simulation parameters

    // Number of primary electrons (avalanches) to simulate
    const unsigned int npe = std::stoi(argv[2]);
    
    // Choose whether to plot the field maps
    bool plotmaps = true;

    // Mesh Boundary Zone
    double MeshBoundary = 0.65; // cm
    double MeshSampleR = 0.45; // cm -- Circle radius to sample within

    // Gas Physics
    double temperature = 293.15; // Kelvin
    double torr = 750.062;
    double pressure = 13.5*torr; // Give pressure in bar and convert it to torr

    // Start Z
    double z0 = 0.65; //cm

    // SEED
    int seed = std::stoi(argv[3]);

    // Choose to run on the grid or not
    int usegrid = std::stoi(argv[4]);

    // Job id
    char *jobid = argv[5];

    // File Home
    std::string home;
    bool terminate;
    
    if (usegrid == 0){
        home= "Files/";
        terminate = false;
    }
    else {
        home = "/n/home05/kvjmistry/packages/GarfieldCode/Electroluminescence/Files/";
        terminate = true;
        plotmaps = false;

    }

    std::string gridfile;
    std::string datafile;
    std::string filehome;

    if (strcmp(argv[6], "align") == 0){
        gridfile = "Aligned/Aligned_Mesh.mphtxt";
        datafile = "Aligned/Aligned_Mesh_Data";
    }
    else if (strcmp(argv[6], "rot") == 0) {
        gridfile = "Rotated/Rotated_Mesh.mphtxt";
        datafile = "Rotated/Rotated_Mesh_Data";
    }
    else if (strcmp(argv[6], "shift") == 0){
        std::cout << "Shifted" << std::endl;
        gridfile = "Shifted/Shifted_Mesh.mphtxt";
        datafile = "Shifted/Shifted_Mesh_Data";
    }
    else {
        std::cout << "Could not read in the mode!" << std::endl;
    }
    

    // ----- 


    // Setup the gas.
    MediumMagboltz gas("xe");
    gas.SetTemperature(temperature);
    gas.SetPressure(pressure);
    gas.Initialise(true);  
    
    // Setup the electric potential map
    ComponentComsol* fm = new ComponentComsol(); // Field Map
    fm->Initialise(home + gridfile ,home + "Mesh_MaterialProperties.txt", home + datafile, "mm");
    
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
        ViewField fieldViewXY;
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
        fieldViewXY.SetArea(-0.7, -0.8, 0.7, 0.7); 
        fieldViewXY.SetCanvas(cf2);
        fieldViewXY.SetComponent(fm);
        fieldViewXY.SetNumberOfContours(100);
        fieldViewXY.SetPlane(0, -1, 0, 0, 0, 0);
        fieldViewXY.Plot("emag", "colz");

        TCanvas* cf3 = new TCanvas("cf3", "", 600, 600);
        cf3->SetLeftMargin(0.16);
        cf3->SetRightMargin(0.25);
        fieldViewXZ.SetArea(-0.7, -0.8, 0.7, 0.7); 
        fieldViewXZ.SetCanvas(cf3);
        fieldViewXZ.SetComponent(fm);
        fieldViewXZ.SetNumberOfContours(100);
        fieldViewXZ.SetPlane(0, 0, -1, 0, 0, -0.51);
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
    rng.SetSeed(seed);
    
    // Calculate a few avalanches.
    for (unsigned int i = 0; i < npe; ++i) {
        std::cout << "--------------------------------\n" << std::endl;

        event++;
        
        // Release the primary electron near the top mesh.
        bool sample_pos = true;

        double x0 = rng.Uniform(-1*MeshBoundary, MeshBoundary);
        double y0 = rng.Uniform(-1*MeshBoundary, MeshBoundary);

        while(sample_pos){
            if (std::sqrt(x0*x0 + y0*y0) < MeshSampleR){
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
        
        std::cout << "  Primary electron starts at (x, y, z) = ("
                    << x0 << ", " << y0 << ", " << z0 
                    << ") with an energy of " << e0 << " eV.\n";
        
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
        
        double x1, y1, z1, t1, e1;
        double x2, y2, z2, t2, e2;
        // Loop over the electrons produced [should be only one!]
        for (int ie = 0; ie < np; ie++) {
            
            int status;
            aval.GetElectronEndpoint(ie, x1, y1, z1, t1, e1, x2, y2, z2, t2, e2, status);

            std::cout << "  Primary electron ends at (x, y, z) = ("
                    << x2 << ", " << y2 << ", " << z2
                    << ") with an energy of " << e2 << " eV.\n";
            
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
        
        std::cout << "  Number of electrons: " << ne << " (" << nTopPlane 
                << " of them ended on the top electrode and " << nBottomPlane 
                << " on the bottom electrode)\n"
                << "  Number of ions: " << ni << "\n"
                << "  Number of excitations: " << nExc << "\n";
    
        metadata.push_back(std::to_string(event)     + "," + 
                           std::to_string(ne)        + "," +
                           std::to_string(ni)        + "," +
                           std::to_string(nEl)       + "," + 
                           std::to_string(nIon)      + "," + 
                           std::to_string(nAtt)      + "," + 
                           std::to_string(nInel)     + "," + 
                           std::to_string(nExc)      + "," + 
                           std::to_string(nTopPlane) + "," +  
                           std::to_string(nBottomPlane) + "," + 
                           std::to_string(x0) + "," + 
                           std::to_string(y0) + "," + 
                           std::to_string(z0) + "," + 
                           std::to_string(e1) + "," + 
                           std::to_string(e2));
    
    }

    // Print the num of VUV photons
    std::cout << "Printing number of VUV photons per event" << std::endl;
    for (const auto& n : nVUV){
        std::cout << n << std::endl;
    }

    if (plotmaps){
        TCanvas* cf4 = new TCanvas("cf4", "", 600, 600);
        ViewFEMesh* meshView = new ViewFEMesh();
        meshView->SetCanvas(cf4);
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

    std::ofstream metafile;
    
    // Initialize the csv file
    metafile.open(Form("Metadata_%s.csv", jobid));

    // metafile << "event,electrons,ions,elastic,ionisations,attachment,inelastic,excitation,top,bottom,start x,start y,start z, start E, end E"<< "\n";
    
    for (unsigned int i = 0; i < metadata.size(); i++){
        std::cout << metadata.at(i)<< "\n";
        metafile << metadata.at(i) << "\n";
    }

    metafile.close();

    // --  

    std::ofstream myfile;
    
    // Initialize the csv file
    myfile.open(Form("EventInfo_%s.csv", jobid));
    
    // myfile << "event,x,y,z,t" << "\n";
    for (unsigned int i = 0; i < evtInfo.size(); i++){
        // std::cout << evtInfo.at(i).at(k_evt)<< "," << evtInfo.at(i).at(k_x) << "," << evtInfo.at(i).at(k_y) << "," << evtInfo.at(i).at(k_z) << ","<< evtInfo.at(i).at(k_t) << "\n";
        myfile << evtInfo.at(i).at(k_evt)<< "," << evtInfo.at(i).at(k_x) << "," << evtInfo.at(i).at(k_y) << "," << evtInfo.at(i).at(k_z) << ","<< evtInfo.at(i).at(k_t) << "\n";
    }

    myfile.close();

    // Choose whether to open the app or not
    if (!terminate){
        app.Run(true);
    }

}



