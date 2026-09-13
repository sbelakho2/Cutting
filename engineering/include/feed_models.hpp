#pragma once
#include <array>
#include <span>
#include <vector>

namespace starz::engineering {
double interpolateTable(std::span<const double> x,std::span<const double> y,double at);
double equalPulleyCentres(double belt_length_mm,double teeth,double pitch_mm);
double s5mPowerCapacity(double rpm);
double s5mTorqueCapacity(double rpm);
struct BeltTension {
    double span_mm{},deflection_mm{},deflection_force_n{},tight_n{},slack_n{},radial_n{};
};
BeltTension beltTension(double initial_n,double centres_mm,double length_mm,
                       double pitch_radius_mm,double torque_nm,double correction_y_n);
struct KeyStress {double shear_mpa{},bearing_mpa{};};
KeyStress keyStress(double torque_nm,double diameter_mm,double width_mm,double side_height_mm,double effective_length_mm);

struct ShaftSegment {double from_mm{},to_mm{},diameter_mm{},hole_mm{};};
struct BeamLoad {double station_mm{},force_n{};};
struct BeamResult {
    double reaction_a_n{},reaction_b_n{},max_displacement_mm{},max_bending_mpa{};
    double force_residual_n{},moment_residual_nmm{};
    std::vector<double> stations_mm,displacements_mm,slopes_rad;
};
// Euler-Bernoulli beam, variable annular cross-section, exact nodal point loads.
// Stations at material changes, support and load points; units N,mm,MPa.
// Does not pretend to be a bearing/contact/keyway solid FEA model.
BeamResult shaftBeam(std::span<const ShaftSegment> sections,double young_mpa,
                     double support_a_mm,double support_b_mm,
                     std::span<const BeamLoad> loads,int subdivisions=3);
std::vector<ShaftSegment> selectedFeedShaftSections();
}