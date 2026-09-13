#include "measurement_geometry.hpp"
#include "shared/measurement_spec.hpp"
#include <BRepCheck_Analyzer.hxx>
#include <BRep_Builder.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Compound.hxx>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void exportReadback(const TopoDS_Shape& shape,const fs::path& path) {
    se::writeStep(shape,path);STEPControl_Reader reader;
    if(reader.ReadFile(path.c_str())!=IFSelect_RetDone||reader.TransferRoots()<1)throw std::runtime_error("Measurement STEP readback failed");
    const auto read=reader.OneShape();const auto a=se::bounds(shape),b=se::bounds(read);
    if(!BRepCheck_Analyzer(read).IsValid()||std::abs(se::volume(shape)-se::volume(read))>se::volume(shape)*1e-6||
       std::abs(a.xmin-b.xmin)+std::abs(a.xmax-b.xmax)+std::abs(a.ymin-b.ymin)+std::abs(a.ymax-b.ymax)+
       std::abs(a.zmin-b.zmin)+std::abs(a.zmax-b.zmax)>1e-4)throw std::runtime_error("Measurement STEP geometry mismatch");
}
}
int main(int argc,char** argv)try {
    fs::path out;bool short_tip=false,wrong_screw=false;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--out"&&i+1<argc)out=argv[++i];
        else if(arg=="--reject-short-tip")short_tip=true;
        else if(arg=="--reject-wheel-M3")wrong_screw=true;
        else if(arg!="--check")throw std::invalid_argument("Unknown measurement CAD argument");
    }
    if(short_tip||wrong_screw) {
        if(short_tip&&wrong_screw)throw std::invalid_argument("Choose one measurement geometry regression");
        bool caught=false;
        try {se::checkMeasuringInterfaces(se::measuringWheelInterface(wrong_screw?3:4),se::measuringEncoderInterface(short_tip?15:20));}
        catch(const std::runtime_error& error) {
            const std::string message=error.what();
            caught=message==(short_tip?"Encoder tip must be20mm from mounting face":"No123 wheel requiresM4 notM3 radial holes");
            if(!caught)throw;
        }
        if(!caught)throw std::runtime_error("Invalid measurement interface was accepted");
        std::cout<<"Incorrect source interpretation rejected\n";return 0;
    }
    if(!out.empty()) {
        fs::create_directories(out/"step");std::ofstream pending(out/"status.json");
        pending<<"{\"run_complete\":false,\"measurement_module_integrated\":false,\"fabrication_released\":false}\n";
        if(!pending)throw std::runtime_error("Cannot initialise measurement CAD status");
    }
    const auto wheel=se::measuringWheelInterface(),encoder=se::measuringEncoderInterface();
    se::checkMeasuringInterfaces(wheel,encoder);
    const auto mount=se::measuringEncoderMount();se::checkMeasuringEncoderMount(mount);
    if(!out.empty()) {
        std::vector<se::HeadPart> interfaces{{"ME-W01","Kuebler No123 dimensioned external/bore/thread envelope",wheel,true,false},
            {"ME-E01","Autonics E40S6 dimensioned body/pilot/shaft/face envelope",encoder,true,false},
            {"ME-C01","Autonics supplied6mm coupling envelope;flexure interior not provided",se::measuringCouplingInterface(),true,false},
            {"ME-B01","SKF626-2Z outer/bore/width interface;no raceway internals",se::measuringBearingInterface(),true,false},
            {"M201","STARZ encoder plate46x46x6",se::measuringEncoderPlate(),false,false}};
        std::ofstream manifest(out/"manifest.csv");manifest<<std::setprecision(12)<<"id,description,volume_mm3,xmin_mm,ymin_mm,zmin_mm,xmax_mm,ymax_mm,zmax_mm\n";
        for(const auto& part:interfaces) {
            exportReadback(part.shape,out/"step"/(part.id+".step"));const auto b=se::bounds(part.shape);
            manifest<<part.id<<",\""<<part.description<<"\","<<se::volume(part.shape)<<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<'\n';
        }
        BRep_Builder builder;TopoDS_Compound compound;builder.MakeCompound(compound);
        for(const auto& part:mount)builder.Add(compound,part.shape);
        exportReadback(compound,out/"encoder-mount.step");
        manifest.flush();if(!manifest)throw std::runtime_error("Measurement CAD manifest write failure");
        std::ofstream status(out/"status.json");
        status<<"{\n  \"revision\":\""<<starz::measurement::revision<<"\",\n  \"run_complete\":true,\n"
            <<"  \"source_scope\":\"catalogue interfaces,not original supplier CAD or purchased-part manufacturing definitions\",\n"
            <<"  \"interface_solids\":5,\n  \"encoder_mount_occurrences\":8,\n  \"encoder_mount_intersections_checked\":true,\n  \"step_readbacks\":true,\n"
            <<"  \"wheel_radial_thread_note\":\"M4x6 callout;four openings depicted;quantity not separately stated\",\n"
            <<"  \"encoder_cable_route_modelled\":false,\n  \"coupling_internal_clearance_verified\":false,\n  \"measurement_module_integrated\":false,\n"
            <<"  \"complete_support_tooling_and_torques_released\":false,\n  \"fabrication_released\":false\n}\n";
        status.flush();if(!status)throw std::runtime_error("Measurement CAD status write failure");
    }
    std::cout<<"Measuring wheel/encoder/bearing/coupling interfaces and8-part encoder mounting assembly checked\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}