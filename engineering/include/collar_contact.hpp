#pragma once
#include "fem.hpp"
#include <array>
#include <filesystem>
#include <string>
#include <vector>

// Desk study: UMCL40-10-20-MC collar / DIN 6885 key contact on the feed
// roller shaft (local linear FEM with a resolved-pressure rigid-collar
// idealisation). Closes the desk part of the "housing/collar contact" gate;
// nothing here releases fabrication.
namespace starz::engineering {

struct CollarKeyGeometry {
    // Shaft section (axis +Z, z in [0, shaft_length]).
    double shaft_diameter_mm=10.0;
    double shaft_length_mm=60.0;
    // DIN 6885 t1 seat (nominal line-to-line with the key width).
    double seat_width_mm=3.0;
    double seat_depth_mm=1.8;
    // Key block: width x height x length.
    double key_width_mm=3.0;
    double key_height_mm=3.0;
    double key_length_mm=20.0;
    // Collar (rigid) station: bore width and axial start.
    double collar_width_mm=20.0;
    double collar_z_min_mm=5.0;
    // Edge blends: seat floor perimeter R0.15 (per the shaft record); the
    // key long edges are rounded R0.1 as a stated stand-in for the real
    // 0.2 x 45 deg shaft-facing chamfer / deburred edges.
    double fillet_floor_mm=0.15;
    double fillet_key_edge_mm=0.10;
    // Loads and support (stated assumptions, see report).
    double torque_nm=5.0;
    double clamp_force_per_screw_n=3500.0; // M4 8.8 at 2.8 Nm, K=0.2.
    double clamp_arc_deg=90.0;             // two opposed arcs about +-X.
    double bore_reaction_arc_deg=120.0;    // rigid bore bearing, opposite the key.
};

struct CollarStudyRun {
    double bulk_mm=0.5;
    double minimum_mm=0.2;
    std::optional<std::filesystem::path> vtu_dir; // if set, write case VTUs
};

struct CollarCaseNumbers {
    double peak_key_vm_mpa=0;
    double peak_key_shear_mpa=0;      // max sqrt(sxy^2+syz^2+sxz^2) in the key
    double key_shear_near_plane_mpa=0; // key tets straddling the shaft-surface plane
    double peak_face_sxx_mpa=0;        // loaded side face (compression, +)
    double peak_seat_wall_vm_mpa=0;    // shaft keyway wall band
    double peak_shaft_vm_mpa=0;        // shaft excluding the key
    double max_displacement_mm=0;
    double applied_face_pressure_mpa=0; // F / actually selected face area
};

struct CollarStudyResult {
    std::size_t nodes=0;
    std::size_t tetrahedra=0;
    double key_force_n=0;
    double ideal_face_pressure_mpa=0;   // F / ((H - t1) * L)
    double key_shear_average_mpa=0;     // F / (b * L)
    double seat_bearing_average_mpa=0;  // F / (t1 * L)
    CollarCaseNumbers clamp_only;
    CollarCaseNumbers torque_only;
    CollarCaseNumbers combined;
    bool solved=false;
    std::string verdict;
};

// Solves clamp-only, torque-only and combined load cases on one conforming
// shaft+key mesh (bonded key; rigid collar via resolved pressures).
CollarStudyResult runCollarContactStudy(const CollarKeyGeometry& geometry,
                                        const CollarStudyRun& run);

// CLI: writes report.md / status.json / VTUs under `out` and returns the
// process code. --report <dir> is the only supported invocation.
int collarContactMain(int argc,char** argv);

} // namespace starz::engineering
