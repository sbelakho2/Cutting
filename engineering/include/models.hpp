#pragma once
#include <array>
#include <string>
#include <vector>

namespace starz::engineering {
double interpolateTorque(double rpm);
double motorDemand(double force_n,double lead_m,double efficiency,double accel_m_s2,
                   double translating_kg,double rotary_kg_m2,double parasitic_nm);
double equivalentLoad(const std::vector<double>& loads,const std::vector<double>& fractions,double exponent=3);
double lifeRevolutions(double dynamic_n,double equivalent_n,double exponent=3);
double eulerLoad(double young_pa,double inertia_m4,double unsupported_m,double effective_length_factor);
double shaftCriticalRpm(double young_pa,double density,double diameter_m,double length_m);
double cantileverDeflection(double force,double length,double young,double width,double height,double poisson);
struct BoltResult { double remaining_clamp_n, bolt_tension_n, shear_capacity_n, proof_utilisation; };
BoltResult boltedJoint(int bolts,double preload_each,double axial_separating,double joint_fraction,
                      double friction,double tensile_area_m2,double proof_pa);
double copperExtension(double tension_n,double length_m,double copper_mm2,double modulus_pa=110e9);
double wireBuckling(double copper_mm2,double strand_diameter_mm,double free_m,double modulus_pa=110e9);
double eccentricWheelError(double circumference_mm,double radial_tir_mm,double travel_mm,double phase_rad);
struct HertzResult { double radius_m, indentation_m, pressure_pa; };
HertzResult sphereContact(double force,double radius,double equivalent_modulus);
struct DancerState { double position, velocity, tension; };
std::vector<DancerState> dancerResponse(double dt,double duration,double mass,double damping,
                                      double stiffness,double speed_step);
struct DecayResult { double minimum_v, maximum_v, final_v, resistor_j, peak_w, balance_error_j; };
DecayResult dcBus(double dt,double duration,double capacitance,double resistance,double initial_v,
                  double returned_energy_j,double return_time_s);
struct DynamicResult { double final_x, maximum_x, initial_energy, final_energy, dissipated_energy, balance_error; };
DynamicResult freeOscillator(double dt,double duration,double mass,double stiffness,double damping,
                            double initial_x,double initial_velocity);
struct ThermalResult { double motor_c, cabinet_c, loss_w; };
ThermalResult thermalSteady(double current,double resistance20,double ambient,double motor_to_air_k_w,
                           double cabinet_airflow_m3_s,double other_heat_w);
double requiredAirflow(double heat_w,double allowed_rise_c,double density=1.18,double heat_capacity=1006);
struct CutEstimate { double ideal_n, worn_n, energy_j; };
CutEstimate cuttingScreen(double copper_mm2,double od_mm,double copper_shear_mpa,
                         double polymer_shear_mpa,double friction,double edge_radius_mm,double indentation_mpa);
struct Check {
    std::string id, category, unit, provenance;
    double value, limit;
    bool passed;
};
std::vector<Check> analyticalChecks();
}