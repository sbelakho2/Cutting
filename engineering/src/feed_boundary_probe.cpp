#include "feed_geometry.hpp"
#include "feed_fem.hpp"
#include "feed_refinement.hpp"
#include "head_geometry.hpp"
#include <BRepClass3d_SolidClassifier.hxx>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
double number(const char* value) {
    std::size_t used=0;const std::string text=value;const double n=std::stod(text,&used);
    require(used==text.size()&&std::isfinite(n)&&n>0,"Expected positive finite size");return n;
}
void close(std::ofstream& stream){stream.close();require(bool(stream),"Boundary probe write failed");}
}
int main(int argc,char** argv)try {
    bool allCentroids=false;std::string focusList="A12_S19_62";
    for(int i=5;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--all-centroids")allCentroids=true;
        else if(arg=="--focus"&&i+1<argc){focusList=argv[++i];require(!focusList.empty(),"Nonempty focus feature list required");}
        else throw std::runtime_error("Use OUTPUT_DIRECTORY baseline|netgen BULK_MM ROOT_MM [--all-centroids] [--focus FEATURE[,FEATURE...]]");
    }
    require(argc>=5,"Use OUTPUT_DIRECTORY baseline|netgen BULK_MM ROOT_MM [--all-centroids] [--focus FEATURE[,FEATURE...]]");
    const fs::path out=argv[1];const std::string method=argv[2];
    require(method=="baseline"||method=="netgen","Unknown boundary probe method");
    const double bulk=number(argv[3]),root=number(argv[4]);require(root>=0.005&&root<=0.2&&bulk>=0.2&&bulk<=5,"Invalid probe mesh limits");
    fs::create_directories(out);
    {std::ofstream state(out/"status.json");state<<"{\"run_complete\":false,\"fabrication_released\":false}\n";close(state);}
    const auto shaft=se::feedShaftShape();std::vector<se::ShaftFeatureGuide> guides;
    for(std::size_t begin=0;begin<=focusList.size();) {
        const auto end=std::min(focusList.find(',',begin),focusList.size());
        const auto name=focusList.substr(begin,end-begin);require(!name.empty(),"Empty focus feature name");
        guides.push_back(se::feedShaftFeatureGuide(shaft,name));begin=end+1;
    }
    se::writeStep(shaft,out/"F101-R6-analysis.step");
    const auto mesh=se::meshStepRefined(out/"F101-R6-analysis.step",bulk,root,[&](const Eigen::Vector3d& p) {
        double focus=bulk;for(const auto& guide:guides)focus=std::min(focus,root+0.7*guide.distance_mm(p));
        return std::min({bulk,0.2+0.45*se::feedShaftRootDistance(p),focus});
    },method=="netgen"?se::RefinedMeshOptimizer::netgen:se::RefinedMeshOptimizer::gmsh);
    require(mesh.geometry_audit.has_value(),"Boundary probe geometry audit missing");const auto& audit=*mesh.geometry_audit;
    require(std::abs(audit.cad_volume_mm3/se::volume(shaft)-1)<1e-9,"STEP changed nominal shaft volume");
    BRepClass3d_SolidClassifier classifier(shaft);
    std::ofstream table(out/"same-surface-cells.csv");table<<std::setprecision(14)
        <<"element_zero_based,Gmsh_surface,centroid_x_mm,centroid_S_mm,centroid_z_mm,CAD_state\n";
    const auto regions=se::feedShaftStressRegions();
    const auto a12Region=std::find_if(regions.begin(),regions.end(),[](const auto& r){return r.name=="A12_groove";});
    require(a12Region!=regions.end(),"A12 reporting region missing");
    std::size_t outside=0,a12Bridges=0,a12Outside=0;
    for(const auto& t:audit.same_surface_tetrahedra) {
        Eigen::Vector3d centroid=Eigen::Vector3d::Zero();
        for(int n:mesh.tetrahedra.at(t.element))centroid+=mesh.nodes_mm.at(static_cast<std::size_t>(n))/4;
        classifier.Perform(gp_Pnt(centroid.x(),centroid.y(),centroid.z()),1e-7);
        const auto state=classifier.State();require(state==TopAbs_IN||state==TopAbs_ON||state==TopAbs_OUT,"Unknown CAD centroid classification");
        const bool exterior=state==TopAbs_OUT,a12=a12Region->contains(centroid);
        if(exterior)++outside;if(a12)++a12Bridges;if(exterior&&a12)++a12Outside;
        table<<t.element<<','<<t.surface<<','<<centroid.x()<<','<<centroid.y()<<','<<centroid.z()<<','
             <<(exterior?"outside":state==TopAbs_IN?"inside":"on")<<'\n';
    }
    close(table);const double volumeError=std::abs(audit.element_volume_mm3/se::volume(shaft)-1);
    require(volumeError<0.005,"Probe exceeds unchanged mesh-volume criterion");
    std::size_t allOutside=0,onBoundary=0;
    if(allCentroids) {
        std::ofstream exterior(out/"outside-cell-centroids.csv");exterior<<std::setprecision(14)<<"element_zero_based,x_mm,S_mm,z_mm\n";
        for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
            Eigen::Vector3d p=Eigen::Vector3d::Zero();
            for(int n:mesh.tetrahedra[e])p+=mesh.nodes_mm.at(static_cast<std::size_t>(n))/4;
            classifier.Perform(gp_Pnt(p.x(),p.y(),p.z()),1e-7);const auto state=classifier.State();
            require(state==TopAbs_IN||state==TopAbs_ON||state==TopAbs_OUT,"Unknown full-mesh centroid classification");
            if(state==TopAbs_ON)++onBoundary;
            if(state==TopAbs_OUT){++allOutside;exterior<<e<<','<<p.x()<<','<<p.y()<<','<<p.z()<<'\n';}
        }
        close(exterior);
    }
    std::ofstream state(out/"status.json");state<<std::setprecision(14)<<"{\"run_complete\":true,\"method\":\""<<method
        <<"\",\"focus_features\":\""<<focusList<<"\",\"bulk_mm\":"<<bulk<<",\"root_mm\":"<<root<<",\"background_root_mm\":0.2,\"focus_growth\":0.7,\"node_limit\":150000"
        <<",\"nodes\":"<<mesh.nodes_mm.size()<<",\"tets\":"<<mesh.tetrahedra.size()<<",\"min_SICN\":"<<audit.min_sicn
        <<",\"volume_relative_error\":"<<volumeError<<",\"same_surface_tets\":"<<audit.same_surface_tetrahedra.size()<<",\"exterior_centroid_tets\":"<<outside
        <<",\"A12_same_surface_tets\":"<<a12Bridges<<",\"A12_exterior_centroid_tets\":"<<a12Outside
        <<",\"all_cell_centroids_checked\":"<<(allCentroids?"true":"false")<<",\"classifier_tolerance_mm\":1e-7"
        <<",\"all_exterior_centroids\":"<<(allCentroids?std::to_string(allOutside):"null")
        <<",\"all_on_boundary_centroids\":"<<(allCentroids?std::to_string(onBoundary):"null")
        <<",\"continuous_containment_verified\":false,\"stress_verified\":false,\"fabrication_released\":false}\n";close(state);
    std::cout<<method<<": nodes="<<mesh.nodes_mm.size()<<", tets="<<mesh.tetrahedra.size()<<", common-surface cells="<<audit.same_surface_tetrahedra.size()
        <<", exterior centroids="<<outside<<", A12="<<a12Bridges<<"/"<<a12Outside<<", minSICN="<<audit.min_sicn<<'\n';
    if(allCentroids)std::cout<<"All "<<mesh.tetrahedra.size()<<" centroids: outside="<<allOutside<<", on="<<onBoundary<<" (CAD tolerance1e-7 mm)\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}