#include "head_geometry.hpp"
#include <STEPControl_Reader.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace se=starz::engineering;
int main(int argc,char** argv) try {
    if(argc<3) throw std::invalid_argument("Output directory and original supplier STEP paths required");
    const fs::path out=argv[1]; fs::create_directories(out);
    std::ofstream table(out/"vendor-geometry.csv"),cylinders(out/"vendor-cylinders.csv"),planes(out/"vendor-planes.csv");
    table<<"file,solids,valid,xmin,ymin,zmin,xmax,ymax,zmax,centroid_x,centroid_y,centroid_z,solid_volume_mm3\n";
    cylinders<<"file,radius_mm,axis_x,axis_y,axis_z,origin_x,origin_y,origin_z,vmin_mm,vmax_mm,umin_rad,umax_rad\n";
    planes<<"file,normal_x,normal_y,normal_z,origin_x,origin_y,origin_z,xmin,ymin,zmin,xmax,ymax,zmax\n";
    table<<std::setprecision(12); cylinders<<std::setprecision(12); planes<<std::setprecision(12);
    for(int n=2;n<argc;++n) {
        const fs::path path=argv[n]; STEPControl_Reader reader;
        if(reader.ReadFile(path.c_str())!=IFSelect_RetDone||reader.TransferRoots()<1) throw std::runtime_error("Supplier STEP import failed: "+path.string());
        const auto shape=reader.OneShape(); const auto b=se::bounds(shape);
        GProp_GProps properties; BRepGProp::VolumeProperties(shape,properties); const auto cg=properties.CentreOfMass();
        int solids=0; for(TopExp_Explorer e(shape,TopAbs_SOLID);e.More();e.Next())++solids;
        const bool valid=BRepCheck_Analyzer(shape).IsValid();
        table<<path.filename().string()<<','<<solids<<','<<(valid?"true":"false")<<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','
             <<b.xmax<<','<<b.ymax<<','<<b.zmax<<','<<cg.X()<<','<<cg.Y()<<','<<cg.Z()<<','<<properties.Mass()<<'\n';
           int index=0;
           for(TopExp_Explorer e(shape,TopAbs_SOLID);e.More();e.Next(),++index) {
              const auto sb=se::bounds(e.Current()); GProp_GProps sp; BRepGProp::VolumeProperties(e.Current(),sp); const auto centre=sp.CentreOfMass();
              table<<path.filename().string()<<"#"<<index<<",1,"<<(BRepCheck_Analyzer(e.Current()).IsValid()?"true":"false")<<','
                  <<sb.xmin<<','<<sb.ymin<<','<<sb.zmin<<','<<sb.xmax<<','<<sb.ymax<<','<<sb.zmax<<','
                  <<centre.X()<<','<<centre.Y()<<','<<centre.Z()<<','<<sp.Mass()<<'\n';
           }
        for(TopExp_Explorer e(shape,TopAbs_FACE);e.More();e.Next()) {
            const auto face=TopoDS::Face(e.Current()); BRepAdaptor_Surface surface(face,true);
            if(surface.GetType()==GeomAbs_Plane) {
                const auto axis=surface.Plane().Axis(); const auto p=axis.Location(); const auto direction=axis.Direction();
                const auto extent=se::bounds(face);
                planes<<path.filename().string()<<','<<direction.X()<<','<<direction.Y()<<','<<direction.Z()<<','
                      <<p.X()<<','<<p.Y()<<','<<p.Z()<<','<<extent.xmin<<','<<extent.ymin<<','<<extent.zmin<<','
                      <<extent.xmax<<','<<extent.ymax<<','<<extent.zmax<<'\n';
            }
            if(surface.GetType()!=GeomAbs_Cylinder)continue;
            const auto cylinder=surface.Cylinder(); const auto axis=cylinder.Axis(); const auto p=axis.Location(); const auto d=axis.Direction();
            double umin,umax,vmin,vmax; BRepTools::UVBounds(face,umin,umax,vmin,vmax);
            cylinders<<path.filename().string()<<','<<cylinder.Radius()<<','<<d.X()<<','<<d.Y()<<','<<d.Z()<<','<<p.X()<<','<<p.Y()<<','<<p.Z()
                     <<','<<vmin<<','<<vmax<<','<<umin<<','<<umax<<'\n';
        }
        std::cout<<path.filename()<<": "<<solids<<" solids, "<<(valid?"valid":"INVALID")<<", bounds "
                 <<b.xmax-b.xmin<<" × "<<b.ymax-b.ymin<<" × "<<b.zmax-b.zmin<<" mm\n";
        if(!valid||solids==0)throw std::runtime_error("Invalid original supplier solid: "+path.string());
    }
    if(!table||!cylinders||!planes)throw std::runtime_error("Vendor inspection output failed");
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}