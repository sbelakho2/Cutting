#include "head_fit.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace starz::engineering;
namespace {
void require(bool ok,const char* text) {if(!ok) throw std::runtime_error(text);}
void near(double a,double b) {require(std::abs(a-b)<1e-9,"Head fit dimensional regression");}
}
int main() try {
    const auto fit=selectedHeadFitEnvelope();
    require(fit.passed&&fit.corners==256,"All independent fit corners must pass");
    near(fit.minimum_motor_projection_mm,0.53);
    near(fit.minimum_stage_projection_mm,0.55);
    near(fit.minimum_shaft_gap_mm,12.58);
    near(fit.minimum_shoulder_gap_mm,0.95);
    near(fit.maximum_motor_projection_mm,4.47);
    near(fit.maximum_stage_projection_mm,1.45);
    const auto nominal=headFit({});
    near(nominal.coupling_near_mm,3); near(nominal.coupling_far_mm,53);
    near(nominal.motor_engagement_mm,16); near(nominal.stage_engagement_mm,16);
    // Old R4 chain: minimum motor shaft with the shortest catalogue coupling.
    require(!headFit({36,19,54,18,49,16,16,2}).full_hubs,
            "Old nominal-only adapter must fail the actual tolerance corner");
    require(!headFit({37,15,54,18,50,19,16,1}).full_hubs,"Oversize stage hub accepted");
    require(!headFit({60,15,54,18,50,16,16,1}).full_hubs,"Touching shaft ends accepted");
    bool rejected=false;
    try {auto p=HeadFitInput{}; p.motor_shaft_mm=std::numeric_limits<double>::quiet_NaN(); (void)headFit(p);}
    catch(const std::invalid_argument&) {rejected=true;}
    require(rejected,"Nonfinite fit input accepted");
    std::cout<<fit.corners<<" tolerance corners and negative-fit regressions passed\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n'; return 1;}