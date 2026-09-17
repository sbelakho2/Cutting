#include "measurement_models.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace c=starz::measurement::catalogue;
namespace d=starz::measurement::design;
void require(bool condition,const char* why){if(!condition)throw std::runtime_error(why);}
void close(double actual,double expected,double tolerance=1e-10) {
    require(std::abs(actual-expected)<=tolerance,"Measurement numeric mismatch");
}
template<class F>void rejects(F action){bool caught=false;try{action();}catch(const std::exception&){caught=true;}require(caught,"Invalid measurement input accepted");}
int main(int argc,char** argv)try {
    const se::WheelCalibration nominal({{0,0},{8192,100}});
    close(nominal.distanceMm(0,8192),100);close(nominal.distanceMm(8192,0),-100);
    close(nominal.distanceMm(-8193,8193),200+200.0/8192);
    close(nominal.distanceMm(-1,0),100.0/8192);close(nominal.maxMmPerCount(),100.0/8192);
    for(auto start:{std::numeric_limits<std::int64_t>::min(),std::numeric_limits<std::int64_t>::max()-10})
        close(nominal.distanceMm(start,start+10),1000.0/8192);
    require(std::isfinite(nominal.distanceMm(std::numeric_limits<std::int64_t>::min(),std::numeric_limits<std::int64_t>::max())),
        "Extreme signed counters overflowed");

    // A synthetic, independently evaluated rolling-distance function. This is
    // a numerical benchmark, not a manufacturer's profile or calibrated wheel.
    constexpr double C=100.07,e=0.1;
    const auto exact=[](std::int64_t count) {
        return C*static_cast<double>(count)/8192+e*std::sin(2*std::numbers::pi*static_cast<double>(count)/8192);
    };
    std::vector<se::WheelCalibrationKnot> knots;
    for(std::size_t i=0;i<=d::calibration_intervals;++i) {
        const auto count=static_cast<std::uint32_t>(i*d::counts_per_calibration_interval);
        knots.push_back({count,exact(count)});
    }
    const se::WheelCalibration calibrated(knots);
    const double bound=se::eccentricMapDistanceInterpolationBound(0.2,d::calibration_intervals);
    double max_error=0;
    for(std::int64_t phase=-8192;phase<8192;++phase) {
        for(std::int64_t span:{-8193LL,-8192LL,-1001LL,-1LL,0LL,1LL,1001LL,8192LL,8193LL,122880LL}) {
            const double distance=calibrated.distanceMm(phase,phase+span);
            max_error=std::max(max_error,std::abs(distance-(exact(phase+span)-exact(phase))));
            close(distance,-calibrated.distanceMm(phase+span,phase),1e-10);
        }
        close(calibrated.distanceMm(phase,phase+8192),C,1e-10);
        close(calibrated.distanceMm(phase,phase+123)+calibrated.distanceMm(phase+123,phase+2500),
              calibrated.distanceMm(phase,phase+2500),1e-10);
    }
    require(max_error<=bound+1e-10,"Sinusoidal map exceeded independent interpolation bound");
    for(auto count:{std::numeric_limits<std::int64_t>::min(),std::numeric_limits<std::int64_t>::max()-8192})
        close(calibrated.distanceMm(count,count+8192),C,1e-10);
    std::vector<se::WheelObservation> fitting;
    for(const auto& knot:knots)if(knot.count>0) {
        fitting.push_back({0,knot.count,knot.travel_mm,0.001});
        fitting.push_back({-8192,static_cast<std::int64_t>(knot.count)-8192,knot.travel_mm,0.002});
        fitting.push_back({knot.count,0,-knot.travel_mm,0.001});
    }
    const auto fitted=se::fitWheelCalibration(fitting);
    for(std::size_t i=0;i<knots.size();++i)close(fitted.knots()[i].travel_mm,knots[i].travel_mm,1e-10);
    for(std::int64_t phase=19;phase<8192;phase+=73) {
        const se::WheelObservation held_out{phase,phase+3007,exact(phase+3007)-exact(phase),0.001};
        const auto residual=se::wheelCalibrationResidual(fitted,held_out);
        require(residual.within_allocation,"Independent synthetic observation failed allocation");
        require(std::abs(residual.error_mm)<=bound+1e-10,"Independent synthetic fit error");
    }
    auto unobservable=fitting;
    for(auto& observation:unobservable)observation={0,8192,C,0.001};
    rejects([&]{se::fitWheelCalibration(unobservable);});
    auto invalid_weight=fitting;invalid_weight[0].reference_uncertainty_mm=0;
    rejects([&]{se::fitWheelCalibration(invalid_weight);});
    auto nonmonotonic=fitting;
    for(auto& observation:nonmonotonic)observation.reference_mm=-observation.reference_mm;
    rejects([&]{se::fitWheelCalibration(nonmonotonic);});
    require(!se::wheelCalibrationResidual(fitted,{0,8192,C+0.1,0.001}).within_allocation,"Large calibration residual was accepted");
    require(!se::wheelCalibrationResidual(fitted,{0,8192,C,0.04}).within_allocation,"Reference uncertainty guard was omitted");
    close(bound,0.1*std::pow(2*std::numbers::pi/32,2)/4);
    close(se::factoryWheelLengthError(1000),1.0);
    close(se::factoryWheelLengthError(200),0.2);
    require(se::factoryWheelLengthError(1000)>0.7,"Factory circumference tolerance must fail 1m length target alone");
    const auto rates=se::wheelRates(500,100);
    close(rates.rpm,300);close(rates.pulse_hz,10240);close(rates.quadrature_edge_hz,40960);
    close(se::wheelRates(0,100).rpm,0);
    close(se::measuringBearingThermalClearance(2,0),2);
    close(se::measuringBearingThermalClearance(2,10),0.5);
    require(se::measuringBearingThermalClearance(2,15)<0,"CN thermal-clearance failure was hidden");
    const double nan=std::numeric_limits<double>::quiet_NaN();
    rejects([]{se::WheelCalibration map({});});
    rejects([]{se::WheelCalibration map({{0,0},{8191,100}});});
    rejects([]{se::WheelCalibration map({{0,1},{8192,100}});});
    rejects([]{se::WheelCalibration map({{0,0},{2000,50},{2000,60},{8192,100}});});
    rejects([]{se::WheelCalibration map({{0,0},{3000,50},{2000,60},{8192,100}});});
    rejects([]{se::WheelCalibration map({{0,0},{2000,50},{3000,49},{8192,100}});});
    rejects([&]{se::WheelCalibration map({{0,0},{2000,nan},{8192,100}});});
    rejects([&]{se::factoryWheelLengthError(nan);});rejects([]{se::factoryWheelLengthError(-1);});
    rejects([]{se::wheelRates(500,0);});rejects([]{se::wheelRates(-1,100);});
    rejects([]{se::eccentricMapDistanceInterpolationBound(0.2,0);});
    rejects([]{se::measuringBearingThermalClearance(2,-1);});
    require(argc==2,"Pass a test artifact directory");
    const std::filesystem::path out=argv[1];std::filesystem::create_directories(out);
    const auto path=out/"calibration.csv";
    const auto write=[&](const char* content){std::ofstream file(path,std::ios::binary);file<<content;require(static_cast<bool>(file),"Test CSV write failed");};
    write("count,travel_mm\r\n0,0\r\n4096,50\r\n8192,100\r\n");
    close(se::readWheelCalibration(path).distanceMm(-1,8192),100+100.0/8192);
    for(const char* text:{"count,travel_mm\n0,0\n8192,100junk\n","count,travel_mm\n0,0\n8192,nan\n",
                         "count,travel_mm\n0,0\n8192,100,extra\n","count,travel_mm\n0,0\n\n8192,100\n",
                         "count,travel_mm\n0,0\n-1,2\n8192,100\n","bad_header\n0,0\n8192,100\n"}) {
        write(text);rejects([&]{se::readWheelCalibration(path);});
    }
    write("count,travel_mm\n0,0\n8192,100\n");
    write("from_count,to_count,reference_mm,reference_uncertainty_mm\r\n-8192,0,100,0.001\r\n");
    const auto observations=se::readWheelObservations(path);
    require(observations.size()==1&&observations.front().from_count==-8192,"Observation CSV readback failed");
    close(observations.front().reference_mm,100);
    for(const char* text:{"from_count,to_count,reference_mm,reference_uncertainty_mm\n0,8192,100,-1\n",
                         "from_count,to_count,reference_mm,reference_uncertainty_mm\n0,8192,nan,0.001\n",
                         "from_count,to_count,reference_mm,reference_uncertainty_mm\n0,8192,100\n",
                         "from_count,to_count,reference_mm,reference_uncertainty_mm\n0,8192,100,0.001,extra\n",
                         "from_count,to_count,reference_mm,reference_uncertainty_mm\n"}) {
        write(text);rejects([&]{se::readWheelObservations(path);});
    }
    std::cout<<"Measuring calibration, reversals, signed-counter extremes, strict CSV, source tolerances and thermal checks passed; synthetic maximum error "
             <<max_error<<" mm versus bound "<<bound<<" mm\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}