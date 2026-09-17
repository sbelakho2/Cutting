#include "measurement_models.hpp"
#include "shared/feed_drive_spec.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <string>

namespace se=starz::engineering;
namespace fs=std::filesystem;
namespace c=starz::measurement::catalogue;
namespace d=starz::measurement::design;
namespace {
bool sameFile(const fs::path& a,const fs::path& b) {
    return fs::weakly_canonical(a)==fs::weakly_canonical(b)||
        (fs::exists(a)&&fs::exists(b)&&fs::equivalent(a,b));
}
void writeMap(const se::WheelCalibration& map,const fs::path& path) {
    std::ofstream file(path);file<<std::setprecision(17)<<"count,travel_mm\n";
    for(const auto& knot:map.knots())file<<knot.count<<','<<knot.travel_mm<<'\n';
    file.flush();
    if(!file)throw std::runtime_error("Cannot write calibration map");
}
void writeObservations(const std::vector<se::WheelObservation>& data,const fs::path& path) {
    std::ofstream file(path);file<<std::setprecision(17)<<"from_count,to_count,reference_mm,reference_uncertainty_mm\n";
    for(const auto& row:data)file<<row.from_count<<','<<row.to_count<<','<<row.reference_mm<<','<<row.reference_uncertainty_mm<<'\n';
    file.flush();
    if(!file)throw std::runtime_error("Cannot write synthetic observations");
}
}
int main(int argc,char** argv)try {
    fs::path out,fit_path,map_path,verification_path;
    bool strict_factory=false;
    for(int i=1;i<argc;++i) {
        const std::string argument=argv[i];
        if(argument=="--require-factory-accuracy")strict_factory=true;
        else {
            fs::path* target=nullptr;
            if(argument=="--report")target=&out;
            else if(argument=="--fit-observations")target=&fit_path;
            else if(argument=="--calibration")target=&map_path;
            else if(argument=="--verify-observations")target=&verification_path;
            else throw std::invalid_argument("Unknown measurement analysis argument");
            if(!target->empty()||i+1>=argc)throw std::invalid_argument("Duplicate argument or missing measurement path");
            *target=argv[++i];
        }
    }
    if(out.empty())throw std::invalid_argument("Use --report output_directory");
    if(!fit_path.empty()&&!map_path.empty())throw std::invalid_argument("Use either observations to fit or an existing calibration map");
    const bool user_data=!fit_path.empty()||!map_path.empty();
    if(user_data!=!verification_path.empty())throw std::invalid_argument("A supplied calibration requires separate verification observations");
    if(!fit_path.empty()&&sameFile(fit_path,verification_path))
        throw std::invalid_argument("Fitting and verification must not use the same file");

    // Inputs and CLI are checked BEFORE marking an output run as pending.
    std::optional<se::WheelCalibration> input_map;
    std::vector<se::WheelObservation> verification;
    if(user_data) {
        input_map=fit_path.empty()?se::readWheelCalibration(map_path):se::fitWheelCalibration(se::readWheelObservations(fit_path));
        verification=se::readWheelObservations(verification_path);
    }
    const auto output_map=out/(user_data?"input-derived-map.csv":"synthetic-map.csv");
    for(const auto& input:{fit_path,map_path,verification_path})if(!input.empty()) {
        for(const auto& name:{"report.md","status.json","checks.csv","raw-error-budget.csv","validation.csv",
                              "synthetic-map.csv","synthetic-fit.csv","synthetic-validation.csv","input-derived-map.csv"})
            if(sameFile(input,out/name))
                throw std::invalid_argument("Output would overwrite a measurement input file");
    }
    fs::create_directories(out);
    {std::ofstream pending(out/"status.json");pending<<"{\"run_complete\":false,\"physical_calibration_qualified\":false,\"fabrication_released\":false}\n";
        if(!pending)throw std::runtime_error("Cannot initialise measurement status");}
    // Separately named SYNTHETIC fixtures exercise both fit and validation
    // paths. They must never be loaded as commissioned hardware calibration.
    if(!user_data) {
        constexpr double circumference=200.14,eccentricity=0.1;
        const auto exact=[](std::int64_t count) {
            return circumference*static_cast<double>(count)/c::counts_per_revolution+
                eccentricity*std::sin(2*std::numbers::pi*static_cast<double>(count)/c::counts_per_revolution);
        };
        std::vector<se::WheelObservation> fit;
        for(std::uint32_t phase=d::counts_per_calibration_interval;phase<=c::counts_per_revolution;phase+=d::counts_per_calibration_interval) {
            fit.push_back({0,phase,exact(phase),0.001});fit.push_back({phase,0,-exact(phase),0.001});
        }
        input_map=se::fitWheelCalibration(fit);
        for(std::int64_t phase=19;phase<c::counts_per_revolution;phase+=73)for(std::int64_t span:{-122880LL,-8193LL,-1001LL,1LL,1001LL,8193LL,122880LL})
            verification.push_back({phase,phase+span,exact(phase+span)-exact(phase),0.001});
        writeObservations(fit,out/"synthetic-fit.csv");writeObservations(verification,out/"synthetic-validation.csv");
    }
    const auto& map=*input_map;writeMap(map,output_map);
    std::ofstream checks(out/"checks.csv"),budget(out/"raw-error-budget.csv"),residuals(out/"validation.csv"),report(out/"report.md");
    checks<<std::setprecision(12)<<"check,value,limit,unit,result,basis\n";
    budget<<std::setprecision(12)<<"commanded_length_mm,factory_scale_error_mm,runout_allocation_mm,quantisation_allocation_mm,tension_allocation_mm,slip_allocation_mm,bounded_total_mm,target_mm,result\n";
    residuals<<std::setprecision(12)<<"from_count,to_count,reference_mm,predicted_mm,error_mm,reference_uncertainty_mm,guarded_error_mm,allocation_mm,result\n";
    int check_count=0,design_failures=0,validation_failures=0;
    const auto check=[&](const char* name,double value,double limit,const char* unit,bool pass,const char* basis) {
        if(!std::isfinite(value)||!std::isfinite(limit))throw std::runtime_error("Nonfinite measurement check");
        checks<<name<<','<<value<<','<<limit<<','<<unit<<','<<(pass?"PASS_SCOPED":"FAIL")<<",\""<<basis<<"\"\n";
        ++check_count;if(!pass)++design_failures;
    };
    const auto rates=se::wheelRates(starz::feed::design::feed_limit_m_s*1000,c::circumference_mm-c::circumference_tolerance_mm);
    check("encoder_speed_factory_min_circumference",rates.rpm,c::encoder_speed_limit_rpm,"rpm",rates.rpm<=c::encoder_speed_limit_rpm,"199.8mm assumed minimum circumference;500mm/s;not loaded circumference");
    check("encoder_A_B_frequency",rates.pulse_hz,c::encoder_pulse_limit_hz,"Hz",rates.pulse_hz<=c::encoder_pulse_limit_hz,"2048A/B_cycles_per_revolution_IO-Link_setting;ifm1000kHz_switching_frequency;do_not_compare_x4_edges_as_A/B_pulses");
    check("nominal_count_spacing",200.0/c::counts_per_revolution,0.025,"mm",200.0/c::counts_per_revolution<=0.025,"2048PPR halves the count spacing;0.025mm quantisation allocation covers half-count0.0122");
    const double factory_error=se::factoryWheelLengthError(1000),cn_clearance=se::measuringBearingThermalClearance(2,10);
    check("factory_scale_error_1m",factory_error,0.7,"mm",factory_error<=0.7,"Assumed200plusminus0.2mm;ifm publishes no tolerance;factory_scale_alone_exceeds_entire1m_target");
    check("CN_10K_ring_clearance_before_fit",cn_clearance,1,"um",cn_clearance>=1,"SourceCNminimum2um;steel12um/m/K;no_interference_loss_yet;thermal_and_fits_unresolved");
    for(double length:{200.0,500.0,1000.0,3000.0}) {
        const double factory=se::factoryWheelLengthError(length),total=factory+0.2+0.025+0.15+0.25,target=0.2+0.0005*length;
        budget<<length<<','<<factory<<",0.2,0.025,0.15,0.25,"<<total<<','<<target<<','<<(total<=target?"PASS_SCOPED":"FAIL")<<'\n';
    }
    double max_error=0,max_guarded=0;bool forward=false,reverse=false;
    for(const auto& observation:verification) {
        const auto result=se::wheelCalibrationResidual(map,observation);
        if(!result.within_allocation)++validation_failures;
        max_error=std::max(max_error,std::abs(result.error_mm));max_guarded=std::max(max_guarded,result.guarded_error_mm);
        forward=forward||observation.to_count>observation.from_count;reverse=reverse||observation.to_count<observation.from_count;
        residuals<<observation.from_count<<','<<observation.to_count<<','<<observation.reference_mm<<','<<result.predicted_mm<<','<<result.error_mm<<','
            <<observation.reference_uncertainty_mm<<','<<result.guarded_error_mm<<','<<result.allocation_mm<<','<<(result.within_allocation?"PASS_RESIDUAL":"FAIL")<<'\n';
    }
    report<<std::setprecision(10)<<"# Measuring-wheel interfaces and calibration model\n\nRevision **"<<starz::measurement::revision<<"**.\n\n"
        <<"## Source corrections\n\nMeasuring wheel **"<<c::wheel<<"** (ifm E60137 \"U:200 N:06\", substituted 2026-09-16 for the unpriced Kübler 8.0000.3123.0006): "
        <<"**200 mm** circumference, **Ø63.66 mm**, Ø6 bore, aluminium wheel with PU surface **90 Shore A**, 64.5 g. ifm publishes no circumference/OD "
        <<"tolerance and no hub/clamping-screw dimensions; the 0.1% (0.2 mm) tolerance and the retained 9.5 mm tread / 14.5 mm overall / Ø16.5 mm hub / Ø9 mm "
        <<"end-land profile stations are assumed placeholders pending the manufacturer drawing at incoming inspection. The four M4×6 radial clamp openings "
        <<"are a retained envelope, not a published ifm screw callout.\n\n"
        <<"Encoder **"<<c::encoder<<"** (ifm RU3100, substituted 2026-09-16 for the unpriced Autonics E40S6-1024-6-L-5): body **Ø58.6×63.5 mm** behind the "
        <<"mounting face, solid **Ø6×10 mm** shaft with no flat, tip **10 mm from the mounting face**. Synchro-flange fixing with **3×M4×0.7** tapped 6 mm "
        <<"deep on **PCD42**; the archived drawing front view labels only Ø42 and Ø58.6, so no pilot is modeled and the plate's Ø36.5-class seat bore "
        <<"follows the ifm mounting practice (confirm the centering feature at receipt). 4.75–30 VDC, HTL/TTL push-pull, factory 1024 PPR "
        <<"parameterised to 2048 PPR via IO-Link (programmable 1…10000), 1000 kHz switching frequency, 12000 rpm, starting torque ≤0.01 Nm, "
        <<"M12 5-pole connector.\n\n"
        <<"## Uncalibrated length and bearing gates\n\nThe factory circumference alone gives **1.0 mm error per commanded metre** (assumed ±0.2 mm on "
        <<"200 mm; ifm publishes no tolerance). "
        <<"Adding the previous0.2 runout,0.05 quantisation,0.15 tension and0.25 slip allocations gives **1.65 mm**, not the earlier1.15 mm. "
        <<"Those four terms are allocations, not measured errors; this is a failing uncalibrated budget, not a worst-case physical guarantee. "
        <<"The separate factory diameter tolerance is not treated as an additional independent scale error. The 2048 PPR IO-Link setting gives "
        <<"0.024414 mm/count (0.0122 mm half-count); the quantisation allocation reduces from0.05 to0.025 mm and still covers the half-count error.\n\n"
        <<"SKF626-2Z CN minimum clearance2 µm leaves only **0.5 µm** with10 K inner-to-outer ring difference, even before a press fit. "
        <<"The chosen CN bearing is not silently accepted with full js5 interference. Its actual matching/thermal design remains open.\n\n"
        <<"At500 mm/s and199.8 mm circumference: "<<rates.rpm<<" rpm, "<<rates.pulse_hz<<" A/B cycles/s and "<<rates.quadrature_edge_hz
        <<" quadrature edges/s. The1000 kHz encoder limit is a pulse-frequency specification; MCU edge capture and electrical receivers are separate work.\n\n"
        <<"## Reversible phase calibration\n\nThe mapping is C times signed whole turns plus a strictly increasing one-turn distance table. "
        <<"Negative counts use floor division; reversal, rollover across the8192-phase boundary and int64 endpoint extremes are tested. "
        <<"Differences are computed without subtracting two large floating-point positions or overflowing signed raw counters. "
        <<"The32-interval weighted least-squares fitter rejects rank deficiency and nonmonotone solutions. Complete-turn measurements alone cannot identify phase error.\n\n"
        <<"Counts must be unwrapped and referred to the calibrated Z-index phase. Missing index, changed encoder/wheel mounting, stale samples, "
        <<"lost edges or changed wire/contact conditions invalidate use. This engineering library does not create those hardware assurances or enable motion.\n\n"
        <<"The chosen metrology allocation is **0.025 mm +0.01% of reference travel**. Each supplied validation observation is checked using "
        <<"|predicted-reference| plus the supplied absolute reference-uncertainty bound. This is one error-budget allocation, not total machine accuracy. "
        <<"A statistical expanded uncertainty is not an absolute guarantee; its coverage must be recorded. No repeatability, coverage or traceability is inferred from a CSV.\n\n"
        <<"## "<<(user_data?"Input-derived numerical validation":"SYNTHETIC benchmark — NOT machine calibration")<<"\n\n"
        <<"Circumference from the map: **"<<map.circumferenceMm()<<" mm**. Local pitch "<<map.minMmPerCount()<<"–"<<map.maxMmPerCount()<<" mm/count. "
        <<verification.size()<<" verification observations; "<<validation_failures<<" allocation failures; max absolute residual **"<<max_error
        <<" mm**, max guarded residual **"<<max_guarded<<" mm**. Forward records: "<<(forward?"yes":"no")<<"; reverse: "<<(reverse?"yes":"no")<<".\n\n"
        <<"For the synthetic first-harmonic0.2 mm TIR profile only, the independent linear-interpolation distance bound is **"
        <<se::eccentricMapDistanceInterpolationBound(0.2,32)<<" mm**. Arbitrary tread lobing, contact deformation, slip and time-varying errors do not inherit this bound. "
        <<"Synthetic files are labelled explicitly and may not be loaded as production calibration.\n\n"
        <<"## Next mechanical interfaces\n\nThe wheel/encoder/626 interfaces are transcribed separately from unprovided OEM interiors. "
        <<"Measuring shaft, bearing housings, pressure arm, wire guides and support attachment must be completed before these become an installed module. "
        <<"The supplied encoder coupling still needs its actual hub/flexure engagement and tightening information. "
        <<"No spring load is applied directly to the encoder shaft. No personnel-safety function depends on this calibration.\n";
    checks.flush();budget.flush();residuals.flush();report.flush();
    if(!checks||!budget||!residuals||!report)throw std::runtime_error("Measurement report write failure");
    std::ofstream status(out/"status.json");
    status<<"{\n  \"revision\":\""<<starz::measurement::revision<<"\",\n  \"run_complete\":true,\n  \"input_data_kind\":\""
          <<(user_data?"user_supplied_unqualified":"synthetic_benchmark")<<"\",\n  \"source_checks\":"<<check_count<<",\n  \"unresolved_source_checks\":"<<design_failures
          <<",\n  \"validation_observations\":"<<verification.size()<<",\n  \"validation_residual_failures\":"<<validation_failures
          <<",\n  \"physical_calibration_qualified\":false,\n  \"hardware_capture_integrated\":false,\n  \"measurement_module_integrated\":false,\n  \"fabrication_released\":false\n}\n";
    status.flush();if(!status)throw std::runtime_error("Measurement status write failure");
    std::cout<<"Measurement report: "<<check_count<<" source checks; "<<design_failures<<" unresolved; "<<verification.size()<<" "
             <<(user_data?"input-derived":"SYNTHETIC")<<" validation observations; "<<validation_failures<<" residual failures\n";
    return validation_failures||(strict_factory&&design_failures)?2:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}