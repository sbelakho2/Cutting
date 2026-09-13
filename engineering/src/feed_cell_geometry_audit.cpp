#include "cad_boundary.hpp"
#include "feed_geometry.hpp"
#include "mesh_io.hpp"
#include <BRepAdaptor_Surface.hxx>
#include <Eigen/Geometry>
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
void close(std::ofstream& f){f.close();require(bool(f),"Cell geometry audit write failed");}
template<class T> T number(const std::string& text) {
    T value{};const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
    require(result.ec==std::errc{}&&result.ptr==text.data()+text.size(),"Invalid cell-list number");return value;
}
std::array<int,3> face(const std::array<int,4>& cell,int omit) {
    std::array<int,3> key{};int n=0;for(int i=0;i<4;++i)if(i!=omit)key[static_cast<std::size_t>(n++)]=cell[static_cast<std::size_t>(i)];
    std::sort(key.begin(),key.end());return key;
}
struct Selected {std::size_t element;Eigen::Vector3d centroid;};
}
int main(int argc,char** argv)try {
    require(argc==4,"Use OUTPUT_DIRECTORY LINEAR_VTU EXTERIOR_CENTROIDS_CSV");const fs::path out=argv[1],vtu=argv[2],csv=argv[3];
    for(const auto& input:{vtu,csv})for(const char* name:{"status.json","cells.csv","points.csv","faces.csv"}) {
        const auto destination=out/name;require(fs::weakly_canonical(input)!=fs::weakly_canonical(destination),"Cell audit input aliases an output");
        if(fs::exists(input)&&fs::exists(destination))require(!fs::equivalent(input,destination),"Cell audit input hard-links an output");
    }
    fs::create_directories(out);{std::ofstream f(out/"status.json");f<<"{\"run_complete\":false,\"fabrication_released\":false}\n";close(f);}
    const auto mesh=se::readLinearVtuGeometry(vtu);std::ifstream input(csv);require(bool(input),"Exterior-centroid input cannot be opened");
    std::string line;std::getline(input,line);if(!line.empty()&&line.back()=='\r')line.pop_back();
    require(line=="element_zero_based,x_mm,S_mm,z_mm","Unexpected exterior-centroid header");std::vector<Selected> selected;std::set<std::size_t> unique;
    while(std::getline(input,line)) {
        if(!line.empty()&&line.back()=='\r')line.pop_back();require(!line.empty()&&line.back()!=',',"Empty exterior-centroid row/field");
        std::istringstream row(line);std::string token;std::vector<std::string> fields;
        while(std::getline(row,token,','))fields.push_back(token);require(fields.size()==4,"Expected four exterior-centroid columns");
        const auto id=number<std::size_t>(fields[0]);require(id<mesh.tetrahedra.size()&&unique.insert(id).second,"Duplicate/out-of-range exterior cell");
        const Eigen::Vector3d expected{number<double>(fields[1]),number<double>(fields[2]),number<double>(fields[3])};require(expected.allFinite(),"Non-finite exterior centroid");
        Eigen::Vector3d centroid=Eigen::Vector3d::Zero();for(int n:mesh.tetrahedra[id])centroid+=mesh.nodes_mm.at(static_cast<std::size_t>(n))/4;
        require((centroid-expected).cwiseAbs().maxCoeff()<1e-7,"Cell list and published VTU centroid disagree");selected.push_back({id,centroid});
    }
    require(input.eof()&&!selected.empty()&&selected.size()<=10000,"Empty/unreadable/oversized exterior-cell list");
    std::map<std::array<int,3>,int> incidence;
    for(const auto& cell:selected)for(int omit=0;omit<4;++omit)incidence.emplace(face(mesh.tetrahedra[cell.element],omit),0);
    for(const auto& cell:mesh.tetrahedra)for(int omit=0;omit<4;++omit) {
        const auto found=incidence.find(face(cell,omit));if(found!=incidence.end())++found->second;
    }
    for(const auto& [key,n]:incidence){(void)key;require(n==1||n==2,"Selected cell has a non-manifold face");}
    const auto shaft=se::feedShaftShape();se::CadBoundaryInspector inspector(shaft);std::set<int> seenFaces;
    std::ofstream cells(out/"cells.csv"),points(out/"points.csv");cells<<std::setprecision(14);points<<std::setprecision(14);
    cells<<"element_zero_based,node0,node1,node2,node3,volume_mm3,boundary_faces,centroid_x_mm,centroid_S_mm,centroid_z_mm,nearest_face,nearest_type,centroid_gap_mm,vertex_outside,edge_midpoint_outside,face_centroid_outside,gauss_point_outside\n";
    points<<"element_zero_based,kind,local_index,mesh_node,mesh_boundary_face,x_mm,S_mm,z_mm,CAD_state,nearest_face,nearest_type,distance_mm,projection_x_mm,projection_S_mm,projection_z_mm,support_dimension,curvature_defined,min_curvature_per_mm,max_curvature_per_mm,gaussian_per_mm2\n";
    std::size_t pointRows=0;double maximumGap=0;
    const auto record=[&](std::size_t element,const char* kind,int local,int node,int boundary,const Eigen::Vector3d& p) {
        const auto result=inspector.inspect(p);seenFaces.insert(result.face);++pointRows;
        points<<element<<','<<kind<<','<<local<<','<<node<<','<<boundary<<','<<p.x()<<','<<p.y()<<','<<p.z()<<','<<se::cadStateName(result.state)
            <<','<<result.face<<','<<se::cadSurfaceName(result.type)<<','<<result.distance_mm<<','<<result.projected_mm.x()<<','<<result.projected_mm.y()<<','<<result.projected_mm.z()
            <<','<<result.support_dimension<<','<<result.curvature.has_value()<<',';
        if(result.curvature)points<<result.curvature->minimum_per_mm<<','<<result.curvature->maximum_per_mm<<','<<result.curvature->gaussian_per_mm2;
        else points<<",,";points<<'\n';return result;
    };
    for(const auto& item:selected) {
        const auto& ids=mesh.tetrahedra[item.element];std::array<Eigen::Vector3d,4> p;
        for(int i=0;i<4;++i)p[static_cast<std::size_t>(i)]=mesh.nodes_mm[static_cast<std::size_t>(ids[static_cast<std::size_t>(i)])];
        const double volume=std::abs((p[1]-p[0]).dot((p[2]-p[0]).cross(p[3]-p[0])))/6;
        require(std::isfinite(volume)&&volume>1e-11,"Degenerate selected tetrahedron");
        const auto centroid=record(item.element,"centroid",0,-1,-1,item.centroid);
        require(centroid.state==TopAbs_OUT,"Cell list does not reproduce exterior CAD centroids");maximumGap=std::max(maximumGap,centroid.distance_mm);
        int vertexOutside=0,edgeOutside=0,faceOutside=0,gaussOutside=0,boundaryFaces=0,index=0;
        for(int i=0;i<4;++i)vertexOutside+=record(item.element,"vertex",i,ids[static_cast<std::size_t>(i)],-1,p[static_cast<std::size_t>(i)]).state==TopAbs_OUT;
        for(int i=0;i<4;++i)for(int j=i+1;j<4;++j)edgeOutside+=record(item.element,"edge_midpoint",index++,-1,-1,(p[static_cast<std::size_t>(i)]+p[static_cast<std::size_t>(j)])/2).state==TopAbs_OUT;
        for(int omit=0;omit<4;++omit) {
            const bool boundary=incidence.at(face(ids,omit))==1;boundaryFaces+=boundary;
            faceOutside+=record(item.element,"face_centroid",omit,-1,boundary,(4*item.centroid-p[static_cast<std::size_t>(omit)])/3).state==TopAbs_OUT;
        }
        // Symmetric four-point degree-two tetrahedral integration locations;
        // these are diagnostic samples, NOT a new element formulation.
        const double a=(5+3*std::sqrt(5.))/20,b=(5-std::sqrt(5.))/20;
        for(int i=0;i<4;++i)gaussOutside+=record(item.element,"gauss_degree2",i,-1,-1,4*b*item.centroid+(a-b)*p[static_cast<std::size_t>(i)]).state==TopAbs_OUT;
        cells<<item.element;for(int n:ids)cells<<','<<n;
        cells<<','<<volume<<','<<boundaryFaces<<','<<item.centroid.x()<<','<<item.centroid.y()<<','<<item.centroid.z()<<','<<centroid.face<<','<<se::cadSurfaceName(centroid.type)
            <<','<<centroid.distance_mm<<','<<vertexOutside<<','<<edgeOutside<<','<<faceOutside<<','<<gaussOutside<<'\n';
    }
    close(cells);close(points);
    std::ofstream faces(out/"faces.csv");faces<<std::setprecision(14)<<"face,type,xmin,Smin,zmin,xmax,Smax,zmax,analytic_axis_defined,origin_x_mm,origin_S_mm,origin_z_mm,axis_x,axis_S,axis_z,major_radius_mm,minor_radius_mm\n";
    for(int id:seenFaces) {
        const auto& faceShape=inspector.face(static_cast<std::size_t>(id));const auto b=se::bounds(faceShape);const BRepAdaptor_Surface surface(faceShape);gp_Pnt origin;gp_Dir axis(0,1,0);double major=0,minor=0;
        if(surface.GetType()==GeomAbs_Torus){const auto t=surface.Torus();origin=t.Location();axis=t.Axis().Direction();major=t.MajorRadius();minor=t.MinorRadius();}
        if(surface.GetType()==GeomAbs_Cylinder){const auto c=surface.Cylinder();origin=c.Location();axis=c.Axis().Direction();major=c.Radius();}
        faces<<id<<','<<se::cadSurfaceName(surface.GetType())<<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<','<<(surface.GetType()==GeomAbs_Torus||surface.GetType()==GeomAbs_Cylinder)
            <<','<<origin.X()<<','<<origin.Y()<<','<<origin.Z()<<','<<axis.X()<<','<<axis.Y()<<','<<axis.Z()<<','<<major<<','<<minor<<'\n';
    }
    close(faces);require(pointRows==19*selected.size(),"Cell audit sample count mismatch");
    std::ofstream status(out/"status.json");status<<std::setprecision(14)<<"{\"run_complete\":true,\"nodes\":"<<mesh.nodes_mm.size()<<",\"tets\":"<<mesh.tetrahedra.size()
        <<",\"CAD_faces\":"<<inspector.faceCount()<<",\"selected_cells\":"<<selected.size()<<",\"point_samples\":"<<pointRows<<",\"maximum_exterior_centroid_gap_mm\":"<<maximumGap
        <<",\"classifier_tolerance_mm\":1e-7,\"continuous_containment_verified\":false,\"stress_verified\":false,\"fabrication_released\":false}\n";close(status);
    std::cout<<"Audited "<<selected.size()<<" retained exterior cells, "<<pointRows<<" samples and exact trimmed CAD faces; maximum centroid gap="<<maximumGap<<" mm\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}