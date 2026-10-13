#pragma once
// Physical qualification of an assembled S1-B machine. Every constant here is
// an acceptance limit or a sampling requirement. No measured value exists in
// this repository: the tool fails closed on blank records and never reports
// a pass without complete, in-range, sufficiently sampled measurements.
//
// R2 (2026-10-13) adds: duplicate-sample rejection (a reused sample identity
// can never count as a distinct piece), per-record provenance
// (machine/build/recipe/calibration/gauge identities, temperature, operator
// slot), per-station force records (every nip and both measuring wheels, not
// one record per side) and a declared coverage statement whose uncovered
// scope is reported as physical remainder.
#include "shared/measurement_spec.hpp"
#include "shared/mechanism_spec.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace starz::engineering::qualification {
inline constexpr std::string_view revision="S1-B-QUAL-R2-2026-10-13";
// Cut-length acceptance: ±(0.05 % + 0.20 mm) → ±0.70 mm at 1000 mm, the S1
// design target in docs/02. It is a target the machine must demonstrate; the
// worst-case tension term cannot currently close it (docs/42 §1.4).
inline constexpr double length_abs_mm=0.20,length_rel=0.0005;
inline constexpr double max_length_gauge_u_mm=0.05;     // steel rule/scale with k=2 uncertainty
inline constexpr double max_force_gauge_u_n=1.0;
inline constexpr double min_temperature_c=15,max_temperature_c=30;
inline constexpr int min_samples_per_cell=30;           // distinct pieces per (wire OD, commanded length)
inline constexpr double min_cpk=1.33;
// Demonstrated coverage: both wire extremes at the shortest and a long length.
inline constexpr double required_wire_od_mm[]={1.2,4.5};
inline constexpr double required_length_mm[]={200,1000};
inline constexpr double od_match_mm=0.05,length_match_mm=0.5;
// Declared coverage R2: the machine's declared operating envelope is wider
// than the demonstrated cells below. These scopes are declared, not covered
// by the record formats here, and are reported as physical remainder — they
// must not be silently omitted from a qualification claim.
inline constexpr const char* declared_physical_remainder[]={
    "3000 mm length cell (declared range; only 200/1000 mm cells are demonstrated)",
    "strip cells (strip length/quality has no qualification cell)",
    "mixed OD/constructions (only OD 1.2/4.5 mm cells; stranded/solid classes not mixed)",
    "relaxed-length measurement (pieces must be measured free, not under feed tension)",
    "thermal boundary cells at 15/30 degC (records only bound 15-30 degC)"};
// Per-station force records (F32): each nip and each measuring wheel, not one
// reading per side. Cassette L/R, near/far nip per docs/12; wheels upstream
// and downstream per docs/13.
inline constexpr const char* required_force_stations[]={
    "NIP_L_NEAR","NIP_L_FAR","NIP_R_NEAR","NIP_R_FAR","WHEEL_UP","WHEEL_DOWN"};

struct LengthRecord {
    std::string sample_id;double wire_od_mm,commanded_mm,measured_mm;
    std::string gauge_id;double gauge_u_mm,temperature_c;
    std::string machine_id,build_id,recipe_id,calibration_id,operator_slot;
};
struct ForceRecord {
    std::string station;      // one of required_force_stations[]
    double wire_od_mm,force_n;
    std::string gauge_id;double gauge_u_n,temperature_c;
    std::string machine_id,build_id,recipe_id,calibration_id,operator_slot;
};
struct CellResult {
    double wire_od_mm,commanded_mm;int n;double mean_error_mm,sd_mm,tolerance_mm,cpk,max_abs_error_mm;bool pass;
};
struct ForceResult { std::string station;double wire_od_mm,force_n,lower_n,upper_n;bool pass; };
struct Report {
    std::vector<CellResult> cells;std::vector<ForceResult> forces;
    bool coverage_complete=false,all_pass=false,duplicates_rejected=false;
    int duplicate_records=0;
    std::vector<std::string> failures;
};

inline double lengthTolerance(double commanded_mm){return length_abs_mm+length_rel*commanded_mm;}
inline bool isRequiredStation(const std::string& s) {
    for(const char* st:required_force_stations)if(s==st)return true;
    return false;
}
inline void requireProvenance(const std::string& machine_id,const std::string& build_id,
    const std::string& recipe_id,const std::string& calibration_id,const std::string& operator_slot,
    const std::string& id) {
    if(machine_id.empty())throw std::invalid_argument(id+" missing provenance: machine_id");
    if(build_id.empty())throw std::invalid_argument(id+" missing provenance: build_id");
    if(recipe_id.empty())throw std::invalid_argument(id+" missing provenance: recipe_id");
    if(calibration_id.empty())throw std::invalid_argument(id+" missing provenance: calibration_id");
    if(operator_slot.empty())throw std::invalid_argument(id+" missing provenance: operator_slot");
}

