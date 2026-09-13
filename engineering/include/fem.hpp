#pragma once
#include <Eigen/Core>
#include <array>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace starz::engineering {
struct MeshSurfaceAudit {
    int tag=0;
    std::string type;
    std::size_t nodes=0,triangles=0;
    double max_node_distance_mm=0,max_facet_centroid_distance_mm=0;
};
struct MeshGeometryAudit {
    double cad_volume_mm3=0,element_volume_mm3=0,min_sicn=1;
    std::vector<MeshSurfaceAudit> surfaces;
    struct SameSurfaceTet {std::size_t element=0;int surface=0;};
    // Each cell is counted once if all four vertices share a CAD surface.
    // This is a topology diagnostic, not a complete curved-containment audit.
    std::vector<SameSurfaceTet> same_surface_tetrahedra;
};
struct TetMesh {
    std::vector<Eigen::Vector3d> nodes_mm;
    std::vector<std::array<int,4>> tetrahedra;
    // Populated for locally refined STEP meshes. Projection checks concern
    // nodes/facet centroids on underlying CAD surfaces, not a Hausdorff bound.
    std::optional<MeshGeometryAudit> geometry_audit;
};
struct ElasticMaterial { double young_pa, poisson, density_kg_m3; };
using Constraint=std::array<std::optional<double>,3>; // Prescribed displacement in metres.
using Constraints=std::function<Constraint(const Eigen::Vector3d& point_mm)>;
struct LoadPatch {
    std::function<bool(const Eigen::Vector3d& centroid_mm)> select;
    Eigen::Vector3d force_n=Eigen::Vector3d::Zero();
    std::optional<Eigen::Vector3d> resultant_point_mm;
    Eigen::Vector3d couple_nm=Eigen::Vector3d::Zero(); // Free moment, in addition to r × F.
};
struct ElasticResult {
    std::vector<Eigen::Vector3d> displacement_m;
    std::vector<double> von_mises_pa;
    double volume_m3=0, energy_j=0, max_displacement_m=0, load_average_displacement_m=0;
    double max_stress_pa=0, p99_stress_pa=0, free_residual_relative=0, force_balance_relative=0;
    double moment_balance_relative=0;
    Eigen::Vector3d total_load_n=Eigen::Vector3d::Zero(), total_reaction_n=Eigen::Vector3d::Zero();
    Eigen::Vector3d total_load_moment_nm=Eigen::Vector3d::Zero(), total_reaction_moment_nm=Eigen::Vector3d::Zero();
    std::vector<double> modal_hz, modal_residual;
};
TetMesh meshStep(const std::filesystem::path& path,double element_size_mm);
TetMesh meshBox(double x_mm,double y_mm,double z_mm,double element_size_mm);
TetMesh meshCylinder(double radius_mm,double length_mm,double element_size_mm); // Axis +Y.
enum class RefinedMeshOptimizer { gmsh,netgen };
// The callback specifies local target size; it does not alter the STEP geometry.
TetMesh meshStepRefined(const std::filesystem::path& path,double bulk_size_mm,double minimum_size_mm,
                       const std::function<double(const Eigen::Vector3d&)>& local_size_mm,
                       RefinedMeshOptimizer optimizer=RefinedMeshOptimizer::gmsh);
struct ElasticLoadCase {
    std::vector<LoadPatch> patches;
    Eigen::Vector3d body_acceleration_m_s2=Eigen::Vector3d::Zero();
};
struct DistributedConstraint {
    // Work-conjugate interpolation: virtual_load^T displacement = value.
    // Unit force gives displacement (m); unit couple gives rotation (rad).
    LoadPatch virtual_load;
    double value=0;
};
using StressTensor6=Eigen::Matrix<double,6,1>; // xx,yy,zz,xy,yz,xz; tensor shear stress.
struct DistributedElasticResult {
    std::vector<Eigen::Vector3d> displacement_m;
    std::vector<StressTensor6> stress_pa;
    std::vector<Eigen::Vector3d> support_force_n,support_moment_nm;
    double volume_m3=0,energy_j=0,max_displacement_m=0;
    double equilibrium_relative=0,constraint_relative=0,force_balance_relative=0,moment_balance_relative=0;
    Eigen::Vector3d total_load_n=Eigen::Vector3d::Zero(),total_load_moment_nm=Eigen::Vector3d::Zero();
};
// Exact Lagrange constraints through an augmented SPD factor: six coordinates
// use rigid equilibrium/lifting; seven–twelve use a small Schur complement.
// Stabilisation cancels algebraically; it is NOT a bearing spring.
std::vector<DistributedElasticResult> solveDistributedElasticCases(
    const TetMesh& mesh,ElasticMaterial material,const std::vector<DistributedConstraint>& constraints,
    const std::vector<ElasticLoadCase>& cases,double stabilisation_scale=1);
// One stiffness assembly/factorisation for multiple load cases with identical
// material/supports. Requested modes are returned in the first result only.
std::vector<ElasticResult> solveElasticCases(const TetMesh& mesh,ElasticMaterial material,const Constraints& constraints,
                           const std::vector<std::vector<LoadPatch>>& load_cases,int modes=0,
                           const Eigen::Vector3d& body_acceleration_m_s2=Eigen::Vector3d::Zero());
ElasticResult solveElastic(const TetMesh& mesh,ElasticMaterial material,const Constraints& constraints,
                           const std::vector<LoadPatch>& loads,int modes=0,
                           const Eigen::Vector3d& body_acceleration_m_s2=Eigen::Vector3d::Zero());
void writeVtu(const std::filesystem::path& path,const TetMesh& mesh,const ElasticResult& result);
}