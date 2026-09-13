#include "head_preview.hpp"
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace starz::engineering {
namespace {
struct Triangle { std::array<gp_Pnt,3> p; std::array<int,3> colour; double depth; };
std::array<double,3> project(const gp_Pnt& p) {
    return {0.8660254*p.X()+0.5*p.Y(),0.25*p.X()-0.4330127*p.Y()-0.8660254*p.Z(),
            0.4330127*p.X()-0.75*p.Y()+0.5*p.Z()};
}
}
void writeHeadPreview(const std::vector<HeadPart>& parts,const std::filesystem::path& path) {
    std::vector<Triangle> triangles;
    double minX=1e9,minY=1e9,maxX=-1e9,maxY=-1e9;
    for(const auto& part:parts) {
        BRepMesh_IncrementalMesh triangulate(part.shape,0.15,false,0.25,true);
        if(!triangulate.IsDone()) throw std::runtime_error("Preview triangulation failed");
        std::array<int,3> colour{137,150,161};
        if(part.id.starts_with("DR")) colour={28,34,42};
        else if(part.id.starts_with("AX")) colour={81,90,100};
        else if(part.id.starts_with("H004")) colour={220,28,48};
        else if(part.id.starts_with("T001")) colour={211,222,232};
        else if(part.id.starts_with("CP")) colour={236,173,66};
        for(TopExp_Explorer e(part.shape,TopAbs_FACE);e.More();e.Next()) {
            TopLoc_Location location;
            const auto face=TopoDS::Face(e.Current()); const auto t=BRep_Tool::Triangulation(face,location);
            if(t.IsNull()) continue;
            for(int i=1;i<=t->NbTriangles();++i) {
                int a,b,c; t->Triangle(i).Get(a,b,c);
                if(face.Orientation()==TopAbs_REVERSED) std::swap(b,c);
                Triangle tri{{t->Node(a).Transformed(location.Transformation()),t->Node(b).Transformed(location.Transformation()),t->Node(c).Transformed(location.Transformation())},colour,0};
                const gp_Vec u(tri.p[0],tri.p[1]),v(tri.p[0],tri.p[2]); gp_Vec normal=u.Crossed(v);
                if(normal.Magnitude()<1e-10) continue; normal.Normalize();
                const double brightness=0.55+0.45*std::abs(normal.Dot(gp_Vec(0.30,-0.35,0.886)));
                for(int& component:tri.colour) component=static_cast<int>(component*brightness);
                for(const auto& point:tri.p) {
                    const auto p=project(point); minX=std::min(minX,p[0]); maxX=std::max(maxX,p[0]);
                    minY=std::min(minY,p[1]); maxY=std::max(maxY,p[1]); tri.depth+=p[2]/3;
                }
                triangles.push_back(tri);
            }
        }
    }
    if(triangles.empty()) throw std::runtime_error("Empty preview");
    std::sort(triangles.begin(),triangles.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    const double scale=std::min(1050/(maxX-minX),620/(maxY-minY));
    const double x0=600-(maxX+minX)*scale/2,y0=465-(maxY+minY)*scale/2;
    std::ofstream f(path); f<<std::fixed<<std::setprecision(3);
    f<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100%\" height=\"100%\" viewBox=\"0 0 1200 900\" style=\"background:#F3F5F7\">\n"
      <<"<title>STARZ S1-B selected cutting head</title><desc>Actual catalogue envelopes and custom load path. Not a complete machine or fabrication release.</desc>\n"
      <<"<rect width=\"1200\" height=\"900\" fill=\"#F3F5F7\"/><rect width=\"1200\" height=\"115\" fill=\"#0B0F14\"/>"
      <<"<image href=\"../../../branding/starz-mark.svg\" x=\"30\" y=\"20\" width=\"190\" height=\"65\"/>"
      <<"<text x=\"260\" y=\"52\" fill=\"white\" font-family=\"Arial,sans-serif\" font-size=\"30\" font-weight=\"bold\">S1-B / PARTS-FIRST CUTTING HEAD</text>"
      <<"<text x=\"262\" y=\"82\" fill=\"#AAB5C2\" font-family=\"Arial,sans-serif\" font-size=\"16\">HIWIN KK86 · 34E1K-85 · R+W MK2 · C++ / OpenCASCADE</text>"
      <<"<rect y=\"111\" width=\"1200\" height=\"4\" fill=\"#E10600\"/>\n";
    for(const auto& t:triangles) {
        f<<"<polygon points=\"";
        for(const auto& p:t.p) { const auto xy=project(p); f<<x0+xy[0]*scale<<','<<y0+xy[1]*scale<<' '; }
        f<<"\" fill=\"rgb("<<t.colour[0]<<','<<t.colour[1]<<','<<t.colour[2]<<")\"/>\n";
    }
    f<<"<rect x=\"30\" y=\"802\" width=\"1140\" height=\"72\" rx=\"8\" fill=\"#0B0F14\"/>"
      <<"<text x=\"52\" y=\"830\" fill=\"white\" font-family=\"Arial,sans-serif\" font-size=\"17\">SELECTED HEAD ONLY — CAD interfaces checked; purchased internals not modelled.</text>"
      <<"<text x=\"52\" y=\"855\" fill=\"#FF7B86\" font-family=\"Arial,sans-serif\" font-size=\"16\">NOT FOR FABRICATION / ENERGISATION. Tooling, complete machine integration and qualification remain open.</text></svg>\n";
    if(!f) throw std::runtime_error("Failed to write head preview");
}
}