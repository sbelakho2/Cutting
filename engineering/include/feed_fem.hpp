#pragma once
#include "fem.hpp"
#include <array>
#include <string>
#include <string_view>

namespace starz::engineering {
struct ShaftStressRegion {
    std::string name;
    std::function<bool(const Eigen::Vector3d&)> contains;
};
std::vector<DistributedConstraint> feedShaftSupports();
std::vector<ElasticLoadCase> feedShaftBasisLoads();
// Basis: pulley Fx/Fz, roller Fx/Fz, coupling-to-roller Ty,
// coupling-to-pulley Ty, body acceleration X/Z.
std::array<double,8> feedShaftCoefficients(double phase_rad,int belt_sign,int roller_x_sign,int roller_z_sign,
                                        double roller_torque_nm,double pulley_torque_nm);
std::vector<std::array<double,2>> feedShaftTorqueVertices(double interface_limit_nm);
// Empty focus retains the original all-root sizing guide. A named feature
// changes mesh sizing only; it never cuts the shaft or removes stress regions.
double feedShaftRootDistance(const Eigen::Vector3d& point_mm,std::string_view focus={});
std::vector<ShaftStressRegion> feedShaftStressRegions();
double vonMises(const StressTensor6& stress);
// If f(phi)=a+b*cos(phi)+c*sin(phi), this is an upper bound on the
// unsampled norm increment between uniform angular samples.
double angularNormRemainder(double cosine_norm,double sine_norm,std::size_t phases);
}