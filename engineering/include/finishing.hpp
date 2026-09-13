#pragma once
// Precision finishing process definitions for the S1-B custom parts.
// Every number here is a process setting or an acceptance limit that the shop
// must meet and record. Nothing here is a measured result.
#include "bearing_fit.hpp"
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace starz::engineering::finishing {
inline constexpr std::string_view revision="S1-B-FINISH-R1-2026-09-13";
// Linear expansion, µm per mm per K. Same steel value as bearing_fit.hpp.
inline constexpr double alpha_steel_um_mm_k=0.012;
inline constexpr double alpha_6082_um_mm_k=0.0234;
inline constexpr double reference_temperature_c=20.0;
// Gauging: ISO14253-1 guard band equal to the expanded uncertainty U(k=2).
inline constexpr double max_seat_gauge_u_um=1.0;      // comparator/air gauge for j5 seats
inline constexpr double max_bore_gauge_u_um=2.0;      // three-point bore gauge, F102/F112
inline constexpr double max_journal_gauge_u_um=2.0;   // 10h6 roller journal micrometer/comparator
inline constexpr double min_usable_window_um=1.0;     // after guard bands
// Cylindrical grinding of 42CrMo4+QT (≈300HB) with an ISO525 A60K5V vitrified
// aluminium-oxide wheel, water-soluble coolant, part between centres.
inline constexpr double grind_stock_diametral_mm=0.30;
inline constexpr double pre_grind_tolerance_mm=0.05;          // turned Ø(nom+0.30)±0.05
inline constexpr double rough_pass_diametral_mm=0.020;
inline constexpr double rough_stop_remaining_mm=0.040;
inline constexpr double finish_pass_diametral_mm=0.005;
inline constexpr double finish_stop_remaining_mm=0.010;
inline constexpr double final_pass_diametral_mm=0.002;
inline constexpr int spark_out_passes=2;
inline constexpr double seat_cylindricity_um=1.0,seat_roundness_um=0.6,seat_ra_um=0.4;
inline constexpr double seat_runout_to_centres_um=2.0;
// Fine boring of the 6082-T6 housings on the 4-axis CNC with a micrometric
// boring head; one finishing pass of fixed radial depth after a gauged semi-finish.
inline constexpr double bore_semi_finish_undersize_mm=0.15;   // diametral
inline constexpr double bore_finish_pass_diametral_mm=0.15;
inline constexpr double bore_cylindricity_um=4.0,bore_ra_um=0.8;
inline constexpr double bore_coaxiality_F102_F112_um=10.0;   // assembled, dowelled
// Surface grinding of H003 (15.00±0.02) and H001 (flatness 0.03/300).
inline constexpr double face_grind_stock_mm=0.30,face_parallelism_um=10.0,face_ra_um=0.8;

inline double toReference(double size_mm,double measured_at_c,double alpha) {
    if(!std::isfinite(size_mm)||!std::isfinite(measured_at_c)||measured_at_c<15||measured_at_c>30)
        throw std::invalid_argument("Measurement temperature outside the 15–30°C record");
    return size_mm-size_mm*alpha*(measured_at_c-reference_temperature_c)/1000.0;
}
struct SeatRecord {
    std::string shaft_serial,seat;      // seat: "S22.5-30.5_d12" or "S73-82_d15"
    double nominal_mm;
    std::string bearing_serial;
    double bearing_bore_mm,bearing_temperature_c;
    std::string gauge_id;double gauge_u_um;
};
struct GrindCard {
    SeatRecord in;
    double bearing_bore_20c_mm,target_mm,accept_lower_mm,accept_upper_mm,guarded_lower_mm,guarded_upper_mm;
    double pre_grind_lower_mm,pre_grind_upper_mm;
    int rough_passes,finish_passes,final_passes;
    double interference_lower_um,interference_upper_um;
};
inline GrindCard grindCard(const SeatRecord& r) {
    if(r.shaft_serial.empty()||r.bearing_serial.empty()||r.gauge_id.empty())
        throw std::invalid_argument("Grind card requires shaft, bearing and gauge identities");
    if(!std::isfinite(r.gauge_u_um)||r.gauge_u_um<=0||r.gauge_u_um>max_seat_gauge_u_um)
        throw std::invalid_argument("Seat gauge U(k=2) must be within (0,"+std::to_string(max_seat_gauge_u_um)+"]µm");
    const double bore20=toReference(r.bearing_bore_mm,r.bearing_temperature_c,alpha_steel_um_mm_k);
    const auto m=matchedJ5Seat(r.nominal_mm,bore20);
    const double gl=m.lower_mm+r.gauge_u_um/1000,gu=m.upper_mm-r.gauge_u_um/1000;
    if(gu-gl<min_usable_window_um/1000-1e-12)
        throw std::runtime_error("Guarded acceptance window below "+std::to_string(min_usable_window_um)+"µm for "+r.shaft_serial+" "+r.seat);
    const double target=(gl+gu)/2;
    // Pass schedule from the upper pre-grind limit so the worst blank still finishes.
    const double stock=r.nominal_mm+grind_stock_diametral_mm+pre_grind_tolerance_mm-target;
    const int rough=static_cast<int>(std::ceil((stock-rough_stop_remaining_mm)/rough_pass_diametral_mm));
    const int finish=static_cast<int>(std::ceil((rough_stop_remaining_mm-finish_stop_remaining_mm)/finish_pass_diametral_mm));
    const int fin=static_cast<int>(std::ceil(finish_stop_remaining_mm/final_pass_diametral_mm));
    return {r,bore20,target,m.lower_mm,m.upper_mm,gl,gu,
            r.nominal_mm+grind_stock_diametral_mm-pre_grind_tolerance_mm,
            r.nominal_mm+grind_stock_diametral_mm+pre_grind_tolerance_mm,
            rough,finish,fin,(target-bore20)*1000-r.gauge_u_um,(target-bore20)*1000+r.gauge_u_um};
}
// Assembly-level acceptance of a finished seat: the measured shaft size must lie
// inside the guarded window at 20°C, else the shaft is regraded or rejected.
inline bool acceptFinishedSeat(const GrindCard& c,double measured_shaft_mm,double shaft_temperature_c) {
    const double s20=toReference(measured_shaft_mm,shaft_temperature_c,alpha_steel_um_mm_k);
    return s20>=c.guarded_lower_mm-1e-12&&s20<=c.guarded_upper_mm+1e-12;
}
struct BoreRecord {
    std::string part_serial,part;   // "F102" or "F112"
    double lower_mm,upper_mm;       // drawing limits
    std::string gauge_id;double gauge_u_um;
};
struct BoreCard {
    BoreRecord in;double guarded_lower_mm,guarded_upper_mm,target_mm,semi_finish_target_mm;
};
inline BoreCard boreCard(const BoreRecord& r) {
    if(r.part_serial.empty()||r.gauge_id.empty())throw std::invalid_argument("Bore card requires part and gauge identities");
    if(!std::isfinite(r.gauge_u_um)||r.gauge_u_um<=0||r.gauge_u_um>max_bore_gauge_u_um)
        throw std::invalid_argument("Bore gauge U(k=2) must be within (0,"+std::to_string(max_bore_gauge_u_um)+"]µm");
    if(!(r.upper_mm>r.lower_mm))throw std::invalid_argument("Bore limits inverted");
    const double gl=r.lower_mm+r.gauge_u_um/1000,gu=r.upper_mm-r.gauge_u_um/1000;
    if(gu-gl<min_usable_window_um/1000-1e-12)throw std::runtime_error("Guarded bore window too small for "+r.part_serial);
    const double target=(gl+gu)/2;
    return {r,gl,gu,target,target-bore_semi_finish_undersize_mm};
}
inline bool acceptFinishedBore(const BoreCard& c,double measured_mm,double temperature_c) {
    const double m20=toReference(measured_mm,temperature_c,alpha_6082_um_mm_k);
    return m20>=c.guarded_lower_mm-1e-12&&m20<=c.guarded_upper_mm+1e-12;
}
}
