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
        v.push_back({"S"+std::to_string(++k),od,L,L+bias+sd_scale*q::lengthTolerance(L)*noise(i),"RULE-1",0.02,21.0,"MC-1","B-1","R-1","CAL-1","OP-A"});
    return v;
}
std::vector<q::ForceRecord> syntheticForces(double nip,double wheel) {
    std::vector<q::ForceRecord> v;
    for(const char* st:q::required_force_stations)for(double od:q::required_wire_od_mm)
        v.push_back({st,od,(st[0]=='W'?wheel:nip),"FG-1",0.5,21.0,"MC-1","B-1","R-1","CAL-1","OP-A"});
    return v;
}
}
int main() {
    expect(std::abs(q::lengthTolerance(1000)-0.70)<1e-12,"tolerance at 1 m is ±0.70 mm");
    expect(std::abs(q::lengthTolerance(200)-0.30)<1e-12,"tolerance at 200 mm is ±0.30 mm");
    const auto good=q::evaluate(syntheticLengths(0.15,0.0),syntheticForces(60,10));
    expect(good.coverage_complete&&good.all_pass&&good.failures.empty(),"capable synthetic set passes");
    expect(good.cells.size()==4&&good.forces.size()==12,"four cells and twelve per-station force records");
    // The audit repro: reusing a sample identity must not count as a distinct piece.
    auto dup=syntheticLengths(0.15,0.0);dup[1].sample_id=dup[0].sample_id;
    const auto dr=q::evaluate(dup,syntheticForces(60,10));
    expect(!dr.all_pass&&dr.duplicates_rejected,"duplicate sample identity fails (all_pass=false)");
    expect(dr.cells.front().n==q::min_samples_per_cell-1,"a reused sample id is counted once, not twice");
    const auto wide=q::evaluate(syntheticLengths(0.6,0.0),syntheticForces(60,10));
    expect(!wide.all_pass,"wide scatter fails Cpk");
    const auto biased=q::evaluate(syntheticLengths(0.15,0.5),syntheticForces(60,10));
    expect(!biased.all_pass,"0.5 mm bias fails the 200 mm cell");
    auto few=syntheticLengths(0.15,0.0);few.resize(few.size()-1);
    expect(!q::evaluate(few,syntheticForces(60,10)).all_pass,"29 samples in a cell fail");
    auto partial=syntheticLengths(0.15,0.0);partial.erase(partial.begin(),partial.begin()+q::min_samples_per_cell);
    const auto pr=q::evaluate(partial,syntheticForces(60,10));
    expect(!pr.coverage_complete&&!pr.all_pass,"missing required cell fails coverage");
    auto few_stations=syntheticForces(60,10);few_stations.pop_back(); // drop WHEEL_DOWN at 4.5 mm
    const auto fs=q::evaluate(syntheticLengths(0.15,0.0),few_stations);
    expect(!fs.coverage_complete&&!fs.all_pass,"missing per-station force record fails coverage");
    expect(!q::evaluate(syntheticLengths(0.15,0.0),syntheticForces(49.6,10)).all_pass,"49.6±0.5 N nip fails the guarded window");
    expect(!q::evaluate(syntheticLengths(0.15,0.0),syntheticForces(60,18.8)).all_pass,"18.8±0.5 N wheel fails the guarded window");
    // Provenance is required per record; blanks are rejected before evaluation.
    bool threw=false;try{auto bad=syntheticLengths(0.15,0);bad[0].machine_id="";q::evaluate(bad,syntheticForces(60,10));}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"blank machine provenance is rejected");
    threw=false;try{auto bad=syntheticLengths(0.15,0);bad[0].operator_slot="";q::evaluate(bad,syntheticForces(60,10));}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"blank operator slot is rejected");
    threw=false;try{auto bad=syntheticForces(60,10);bad[0].temperature_c=31;q::evaluate(syntheticLengths(0.15,0),bad);}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"out-of-range force temperature is rejected");
    threw=false;try{auto bad=syntheticForces(60,10);bad[0].station="RIGHT_NIP";q::evaluate(syntheticLengths(0.15,0),bad);}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"legacy side-level station is rejected");
    // Mixed provenance is not a qualified data set even when numerically capable.
    auto mixed=syntheticLengths(0.15,0.0);mixed[5].build_id="B-2";
    expect(!q::evaluate(mixed,syntheticForces(60,10)).all_pass,"mixed build provenance fails");
    threw=false;try{auto bad=syntheticLengths(0.15,0);bad[0].temperature_c=31;q::evaluate(bad,syntheticForces(60,10));}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"out-of-range temperature is rejected");
    threw=false;try{auto bad=syntheticLengths(0.15,0);bad[0].gauge_u_mm=0.1;q::evaluate(bad,syntheticForces(60,10));}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"coarse gauge is rejected");
    threw=false;try{q::evaluate({},{});}catch(const std::invalid_argument&){threw=true;}
    expect(threw,"empty records are rejected");
    if(failures){std::fprintf(stderr,"%d qualification checks failed\n",failures);return 1;}
    std::puts("qualification arithmetic checks passed");return 0;
}
