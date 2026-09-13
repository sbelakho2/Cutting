#include "shaft_sections.hpp"
#include "head_geometry.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pln.hxx>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace starz::engineering {
ShaftSectionProperties shaftSection(const TopoDS_Shape& shaft,double station) {
    if(shaft.IsNull()||!std::isfinite(station))throw std::invalid_argument("Invalid shaft section request");
    const auto envelope=bounds(shaft);
    if(station<=envelope.ymin+1e-6||station>=envelope.ymax-1e-6)
        throw std::out_of_range("Section must be strictly inside shaft ends");
    const gp_Pln plane(gp_Pnt(0,station,0),gp_Dir(0,1,0));
    const double reach=1+2*std::max({std::abs(envelope.xmin),std::abs(envelope.xmax),std::abs(envelope.zmin),std::abs(envelope.zmax)});
    const auto planeFace=BRepBuilderAPI_MakeFace(plane,-reach,reach,-reach,reach).Face();
    // Preserve the boolean builder's native pcurves and inner-wire topology.
    // Reconstructing a face from disconnected Section edges loses valid
    // parameter curves at the three-way blended slot lips in OCC7.9.3.
    BRepAlgoAPI_Common common(shaft,planeFace);common.Build();
    if(!common.IsDone()||!BRepCheck_Analyzer(common.Shape()).IsValid())throw std::runtime_error("Invalid shaft/plane common section");
    int faces=0;for(TopExp_Explorer e(common.Shape(),TopAbs_FACE);e.More();e.Next())++faces;
    if(faces!=1)throw std::runtime_error("Shaft section must be one connected face with its native holes");
    GProp_GProps props;const double error=BRepGProp::SurfaceProperties(common.Shape(),props,1e-10);
    const double area=props.Mass();const auto centre=props.CentreOfMass();const auto tensor=props.MatrixOfInertia();
    const double Ixx=tensor.Value(1,1),Izz=tensor.Value(3,3),Ixz=tensor.Value(1,3);
    const double mean=(Ixx+Izz)/2,spread=std::hypot((Ixx-Izz)/2,Ixz);
    if(!std::isfinite(area)||area<=0||!std::isfinite(mean)||mean-spread<=0||!std::isfinite(error)||error<0)
        throw std::runtime_error("Non-positive/non-finite shaft section properties");
    if(error>1e-8)throw std::runtime_error("Shaft section surface integration estimate too large");
    return {station,area,centre.X(),centre.Z(),Ixx,Izz,Ixz,mean-spread,mean+spread,error};
}
}