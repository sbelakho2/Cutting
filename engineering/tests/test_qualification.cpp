// Unit checks of the qualification arithmetic with synthetic records.
// Nothing here is a machine measurement.
#include "qualification.hpp"
#include <cstdio>
#include <cstdlib>

namespace q=starz::engineering::qualification;
namespace {
int failures=0;
void expect(bool ok,const char* what){if(!ok){std::fprintf(stderr,"FAILED: %s\n",what);++failures;}}
// Deterministic pseudo-noise in (−1,1) with zero mean over the 30 samples.
double noise(int i){return ((i*37)%61-30)/30.0;}
std::vector<q::LengthRecord> syntheticLengths(double sd_scale,double bias) {
    std::vector<q::LengthRecord> v;int k=0;
    for(double od:q::required_wire_od_mm)for(double L:q::required_length_mm)for(int i=0;i<q::min_samples_per_cell;++i)
        v.push_back({"S"+std::to_string(++k),od,L,L+bias+sd_scale*q::lengthTolerance(L)*noise(i),"RULE-1",0.02,21.0});
    return v;
}
std::vector<q::ForceRecord> syntheticForces(double nip,double wheel) {
    std::vector<q::ForceRecord> v;
    for(const char* st:{"RIGHT_NIP","LEFT_NIP"})for(double od:q::required_wire_od_mm)v.push_back({st,od,nip,"FG-1",0.5});
    for(double od:q::required_wire_od_mm)v.push_back({"WHEEL",od,wheel,"FG-1",0.5});
    return v;
}
}
int main() {
    expect(std::abs(q::lengthTolerance(1000)-0.70)<1e-12,"tolerance at 1 m is ±0.70 mm");
    expect(std::abs(q::lengthTolerance(200)-0.30)<1e-12,"tolerance at 200 mm is ±0.30 mm");
    const auto good=q::evaluate(syntheticLengths(0.15,0.0),syntheticForces(60,10));
    expect(good.coverage_complete&&good.all_pass&&good.failures.empty(),"capable synthetic set passes");
    expect(good.cells.size()==4&&good.forces.size()==6,"four cells and six force records");
    const auto wide=q::evaluate(syntheticLengths(0.6,0.0),syntheticForces(60,10));
    expect(!wide.all_pass,"wide scatter fails Cpk");
    const auto biased=q::evaluate(syntheticLengths(0.15,0.5),syntheticForces(60,10));
    expect(!biased.all_pass,"0.5 mm bias fails the 200 mm cell");
    auto few=syntheticLengths(0.15,0.0);few.resize(few.size()-1);
    expect(!q::evaluate(few,syntheticForces(60,10)).all_pass,"29 samples in a cell fail");
    auto partial=syntheticLengths(0.15,0.0);partial.erase(partial.begin(),partial.begin()+q::min_samples_per_cell);
    const auto pr=q::evaluate(partial,syntheticForces(60,10));
    expect(!pr.coverage_complete&&!pr.all_pass,"missing required cell fails coverage");
    expect(!q::evaluate(syntheticLengths(0.15,0.0),syntheticForces(49.6,10)).all_pass,"49.6±0.5 N nip fails the guarded window");
    expect(!q::evaluate(syntheticLengths(0.15,0.0),syntheticForces(60,18.8)).all_pass,"18.8±0.5 N wheel fails the guarded window");
    bool threw=false;try{auto bad=syntheticLengths(0.15,0);bad[0].temperature_c=31;q::evaluate(bad,syntheticForces(60,10));}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"out-of-range temperature is rejected");
    threw=false;try{auto bad=syntheticLengths(0.15,0);bad[0].gauge_u_mm=0.1;q::evaluate(bad,syntheticForces(60,10));}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"coarse gauge is rejected");
    threw=false;try{q::evaluate({},{});}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"empty records are rejected");
    if(failures){std::fprintf(stderr,"%d qualification checks failed\n",failures);return 1;}
    std::puts("qualification arithmetic checks passed");return 0;
}
