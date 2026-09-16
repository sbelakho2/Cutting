#pragma once
#include "fem.hpp"
#include <Eigen/Core>
#include <TopoDS_Shape.hxx>
#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace starz::engineering {
// Field-carrying production VTU: geometry through the existing fail-closed
// geometry reader plus the point displacement (mm) and cell von Mises (MPa)
// arrays. Strictly additive to mesh_io; the geometry-only reader is unchanged.
struct VtuField {
    TetMesh mesh;
    std::vector<Eigen::Vector3d> displacement_mm;
    std::vector<double> von_mises_mpa;
};
VtuField readLinearVtuField(const std::filesystem::path& path);

// Element-local barycentric displacement lookup over a conforming global
// tetrahedral mesh, built from up to eight superposed basis displacement
// fields (the shaft load bases). A uniform grid index bounds the search.
//
// Exact containment is required inside the mesh. Points that classify on or
// near the CAD boundary can fall micrometres outside the flat-facet mesh hull
// (the facet sagitta); those are supported only through nearest-hull
// projection with an explicit tolerance, and the gap is recorded. Every other
// unsupported point raises an error (the outside-support error path) instead
// of extrapolating.
struct DisplacementInterpolationAudit {
    std::size_t exact_hits=0,boundary_projections=0,unsupported=0;
    double max_gap_mm=0,cell_size_mm=0;
};
class DisplacementFieldInterpolator {
public:
    DisplacementFieldInterpolator();
    ~DisplacementFieldInterpolator();
    DisplacementFieldInterpolator(const DisplacementFieldInterpolator&)=delete;
    DisplacementFieldInterpolator& operator=(const DisplacementFieldInterpolator&)=delete;
    void build(const TetMesh& mesh,const std::vector<std::vector<Eigen::Vector3d>>& basis_fields_mm,
               double boundary_tolerance_mm=0.02);
    Eigen::Vector3d interpolate(std::size_t field,const Eigen::Vector3d& point_mm) const;
    Eigen::Vector3d combine(const std::array<double,8>& weights,const Eigen::Vector3d& point_mm) const;
    const DisplacementInterpolationAudit& audit() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Slab submodel: the shaft section |S-station|<=half_mm cut from the exact
// shaft B-rep (BRepAlgoAPI_Common with a padded box). The cut faces are the
// two planar cross-section disks; the shaft surface between them stays free.
struct SubmodelSlab {
    TopoDS_Shape shape;
    double station_mm=0,half_mm=0;
};
SubmodelSlab cutShaftSlab(const TopoDS_Shape& shaft,double station_mm,double half_mm);

// Per-node cut classification of a slab submodel mesh: a boundary triangle all
// of whose vertices lie on one cut plane is a Dirichlet cut face. Every other
// boundary node lies on the retained shaft surface and stays traction-free.
struct SubmodelCutClassification {
    std::vector<char> constrained;
    std::size_t cut_nodes=0,boundary_nodes=0;
};
SubmodelCutClassification classifySlabCutNodes(const TetMesh& submesh,const SubmodelSlab& slab,
                                               double plane_tolerance_mm=1e-3);

// One submodel load state: a Dirichlet boundary configuration plus the
// material-frame body acceleration. All cases share the same constrained-node
// pattern; the factorised stiffness is reused across them.
struct SubmodelCase {
    std::vector<std::array<std::optional<double>,3>> prescribed_m;
    Eigen::Vector3d body_acceleration_m_s2=Eigen::Vector3d::Zero();
};
// Direct-solver budget retained at 150000 nodes for solver parity with the
// full-shaft series; CHOLMOD supernodal Cholesky with METIS ordering.
struct SubmodelSolveResult {
    ElasticResult field;
    std::vector<StressTensor6> stress_pa;
};
std::vector<SubmodelSolveResult> solveSubmodelCases(const TetMesh& mesh,ElasticMaterial material,
                                                    const std::vector<SubmodelCase>& cases);

// Peterson/Pilkey shoulder-fillet curve fit for a stepped round bar in
// bending. Form: Kt = C1 + C2*(2h/D) + C3*(2h/D)^2 + C4*(2h/D)^3 with
// h=(D-d)/2 and the C's polynomials in sqrt(h/r) and h/r.
// Reference: W.D. Pilkey, Peterson's Stress Concentration Factors, 3rd ed.,
// Wiley 2008, Table 3.3 (bending), fit range 2 < h/r <= 20. The implemented
// coefficient set reproduces the independent Shigley Table A-15-9 chart value
// Kt=1.74 at D/d=1.5, r/d=0.1 (computed 1.7433). Returns nullopt outside the
// published range; nothing is extrapolated.
std::optional<double> pilkeyShoulderBendingKt(double D_mm,double d_mm,double r_mm);

// Nominal cross-section stress decomposition of a stress field over an axial
// band of element centroids: axial mean, bending magnitude at the section
// surface radius, torsion shear magnitude at the surface and mean von Mises.
struct NominalSectionStress {
    double section_radius_mm=0;
    double axial_mean_pa=0,bending_surface_pa=0,torsion_surface_pa=0,von_mises_mean_pa=0;
    std::size_t elements=0;
};
NominalSectionStress nominalBandStress(const TetMesh& mesh,const std::vector<StressTensor6>& stress_pa,
                                       double station_mm,double half_band_mm);

// Distance from a point to the perimeter curve of a key-floor capsule: a
// rounded rectangle in the x-S plane at height z0, spanning S in [begin,end]
// with half-width radius. Used by the submodel sizing guide with the same
// zero set as the published shaft root guides (perimeter minus fillet).
double capsulePerimeterDistance(const Eigen::Vector3d& p,double begin_mm,double end_mm,
                                double radius_mm,double z0_mm);
}
