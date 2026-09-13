#pragma once
// Physical qualification of an assembled S1-B machine. Every constant here is
// an acceptance limit or a sampling requirement. No measured value exists in
// this repository: the tool fails closed on blank records and never reports
// a pass without complete, in-range, sufficiently sampled measurements.
#include "shared/measurement_spec.hpp"
#include "shared/mechanism_spec.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace starz::engineering::qualification {
inline constexpr std::string_view revision="S1-B-QUAL-R1-2026-09-13";
// Cut-length acceptance: ±(0.05 % + 0.20 mm) → ±0.70 mm at 1000 mm, the S1
// design target in docs/02. It is a target the machine must demonstrate.
inline constexpr double length_abs_mm=0.20,length_rel=0.0005;
inline constexpr double max_length_gauge_u_mm=0.05;     // steel rule/scale with k=2 uncertainty
inline constexpr double max_force_gauge_u_n=1.0;
inline constexpr double min_temperature_c=15,max_temperature_c=30;
inline constexpr int min_samples_per_cell=30;           // per (wire OD, commanded length)
inline constexpr double min_cpk=1.33;
// Required coverage: both wire extremes at the shortest and a long length.
inline constexpr double required_wire_od_mm[]={1.2,4.5};
inline constexpr double required_length_mm[]={200,1000};
inline constexpr double od_match_mm=0.05,length_match_mm=0.5;

struct LengthRecord {
    std::string sample_id;double wire_od_mm,commanded_mm,measured_mm;
    std::string gauge_id;double gauge_u_mm,temperature_c;
};
struct ForceRecord {
    std::string station;      // RIGHT_NIP, LEFT_NIP, WHEEL
    double wire_od_mm,force_n;std::string gauge_id;double gauge_u_n;
};
struct CellResult {
    double wire_od_mm,commanded_mm;int n;double mean_error_mm,sd_mm,tolerance_mm,cpk,max_abs_error_mm;bool pass;
};
struct ForceResult { std::string station;double wire_od_mm,force_n,lower_n,upper_n;bool pass; };
struct Report {
    std::vector<CellResult> cells;std::vector<ForceResult> forces;
    bool coverage_complete=false,all_pass=false;std::vector<std::string> failures;
};

inline double lengthTolerance(double commanded_mm){return length_abs_mm+length_rel*commanded_mm;}

inline void validate(const LengthRecord& r) {
    if(r.sample_id.empty()||r.gauge_id.empty())throw std::invalid_argument("Length record missing sample or gauge identity");
    for(double v:{r.wire_od_mm,r.commanded_mm,r.measured_mm,r.gauge_u_mm,r.temperature_c})
        if(!std::isfinite(v))throw std::invalid_argument("Length record "+r.sample_id+" has a blank or non-numeric field");
    if(r.wire_od_mm<1.2-od_match_mm||r.wire_od_mm>4.5+od_match_mm)throw std::invalid_argument("Length record "+r.sample_id+" wire OD outside 1.2–4.5 mm");
    if(r.commanded_mm<200||r.commanded_mm>20000)throw std::invalid_argument("Length record "+r.sample_id+" commanded length outside 200–20000 mm");
    if(r.measured_mm<=0)throw std::invalid_argument("Length record "+r.sample_id+" measured length not positive");
    if(r.gauge_u_mm<=0||r.gauge_u_mm>max_length_gauge_u_mm)throw std::invalid_argument("Length record "+r.sample_id+" gauge uncertainty outside (0,0.05] mm");
    if(r.temperature_c<min_temperature_c||r.temperature_c>max_temperature_c)throw std::invalid_argument("Length record "+r.sample_id+" temperature outside 15–30 °C");
}
inline void validate(const ForceRecord& r) {
    if(r.station!="RIGHT_NIP"&&r.station!="LEFT_NIP"&&r.station!="WHEEL")throw std::invalid_argument("Force record station must be RIGHT_NIP, LEFT_NIP or WHEEL");
    if(r.gauge_id.empty())throw std::invalid_argument("Force record missing gauge identity");
    for(double v:{r.wire_od_mm,r.force_n,r.gauge_u_n})if(!std::isfinite(v))throw std::invalid_argument("Force record "+r.station+" has a blank or non-numeric field");
    if(r.wire_od_mm<1.2-od_match_mm||r.wire_od_mm>4.5+od_match_mm)throw std::invalid_argument("Force record "+r.station+" wire OD outside 1.2–4.5 mm");
    if(r.gauge_u_n<=0||r.gauge_u_n>max_force_gauge_u_n)throw std::invalid_argument("Force record "+r.station+" gauge uncertainty outside (0,1] N");
}

inline Report evaluate(const std::vector<LengthRecord>& lengths,const std::vector<ForceRecord>& forces) {
    if(lengths.empty()||forces.empty())throw std::invalid_argument("No measurements: qualification cannot be evaluated");
    for(const auto& r:lengths)validate(r);
    for(const auto& r:forces)validate(r);
    Report rep;
    std::map<std::pair<long,long>,std::vector<double>> cells;
    for(const auto& r:lengths)cells[{std::lround(r.wire_od_mm*10),std::lround(r.commanded_mm)}].push_back(r.measured_mm-r.commanded_mm);
    for(const auto& [key,errors]:cells) {
        CellResult c{key.first/10.0,double(key.second),int(errors.size()),0,0,lengthTolerance(double(key.second)),0,0,false};
        for(double e:errors){c.mean_error_mm+=e;c.max_abs_error_mm=std::max(c.max_abs_error_mm,std::abs(e));}
        c.mean_error_mm/=c.n;
        for(double e:errors)c.sd_mm+=(e-c.mean_error_mm)*(e-c.mean_error_mm);
        c.sd_mm=c.n>1?std::sqrt(c.sd_mm/(c.n-1)):0;
        // Guard band: the gauge uncertainty of the worst gauge in the cell is subtracted from the tolerance.
        double u=0;for(const auto& r:lengths)if(std::lround(r.wire_od_mm*10)==key.first&&std::lround(r.commanded_mm)==key.second)u=std::max(u,r.gauge_u_mm);
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
        if(r.station=="WHEEL"){f.lower_n=mechanism::design::wheel_force_min_n;f.upper_n=mechanism::design::wheel_force_max_n;}
        else{f.lower_n=mechanism::design::nip_force_min_n;f.upper_n=mechanism::design::nip_force_max_n;}
        f.pass=r.force_n-r.gauge_u_n>=f.lower_n&&r.force_n+r.gauge_u_n<=f.upper_n;
        if(!f.pass)rep.failures.push_back("Force "+r.station+" OD "+std::to_string(r.wire_od_mm)+": "+std::to_string(r.force_n)+" N outside guarded window");
        force_seen[r.station][std::lround(r.wire_od_mm*10)]=true;rep.forces.push_back(f);
    }
    for(const char* st:{"RIGHT_NIP","LEFT_NIP","WHEEL"})for(double od:required_wire_od_mm)
        if(!force_seen[st].count(std::lround(od*10))){covered=false;rep.failures.push_back(std::string("Missing force record ")+st+" at OD "+std::to_string(od));}
    rep.coverage_complete=covered;
    rep.all_pass=covered&&std::all_of(rep.cells.begin(),rep.cells.end(),[](const CellResult& c){return c.pass;})&&
                 std::all_of(rep.forces.begin(),rep.forces.end(),[](const ForceResult& f){return f.pass;});
    return rep;
}
}
