#include "containment_audit.hpp"
#include "cad_boundary.hpp"
#include "feed_geometry.hpp"
#include "head_geometry.hpp"
#include "mesh_io.hpp"
#include <BRepCheck_Analyzer.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <gp_Pnt.hxx>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void close(std::ofstream& stream){stream.close();require(bool(stream),"Containment audit write failed");}
double number(const std::string& text) {
    require(!text.empty()&&text.front()!=' '&&text.back()!=' ',"Malformed containment audit number");
    std::size_t used=0;const double value=std::stod(text,&used);
    require(used==text.size()&&std::isfinite(value),"Non-finite containment audit number");return value;
}
std::size_t count(const std::string& text,std::size_t maximum) {
    std::size_t value=0;const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
    require(result.ec==std::errc{}&&result.ptr==text.data()+text.size()&&value>0&&value<=maximum,"Invalid containment audit count");return value;
}
std::vector<std::string> split(const std::string& line,char separator,std::size_t fields) {
    std::vector<std::string> result;std::size_t begin=0;
    for(;;) {
        const auto end=line.find(separator,begin);
        result.push_back(line.substr(begin,end==std::string::npos?line.size():end-begin));
        if(end==std::string::npos)break;begin=end+1;
    }
    require(result.size()==fields&&std::all_of(result.begin(),result.end(),[](const std::string& f){return !f.empty();}),"Wrong containment audit column count");
    return result;
}
struct LevelExpectation {
    std::size_t level=0;double bulk_mm=0,root_mm=0;std::size_t nodes=0,tetrahedra=0;double cad_volume_mm3=0;
};
std::vector<LevelExpectation> readMeshQuality(const fs::path& path) {
    require(fs::is_regular_file(path),"mesh-quality.csv missing");
    std::ifstream input(path);require(bool(input),"mesh-quality.csv cannot be opened");
    std::string line;std::getline(input,line);if(!line.empty()&&line.back()=='\r')line.pop_back();
    require(line=="bulk_mm,root_mm,nodes,tets,min_SICN,CAD_volume_mm3,mesh_volume_mm3,relative_volume_error","Unexpected mesh-quality.csv header");
    std::vector<LevelExpectation> levels;double previousBulk=std::numeric_limits<double>::infinity(),previousRoot=std::numeric_limits<double>::infinity();
    while(std::getline(input,line)) {
        if(!line.empty()&&line.back()=='\r')line.pop_back();require(!line.empty()&&line.back()!=',',"Empty mesh-quality row/field");
        const auto fields=split(line,',',8);
        const double bulk=number(fields[0]),root=number(fields[1]),cadVolume=number(fields[5]);
        (void)number(fields[4]);(void)number(fields[6]);(void)number(fields[7]);
        require(bulk>0&&root>0&&root<=bulk&&cadVolume>0,"Invalid mesh-level sizing");
        require(bulk<previousBulk&&root<previousRoot,"Mesh levels must strictly refine");
        levels.push_back({levels.size()+1,bulk,root,count(fields[2],150000),count(fields[3],2000000),cadVolume});
        previousBulk=bulk;previousRoot=root;
    }
    require(levels.size()==4,"Expected exactly four mesh levels");
    return levels;
}
std::string readStatus(const fs::path& path) {
    require(fs::is_regular_file(path),"status.json missing");
    std::ifstream input(path,std::ios::binary);require(bool(input),"status.json cannot be opened");
    const std::string text{std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};require(!input.bad(),"status.json read failed");
    for(const char* expected:{"\"run_complete\":true","\"mesh_levels\":4","\"mesh_optimizer\":\"netgen\"","\"node_limit\":150000","\"focus_growth\":0.45","\"mesh_focus_feature\":\"concave_fillet_tori\""})
        require(text.find(expected)!=std::string::npos,"status.json does not identify the final four-mesh torus series");
    const std::string key="\"continuous_CAD_containment_verified\":";
    const auto at=text.find(key);
    require(at!=std::string::npos&&text.find(key,at+key.size())==std::string::npos,"Containment flag missing or duplicated in status.json");
    const std::string_view rest(text.data()+at+key.size(),text.size()-at-key.size());
    require(rest.starts_with("true,")||rest.starts_with("true}")||rest.starts_with("false,")||rest.starts_with("false}"),"Unexpected containment flag value");
    return text;
}
void setContainmentVerified(std::string& statusText) {
    const std::string key="\"continuous_CAD_containment_verified\":";
    if(statusText.find(key+"true")!=std::string::npos)return;
    const auto at=statusText.find(key+"false");
    require(at!=std::string::npos&&statusText.find(key+"false",at+key.size())==std::string::npos,"Containment false token missing or duplicated");
    statusText.replace(at+key.size(),5,"true");
}
struct Violation {std::size_t level=0,element=0;Eigen::Vector3d centroid=Eigen::Vector3d::Zero();TopAbs_State state=TopAbs_UNKNOWN;};
}
se::ContainmentAuditResult se::runContainmentAudit(const TopoDS_Shape& solid,const fs::path& directory) {
    require(fs::is_directory(directory),"Mesh directory missing");
    const auto expectations=readMeshQuality(directory/"mesh-quality.csv");
    const auto statusText=readStatus(directory/"status.json");
    require(!solid.IsNull()&&BRepCheck_Analyzer(solid).IsValid(),"Invalid containment audit solid");
    CadBoundaryInspector inspector(solid);
    const double volume=se::volume(solid);require(std::isfinite(volume)&&volume>0,"Non-finite CAD solid volume");
    for(const auto& expected:expectations)
        require(std::abs(volume/expected.cad_volume_mm3-1)<1e-9,"STEP changed nominal shaft volume");
    std::vector<TetMesh> meshes;meshes.reserve(expectations.size());
    for(const auto& expected:expectations) {
        const auto name=std::to_string(expected.level);
        const auto working=se::readLinearVtuGeometry(directory/("working-"+name+".vtu"));
        const auto fault=se::readLinearVtuGeometry(directory/("fault-"+name+".vtu"));
        require(working.nodes_mm.size()==expected.nodes&&working.tetrahedra.size()==expected.tetrahedra,"Working VTU disagrees with mesh-quality.csv");
        require(fault.nodes_mm.size()==expected.nodes&&fault.tetrahedra.size()==expected.tetrahedra,"Fault VTU disagrees with mesh-quality.csv");
        require(working.nodes_mm==fault.nodes_mm&&working.tetrahedra==fault.tetrahedra,"Working/fault geometry must be the one shared mesh");
        meshes.push_back(std::move(working));
    }
    BRepClass3d_SolidClassifier classifier(solid);
    ContainmentAuditResult result;result.classifier_tolerance_mm=1e-7;result.cad_volume_mm3=volume;
    std::vector<Violation> violations;
    for(std::size_t levelIndex=0;levelIndex<meshes.size();++levelIndex) {
        const auto& expected=expectations[levelIndex];const auto& mesh=meshes[levelIndex];
        std::clog<<"Containment audit level "<<expected.level<<" ("<<expected.bulk_mm<<'/'<<expected.root_mm<<" mm): classifying "<<expected.tetrahedra<<" centroids\n";
        ContainmentLevelResult level{expected.level,expected.bulk_mm,expected.root_mm,expected.nodes,expected.tetrahedra,0,0,0};
        for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
            Eigen::Vector3d centroid=Eigen::Vector3d::Zero();
            for(int n:mesh.tetrahedra[e])centroid+=mesh.nodes_mm.at(static_cast<std::size_t>(n))/4;
            require(centroid.allFinite(),"Non-finite tetrahedron centroid");
            classifier.Perform(gp_Pnt(centroid.x(),centroid.y(),centroid.z()),1e-7);
            const auto state=classifier.State();
            require(state==TopAbs_IN||state==TopAbs_ON||state==TopAbs_OUT,"Unknown CAD centroid classification");
            if(state==TopAbs_IN)++level.inside;
            else if(state==TopAbs_OUT)++level.outside;
            else ++level.on_boundary;
            if(state!=TopAbs_IN)violations.push_back({expected.level,e,centroid,state});
        }
        require(level.inside+level.outside+level.on_boundary==level.tetrahedra,"Containment count mismatch");
        result.levels.push_back(level);
    }
    result.all_inside=violations.empty();
    {std::ofstream table(directory/"containment-audit.csv");table<<std::setprecision(14)
        <<"level,bulk_mm,root_mm,nodes,tetrahedra,inside,outside,on_boundary\n";
     for(const auto& level:result.levels)table<<level.level<<','<<level.bulk_mm<<','<<level.root_mm<<','<<level.nodes<<','<<level.tetrahedra
        <<','<<level.inside<<','<<level.outside<<','<<level.on_boundary<<'\n';close(table);}
    {std::ofstream state(directory/"containment-audit.json");state<<std::setprecision(14)<<"{\"run_complete\":true,\"classifier_tolerance_mm\":1e-7,\"cad_volume_mm3\":"
        <<result.cad_volume_mm3<<",\"all_inside_verified\":"<<(result.all_inside?"true":"false")<<",\"levels\":[";
     for(std::size_t i=0;i<result.levels.size();++i) {
        const auto& level=result.levels[i];if(i)state<<',';
        state<<"{\"level\":"<<level.level<<",\"bulk_mm\":"<<level.bulk_mm<<",\"root_mm\":"<<level.root_mm<<",\"nodes\":"<<level.nodes
             <<",\"tetrahedra\":"<<level.tetrahedra<<",\"inside\":"<<level.inside<<",\"outside\":"<<level.outside<<",\"on_boundary\":"<<level.on_boundary<<'}';
     }
     state<<"]}\n";close(state);}
    if(!violations.empty()) {
        std::ofstream exterior(directory/"containment-violations.csv");
        exterior<<std::setprecision(14)<<"level,element_zero_based,x_mm,S_mm,z_mm,state,nearest_face_one_based,distance_mm,projected_x_mm,projected_S_mm,projected_z_mm\n";
        for(const auto& violation:violations) {
            const auto detail=inspector.inspect(violation.centroid);
            exterior<<violation.level<<','<<violation.element<<','<<violation.centroid.x()<<','<<violation.centroid.y()<<','<<violation.centroid.z()<<','
                    <<se::cadStateName(violation.state)<<','<<detail.face<<','<<detail.distance_mm<<','
                    <<detail.projected_mm.x()<<','<<detail.projected_mm.y()<<','<<detail.projected_mm.z()<<'\n';
        }
        close(exterior);
    } else {
        auto text=statusText;setContainmentVerified(text);
        std::ofstream state(directory/"status.json",std::ios::binary);state<<text;close(state);
    }
    return result;
}
#ifndef STARZ_CONTAINMENT_NO_MAIN
int main(int argc,char** argv)try {
    require(argc==2,"Use MESH_DIRECTORY");
    const auto solid=se::feedShaftShape();
    const auto result=se::runContainmentAudit(solid,argv[1]);
    std::size_t outside=0,onBoundary=0,tetrahedra=0;
    std::cout<<std::setprecision(14)<<"Exact CAD containment audit (BRep point-in-solid, tolerance 1e-7 mm, solid volume "<<result.cad_volume_mm3<<" mm3):\n";
    for(const auto& level:result.levels) {
        outside+=level.outside;onBoundary+=level.on_boundary;tetrahedra+=level.tetrahedra;
        std::cout<<"  level "<<level.level<<" bulk/root "<<level.bulk_mm<<'/'<<level.root_mm<<" mm: nodes="<<level.nodes<<", tets="<<level.tetrahedra
                 <<", inside="<<level.inside<<", outside="<<level.outside<<", on="<<level.on_boundary<<'\n';
    }
    if(result.all_inside)std::cout<<"All "<<tetrahedra<<" centroids strictly inside the exact CAD solid; continuous_CAD_containment_verified=true\n";
    else {std::cout<<"Violations: outside="<<outside<<", on-boundary="<<onBoundary<<"; continuous_CAD_containment_verified stays false\n";return 1;}
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
#endif
