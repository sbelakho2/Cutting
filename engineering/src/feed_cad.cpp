#include "feed_geometry.hpp"
#include "shared/feed_drive_spec.hpp"
#include <BRep_Builder.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Compound.hxx>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc,char** argv) try {
    namespace se=starz::engineering;namespace fs=std::filesystem;
    fs::path out;
    bool assemblyOnly=false,cassette=false;
    double wireDiameter=starz::feed::design::wire_diameter_max;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--out"&&i+1<argc)out=argv[++i];
        else if(arg=="--cassette")cassette=true;
        else if(arg=="--wire-od"&&i+1<argc)wireDiameter=std::stod(argv[++i]);
        else if(arg=="--check-assembly-paths")assemblyOnly=true;
        else if(arg=="--check-machining-features") {
            se::checkFeedMachiningFeatures();
            std::cout<<"Shaft ends,slot blends,roots,relief,thread/drill depth and groove corner fit checked\n";return 0;
        }
        else if(arg=="--reject-unfinished-edges") {
            se::rejectUnfinishedFeedMachining();
            std::cout<<"Unfinished R5 shaft edges correctly rejected\n";return 0;
        }
        else if(arg=="--reject-old-housing"||arg=="--reject-old-washer") {
            se::rejectOldFeedAssemblyPaths(arg=="--reject-old-washer");
            std::cout<<"Old blocked feed assembly path correctly rejected\n";return 0;
        }
        else if(arg!="--check")throw std::invalid_argument("Unknown feed CAD argument");
    }
    if(!out.empty()) {
        fs::create_directories(out/"step");
        std::ofstream pending(out/"status.json");
        pending<<"{\"run_complete\":false,\"fabrication_released\":false}\n";
        if(!pending)throw std::runtime_error("Cannot initialise feed CAD status");
    }
    if(cassette&&assemblyOnly)throw std::invalid_argument("Assembly-path option applies to the separate cartridge");
    se::checkFeedMachiningFeatures();
    auto parts=cassette?se::feedCassette(wireDiameter):se::feedCartridge();
    if(!assemblyOnly)se::checkFeedCartridge(parts);
    if(!cassette)se::checkFeedAssemblyPaths(parts);
    if(!out.empty()) {
        std::ofstream manifest(out/"manifest.csv");
        manifest<<"id,description,kind,xmin_mm,ymin_mm,zmin_mm,xmax_mm,ymax_mm,zmax_mm\n"<<std::setprecision(12);
        BRep_Builder builder;TopoDS_Compound assembly;builder.MakeCompound(assembly);
        const auto write=[&](const TopoDS_Shape& shape,const fs::path& path) {
            se::writeStep(shape,path);STEPControl_Reader reader;
            if(reader.ReadFile(path.c_str())!=IFSelect_RetDone||reader.TransferRoots()<1)
                throw std::runtime_error("Feed STEP readback failed");
            const auto result=reader.OneShape();const auto a=se::bounds(shape),b=se::bounds(result);
            if(!BRepCheck_Analyzer(result).IsValid()||std::abs(se::volume(shape)-se::volume(result))>1e-6*se::volume(shape)||
                std::abs(a.xmin-b.xmin)+std::abs(a.ymin-b.ymin)+std::abs(a.zmin-b.zmin)+
                std::abs(a.xmax-b.xmax)+std::abs(a.ymax-b.ymax)+std::abs(a.zmax-b.zmax)>1e-4)
                throw std::runtime_error("Feed STEP topology/dimension mismatch");
        };
        for(const auto& p:parts) {
            write(p.shape,out/"step"/(p.id+".step"));builder.Add(assembly,p.shape);
            const auto b=se::bounds(p.shape);
            manifest<<p.id<<",\""<<p.description<<"\","<<(p.purchased?"purchased_interface":"custom_or_secondary_operation")
                    <<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<'\n';
        }
        write(assembly,out/(cassette?"feed-cassette.step":"feed-cartridge.step"));
        std::ofstream status(out/"status.json");
        status<<"{\n  \"revision\":\""<<starz::feed::revision<<"\",\n  \"run_complete\":true,\n"
              <<"  \"scope\":\""<<(cassette?"four-roller placement and belt backing envelope;support/pressure slides absent":"one driven roller cartridge and motor interface")<<"\",\n  \"occurrences\":"<<parts.size()<<",\n"
              <<"  \"step_readback\":true,\n  \"static_intersections_checked\":true,\n"
              <<"  \"critical_continuous_axial_insertion_paths_checked\":"<<(cassette?"false":"true")<<",\n"
              <<"  \"insertion_scope\":\""<<(cassette?"complete-cassette insertion not checked;see separate cartridge record":"MB2 tang before key;preassembled rotor/pulley/front module from+S with module pins removed;rear bearing from-S;dowel service-punch access")<<"\",\n"
              <<"  \"all_tool_and_assembly_paths_verified\":false,\n"
              <<"  \"motor_geometry\":\"original supplier STEP without modification\",\n"
              <<"  \"threads\":\"nominal-major cosmetic;drawing callouts govern machining\",\n"
              <<"  \"shaft_machining_features_checked\":true,\n"
              <<"  \"shaft_root_scope\":\"bearing/neck shoulders;A12 groove;key-seat floors;M15 runout;end and thread-lead chamfers;washer-slot plan/intersection/floor blends;local stress not qualified\",\n"
              <<"  \"blind_tap_scope\":\"M5 full14mm after0.2entry;minor drill18mm cylindrical depth plus118degree point\",\n"
              <<"  \"bearing_fit_geometry\":\"nominal bearing/shaft solids;matched3–6um press-fit deformation belongs to separate fit model\",\n"
              <<"  \"purchased_bearings_pulley_coupling\":\"catalogue interference geometry;not OEM internal solids\",\n"
              <<"  \"pressure_carriage_integrated\":false,\n  \"belt_sweep_checked\":false,\n"
              <<"  \"housing_stress_verified\":false,\n  \"all_fasteners_integrated\":false,\n"
              <<"  \"fabrication_released\":false,\n  \"full_machine_integrated\":false\n}\n";
        if(!manifest||!status)throw std::runtime_error("Feed output failed");
    }
    std::cout<<(cassette?"Four-roller feed layout and belt-backing static interfaces checked\n":(assemblyOnly?"Feed critical continuous insertion paths checked\n":"Driven feed cartridge: solids,static interfaces and critical continuous insertion paths checked\n"));
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}