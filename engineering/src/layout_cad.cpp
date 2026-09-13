#include "feed_geometry.hpp"
#include "shared/cutting_head_spec.hpp"
#include "shared/feed_drive_spec.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRep_Builder.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Ax1.hxx>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace fs=std::filesystem;
namespace {
bool disjoint(const se::BoxBounds& a,const se::BoxBounds& b) {
    return a.xmax<=b.xmin+1e-7||b.xmax<=a.xmin+1e-7||a.ymax<=b.ymin+1e-7||b.ymax<=a.ymin+1e-7||
        a.zmax<=b.zmin+1e-7||b.zmax<=a.zmin+1e-7;
}
double xGap(const se::BoxBounds& a,const se::BoxBounds& b) {
    return std::max(b.xmin-a.xmax,a.xmin-b.xmax);
}
void compareGroups(const std::vector<se::HeadPart>& a,const std::vector<se::HeadPart>& b) {
    std::vector<se::BoxBounds> A,B;A.reserve(a.size());B.reserve(b.size());
    for(const auto& p:a)A.push_back(se::bounds(p.shape));
    for(const auto& p:b)B.push_back(se::bounds(p.shape));
    for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<b.size();++j) {
        const auto& x=a[i];const auto& y=b[j];
        if(disjoint(A[i],B[j]))continue;
        BRepAlgoAPI_Common common(x.shape,y.shape);common.Build();
        if(!common.IsDone())throw std::runtime_error("Layout boolean failed");
        const double v=se::volume(common.Shape());
        if(v>1e-4)throw std::runtime_error("Layout interference "+x.id+" / "+y.id+": "+std::to_string(v)+" mm3");
    }
}
void readback(const TopoDS_Shape& shape,const fs::path& path) {
    se::writeStep(shape,path);STEPControl_Reader reader;
    if(reader.ReadFile(path.c_str())!=IFSelect_RetDone||reader.TransferRoots()<1)throw std::runtime_error("Layout STEP readback failed");
    const auto r=reader.OneShape();const auto a=se::bounds(shape),b=se::bounds(r);
    if(!BRepCheck_Analyzer(r).IsValid()||std::abs(se::volume(shape)-se::volume(r))>se::volume(shape)*1e-6||
        std::abs(a.xmin-b.xmin)+std::abs(a.xmax-b.xmax)+std::abs(a.ymin-b.ymin)+std::abs(a.ymax-b.ymax)+
        std::abs(a.zmin-b.zmin)+std::abs(a.zmax-b.zmax)>1e-4)throw std::runtime_error("Layout STEP dimension/topology mismatch");
}
}
int main(int argc,char** argv)try {
    fs::path out;double diameter=4.5;bool reject=false;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--out"&&i+1<argc)out=argv[++i];
        else if(arg=="--wire-od"&&i+1<argc)diameter=std::stod(argv[++i]);
        else if(arg=="--reject-near-motor")reject=true;
        else if(arg!="--check")throw std::invalid_argument("Unknown layout argument");
    }
    if(!out.empty()) {
        fs::create_directories(out);std::ofstream pending(out/"status.json");
        pending<<"{\"run_complete\":false,\"full_machine_integrated\":false,\"fabrication_released\":false}\n";
        if(!pending)throw std::runtime_error("Cannot initialise layout status");
    }
    const auto head=se::headAssembly();auto right=se::feedCassette(diameter),left=right;
    gp_Trsf turn;turn.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),std::numbers::pi);
    for(auto& p:left){p.id="LEFT_"+p.id;p.shape=BRepBuilderAPI_Transform(p.shape,turn,true).Shape();}
    for(auto& p:right)p.id="RIGHT_"+p.id;
    if(reject) {
        // Put the outer motor/plate/coupling at the near shaft. This recreates
        // the packaging conflict without modifying the imported motor solid.
        gp_Trsf shift;shift.SetTranslation(gp_Vec(-starz::feed::design::nominal_centres,0,0));
        for(auto& p:right)if(p.id=="RIGHT_L2_FM01"||p.id=="RIGHT_L2_F111"||p.id=="RIGHT_L2_CP02")
            p.shape=BRepBuilderAPI_Transform(p.shape,shift,true).Shape();
        try {compareGroups(head,right);}
        catch(const std::runtime_error& e) {
            if(std::string(e.what()).starts_with("Layout interference")){std::cout<<"Near-motor layout interference correctly rejected\n";return 0;}
            throw;
        }
        throw std::runtime_error("Near-motor layout was not rejected");
    }
    compareGroups(head,right);compareGroups(head,left);compareGroups(left,right);
    double gap=std::numeric_limits<double>::infinity();
    std::vector<se::BoxBounds> feedBoxes;feedBoxes.reserve(left.size()+right.size());
    for(const auto* feed:{&left,&right})for(const auto& p:*feed)feedBoxes.push_back(se::bounds(p.shape));
    for(const auto& h:head)if(h.moving) {
        const auto hb=se::bounds(h.shape);
        for(const auto& fb:feedBoxes) {
            const double g=xGap(hb,fb);gap=std::min(gap,g);
            // Both jaws translate solely inY. A strictXseparator proves every
            // independent jaw position,not merely sampled open/closed poses.
            if(g<=1e-5)throw std::runtime_error("No continuous jaw/feed X-separation certificate");
        }
    }
    const std::size_t count=head.size()+left.size()+right.size();
    if(!out.empty()) {
        BRep_Builder builder;TopoDS_Compound compound;builder.MakeCompound(compound);
        std::ofstream manifest(out/"manifest.csv");manifest<<std::setprecision(12)<<"id,description,xmin_mm,ymin_mm,zmin_mm,xmax_mm,ymax_mm,zmax_mm\n";
        for(const auto* group:{&head,static_cast<const std::vector<se::HeadPart>*>(&left),static_cast<const std::vector<se::HeadPart>*>(&right)})for(const auto& p:*group) {
            builder.Add(compound,p.shape);const auto b=se::bounds(p.shape);
            manifest<<p.id<<",\""<<p.description<<"\","<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<'\n';
        }
        readback(compound,out/"head-and-feed-layout.step");
        std::ofstream status(out/"status.json");
        status<<"{\n  \"head_revision\":\""<<starz::head::revision<<"\",\n  \"feed_revision\":\""<<starz::feed::revision<<"\",\n"
              <<"  \"run_complete\":true,\n  \"scope\":\"selected head and eight-roller placements;support frame,pressure/tensioning slides,measuring modules and guards absent\",\n"
              <<"  \"occurrences\":"<<count<<",\n  \"wire_od_mm\":"<<diameter<<",\n  \"left_cassette_transform\":\"proper180degreeZrotation,not reflection of purchased parts\",\n"
              <<"  \"cross_subsystem_static_intersections_checked\":true,\n  \"minimum_moving_head_to_feed_X_gap_mm\":"<<gap<<",\n"
              <<"  \"independent_jaw_feed_sweep_certificate\":\"strictXseparation for allYtranslations;head-internal sweep verified separately\",\n"
              <<"  \"step_readback\":true,\n  \"frame_integrated\":false,\n  \"full_machine_integrated\":false,\n  \"fabrication_released\":false\n}\n";
        if(!manifest||!status)throw std::runtime_error("Layout output failed");
    }
    std::cout<<count<<"-part head/feed layout checked;continuous jaw/feed X-gap "<<gap<<" mm\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}