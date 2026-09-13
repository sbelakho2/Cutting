#include "head_geometry.hpp"
#include "head_preview.hpp"
#include "head_fit.hpp"
#include "shared/cutting_head_spec.hpp"
#include <BRep_Builder.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Reader.hxx>
#include <StlAPI_Reader.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Compound.hxx>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace se=starz::engineering;
namespace fs=std::filesystem;
namespace h=starz::head;
namespace {
void readback(const fs::path& step,const TopoDS_Shape& original) {
    STEPControl_Reader r;
    if(r.ReadFile(step.string().c_str())!=IFSelect_RetDone || r.TransferRoots()<1)
        throw std::runtime_error("STEP readback failed: "+step.string());
    const auto s=r.OneShape();
    if(!BRepCheck_Analyzer(s).IsValid() || std::abs(se::volume(s)-se::volume(original))>1e-6*se::volume(original))
        throw std::runtime_error("STEP topology/volume mismatch");
    const auto a=se::bounds(original),b=se::bounds(s);
    if(std::abs(a.xmin-b.xmin)+std::abs(a.ymin-b.ymin)+std::abs(a.zmin-b.zmin)+
       std::abs(a.xmax-b.xmax)+std::abs(a.ymax-b.ymax)+std::abs(a.zmax-b.zmax)>1e-4)
        throw std::runtime_error("STEP dimension mismatch");
}
}
int main(int argc,char** argv) try {
    fs::path out;
    bool rejectCoupling=false,rejectMotor=false,rejectSweep=false,rejectFastener=false;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--out" && i+1<argc) out=argv[++i];
        else if(arg=="--reject-coupling") rejectCoupling=true;
        else if(arg=="--reject-motor") rejectMotor=true;
        else if(arg=="--reject-sweep") rejectSweep=true;
        else if(arg=="--reject-fastener") rejectFastener=true;
        else if(arg!="--check") throw std::invalid_argument("Unknown CAD argument: "+arg);
    }
    if(!out.empty()) {
        fs::create_directories(out);
        std::ofstream pending(out/"status.json");
        pending<<"{\"revision\":\""<<h::revision<<"\",\"run_complete\":false,\"production_ready\":false}\n";
        if(!pending) throw std::runtime_error("Cannot initialise CAD run status");
    }
    if(rejectSweep) {
        auto p=se::headAssembly();
        p.push_back({"PROBE_F","Thin moving regression probe",BRepPrimAPI_MakeBox(gp_Pnt(200,0,100),1,0.02,1).Shape(),false,true});
        p.push_back({"OBSTACLE","Thin fixed regression obstacle",BRepPrimAPI_MakeBox(gp_Pnt(200,0.49,100),1,0.02,1).Shape(),false,false});
        se::checkHead(p,0); // No initial collision: the obstacle is between old samples.
        try {se::checkHeadSweep(p);}
        catch(const std::exception& e) {
            if(std::string(e.what()).find("Swept collision: PROBE_F / OBSTACLE")!=std::string::npos) {
                std::cout<<"Between-sample collision rejected\n"; return 0;
            }
            throw;
        }
        throw std::runtime_error("Regression: between-sample collision accepted");
    }
    if(rejectFastener) {
        auto p=se::headAssembly();
        for(auto& part:p)if(part.id=="HF0-B1_F"||part.id=="HF0-W1_F") {
            // Recreate the former5.5mm counterbore placement with an actual
            // standard screw head. It must collide with the original motor pilot.
            gp_Trsf t;t.SetTranslation(gp_Vec(0,-3,0));
            part.shape=BRepBuilderAPI_Transform(part.shape,t,true).Shape();
        }
        try {se::checkHead(p,0);}
        catch(const std::exception& e) {
            const std::string message=e.what();
            if(message.find("DR01-M_F interferes with HF0-B1_F")!=std::string::npos) {
                std::cout<<"Recessed mounting screw / motor pilot collision rejected\n";return 0;
            }
            throw;
        }
        throw std::runtime_error("Regression: screw-head / motor pilot collision accepted");
    }
    if(rejectCoupling || rejectMotor) {
        auto p=se::headAssembly();
        for(auto& part:p) if(part.id==(rejectCoupling?"CP01_F":"DR01-M_F")) {
            gp_Trsf t; t.SetScale(gp_Pnt(0,0,72),rejectCoupling?1.25:0.5);
            part.shape=BRepBuilderAPI_Transform(part.shape,t,true).Shape();
        }
        try { se::checkHead(p,0); }
        catch(const std::exception&) { std::cout<<"Invalid supplier envelope rejected\n"; return 0; }
        throw std::runtime_error("Regression: invalid supplier envelope accepted");
    }
    // The continuous interval proof covers every independent jaw position.
    // Rechecking six full static supplier assemblies adds no collision coverage.
    // Retain a separate end-position check for the catalogue carriage reserve.
    auto parts=se::headAssembly();
    se::checkHeadSweep(parts);
    se::checkHead(se::headAssembly(h::design::blade_limit),h::design::blade_limit);
    if(!out.empty()) {
        fs::create_directories(out/"step"); fs::create_directories(out/"stl");
        std::ofstream manifest(out/"manifest.csv");
        manifest<<"id,description,status,xmin_mm,ymin_mm,zmin_mm,xmax_mm,ymax_mm,zmax_mm,envelope_volume_mm3\n";
        se::writeHeadPreview(parts,out/"head-preview.svg");
        BRep_Builder builder; TopoDS_Compound assembly; builder.MakeCompound(assembly);
        for(const auto& p:parts) {
            builder.Add(assembly,p.shape);
            const auto step=out/"step"/(p.id+".step"); se::writeStep(p.shape,step); readback(step,p.shape);
            BRepMesh_IncrementalMesh mesh(p.shape,0.06,false,0.20,true);
            if(!mesh.IsDone()) throw std::runtime_error("STL tessellation failed");
            const auto stl=out/"stl"/(p.id+".stl"); StlAPI_Writer writer;
            if(!writer.Write(p.shape,stl.string().c_str())) throw std::runtime_error("STL write failed");
            TopoDS_Shape triangles; StlAPI_Reader reader;
            if(!reader.Read(triangles,stl.string().c_str()) || triangles.IsNull()) throw std::runtime_error("STL read failed");
            const auto b=se::bounds(p.shape), bt=se::bounds(triangles);
                if(std::abs(b.xmax-bt.xmax)>0.10 || std::abs(b.ymax-bt.ymax)>0.10 || std::abs(b.zmax-bt.zmax)>0.10 ||
                    std::abs(b.xmin-bt.xmin)>0.10 || std::abs(b.ymin-bt.ymin)>0.10 || std::abs(b.zmin-bt.zmin)>0.10)
                throw std::runtime_error("STL bounds outside tessellation allowance");
            manifest<<p.id<<",\""<<p.description<<"\","<<(p.purchased?"purchased_envelope":"custom_design")
              <<std::setprecision(12)<<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<','<<se::volume(p.shape)<<'\n';
        }
        se::writeStep(assembly,out/"head-assembly.step"); readback(out/"head-assembly.step",assembly);
        se::writeStep(se::holderShape(),out/"holder-local.step");
        se::writeStep(se::datumShape(),out/"datum-local.step");
        se::writeStep(se::adapterShape(),out/"adapter-local.step");
        const auto fit=se::selectedHeadFitEnvelope();
        std::ofstream fitTable(out/"coupling-fit.csv");
        fitTable<<"check,value_mm,basis\n"<<std::setprecision(12)
            <<"minimum_motor_tip_beyond_hub,"<<fit.minimum_motor_projection_mm<<",256 bounded tolerance corners\n"
            <<"minimum_stage_tip_beyond_hub,"<<fit.minimum_stage_projection_mm<<",256 bounded tolerance corners\n"
            <<"maximum_motor_tip_beyond_hub,"<<fit.maximum_motor_projection_mm<<",bellows internal clearance inspection required\n"
            <<"maximum_stage_tip_beyond_hub,"<<fit.maximum_stage_projection_mm<<",bellows internal clearance inspection required\n"
            <<"minimum_shaft_end_gap,"<<fit.minimum_shaft_gap_mm<<",independent worst case\n"
            <<"minimum_locknut_clearance,"<<fit.minimum_shoulder_gap_mm<<",measured feeler setting\n";
        std::ofstream report(out/"status.json");
        report<<"{\n  \"revision\": \""<<h::revision<<"\",\n  \"scope\": \"selected cutting head only\",\n"
                    <<"  \"run_complete\": true,\n"
          <<"  \"occurrences\": "<<parts.size()<<",\n  \"checked_endpoint_closures_mm\": [0,4.2],\n"
          <<"  \"step_and_stl_readback\": true,\n  \"cutting_motor_models\": \"original supplier STEP, including cables and connectors; supplied parallel key removed for keyless coupling\",\n"
          <<"  \"other_purchased_models\": \"catalogue interface geometry\",\n"
          <<"  \"adapter_mounting_fasteners\": \"M5x16 with9ODx1 washers;8.5mm recess;M5x25 motor screws included\",\n"
          <<"  \"intentional_thread_intersections\": \"motor screw major diameter inside exact M5 adapter tapping zones only\",\n"
          <<"  \"axial_coupling_tolerance_corners\": "<<fit.corners<<",\n"
          <<"  \"full_hub_tolerance_check\": true,\n"
          <<"  \"fit_basis\": \"supplier motor/coupling tolerances plus explicit incoming inspection limits and 1.00mm shoulder setting\",\n"
          <<"  \"full_machine_integrated\": false,\n  \"all_swept_collisions_checked\": false,\n"
          <<"  \"selected_head_independent_jaw_sweep\": true,\n  \"jaw_range_mm\": [0,4.2],\n"
          <<"  \"sweep_method\": \"relative-translation interval distance bound; unresolved intervals fail\",\n"
          <<"  \"fabrication_released\": false,\n  \"tooling_qualified\": false,\n  \"production_ready\": false\n}\n";
        if(!manifest || !report || !fitTable) throw std::runtime_error("Output write error");
    }
    std::cout<<"Selected-head solids, interfaces and independent-jaw translation sweeps checked; NOT a machine release.\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }