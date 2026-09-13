#pragma once
#include "head_geometry.hpp"

namespace starz::engineering {
// Mechanism integration solids in machine coordinates (RIGHT side; the CLI
// rotates a copy 180 degrees about Z for the LEFT side, like the cassettes).
struct CarrierSetting {
    double carrier_top_z, carried_mass_kg, spring_length_mm, spring_force_n;
    double adjuster_protrusion_mm, nip_force_n;
};
CarrierSetting carrierSetting(double wire_diameter_mm,double carried_mass_kg,double adjuster_protrusion_mm);
double nominalCarrierProtrusion(double carried_mass_kg);
double carriedMassEstimate(const std::vector<HeadPart>& cassette,const std::vector<HeadPart>& carrier_group);
std::vector<HeadPart> pressureCarrierGroup(double wire_diameter_mm,double adjuster_protrusion_mm);
std::vector<HeadPart> wireGuideGroup(double wire_diameter_mm);
struct ArmSetting { double arm_top_z, spring_length_mm, spring_force_n, adjuster_protrusion_mm, wheel_force_n; };
ArmSetting armSetting(double wire_diameter_mm,double adjuster_protrusion_mm);
double nominalArmProtrusion();
std::vector<HeadPart> measuringModuleGroup(double wire_diameter_mm,double adjuster_protrusion_mm);
std::vector<HeadPart> frameGroup();
// Static, preliminary enclosure/guard accommodation only. `block_wire_passage`
// intentionally removes the pass-throughs for the negative route test.
std::vector<HeadPart> guardGroup(bool block_wire_passage=false);
// Static envelope only: no selected spool, brake, tension mechanism or support
// load model is implied by this group.
std::vector<HeadPart> payoffGroup();
// Every part is a valid single solid (purchased parts may be compounds).
void checkMechanismSolids(const std::vector<HeadPart>& parts);
// Exact boolean interference between two groups (or within one when a==b).
void checkNoInterference(const std::vector<HeadPart>& a,const std::vector<HeadPart>& b,const char* label);
}
