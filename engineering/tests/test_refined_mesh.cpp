#include "fem.hpp"
#include "head_geometry.hpp"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Eigen/Geometry>
#include <gp_Ax2.hxx>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace fs=std::filesystem;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
}
int main(int argc,char** argv)try {
    require(argc==2,"Mesh audit test requires an output directory");const fs::path out=argv[1];fs::create_directories(out);
    for(bool curved:{false,true})for(auto optimizer:{se::RefinedMeshOptimizer::gmsh,se::RefinedMeshOptimizer::netgen}) {
        const TopoDS_Shape shape=curved?BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0,1,0)),3,8).Shape():BRepPrimAPI_MakeBox(4,6,8).Shape();
        const double exact=curved?72*std::numbers::pi:192;const auto path=out/(curved?"cylinder.step":"box.step");se::writeStep(shape,path);
        const auto sizing=[](const Eigen::Vector3d& p){return 0.2+0.025*std::abs(p.y());};
        const auto mesh=se::meshStepRefined(path,0.4,0.2,sizing,optimizer);
        require(mesh.geometry_audit.has_value(),"Missing refined mesh audit");const auto& audit=*mesh.geometry_audit;
        require(std::abs(audit.cad_volume_mm3/exact-1)<1e-12,"Imported analytic solid volume changed");
        require(audit.min_sicn>0&&audit.min_sicn<=1,"Invalid signed element quality");
        require(std::abs(audit.element_volume_mm3/exact-1)<(curved?0.005:1e-10),"Analytic mesh volume mismatch");
        long double independentVolume=0;
        for(const auto& t:mesh.tetrahedra) {
            const auto& p=mesh.nodes_mm[static_cast<std::size_t>(t[0])];
            const Eigen::Vector3d a=mesh.nodes_mm[static_cast<std::size_t>(t[1])]-p,b=mesh.nodes_mm[static_cast<std::size_t>(t[2])]-p,c=mesh.nodes_mm[static_cast<std::size_t>(t[3])]-p;
            const double v=std::abs(a.dot(b.cross(c)))/6;require(std::isfinite(v)&&v>0,"Invalid independent tetrahedron volume");independentVolume+=v;
        }
        require(std::abs(static_cast<double>(independentVolume)/audit.element_volume_mm3-1)<1e-11,"Independent tetrahedral volume audit disagrees");
        require(audit.surfaces.size()==(curved?3U:6U),"Unexpected analytic face count");bool sawCurve=false;
        for(const auto& face:audit.surfaces) {
            require(face.nodes>0&&face.triangles>0&&face.max_node_distance_mm<1e-8,"Surface nodes do not match analytic CAD");
            if(face.type=="Cylinder"){sawCurve=true;require(face.max_facet_centroid_distance_mm>0&&face.max_facet_centroid_distance_mm<0.05,"Curved facet diagnostic missing/invalid");}
            else require(face.max_facet_centroid_distance_mm<1e-8,"Planar facets do not match plane");
        }
        require(sawCurve==curved,"Curved face classification mismatch");
        if(optimizer==se::RefinedMeshOptimizer::netgen)
            require(audit.same_surface_tetrahedra.empty(),"Optimized analytic benchmark contains a same-surface tetrahedron");
        bool rejected=false;
        try{(void)se::meshStepRefined(path,0.4,0.2,sizing,static_cast<se::RefinedMeshOptimizer>(37));}
        catch(const std::runtime_error&){rejected=true;}
        require(rejected,"Invalid optimizer enum was not rejected");
    }
    std::cout<<"Refined planar/curved STEP, both optimizers: quality, original surfaces, topology and independent volume audits passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}