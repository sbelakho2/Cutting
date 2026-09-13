#include "feed_geometry.hpp"
#include "feed_fem.hpp"
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
double numeric(const std::string& s){std::size_t used=0;const double x=std::stod(s,&used);require(used==s.size()&&std::isfinite(x),"Invalid peak coordinate");return x;}
const char* surfaceName(GeomAbs_SurfaceType type) {
    switch(type) {
    case GeomAbs_Plane:return "plane";case GeomAbs_Cylinder:return "cylinder";
    case GeomAbs_Cone:return "cone";case GeomAbs_Sphere:return "sphere";
    case GeomAbs_Torus:return "torus";case GeomAbs_BSplineSurface:return "bspline";
    case GeomAbs_BezierSurface:return "bezier";case GeomAbs_SurfaceOfRevolution:return "revolution";
    case GeomAbs_SurfaceOfExtrusion:return "extrusion";case GeomAbs_OffsetSurface:return "offset";
    default:return "other";
    }
}
void close(std::ofstream& f){f.close();require(bool(f),"Geometry audit write failed");}
}
int main(int argc,char** argv)try {
    require(argc>=3,"Use OUTPUT_DIRECTORY REGIONAL_CSV...");const fs::path out=argv[1];
    for(int arg=2;arg<argc;++arg)for(const char* name:{"status.json","faces.csv","peak-geometry.csv"}) {
        const fs::path source=argv[arg],destination=out/name;
        require(fs::weakly_canonical(source)!=fs::weakly_canonical(destination),"Audit input aliases an output");
        if(fs::exists(source)&&fs::exists(destination))require(!fs::equivalent(source,destination),"Audit input hard-links an output");
    }
    fs::create_directories(out);
    {std::ofstream f(out/"status.json");f<<"{\"run_complete\":false,\"fabrication_released\":false}\n";close(f);}
    const auto shape=se::feedShaftShape();TopTools_IndexedMapOfShape faces;TopExp::MapShapes(shape,TopAbs_FACE,faces);
    std::ofstream table(out/"faces.csv");table<<std::setprecision(14)
        <<"face,type,area_mm2,xmin,Smin,zmin,xmax,Smax,zmax,umin,umax,vmin,vmax,analytic_axis_defined,origin_x,origin_S,origin_z,axis_x,axis_S,axis_z,major_radius_mm,minor_radius_mm\n";
    for(int i=1;i<=faces.Extent();++i) {
        const auto face=TopoDS::Face(faces(i));const BRepAdaptor_Surface surface(face);const auto b=se::bounds(face);
        GProp_GProps props;BRepGProp::SurfaceProperties(face,props);gp_Pnt origin;gp_Dir axis(0,1,0);double major=0,minor=0;
        if(surface.GetType()==GeomAbs_Torus){const auto t=surface.Torus();origin=t.Location();axis=t.Axis().Direction();major=t.MajorRadius();minor=t.MinorRadius();}
        if(surface.GetType()==GeomAbs_Cylinder){const auto t=surface.Cylinder();origin=t.Location();axis=t.Axis().Direction();major=t.Radius();}
        table<<i<<','<<surfaceName(surface.GetType())<<','<<props.Mass()<<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<','
             <<surface.FirstUParameter()<<','<<surface.LastUParameter()<<','<<surface.FirstVParameter()<<','<<surface.LastVParameter()<<','
             <<(surface.GetType()==GeomAbs_Torus||surface.GetType()==GeomAbs_Cylinder)<<','
             <<origin.X()<<','<<origin.Y()<<','<<origin.Z()<<','<<axis.X()<<','<<axis.Y()<<','<<axis.Z()<<','<<major<<','<<minor<<'\n';
    }
    close(table);
    std::ofstream peaks(out/"peak-geometry.csv");peaks<<std::setprecision(14)
        <<"source,bulk_mm,root_mm,load_class,region,x_mm,S_mm,z_mm,raw_MPa,CAD_state,nearest_face,nearest_type,distance_mm,nearest_torus,torus_distance_mm,all_root_guide_mm\n";
    std::size_t rows=0;const auto regions=se::feedShaftStressRegions();
    const std::string header="bulk_mm,root_mm,nodes,tets,load_class,region,sampled_vm_MPa,continuous_phase_upper_MPa,peak_x_mm,peak_S_mm,peak_z_mm,phase_deg,sign_index,torque_vertex";
    for(int arg=2;arg<argc;++arg) {
        const fs::path input=argv[arg];std::ifstream csv(input);require(bool(csv),"Regional CSV cannot be opened");std::string line;std::getline(csv,line);
        if(!line.empty()&&line.back()=='\r')line.pop_back();require(line==header,"Unexpected regional CSV header");const auto initialRows=rows;
        while(std::getline(csv,line)) {
            if(!line.empty()&&line.back()=='\r')line.pop_back();require(!line.empty()&&line.back()!=',',"Empty regional CSV field");
            std::stringstream record(line);std::vector<std::string> fields;std::string field;
            while(std::getline(record,field,','))fields.push_back(field);require(fields.size()==14,"Expected14 regional CSV columns");
            std::array<double,14> values{};for(std::size_t j=0;j<14;++j)if(j!=4&&j!=5)values[j]=numeric(fields[j]);
            require(values[0]>0&&values[1]>0&&values[1]<=values[0],"Invalid regional mesh sizes");
            require(values[2]>=4&&values[3]>=1&&values[2]==std::floor(values[2])&&values[3]==std::floor(values[3]),"Invalid regional mesh counts");
            require((fields[4]=="working"||fields[4]=="fault")&&std::any_of(regions.begin(),regions.end(),[&](const auto& r){return r.name==fields[5];}),"Unknown regional load or region");
            require(values[6]>0&&values[7]>=values[6]&&values[11]>=0&&values[11]<360,"Invalid regional stresses or phase");
            require(values[12]>=0&&values[12]<=7&&values[12]==std::floor(values[12])&&values[13]>=0&&values[13]<=5&&values[13]==std::floor(values[13]),"Invalid regional load-case indices");
            const Eigen::Vector3d p{numeric(fields[8]),numeric(fields[9]),numeric(fields[10])};
            const auto vertex=BRepBuilderAPI_MakeVertex(gp_Pnt(p.x(),p.y(),p.z())).Shape();
            int nearest=0,nearestTorus=0;double distance=std::numeric_limits<double>::infinity(),torusDistance=distance;
            for(int i=1;i<=faces.Extent();++i) {
                BRepExtrema_DistShapeShape projection(vertex,faces(i));projection.Perform();require(projection.IsDone()&&projection.NbSolution()>0,"CAD peak projection failed");
                const double value=projection.Value();require(std::isfinite(value)&&value>=0,"Invalid CAD peak distance");
                if(value<distance){distance=value;nearest=i;}
                if(BRepAdaptor_Surface(TopoDS::Face(faces(i))).GetType()==GeomAbs_Torus&&value<torusDistance){torusDistance=value;nearestTorus=i;}
            }
            BRepClass3d_SolidClassifier classifier(shape,gp_Pnt(p.x(),p.y(),p.z()),1e-7);
            const auto state=classifier.State();const char* stateName=state==TopAbs_IN?"inside":state==TopAbs_ON?"on":state==TopAbs_OUT?"outside":"unknown";
            peaks<<input.parent_path().filename().string()<<','<<fields[0]<<','<<fields[1]<<','<<fields[4]<<','<<fields[5]<<','<<p.x()<<','<<p.y()<<','<<p.z()<<','
                 <<fields[6]<<','<<stateName<<','<<nearest<<','<<surfaceName(BRepAdaptor_Surface(TopoDS::Face(faces(nearest))).GetType())<<','<<distance<<','
                 <<nearestTorus<<','<<torusDistance<<','<<se::feedShaftRootDistance(p)<<'\n';++rows;
        }
        require(csv.eof()&&rows>initialRows,"Regional CSV read failed or contains no peaks");
    }
    close(peaks);std::ofstream status(out/"status.json");status<<"{\"run_complete\":true,\"faces\":"<<faces.Extent()<<",\"peak_rows\":"<<rows
        <<",\"stress_convergence_verified\":false,\"fabrication_released\":false}\n";close(status);
    std::cout<<"Audited "<<faces.Extent()<<" original CAD faces and "<<rows<<" peak locations\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}