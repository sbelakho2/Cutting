#pragma once
#include "head_geometry.hpp"
#include <BRepClass3d_SolidClassifier.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <TopoDS_Face.hxx>
#include <Eigen/Core>
#include <optional>
#include <vector>

namespace starz::engineering {
struct SurfaceCurvature {
    // Principal curvatures use the CAD surface parameter normal, not an
    // asserted outward material normal. Gaussian curvature is orientation-free.
    double minimum_per_mm=0,maximum_per_mm=0,gaussian_per_mm2=0;
};
struct CadBoundaryPoint {
    TopAbs_State state=TopAbs_UNKNOWN;
    int face=0,support_dimension=-1;
    GeomAbs_SurfaceType type=GeomAbs_OtherSurface;
    double distance_mm=0;
    Eigen::Vector3d projected_mm=Eigen::Vector3d::Zero();
    std::optional<SurfaceCurvature> curvature;
};
class CadBoundaryInspector {
public:
    explicit CadBoundaryInspector(const TopoDS_Shape& solid);
    CadBoundaryPoint inspect(const Eigen::Vector3d& p);
    std::size_t faceCount() const {return faces_.size();}
    const TopoDS_Face& face(std::size_t one_based) const;
private:
    struct Face {TopoDS_Face shape;BoxBounds bounds;};
    BRepClass3d_SolidClassifier classifier_;
    std::vector<Face> faces_;
};
const char* cadStateName(TopAbs_State state);
const char* cadSurfaceName(GeomAbs_SurfaceType type);
}