inline void validate(const LengthRecord& r) {
    if(r.sample_id.empty()||r.gauge_id.empty())throw std::invalid_argument("Length record missing sample or gauge identity");
    for(double v:{r.wire_od_mm,r.commanded_mm,r.measured_mm,r.gauge_u_mm,r.temperature_c})
        if(!std::isfinite(v))throw std::invalid_argument("Length record "+r.sample_id+" has a blank or non-numeric field");
    if(r.wire_od_mm<1.2-od_match_mm||r.wire_od_mm>4.5+od_match_mm)throw std::invalid_argument("Length record "+r.sample_id+" wire OD outside 1.2–4.5 mm");
    if(r.commanded_mm<200||r.commanded_mm>20000)throw std::invalid_argument("Length record "+r.sample_id+" commanded length outside 200–20000 mm");
    if(r.measured_mm<=0)throw std::invalid_argument("Length record "+r.sample_id+" measured length not positive");
    if(r.gauge_u_mm<=0||r.gauge_u_mm>max_length_gauge_u_mm)throw std::invalid_argument("Length record "+r.sample_id+" gauge uncertainty outside (0,0.05] mm");
    if(r.temperature_c<min_temperature_c||r.temperature_c>max_temperature_c)throw std::invalid_argument("Length record "+r.sample_id+" temperature outside 15–30 °C");
    requireProvenance(r.machine_id,r.build_id,r.recipe_id,r.calibration_id,r.operator_slot,r.sample_id);
}
inline void validate(const ForceRecord& r) {
    if(!isRequiredStation(r.station))throw std::invalid_argument("Force record station must be one of the required nip/wheel stations");
    if(r.gauge_id.empty())throw std::invalid_argument("Force record missing gauge identity");
    for(double v:{r.wire_od_mm,r.force_n,r.gauge_u_n,r.temperature_c})if(!std::isfinite(v))throw std::invalid_argument("Force record "+r.station+" has a blank or non-numeric field");
    if(r.wire_od_mm<1.2-od_match_mm||r.wire_od_mm>4.5+od_match_mm)throw std::invalid_argument("Force record "+r.station+" wire OD outside 1.2–4.5 mm");
    if(r.gauge_u_n<=0||r.gauge_u_n>max_force_gauge_u_n)throw std::invalid_argument("Force record "+r.station+" gauge uncertainty outside (0,1] N");
    if(r.temperature_c<min_temperature_c||r.temperature_c>max_temperature_c)throw std::invalid_argument("Force record "+r.station+" temperature outside 15–30 °C");
    requireProvenance(r.machine_id,r.build_id,r.recipe_id,r.calibration_id,r.operator_slot,r.station);
}

