#pragma once
#include "fem.hpp"
#include <array>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Desk closure of the "housing compliance" and "thermal field" halves of the
// bearings/joints/thermal gate (docs/33 gate 10; docs/12:38,68):
//  - Study 1: F101 front bearing seat contact compliance. The Ø15 j5 seat and
//    the 6002-2Z/C3 rings are meshed with the F112 housing block; the matched
//    shaft interference is applied as a Lamé-resolved pressure pair (the
//    collar_contact resolved-pressure pattern) and the fit record's
//    outer-ring/housing value is applied on the bore. Report the contact
//    pressures, peak von Mises vs the 42CrMo4 / bearing-steel / 6082-T6
//    allowables, and the rigid (four-mesh style) vs compliant seat-boundary
//    difference.
//  - Study 2: steady-state linear thermal FEM of the F101 shaft front half
//    (S31.5..S113): 30 W standstill motor holding loss at the pulley seat,
//    conduction along 42CrMo4 (k=42.7 W/mK), h=10 W/m^2K still-air convection,
//    bearing seat as a conduction path to the 25 degC housing, roller
//    surfaces pinned at 25 degC. The front-seat temperature rise is checked
//    against the bearing-fit record's explicit temperature envelope.
// Nothing here releases fabrication; the instrumented physical test remains.
namespace starz::engineering {

// ---------------------------------------------------------------------------
// Study 1 — bearing/housing contact compliance
// ---------------------------------------------------------------------------

struct BearingContactGeometry {
    // Local axis +Z. The 6002 seat occupies z in [0, 9]; the shaft extends
    // rearward to z=-rear_extension and forward through the Ø19 shoulder and
    // Ø12 neck (plain steps, no fillets: the study isolates the support
    // boundary, not the shoulder blends).
    double seat_diameter_mm=14.995;   // meshed stand-in 5 um under the 15.000
                                      // ring bore so the coincident conforming
                                      // surfaces stay separable for the
                                      // resolved-pressure patches; the real
                                      // seat is 14.997..15.005 and the fit is
                                      // applied as pressure, not geometry.
    double rear_extension_mm=8.0;
    double shoulder_diameter_mm=19.0;
    double shoulder_length_mm=5.0;
    double neck_diameter_mm=12.0;
    double neck_length_mm=3.0;
    double seat_length_mm=9.0;        // 6002 width
    double inner_ring_od_mm=19.74;    // nominal 6002 inner-ring OD (model idealisation)
    double outer_ring_id_mm=27.26;    // nominal 6002 outer-ring ID (model idealisation)
    double outer_ring_od_mm=32.00;    // 6002 OD, tolerance 0/-0.009 per the catalogue record
    double housing_x_mm=46.0, housing_y_mm=37.5, housing_z_mm=19.02;  // F112
    double housing_bore_diameter_mm=32.3; // meshed stand-in 0.15 mm over the Ø32.0135
                                          // CAD bore (record 32.008..32.019) to keep the
                                          // near-coincident bore/ring-OD surfaces separable;
                                          // the record's clearance is applied as data.
    double shaft_interference_diametral_um=6.0;  // matched j5 worst case (record 3..6 um)
    double housing_clearance_diametral_um=8.0;   // record minimum (bore 32.008 vs OD 32.000)
    double radial_load_n=310.0;         // front-bearing peak resultant (feed-analysis record)
    double chain_stiffness_n_per_um=35.0; // bearing balls + aluminium bore wall in series, stated
};

struct BearingContactRun {
    double bulk_mm=2.2;
    double minimum_mm=0.35;
    std::optional<std::filesystem::path> vtu_dir;
};

struct BearingContactCase {
    double peak_seat_vm_mpa=0;      // shaft seat band r<7.49, z in [0.1,8.9]
    double peak_ring_vm_mpa=0;      // inner ring band r in [7.52,9.85], z in [0.1,8.9]
    double peak_housing_wall_vm_mpa=0; // housing wall band away from the ring station
    double max_displacement_um=0;
};

struct BearingContactResult {
    std::size_t nodes=0,tetrahedra=0;
    double lamé_pressure_mpa=0;             // analytic shrink-fit pressure at the given interference
    double resolved_seat_pressure_mpa=0;    // pressure applied on seat + ring ID
    double housing_contact_pressure_mpa=0;  // from the fit record value (clearance -> 0)
    double shaft_seat_allowable_mpa=650;    // 42CrMo4+QT certified minimum yield
    double ring_allowable_mpa=1700;         // hardened 100Cr6 ring, handbook value; cert governs
    double housing_allowable_mpa=260;       // 6082-T6 Rp0.2, handbook value; cert governs
    BearingContactCase fit_only;            // resolved pressure pair, compliant chain supports
    BearingContactCase rigid_load;          // four-mesh style fixed seat + radial load
    BearingContactCase compliant_load;      // fit pressure + radial load, compliant chain
    BearingContactCase bore_wall_load;      // radial load resolved on the housing bore wall
    BearingContactCase chain_soft,chain_stiff; // compliant_load with chain x0.5 / x2
    double load_induced_seat_mpa=0;         // compliant_load - fit_only (load share on the seat)
    double boundary_effect_percent=0;       // (rigid - load-induced) / rigid, seat peak
    bool solved=false;
    std::string verdict;
};

// Analytic Lamé shrink-fit pressure (Pa) for a solid hub and a sleeve
// (outer diameter `outer_diameter_mm`) at radial interference `radial_interference_um`.
double laméPressure(double interface_diameter_mm,double outer_diameter_mm,double radial_interference_um,
                    double e_hub_pa,double nu_hub,double e_sleeve_pa,double nu_sleeve);
// Hoop stress (Pa) at the sleeve ID under the resolved pressure p (Pa).
double laméSleeveHoop(double interface_diameter_mm,double outer_diameter_mm,double pressure_pa);

BearingContactResult runBearingContactStudy(const BearingContactGeometry& g,const BearingContactRun& run);

// ---------------------------------------------------------------------------
// Study 2 — front-half steady-state thermal field
// ---------------------------------------------------------------------------

struct ThermalBoundary {
    enum class Kind { flux, convection, prescribed, conduction } kind=Kind::convection;
    std::function<bool(const Eigen::Vector3d&)> select; // boundary face-centroid test, mm
    double value=0;     // flux W/m^2 | h W/m^2K | prescribed T degC | h W/m^2K
    double sink_c=25;   // convection/conduction ambient/sink temperature
};

struct ThermalInput {
    double conductivity_w_mk=42.7;      // 42CrMo4
    double ambient_c=25.0;              // housing / still air
    double source_w=30.0;               // standstill motor holding loss (2*5^2*0.6)
    double convection_h_w_m2k=10.0;     // still air
    double seat_conductance_h_w_m2k=2800.0; // stated series chain: press contact
                                            // 8000 W/m^2K + balls/grease 2 W/K +
                                            // grease gap -> ~1.2 W/K over the seat
    double shaft_start_s_mm=31.5;       // local z = S - 31.5
    double shaft_end_s_mm=113.0;
};

struct ThermalRun {
    double bulk_mm=1.6;
    double minimum_mm=0.4;
    std::optional<std::filesystem::path> vtu_dir;
};

struct ThermalResult {
    std::size_t nodes=0,tetrahedra=0;
    double seat_max_c=0,seat_mean_c=0;
    double rise_max_k=0;
    double power_in_w=0,power_out_w=0,balance_relative=0;
    std::vector<double> temperature_c;
};

struct ThermalStudyOutput {
    ThermalResult central;
    std::vector<std::pair<double,ThermalResult>> sensitivity; // (h_seat W/m^2K, result)
    double envelope_shaft_above_housing_k=20.0; // 0-5 outer + 0-10 inner + 0-5 shaft (fit record)
    bool envelope_met_central=false;
    bool solved=false;
    std::string verdict;
};

// Solve one steady-state linear conduction problem on `mesh` with the given
// boundary conditions (k in W/mK). Prescribed temperatures are eliminated;
// convection/conduction surfaces become nodal conductances; flux surfaces
// become nodal sources. The 150000-node cap and the power balance apply.
ThermalResult solveThermal(const TetMesh& mesh,double conductivity_w_mk,
                           const std::vector<ThermalBoundary>& bc);

ThermalStudyOutput runBearingThermalStudy(const ThermalInput& in,const ThermalRun& run);

// CLI: --report <dir> writes report.md / status.json / VTUs and returns the
// process code (0 on a completed run; the verdicts live in status.json).
int bearingContactThermalMain(int argc,char** argv);

} // namespace starz::engineering
