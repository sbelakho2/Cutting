#pragma once
#include <TopoDS_Shape.hxx>
#include <filesystem>
#include <vector>

namespace starz::engineering {
// One four-mesh level of the whole-shaft torus series: exact BRep
// point-in-solid classification of every tetrahedron centroid (working and
// fault VTUs of the level must carry the identical shared mesh).
struct ContainmentLevelResult {
    std::size_t level=0;
    double bulk_mm=0,root_mm=0;
    std::size_t nodes=0,tetrahedra=0;
    std::size_t inside=0,outside=0,on_boundary=0;
};
struct ContainmentAuditResult {
    double classifier_tolerance_mm=1e-7;
    double cad_volume_mm3=0;
    std::vector<ContainmentLevelResult> levels;
    bool all_inside=false;
};
// Continuous CAD-containment audit of the four-level working/fault VTU mesh
// set in meshDirectory against the exact solid. Every tetrahedron centroid of
// every level is classified by BRepClass3d_SolidClassifier at the fixed
// 1e-7 mm tolerance; TopAbs_ON and TopAbs_OUT are both violations, so
// all_inside means 0 outside and 0 on-boundary-within-tolerance across all
// levels. The audit fails closed (throws) on any missing, mismatched or
// unverified input; it never weakens the tolerance or the mesh identity
// requirements. On zero violations it flips
// continuous_CAD_containment_verified:true in the directory status.json
// without touching any other field; on violations it leaves status.json
// unmodified and writes per-level counts plus the violating centroids.
ContainmentAuditResult runContainmentAudit(const TopoDS_Shape& solid,const std::filesystem::path& meshDirectory);
}