inline Report evaluate(const std::vector<LengthRecord>& lengths,const std::vector<ForceRecord>& forces) {
    if(lengths.empty()||forces.empty())throw std::invalid_argument("No measurements: qualification cannot be evaluated");
    for(const auto& r:lengths)validate(r);
    for(const auto& r:forces)validate(r);
    Report rep;
    // Duplicate sample identities: a reused sample id is not a distinct
    // piece. The first record of each id counts once; every repeat is
    // recorded as a failure and can never inflate a cell's n.
    std::map<std::string,int> sample_count;
    for(const auto& r:lengths)++sample_count[r.sample_id];
    for(const auto& [id,count]:sample_count)if(count>1){
        rep.duplicates_rejected=true;rep.duplicate_records+=count-1;
        rep.failures.push_back("Duplicate sample identity "+id+" used "+std::to_string(count)+" times - a reused sample cannot count as a distinct piece");
    }
    // A campaign must be one machine, one build, one recipe: mixed provenance
    // is not a qualified data set.
    bool mixed=false;
    for(const auto& r:lengths){
        if(r.machine_id!=lengths.front().machine_id){mixed=true;rep.failures.push_back("Mixed machine_id in length records: "+r.machine_id);}
        if(r.build_id!=lengths.front().build_id){mixed=true;rep.failures.push_back("Mixed build_id in length records: "+r.build_id);}
        if(r.recipe_id!=lengths.front().recipe_id){mixed=true;rep.failures.push_back("Mixed recipe_id in length records: "+r.recipe_id);}
    }
    for(const auto& r:forces){
        if(r.machine_id!=lengths.front().machine_id){mixed=true;rep.failures.push_back("Mixed machine_id in force records: "+r.machine_id);}
        if(r.build_id!=lengths.front().build_id){mixed=true;rep.failures.push_back("Mixed build_id in force records: "+r.build_id);}
        if(r.recipe_id!=lengths.front().recipe_id){mixed=true;rep.failures.push_back("Mixed recipe_id in force records: "+r.recipe_id);}
    }
    std::map<std::pair<long,long>,std::vector<double>> cells;
    std::set<std::string> counted;
    for(const auto& r:lengths)if(counted.insert(r.sample_id).second)
        cells[{std::lround(r.wire_od_mm*10),std::lround(r.commanded_mm)}].push_back(r.measured_mm-r.commanded_mm);
    for(const auto& [key,errors]:cells) {
        CellResult c{key.first/10.0,double(key.second),int(errors.size()),0,0,lengthTolerance(double(key.second)),0,0,false};
        for(double e:errors){c.mean_error_mm+=e;c.max_abs_error_mm=std::max(c.max_abs_error_mm,std::abs(e));}
        c.mean_error_mm/=c.n;
        for(double e:errors)c.sd_mm+=(e-c.mean_error_mm)*(e-c.mean_error_mm);
        c.sd_mm=c.n>1?std::sqrt(c.sd_mm/(c.n-1)):0;
        // Guard band: the gauge uncertainty of the worst gauge in the cell is subtracted from the tolerance.
        double u=0;for(const auto& r:lengths)if(counted.count(r.sample_id)&&std::lround(r.wire_od_mm*10)==key.first&&std::lround(r.commanded_mm)==key.second)u=std::max(u,r.gauge_u_mm);
        const double t=c.tolerance_mm-u;
        c.cpk=c.sd_mm>0?(t-std::abs(c.mean_error_mm))/(3*c.sd_mm):0;
        c.pass=c.n>=min_samples_per_cell&&c.sd_mm>0&&c.max_abs_error_mm<=t&&c.cpk>=min_cpk;
        if(!c.pass)rep.failures.push_back("Length cell OD "+std::to_string(c.wire_od_mm)+" L "+std::to_string(c.commanded_mm)+": n="+std::to_string(c.n)+" max|e|="+std::to_string(c.max_abs_error_mm)+" Cpk="+std::to_string(c.cpk));
        rep.cells.push_back(c);
    }
    bool covered=true;
    for(double od:required_wire_od_mm)for(double L:required_length_mm) {
        const bool found=std::any_of(rep.cells.begin(),rep.cells.end(),[&](const CellResult& c){return std::abs(c.wire_od_mm-od)<=od_match_mm&&std::abs(c.commanded_mm-L)<=length_match_mm;});
        if(!found){covered=false;rep.failures.push_back("Missing required cell OD "+std::to_string(od)+" L "+std::to_string(L));}
    }
    std::map<std::string,std::map<long,bool>> force_seen;
    for(const auto& r:forces) {
        ForceResult f{r.station,r.wire_od_mm,r.force_n,0,0,false};
        if(r.station=="WHEEL_UP"||r.station=="WHEEL_DOWN"){f.lower_n=mechanism::design::wheel_force_min_n;f.upper_n=mechanism::design::wheel_force_max_n;}
        else{f.lower_n=mechanism::design::nip_force_min_n;f.upper_n=mechanism::design::nip_force_max_n;}
        f.pass=r.force_n-r.gauge_u_n>=f.lower_n&&r.force_n+r.gauge_u_n<=f.upper_n;
        if(!f.pass)rep.failures.push_back("Force "+r.station+" OD "+std::to_string(r.wire_od_mm)+": "+std::to_string(r.force_n)+" N outside guarded window");
        force_seen[r.station][std::lround(r.wire_od_mm*10)]=true;rep.forces.push_back(f);
    }
    for(const char* st:required_force_stations)for(double od:required_wire_od_mm)
        if(!force_seen[st].count(std::lround(od*10))){covered=false;rep.failures.push_back(std::string("Missing force record ")+st+" at OD "+std::to_string(od));}
    rep.coverage_complete=covered;
    rep.all_pass=covered&&!rep.duplicates_rejected&&!mixed&&
                 std::all_of(rep.cells.begin(),rep.cells.end(),[](const CellResult& c){return c.pass;})&&
                 std::all_of(rep.forces.begin(),rep.forces.end(),[](const ForceResult& f){return f.pass;});
    return rep;
}
}
