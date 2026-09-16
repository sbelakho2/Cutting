#include "containment_audit.hpp"
#include <BRepPrimAPI_MakeBox.hxx>
#include <Eigen/Core>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
void write(const fs::path& path,const std::string& text){std::ofstream f(path,std::ios::binary);f<<text;f.close();require(bool(f),"Fixture write failed");}
std::string read(const fs::path& path){std::ifstream f(path,std::ios::binary);require(bool(f),"Fixture read failed");return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
void replaceToken(std::string& text,const std::string& old,const std::string& value) {
    const auto at=text.find(old);require(at!=std::string::npos,"Fixture token missing");text.replace(at,old.size(),value);
}
std::string vtu(const std::vector<Eigen::Vector3d>& points,const std::vector<std::array<int,4>>& tets) {
    std::ostringstream s;s<<std::setprecision(14);
    s<<"<VTKFile type=\"UnstructuredGrid\"><UnstructuredGrid><Piece NumberOfPoints=\""<<points.size()<<"\" NumberOfCells=\""<<tets.size()<<"\">"
     <<"<Points><DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">";
    for(const auto& p:points)s<<p.x()<<' '<<p.y()<<' '<<p.z()<<' ';
    s<<"</DataArray></Points><Cells><DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">";
    for(const auto& t:tets)for(int n:t)s<<n<<' ';
    s<<"</DataArray><DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">";
    for(std::size_t i=0;i<tets.size();++i)s<<4*(i+1)<<' ';
    s<<"</DataArray><DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">";
    for(std::size_t i=0;i<tets.size();++i)s<<"10 ";
    s<<"</DataArray></Cells></Piece></UnstructuredGrid></VTKFile>";
    return s.str();
}
struct Fixture {std::vector<Eigen::Vector3d> points;std::vector<std::array<int,4>> tets;};
Fixture merge(const std::vector<Fixture>& parts) {
    Fixture out;
    for(const auto& part:parts) {
        const int shift=static_cast<int>(out.points.size());
        for(const auto& p:part.points)out.points.push_back(p);
        for(const auto& t:part.tets)out.tets.push_back({t[0]+shift,t[1]+shift,t[2]+shift,t[3]+shift});
    }
    return out;
}
void writeSeries(const fs::path& dir,const std::vector<Fixture>& levels,double cadVolume) {
    fs::create_directories(dir);
    const double bulk[4]={2,1.6,1.2,1},root[4]={0.3,0.2,0.13,0.09};
    {std::ofstream q(dir/"mesh-quality.csv");q<<std::setprecision(14)
        <<"bulk_mm,root_mm,nodes,tets,min_SICN,CAD_volume_mm3,mesh_volume_mm3,relative_volume_error\n";
     for(int i=0;i<4;++i)q<<bulk[i]<<','<<root[i]<<','<<levels[static_cast<std::size_t>(i)].points.size()<<','<<levels[static_cast<std::size_t>(i)].tets.size()
        <<",0.5,"<<cadVolume<<','<<cadVolume*1.001<<",0.001\n";}
    write(dir/"status.json","{\"revision\":\"test\",\"run_complete\":true,\"mesh_levels\":4,\"mesh_focus_region\":\"all_roots\","
        "\"background_root_mm\":0,\"focus_growth\":0.45,\"mesh_focus_feature\":\"concave_fillet_tori\",\"sizing_face_count\":9,"
        "\"mesh_optimizer\":\"netgen\",\"continuous_CAD_containment_verified\":false,\"phases\":48,\"node_limit\":150000}\n");
    for(int i=0;i<4;++i) {
        const auto geometry=vtu(levels[static_cast<std::size_t>(i)].points,levels[static_cast<std::size_t>(i)].tets);
        write(dir/("working-"+std::to_string(i+1)+".vtu"),geometry);
        write(dir/("fault-"+std::to_string(i+1)+".vtu"),geometry);
    }
}
Fixture innerTet(const Eigen::Vector3d& base) {
    return {{base,{base.x()+1,base.y(),base.z()},{base.x(),base.y()+1,base.z()},{base.x(),base.y(),base.z()+1}},{{0,1,2,3}}};
}
}
int main(int argc,char** argv)try {
    require(argc==2,"Containment audit test requires an output directory");
    const fs::path out=argv[1];fs::create_directories(out);
    const TopoDS_Shape solid=BRepPrimAPI_MakeBox(gp_Pnt(0,0,0),10,10,10).Solid();
    const Fixture onBoundary{{{-1,1,1},{1,1,1},{0,2,1},{0,1,2}},{{0,1,2,3}}};
    const Fixture outside{{{10,0,0},{11,0,0},{10,1,0},{10,0,1}},{{0,1,2,3}}};
    std::vector<Fixture> violating;
    violating.push_back(innerTet({1,1,1}));
    violating.push_back(merge({innerTet({5,5,5}),innerTet({2,5,5})}));
    violating.push_back(merge({innerTet({1,1,1}),onBoundary}));
    violating.push_back(merge({innerTet({1,1,1}),outside}));
    const fs::path violationsDir=out/"violating-series";
    writeSeries(violationsDir,violating,1000);
    const auto before=read(violationsDir/"status.json");
    const auto result=se::runContainmentAudit(solid,violationsDir);
    require(!result.all_inside,"Violating series must not verify");
    require(result.levels.size()==4&&result.classifier_tolerance_mm==1e-7,"Audit shape changed");
    require(result.levels[0].inside==1&&result.levels[0].outside==0&&result.levels[0].on_boundary==0,"Level 1 counts wrong");
    require(result.levels[1].inside==2&&result.levels[1].outside==0&&result.levels[1].on_boundary==0,"Level 2 counts wrong");
    require(result.levels[2].inside==1&&result.levels[2].outside==0&&result.levels[2].on_boundary==1,"Level 3 counts wrong");
    require(result.levels[3].inside==1&&result.levels[3].outside==1&&result.levels[3].on_boundary==0,"Level 4 counts wrong");
    require(read(violationsDir/"status.json")==before,"Violating series must not touch status.json");
    const auto violations=read(violationsDir/"containment-violations.csv");
    require(violations.find(",on,")!=std::string::npos&&violations.find(",outside,")!=std::string::npos,"Violation states not reported");
    require(violations.find("3,1,")!=std::string::npos&&violations.find("4,1,")!=std::string::npos,"Violation level/element not reported");
    require(read(violationsDir/"containment-audit.json").find("\"all_inside_verified\":false")!=std::string::npos,"Audit JSON verdict wrong");
    require(read(violationsDir/"containment-audit.csv").find("level,bulk_mm,root_mm,nodes,tetrahedra,inside,outside,on_boundary")==0,"Audit CSV header wrong");
    std::vector<Fixture> clean;
    for(int i=0;i<4;++i)clean.push_back(innerTet(Eigen::Vector3d{1+1.5*i,1,1}));
    const fs::path cleanDir=out/"clean-series";
    writeSeries(cleanDir,clean,1000);
    const auto cleanResult=se::runContainmentAudit(solid,cleanDir);
    require(cleanResult.all_inside&&cleanResult.levels.size()==4,"Clean series must verify");
    for(const auto& level:cleanResult.levels)require(level.outside==0&&level.on_boundary==0&&level.inside==level.tetrahedra,"Clean level counts wrong");
    const auto status=read(cleanDir/"status.json");
    require(status.find("\"continuous_CAD_containment_verified\":true")!=std::string::npos&&
            status.find("\"continuous_CAD_containment_verified\":false")==std::string::npos,"Containment flag not flipped");
    require(!fs::exists(cleanDir/"containment-violations.csv"),"Clean series must not write violations");
    const auto rerun=se::runContainmentAudit(solid,cleanDir);
    require(rerun.all_inside&&read(cleanDir/"status.json")==status,"Idempotent re-audit failed");
    int rejected=0;
    const auto reject=[&](const fs::path& dir,const char* label,const auto& mutate) {
        fs::remove_all(dir);writeSeries(dir,clean,1000);mutate(dir);
        bool threw=false;try{(void)se::runContainmentAudit(solid,dir);}catch(const std::runtime_error&){threw=true;}
        const auto message=std::string("Unverified input accepted: ")+label;
        require(threw,message.c_str());++rejected;
        fs::remove_all(dir);
    };
    reject(out/"r1","missing fault VTU",[](const fs::path& d){fs::remove(d/"fault-2.vtu");});
    reject(out/"r2","working/fault geometry mismatch",[](const fs::path& d){
        auto text=read(d/"fault-3.vtu");replaceToken(text,">4 1 1 ",">4.001 1 1 ");write(d/"fault-3.vtu",text);});
    reject(out/"r3","mesh-quality count mismatch",[](const fs::path& d){
        auto text=read(d/"mesh-quality.csv");replaceToken(text,",4,",",5,");write(d/"mesh-quality.csv",text);});
    reject(out/"r4","non-refining levels",[](const fs::path& d){
        auto text=read(d/"mesh-quality.csv");replaceToken(text,"1.6,","2.1,");write(d/"mesh-quality.csv",text);});
    reject(out/"r5","missing status.json",[](const fs::path& d){fs::remove(d/"status.json");});
    reject(out/"r6","incomplete series status",[](const fs::path& d){
        auto text=read(d/"status.json");replaceToken(text,"\"run_complete\":true","\"run_complete\":false");write(d/"status.json",text);});
    reject(out/"r7","flag absent",[](const fs::path& d){
        auto text=read(d/"status.json");replaceToken(text,"\"continuous_CAD_containment_verified\":false,","");write(d/"status.json",text);});
    reject(out/"r8","wrong optimizer series",[](const fs::path& d){
        auto text=read(d/"status.json");replaceToken(text,"\"mesh_optimizer\":\"netgen\"","\"mesh_optimizer\":\"gmsh\"");write(d/"status.json",text);});
    reject(out/"r9","duplicated flag",[](const fs::path& d){
        auto text=read(d/"status.json");replaceToken(text,"\"phases\"","\"continuous_CAD_containment_verified\":false,\"phases\"");write(d/"status.json",text);});
    reject(out/"r10","wrong CAD volume",[](const fs::path& d){
        auto text=read(d/"mesh-quality.csv");replaceToken(text,",1000,",",999,");write(d/"mesh-quality.csv",text);});
    reject(out/"r11","malformed flag value",[](const fs::path& d){
        auto text=read(d/"status.json");replaceToken(text,"\"continuous_CAD_containment_verified\":false","\"continuous_CAD_containment_verified\":falsex");write(d/"status.json",text);});
    reject(out/"r12","empty VTU geometry",[](const fs::path& d){write(d/"working-4.vtu","<VTKFile type=\"UnstructuredGrid\"></VTKFile>");});
    std::cout<<"Containment audit classification, flag flip, idempotence and "<<rejected<<" fail-closed cases passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
