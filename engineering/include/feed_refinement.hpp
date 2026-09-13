#pragma once
#include <Eigen/Core>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <string>
#include <string_view>
#include <vector>

namespace starz::engineering {
enum class RefinementSurface { cylinder,torus,bounded_other };
struct ShaftRefinementFace {
    int cad_face=0;
    RefinementSurface kind=RefinementSurface::bounded_other;
    TopoDS_Face shape;
    Eigen::Vector3d lower_mm=Eigen::Vector3d::Zero(),upper_mm=Eigen::Vector3d::Zero();
    Eigen::Vector3d origin_mm=Eigen::Vector3d::Zero(),axis=Eigen::Vector3d::UnitY();
    double major_radius_mm=0,minor_radius_mm=0;
};
struct ShaftFeatureGuide {
    std::string feature,region;
    std::vector<ShaftRefinementFace> faces;
    // Lower-bound sizing distance to selected nominal CAD faces (or their
    // explicitly clipped axial portions), not a mesh or stress error estimate.
    double distance_mm(const Eigen::Vector3d& point_mm) const;
};
ShaftFeatureGuide feedShaftFeatureGuide(const TopoDS_Shape& shaft,std::string_view feature);
}