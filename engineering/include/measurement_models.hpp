#pragma once
#include "shared/measurement_spec.hpp"
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace starz::engineering {
struct WheelCalibrationKnot {
    std::uint32_t count;
    double travel_mm;
};
// A mathematical mapping only. It neither creates an index reference nor
// approves a wheel, recipe, calibration record or hardware motion.
// Counts must be unwrapped and referenced to the same latched Z phase used
// during calibration. Never pass a wrapping MCU register as an int64 count.
class WheelCalibration {
public:
    explicit WheelCalibration(std::vector<WheelCalibrationKnot> knots);
    double circumferenceMm() const;
    double distanceMm(std::int64_t from_count,std::int64_t to_count) const;
    double maxMmPerCount() const;
    double minMmPerCount() const;
    const std::vector<WheelCalibrationKnot>& knots() const {return knots_;}
private:
    double phaseTravel(std::uint32_t phase) const;
    std::vector<WheelCalibrationKnot> knots_;
    double min_pitch_=0,max_pitch_=0;
};
WheelCalibration readWheelCalibration(const std::filesystem::path& csv_path);

struct WheelObservation {
    std::int64_t from_count,to_count;
    double reference_mm,reference_uncertainty_mm;
};
std::vector<WheelObservation> readWheelObservations(const std::filesystem::path& csv_path);
// Weighted least squares on the selected 32-interval grid. Rank deficient
// records (e.g. only complete turns) cannot determine the phase map and fail.
// Observations used to fit must NOT double as independent validation data.
WheelCalibration fitWheelCalibration(std::span<const WheelObservation> observations);
struct WheelResidual {
    double predicted_mm,error_mm,guarded_error_mm,allocation_mm;
    bool within_allocation;
};
WheelResidual wheelCalibrationResidual(const WheelCalibration& calibration,const WheelObservation& observation);

// True travel when nominally scaled counts command the requested length.
// Factory tolerance does not include pressure/temperature/slip/wear.
double factoryWheelLengthError(double commanded_length_mm);
// Nominal encoder-rate equations; circumference must be the applicable actual
// or calibrated value. Manufacturer frequency is A/B cycles, not x4 edges.
struct WheelRates {double rpm,pulse_hz,quadrature_edge_hz;};
WheelRates wheelRates(double speed_mm_s,double circumference_mm);
// Piecewise-linear interpolation of e*sin(theta): absolute position bound
// e*h^2/8; a distance uses TWO endpoint errors. Not arbitrary tread lobing.
double eccentricMapDistanceInterpolationBound(double radial_tir_mm,std::size_t intervals);
// A source-defined CN example with no fit reduction: ring temperature alone
// can consume the small clearance. This is not a measured thermal field.
double measuringBearingThermalClearance(double initial_clearance_um,double inner_minus_outer_k);
}