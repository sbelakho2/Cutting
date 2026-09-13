#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace starz::engineering {
struct BearingFitInput {
    double nominal_bore_mm,nominal_outer_mm;
    double initial_clearance_um,shaft_interference_um,housing_clearance_um;
    double shaft_minus_inner_k,inner_minus_outer_k,outer_minus_housing_k,housing_c;
};
struct BearingFitResult {
    double hot_shaft_interference_um,hot_housing_clearance_um,operating_clearance_um;
};
inline BearingFitResult bearingFit(const BearingFitInput& x) {
    for(double v:{x.nominal_bore_mm,x.nominal_outer_mm,x.initial_clearance_um,x.shaft_interference_um,
                  x.housing_clearance_um,x.shaft_minus_inner_k,x.inner_minus_outer_k,x.outer_minus_housing_k,x.housing_c})
        if(!std::isfinite(v))throw std::invalid_argument("Nonfinite bearing-fit input");
    if(x.nominal_bore_mm<=0||x.nominal_outer_mm<=x.nominal_bore_mm||x.initial_clearance_um<0||
       x.shaft_minus_inner_k<0||x.inner_minus_outer_k<0||x.outer_minus_housing_k<0)
        throw std::invalid_argument("Invalid bearing-fit geometry/temperature ordering");
    // Steel12µm/m/K;6082aluminium23.4µm/m/K. Positive free-housing
    // clearance is checked independently: no cast-iron housing factor is used.
    const double hot_i=x.shaft_interference_um+0.012*x.nominal_bore_mm*x.shaft_minus_inner_k;
    const double hot_h=x.housing_clearance_um+x.nominal_outer_mm*
        (0.0114*(x.housing_c-20)-0.012*x.outer_minus_housing_k);
    // Deliberately use1.0 as the reduction-factor upper bound rather than
    // reading favourable values from SKF's solid-shaft/thick-steel-housing plot.
    const double fit_loss=std::max(0.0,hot_i)+std::max(0.0,-hot_h);
    const double thermal_loss=0.012*x.inner_minus_outer_k*(x.nominal_bore_mm+x.nominal_outer_mm)/2;
    return {hot_i,hot_h,x.initial_clearance_um-fit_loss-thermal_loss};
}
struct ShaftMatch {double lower_mm,upper_mm;};
inline ShaftMatch matchedJ5Seat(double nominal,double measured_bearing_bore) {
    if(!std::isfinite(nominal)||!std::isfinite(measured_bearing_bore)||nominal<=10||nominal>18||
        measured_bearing_bore<nominal-0.007-1e-10||measured_bearing_bore>nominal+1e-10)
        throw std::invalid_argument("Selected P6 bearing bore outside10–18mm record");
    // ISO286j5 for10<d≤18:−3/+5µm. Intersect that zone with the
    // specified3–6µm diametral matching window;these are actual-size limits.
    const ShaftMatch out{std::max(nominal-0.003,measured_bearing_bore+0.003),
                         std::min(nominal+0.005,measured_bearing_bore+0.006)};
    if(out.lower_mm>out.upper_mm)throw std::runtime_error("No feasible selected-bearing match");
    return out;
}
}