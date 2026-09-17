#include "measurement_models.hpp"
#include <Eigen/QR>
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>

namespace starz::engineering {
namespace {
namespace c=measurement::catalogue;
void positive(double value,const char* name) {
    if(!std::isfinite(value)||value<=0)throw std::invalid_argument(name);
}
void nonnegative(double value,const char* name) {
    if(!std::isfinite(value)||value<0)throw std::invalid_argument(name);
}
struct CountPosition {std::int64_t turns;std::uint32_t phase;};
CountPosition splitCount(std::int64_t count) {
    constexpr std::int64_t n=c::counts_per_revolution;
    auto turns=count/n,phase=count%n;
    if(phase<0){phase+=n;--turns;}
    return {turns,static_cast<std::uint32_t>(phase)};
}
template<class T>T number(std::string_view text) {
    T result{};
    const auto parsed=std::from_chars(text.data(),text.data()+text.size(),result);
    if(text.empty()||parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size())
        throw std::invalid_argument("Invalid calibration CSV number");
    return result;
}
void validateObservation(const WheelObservation& observation) {
    if(!std::isfinite(observation.reference_mm))throw std::invalid_argument("Nonfinite reference travel");
    nonnegative(observation.reference_uncertainty_mm,"Invalid reference uncertainty");
}
Eigen::VectorXd phaseBasis(std::uint32_t phase) {
    constexpr auto intervals=measurement::design::calibration_intervals;
    constexpr auto width=measurement::design::counts_per_calibration_interval;
    Eigen::VectorXd result=Eigen::VectorXd::Zero(intervals);
    const auto cell=phase/width;
    const double fraction=static_cast<double>(phase%width)/width;
    // Node0 is fixed at zero; columns represent nodes1…32, including C.
    if(cell>0)result[cell-1]=1-fraction;
    result[cell]=fraction;
    return result;
}
}
WheelCalibration::WheelCalibration(std::vector<WheelCalibrationKnot> knots):knots_(std::move(knots)) {
    if(knots_.size()<2||knots_.size()>c::counts_per_revolution+1||knots_.front().count!=0||
       knots_.front().travel_mm!=0||knots_.back().count!=c::counts_per_revolution)
        throw std::invalid_argument("Calibration must span count0 to the counts-per-revolution end from travel0");
    min_pitch_=std::numeric_limits<double>::infinity();
    for(std::size_t i=0;i<knots_.size();++i) {
        if(!std::isfinite(knots_[i].travel_mm))throw std::invalid_argument("Nonfinite calibration travel");
        if(i==0)continue;
        const auto& a=knots_[i-1];const auto& b=knots_[i];
        if(b.count<=a.count||b.travel_mm<=a.travel_mm)
            throw std::invalid_argument("Calibration count and travel must be strictly increasing");
        const double pitch=(b.travel_mm-a.travel_mm)/(b.count-a.count);
        positive(pitch,"Invalid calibration local pitch");
        min_pitch_=std::min(min_pitch_,pitch);max_pitch_=std::max(max_pitch_,pitch);
    }
}
double WheelCalibration::circumferenceMm() const {return knots_.back().travel_mm;}
double WheelCalibration::minMmPerCount() const {return min_pitch_;}
double WheelCalibration::maxMmPerCount() const {return max_pitch_;}
double WheelCalibration::phaseTravel(std::uint32_t phase) const {
    const auto next=std::upper_bound(knots_.begin(),knots_.end(),phase,
        [](std::uint32_t value,const WheelCalibrationKnot& knot){return value<knot.count;});
    const auto& a=*(next-1);const auto& b=*next;
    const double ratio=static_cast<double>(phase-a.count)/(b.count-a.count);
    return std::lerp(a.travel_mm,b.travel_mm,ratio);
}
double WheelCalibration::distanceMm(std::int64_t from_count,std::int64_t to_count) const {
    const auto a=splitCount(from_count),b=splitCount(to_count);
    // Do not subtract raw int64 counters or two huge absolute floating-point
    // positions. For the fixed counts-per-revolution N the quotient
    // difference always fits int64.
    static_assert(c::counts_per_revolution>=4);
    const auto turns=b.turns-a.turns;
    const long double travel=static_cast<long double>(turns)*circumferenceMm()+
        (static_cast<long double>(phaseTravel(b.phase))-phaseTravel(a.phase));
    const double result=static_cast<double>(travel);
    if(!std::isfinite(result))throw std::overflow_error("Calibration distance overflow");
    return result;
}
WheelCalibration readWheelCalibration(const std::filesystem::path& path) {
    std::ifstream input(path);if(!input)throw std::runtime_error("Cannot open calibration CSV");
    std::string line;
    const auto readLine=[&]() {
        if(!std::getline(input,line))return false;
        if(!line.empty()&&line.back()=='\r')line.pop_back();
        return true;
    };
    if(!readLine()||line!="count,travel_mm")throw std::invalid_argument("Calibration CSV header must be count,travel_mm");
    std::vector<WheelCalibrationKnot> knots;
    while(readLine()) {
        if(knots.size()>=c::counts_per_revolution+1)throw std::invalid_argument("Too many calibration CSV rows");
        const auto separator=line.find(',');
        if(separator==std::string::npos||line.find(',',separator+1)!=std::string::npos)
            throw std::invalid_argument("Calibration CSV requires exactly two columns");
        const std::string_view row=line;
        knots.push_back({number<std::uint32_t>(row.substr(0,separator)),number<double>(row.substr(separator+1))});
    }
    if(!input.eof())throw std::runtime_error("Calibration CSV read failure");
    return WheelCalibration(std::move(knots));
}
std::vector<WheelObservation> readWheelObservations(const std::filesystem::path& path) {
    std::ifstream input(path);if(!input)throw std::runtime_error("Cannot open measurement observations CSV");
    std::string line;
    const auto readLine=[&]() {
        if(!std::getline(input,line))return false;
        if(!line.empty()&&line.back()=='\r')line.pop_back();
        return true;
    };
    if(!readLine()||line!="from_count,to_count,reference_mm,reference_uncertainty_mm")
        throw std::invalid_argument("Invalid measurement observations CSV header");
    std::vector<WheelObservation> result;
    while(readLine()) {
        if(result.size()>=65536)throw std::invalid_argument("Too many measurement observation rows");
        std::array<std::string_view,4> fields;
        std::string_view row=line;
        for(std::size_t i=0;i<fields.size();++i) {
            const auto comma=row.find(',');
            if((i+1<fields.size())!=(comma!=std::string_view::npos))
                throw std::invalid_argument("Measurement observations require exactly four columns");
            fields[i]=row.substr(0,comma);
            if(comma!=std::string_view::npos)row.remove_prefix(comma+1);
        }
        WheelObservation observation{number<std::int64_t>(fields[0]),number<std::int64_t>(fields[1]),
            number<double>(fields[2]),number<double>(fields[3])};
        validateObservation(observation);result.push_back(observation);
    }
    if(!input.eof())throw std::runtime_error("Measurement observations CSV read failure");
    if(result.empty())throw std::invalid_argument("Empty measurement observations");
    return result;
}
WheelCalibration fitWheelCalibration(std::span<const WheelObservation> observations) {
    constexpr Eigen::Index n=measurement::design::calibration_intervals;
    if(observations.size()<static_cast<std::size_t>(n)||observations.size()>65536)
        throw std::invalid_argument("Calibration fit requires32 to65536 observations");
    const auto rows=static_cast<Eigen::Index>(observations.size());
    Eigen::MatrixXd a(rows,n);Eigen::VectorXd b(rows);
    double minimum_uncertainty=std::numeric_limits<double>::infinity();
    for(const auto& observation:observations) {
        validateObservation(observation);
        positive(observation.reference_uncertainty_mm,"Fit observations require positive uncertainty");
        minimum_uncertainty=std::min(minimum_uncertainty,observation.reference_uncertainty_mm);
    }
    for(Eigen::Index row=0;row<rows;++row) {
        const auto& observation=observations[static_cast<std::size_t>(row)];
        const auto from=splitCount(observation.from_count),to=splitCount(observation.to_count);
        // Evaluate while both temporary basis vectors are alive. Saving this
        // Eigen expression with auto would retain dangling operand references.
        Eigen::VectorXd coefficients=phaseBasis(to.phase)-phaseBasis(from.phase);
        coefficients[n-1]+=static_cast<double>(to.turns-from.turns);
        const double weight=minimum_uncertainty/observation.reference_uncertainty_mm;
        a.row(row)=weight*coefficients.transpose();b[row]=weight*observation.reference_mm;
    }
    if(!a.allFinite()||!b.allFinite())throw std::invalid_argument("Nonfinite weighted calibration system");
    Eigen::ColPivHouseholderQR<Eigen::MatrixXd> qr(a);qr.setThreshold(1e-12);
    if(qr.rank()!=n)throw std::invalid_argument("Calibration observations do not resolve every phase-map degree of freedom");
    const Eigen::VectorXd solution=qr.solve(b);
    if(!solution.allFinite())throw std::runtime_error("Calibration fit returned nonfinite values");
    std::vector<WheelCalibrationKnot> knots{{0,0}};
    for(Eigen::Index i=0;i<n;++i)knots.push_back({static_cast<std::uint32_t>(i+1)*measurement::design::counts_per_calibration_interval,solution[i]});
    // A least-squares fit that folds backward is rejected, not silently clipped.
    return WheelCalibration(std::move(knots));
}
WheelResidual wheelCalibrationResidual(const WheelCalibration& calibration,const WheelObservation& observation) {
    validateObservation(observation);
    const double predicted=calibration.distanceMm(observation.from_count,observation.to_count);
    const double error=predicted-observation.reference_mm;
    const double guarded=std::abs(error)+observation.reference_uncertainty_mm;
    const double allocation=measurement::design::calibration_absolute_allocation_mm+
        measurement::design::calibration_relative_allocation*std::abs(observation.reference_mm);
    if(!std::isfinite(error)||!std::isfinite(guarded)||!std::isfinite(allocation))
        throw std::overflow_error("Calibration residual overflow");
    return {predicted,error,guarded,allocation,guarded<=allocation};
}
double factoryWheelLengthError(double commanded_length_mm) {
    nonnegative(commanded_length_mm,"Invalid commanded length");
    return commanded_length_mm*(c::circumference_tolerance_mm/c::circumference_mm);
}
WheelRates wheelRates(double speed_mm_s,double circumference_mm) {
    nonnegative(speed_mm_s,"Invalid wheel speed");positive(circumference_mm,"Invalid wheel circumference");
    const double turns_s=speed_mm_s/circumference_mm;
    const WheelRates result{60*turns_s,c::pulses_per_revolution*turns_s,c::counts_per_revolution*turns_s};
    if(!std::isfinite(result.rpm)||!std::isfinite(result.pulse_hz)||!std::isfinite(result.quadrature_edge_hz))
        throw std::overflow_error("Wheel rate overflow");
    return result;
}
double eccentricMapDistanceInterpolationBound(double radial_tir_mm,std::size_t intervals) {
    nonnegative(radial_tir_mm,"Invalid radial TIR");
    if(intervals<2)throw std::invalid_argument("At least two interpolation intervals required");
    const double step=2*std::numbers::pi/static_cast<double>(intervals);
    return (radial_tir_mm/2)*step*step/4;
}
double measuringBearingThermalClearance(double initial_clearance_um,double inner_minus_outer_k) {
    nonnegative(initial_clearance_um,"Invalid initial bearing clearance");
    nonnegative(inner_minus_outer_k,"Invalid bearing ring temperature difference");
    return initial_clearance_um-0.012*(c::bearing_d_mm+c::bearing_D_mm)/2*inner_minus_outer_k;
}
}