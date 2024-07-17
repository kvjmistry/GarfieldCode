#include <cstdlib>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <TApplication.h>
#include <TCanvas.h>
#include <Garfield/ComponentComsol.hh>
#include <Garfield/ViewField.hh>
#include <Garfield/ViewFEMesh.hh>
#include <Garfield/ViewSignal.hh>
#include <Garfield/MediumMagboltz.hh>
#include <Garfield/Sensor.hh>
#include <Garfield/AvalancheMicroscopic.hh>
#include <Garfield/AvalancheMC.hh>
#include <Garfield/Random.hh>

using namespace Garfield;

double GetRandomInRange(double min, double max) {
    return min + (max - min) * RndmUniform();
}

void RunSimulation(const std::string& fieldMapBase, std::ofstream& outfile, double i_diam, double o_diam, double cell_height, double voltage, unsigned int r) {
    // Setup the gas.
    MediumMagboltz gas("xe", 98., "co2", 2.0);
    gas.SetTemperature(293.15);
    gas.SetPressure(760.);
    gas.LoadIonMobility("/home/argon/Projects/Krishan/garfieldpp/Data/IonMobility_Xe+_Xe.txt");
    gas.SetMaxElectronEnergy(200.);
    gas.Initialise();

    ComponentComsol fm;
    std::string fieldMap = fieldMapBase;
    std::string fieldFile = fieldMapBase;
    fieldFile.replace(fieldFile.find(".mphtxt"), 7, "Field_" + std::to_string(static_cast<int>(voltage)) + "V.txt");
    fm.Initialise(fieldMap, "../HexMat.txt", fieldFile, "m");
    fm.SetGas(&gas);

    Sensor sensor;
    sensor.AddComponent(&fm);
    sensor.SetArea(-2.*cell_height, -2.*cell_height, 0, 2.*cell_height, 2.*cell_height, 2*cell_height);

    AvalancheMicroscopic aval;
    aval.SetSensor(&sensor);
    AvalancheMC drift;
    drift.SetSensor(&sensor);
    drift.SetDistanceSteps(2.e-4);

    std::array<double, 3> ef{0, 0, 0}; 
    Garfield::Medium* medium = nullptr;
    int status = 0;
    sensor.ElectricField(0, 0, 1.5 * i_diam, ef[0], ef[1], ef[2], medium, status);
    constexpr unsigned int nEvents = 100;
    for (unsigned int i = 0; i < nEvents; ++i) {
        if (i % 10 == 0){
        std::cout << "Event =  " << i  << "/" << nEvents << "\n";
        }
        const double x0 = GetRandomInRange(0.1*i_diam, 0.5*i_diam);
        const double y0 = GetRandomInRange(0.1*i_diam, 0.5*i_diam); 
        const double z0 = i_diam;
        const double t0 = 0.; 
        const double e0 = 0.1;
        aval.AvalancheElectron(x0, y0, z0, t0, e0, 0., 0., 0.);
        int ne = 0, ni = 0;
        aval.GetAvalancheSize(ne, ni);
        const unsigned int np = aval.GetNumberOfElectronEndpoints();
        if (np < 2){
          if (i > 0){
          --i;
          }
        } else {
          outfile << voltage << "," << r << "," << i << "," << np << "\n";
        }
    }
    outfile.flush();
}

int main(int argc, char *argv[]) {

    // std::array<double, 8> voltages = {2500, 3000, 3500, 4000, 5000, 6000, 7000, 8000};
    // std::array<std::string, 10> fieldMaps = {
    //     "02mmHex/2mmHex.mphtxt", "04mmHex/4mmHex.mphtxt", "06mmHex/6mmHex.mphtxt",
    //     "08mmHex/8mmHex.mphtxt", "10mmHex/10mmHex.mphtxt", "12mmHex/12mmHex.mphtxt",
    //     "14mmHex/14mmHex.mphtxt", "16mmHex/16mmHex.mphtxt", "18mmHex/18mmHex.mphtxt",
    //     "20mmHex/20mmHex.mphtxt"
    // };

    // for (unsigned int r = 1; r <= 10; ++r) {
    //     std::cout << "Current Radius = " << r  <<  std::endl;
    //     for (const auto& voltage : voltages) {
    //         std::cout << "Current Voltage = " << voltage <<  std::endl;
    //         double i_diam = 0.1 * r;
    //         double o_diam = 0.125 * r;
    //         double cell_height = 0.25 * r;
    //         RunSimulation(fieldMaps[r-1], outfile, i_diam, o_diam, cell_height, voltage, r);
    //     }
    // }

    std::string outputFile = "combined_electron_endpoints_gain.txt";
    std::ofstream outfile(outputFile, std::ios::out);
    outfile << "field,radius,evnt,gain\n";

    int r = std::stoi(argv[1]);
    double voltage = std::stoi(argv[2]);
    const char* fieldmap_char = argv[3];
    std::string fieldmap = fieldmap_char;
    std::cout << "Radius = " << r  << ",  Volatage: " << voltage <<  std::endl;
    std::cout << "fieldmap: " << fieldmap  << std::endl;

    double i_diam = 0.1 * r;
    double o_diam = 0.125 * r;
    double cell_height = 0.25 * r;

    RunSimulation(fieldmap, outfile, i_diam, o_diam, cell_height, voltage, r);


    outfile.close();
    return 0;
}

