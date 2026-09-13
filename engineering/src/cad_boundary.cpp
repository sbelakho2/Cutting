#include "cad_boundary.hpp"
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepLProp_SLProps.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace starz::engineering {
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
}
const char* cadStateName(TopAbs_State state) {
    switch(state){case TopAbs_IN:return "inside";case TopAbs_ON:return "on";case TopAbs_OUT:return "outside";default:throw std::runtime_error("Unknown CAD point classification");}
}
const char* cadSurfaceName(GeomAbs_SurfaceType type) {
    switch(type) {
    case GeomAbs_Plane:return "plane";case GeomAbs_Cylinder:return "cylinder";
    case GeomAbs_Cone:return "cone";case GeomAbs_Sphere:return "sphere";
    case GeomAbs_Torus:return "torus";case GeomAbs_BSplineSurface:return "bspline";
    case GeomAbs_BezierSurface:return "bezier";case GeomAbs_SurfaceOfRevolution:return "revolution";
    case GeomAbs_SurfaceOfExtrusion:return "extrusion";case GeomAbs_OffsetSurface:return "offset";
    default:return "other";
    }
}
CadBoundaryInspector::CadBoundaryInspector(const TopoDS_Shape& solid):classifier_(solid) {
    require(!solid.IsNull()&&BRepCheck_Analyzer(solid).IsValid(),"Invalid CAD boundary inspection shape");
    TopTools_IndexedMapOfShape solids;TopExp::MapShapes(solid,TopAbs_SOLID,solids);require(solids.Extent()==1,"Boundary inspection requires one solid");
    TopTools_IndexedMapOfShape faces;TopExp::MapShapes(solid,TopAbs_FACE,faces);
    for(int i=1;i<=faces.Extent();++i)faces_.push_back({TopoDS::Face(faces(i)),bounds(faces(i))});
    require(!faces_.empty(),"CAD boundary has no faces");
}
const TopoDS_Face& CadBoundaryInspector::face(std::size_t one_based) const {
    require(one_based>0,"CAD face IDs are one-based");return faces_.at(one_based-1).shape;
}
CadBoundaryPoint CadBoundaryInspector::inspect(const Eigen::Vector3d& p) {
    require(p.allFinite(),"Non-finite CAD inspection point");const gp_Pnt point(p.x(),p.y(),p.z());
    classifier_.Perform(point,1e-7);CadBoundaryPoint result;result.state=classifier_.State();(void)cadStateName(result.state);
    result.distance_mm=std::numeric_limits<double>::infinity();const auto vertex=BRepBuilderAPI_MakeVertex(point).Shape();
    for(std::size_t i=0;i<faces_.size();++i) {
        const auto& item=faces_[i];const auto& b=item.bounds;
        const Eigen::Vector3d lower{b.xmin,b.ymin,b.zmin},upper{b.xmax,b.ymax,b.zmax};
        const double bound=(lower-p).cwiseMax(p-upper).cwiseMax(0.).norm();
        if(bound>result.distance_mm+1e-9)continue;
        BRepExtrema_DistShapeShape projection(vertex,item.shape);projection.Perform();
        require(projection.IsDone()&&projection.NbSolution()>0,"Trimmed CAD face projection failed");
        const double distance=projection.Value();require(std::isfinite(distance)&&distance>=0,"Non-finite CAD boundary distance");
        if(distance>=result.distance_mm)continue;
        result.face=static_cast<int>(i+1);result.distance_mm=distance;result.curvature.reset();
        const BRepAdaptor_Surface surface(item.shape);result.type=surface.GetType();const auto projected=projection.PointOnShape2(1);
        result.projected_mm={projected.X(),projected.Y(),projected.Z()};
        const auto support=projection.SupportTypeShape2(1);
        result.support_dimension=support==BRepExtrema_IsVertex?0:support==BRepExtrema_IsOnEdge?1:2;
        if(support==BRepExtrema_IsInFace) {
            double u=0,v=0;projection.ParOnFaceS2(1,u,v);BRepLProp_SLProps properties(surface,u,v,2,1e-9);
            if(properties.IsCurvatureDefined()) {
                SurfaceCurvature curvature{properties.MinCurvature(),properties.MaxCurvature(),properties.GaussianCurvature()};
                require(std::isfinite(curvature.minimum_per_mm)&&std::isfinite(curvature.maximum_per_mm)&&std::isfinite(curvature.gaussian_per_mm2),"Non-finite CAD surface curvature");
                result.curvature=curvature;
            }
        }
    }
    require(result.face>0&&std::isfinite(result.distance_mm)&&result.projected_mm.allFinite(),"Missing nearest trimmed CAD face");
    require(std::abs((p-result.projected_mm).norm()-result.distance_mm)<1e-7,"CAD closest point and distance disagree");return result;
}
}