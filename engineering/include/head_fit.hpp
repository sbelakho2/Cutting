#pragma once
#include "shared/cutting_head_spec.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace starz::engineering {
// Incoming inspection limits for dimensions whose tolerances are not printed
// in the HIWIN/R+W catalogue. These are STARZ acceptance requirements, not
// invented manufacturer tolerances. Assemble against the measured shoulder.
struct HeadFitInput {
    double motor_shaft_mm = head::catalogue::motor_shaft;
    double adapter_mm = head::design::adapter_thickness;
    double shoulder_from_f0_mm = 54;
    double stage_exposed_mm = 18;
    double coupling_free_length_mm = head::catalogue::coupling_length;
    double stage_hub_mm = head::catalogue::coupling_fit;
    double motor_hub_mm = head::catalogue::coupling_fit;
    double shoulder_gap_mm = head::design::stage_hub_shoulder_gap;
};
struct HeadFitResult {
    double coupling_near_mm, coupling_far_mm;
    double motor_engagement_mm, stage_engagement_mm;
    double motor_projection_beyond_hub_mm, stage_projection_beyond_hub_mm;
    double shaft_end_gap_mm, shoulder_gap_mm;
    bool full_hubs;
};
inline HeadFitResult headFit(const HeadFitInput& p) {
    for(double v:std::array{p.motor_shaft_mm,p.adapter_mm,p.shoulder_from_f0_mm,
             p.stage_exposed_mm,p.coupling_free_length_mm,p.stage_hub_mm,p.motor_hub_mm,p.shoulder_gap_mm})
        if(!std::isfinite(v)||v<=0) throw std::invalid_argument("Invalid cutting coupling fit input");
    if(p.coupling_free_length_mm<=p.motor_hub_mm+p.stage_hub_mm ||
       p.stage_exposed_mm>=p.shoulder_from_f0_mm)
        throw std::invalid_argument("Invalid shaft/hub ordering");
    const double far=p.shoulder_from_f0_mm-p.shoulder_gap_mm;
    const double near=far-p.coupling_free_length_mm;
    const double motorTip=p.motor_shaft_mm-p.adapter_mm;
    const double stageTip=p.shoulder_from_f0_mm-p.stage_exposed_mm;
    const double motorEngagement=std::max(0.,std::min(motorTip,near+p.motor_hub_mm)-std::max(-p.adapter_mm,near));
    const double stageEngagement=std::max(0.,std::min(p.shoulder_from_f0_mm,far)-std::max(stageTip,far-p.stage_hub_mm));
    const double motorProjection=motorTip-near-p.motor_hub_mm;
    const double stageProjection=far-p.stage_hub_mm-stageTip;
    return {near,far,motorEngagement,stageEngagement,motorProjection,stageProjection,
            stageTip-motorTip,p.shoulder_gap_mm,
            motorEngagement>=p.motor_hub_mm-1e-10 && stageEngagement>=p.stage_hub_mm-1e-10 && stageTip>motorTip};
}
struct HeadFitEnvelope {
    std::size_t corners=0;
    double minimum_motor_projection_mm=1e9, minimum_stage_projection_mm=1e9;
    double minimum_shaft_gap_mm=1e9, minimum_shoulder_gap_mm=1e9;
    double maximum_motor_projection_mm=0, maximum_stage_projection_mm=0;
    bool passed=true;
};
inline HeadFitEnvelope selectedHeadFitEnvelope() {
    namespace c=head::catalogue;
    namespace d=head::design;
    HeadFitEnvelope out;
    // Eight independent bounded dimensions: all 256 corners, not RSS.
    for(double shaft:{c::motor_shaft-1,c::motor_shaft+1})
    for(double adapter:{d::adapter_thickness-d::adapter_thickness_tolerance,d::adapter_thickness+d::adapter_thickness_tolerance})
    for(double shoulder:{53.8,54.2})
    for(double exposed:{17.8,18.2})
    for(double length:{c::coupling_length_min,c::coupling_length_max})
    for(double stageHub:{15.8,16.2})
    for(double motorHub:{15.8,16.2})
    for(double gap:{0.95,1.05}) {
        const auto r=headFit({shaft,adapter,shoulder,exposed,length,stageHub,motorHub,gap});
        ++out.corners;
        out.passed=out.passed&&r.full_hubs;
        out.minimum_motor_projection_mm=std::min(out.minimum_motor_projection_mm,r.motor_projection_beyond_hub_mm);
        out.minimum_stage_projection_mm=std::min(out.minimum_stage_projection_mm,r.stage_projection_beyond_hub_mm);
        out.minimum_shaft_gap_mm=std::min(out.minimum_shaft_gap_mm,r.shaft_end_gap_mm);
        out.minimum_shoulder_gap_mm=std::min(out.minimum_shoulder_gap_mm,r.shoulder_gap_mm);
        out.maximum_motor_projection_mm=std::max(out.maximum_motor_projection_mm,r.motor_projection_beyond_hub_mm);
        out.maximum_stage_projection_mm=std::max(out.maximum_stage_projection_mm,r.stage_projection_beyond_hub_mm);
    }
    return out;
}
}