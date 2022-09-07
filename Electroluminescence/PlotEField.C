// This script illustrates the simulation of VUV electroluminescence 
// and its properties in pure Xe between two hexagonal meshes).
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TF2.h>

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
# evt id, num e-, seed, grid, jobid, rotated
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

void userHandle(double x, double y, double z, double t,
                int type, int level, Garfield::Medium* /*m*/) {

    // Skip inelastic collisions that are not excitations.
    if (type != 4) return;

    std::vector<float> temp;

    // Save information in a vector
    temp.push_back(event);
    temp.push_back(x);
    temp.push_back(y);
    temp.push_back(z);
    temp.push_back(t);

    evtInfo.push_back(temp);
}




int main(int argc, char * argv[]) {
    
    TApplication app("app", &argc, argv);

    // Simulation parameters

    // Gas Physics
    double temperature = 293.15; // Kelvin
    double torr = 750.062;
    double pressure = 13.5*torr; // Give pressure in bar and convert it to torr

    // File Home
    std::string home = "Files/";

    // ----- 


    // Setup the gas.
    MediumMagboltz gas("xe");
    gas.SetTemperature(temperature);
    gas.SetPressure(pressure);
    gas.Initialise(true);  
    
    // Setup the electric potential map
    ComponentComsol* fm = new ComponentComsol(); // Field Map
    fm->Initialise(home + "Aligned_Mesh.mphtxt",home + "Mesh_MaterialProperties.txt", home + "Aligned_Mesh_Data", "mm");
    
    // Print some information about the cell dimensions.
    fm->PrintRange();

    // Associate the gas with the corresponding field map material.
    fm->SetGas(&gas); 
    fm->PrintMaterials();
    fm->Check();

    // Plot the fieldmaps
    ViewField fieldViewXY;

    for (double z = 0.8; z>=-0.8;z-=0.01){
        // std::cout << z<< std::endl;
        TCanvas* c = new TCanvas("c", "", 600, 600);
        c->SetLeftMargin(0.16);
        c->SetRightMargin(0.25);
        fieldViewXY.SetArea(-0.7, -0.8, 0.7, 0.7); 
        fieldViewXY.SetCanvas(c);
        fieldViewXY.SetComponent(fm);
        fieldViewXY.SetNumberOfContours(100);
        fieldViewXY.SetPlane(0, 0, -1, 0, 0, z);
        fieldViewXY.SetElectricFieldRange(-10, 25000);
        fieldViewXY.Plot("emag", "colz");
        c->SetTitle(Form("z = %0.2f cm", z));
        c->Print(Form("Plots/E_xy/E_xy_%0.2f.png",z));
        delete c;
    }


    // Choose whether to open the app or not
    // app.Run(true);

}
