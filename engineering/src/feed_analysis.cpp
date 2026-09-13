#include "feed_models.hpp"
#include "shared/feed_drive_spec.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <stdexcept>

int main(int argc,char** argv) try {
    namespace fs=std::filesystem;namespace se=starz::engineering;
    namespace c=starz::feed::catalogue;namespace d=starz::feed::design;
    constexpr double pi=std::numbers::pi;
    if(argc!=3||std::string(argv[1])!="--report")throw std::invalid_argument("Use --report output_directory");
    const fs::path out=argv[2];fs::create_directories(out);
    {std::ofstream pending(out/"status.json");pending<<"{\"run_complete\":false,\"fabrication_released\":false}\n";}
    std::ofstream checks(out/"checks.csv"),speeds(out/"belt-speed.csv"),loads(out/"shaft-loads.csv"),report(out/"report.md");
    checks<<std::setprecision(12)<<"check,value,limit,unit,result,basis\n";
    speeds<<std::setprecision(12)<<"rpm,capacity_W,capacity_Nm,running_service_margin,fault_margin\n";
    loads<<std::setprecision(12)<<"belt_Fx_N,roller_Fx_N,roller_Fz_N,rear_load_N,front_load_N,max_deflection_bound_mm,nominal_bending_bound_MPa,balance_N,balance_Nmm\n";
    int failures=0,count=0;
    const auto check=[&](const char* id,double value,double limit,const char* unit,bool pass,const char* basis) {
        checks<<id<<','<<value<<','<<limit<<','<<unit<<','<<(pass?"PASS_SCOPED":"FAIL")<<",\""<<basis<<"\"\n";
        ++count;if(!pass)++failures;
    };
    const double centres=se::equalPulleyCentres(c::belt_length,c::pulley_teeth,c::pitch_mm);
    check("belt_centres",centres,62.5,"mm",std::abs(centres-62.5)<1e-9,"Exact equal-pulley pitch-length equation");
    check("inward_adjustment",centres-d::min_centres,10,"mm",centres-d::min_centres>=10,"MISUMI Table31;physical slide not yet integrated");
    check("outward_adjustment",d::max_centres-centres,2,"mm",d::max_centres-centres>=2,"MISUMI Table31;physical slide not yet integrated");
    double min_capacity=1e100;
    std::vector<double> rpm{d::shaft_limit_rpm};
    for(double r:c::power_rpm)if(r<=d::shaft_limit_rpm)rpm.push_back(r);
    std::sort(rpm.begin(),rpm.end());
    for(double r:rpm) {
        const double cap=se::s5mTorqueCapacity(r);min_capacity=std::min(min_capacity,cap);
        speeds<<r<<','<<se::s5mPowerCapacity(r)<<','<<cap<<','<<cap/(d::running_torque_nm*d::reversal_service_factor)
              <<','<<cap/d::mechanical_fault_torque_nm<<'\n';
    }
    check("reversing_running_capacity",min_capacity/(d::running_torque_nm*d::reversal_service_factor),1,"ratio",
          min_capacity>=d::running_torque_nm*d::reversal_service_factor,"Printed Table39;Kb2.84;Km1;service factor2.5 on1Nm operating envelope");
    check("fault_torque_capacity",min_capacity/d::mechanical_fault_torque_nm,2,"ratio",min_capacity>=2*d::mechanical_fault_torque_nm,
          "50-to260rpm catalogue screen;5Nm isolated mechanical fault case is not multiplied by a running-duty factor");
    const double radius=c::pulley_teeth*c::pitch_mm/(2*pi);
    const auto tension=se::beltTension(d::tension_min_n,centres,c::belt_length,radius,d::mechanical_fault_torque_nm,c::tension_y_n);
    check("fault_slack_side",tension.slack_n,0,"N",tension.slack_n>0,"Equal-span elastic tension redistribution at180N initial tension");
    check("maximum_initial_tension",d::tension_max_n,c::tension_max_n,"N",d::tension_max_n<=c::tension_max_n,"MISUMI maximum221N applies to INITIAL tension,not tight-side loaded tension");
      const double wire_half=d::wire_diameter_max/2+d::wire_lane_tolerance;
      const double pocket_margin=d::wire_contact_station-d::roller_station-d::mc_centre_tolerance-2.1-wire_half;
      const double edge_margin=d::roller_end-0.2-d::wire_contact_station-wire_half;
      check("wire_to_MC_pocket_margin",pocket_margin,0.4,"mm",pocket_margin>=0.4,"MC_M+1_urethane_access_diameter4mm;incoming_max4.2mm_and_centre_plusminus0.15mm;wire4.5mm;assembled_lane_position_plusminus0.20mm");
      check("wire_to_tread_edge_margin",edge_margin,2,"mm",edge_margin>=2,"Roller_length_minus0.2mm;full_wire_radius_envelope;uncompressed_geometry_not_contact_stress");
          const double grip_margin=d::minimum_finished_length-2*d::maximum_strip_length-(d::near_nip_x+d::max_centres);
          check("short_part_insulated_grip",grip_margin,d::minimum_insulated_grip_margin,"mm",grip_margin>=d::minimum_insulated_grip_margin,
                "200mm_part;25mm_leading_strip;25mm_tail_backfeed;far_nip_at_maximum64.5mm_centre_setting");
          const double roller_hub_clearance_um=(c::roller_bore_max-9.991)*1000;
          check("roller_hub_diametral_clearance",roller_hub_clearance_um,40,"um",roller_hub_clearance_um<=40,
                "10.000–10.030incoming_bore_covers_currentH7drawing_and_archivedEUplus0.01plus0.03;10h6shaft;positivekey_not_friction_drive");
    const double tap_clearance=(d::retaining_drill_full_depth-0.05)-
        (d::retaining_entry_chamfer+0.02+d::retaining_full_thread_depth+0.05+3*0.8);
    check("blind_tap_lead_clearance",tap_clearance,1,"mm",tap_clearance>=1,
          "18plusminus0.05cylindrical_drill_depth;0.2plusminus0.02entry;14.00to14.05full_thread;maximum3pitch2.4mm_tap_lead");
    check("groove_root_radius_limit",d::groove_root_radius+0.01,0.1*(c::seeger_a12_width-c::seeger_a12_width_minus),"mm",
          d::groove_root_radius+0.01<=0.1*(c::seeger_a12_width-c::seeger_a12_width_minus),
          "SEEGER2022page129_section8.1.3_loaded_side_radius_at_most0.1_times_ring_thickness");
    check("groove_flat_width",c::seeger_a12_groove_width_min-2*(d::groove_root_radius+0.01),c::seeger_a12_width,"mm",
          c::seeger_a12_groove_width_min-2*(d::groove_root_radius+0.01)+1e-12>=c::seeger_a12_width,
          "1.10minimum_groove_minus_two0.05maximum_roots_retains1.00maximum_ring_width");
    const double key_corner_reserve=(d::key_inside_chamfer-0.02)-(d::key_floor_radius+0.02);
    check("key_corner_relief_reserve",key_corner_reserve,0,"mm",key_corner_reserve>0,
          "0.20plusminus0.02inside_key_chamfer_exceeds0.15plusminus0.02key_seat_floor_radius_before_side_clearance");
    const auto low=se::beltTension(180,centres,225,radius,0,151),target=se::beltTension(190,centres,225,radius,0,151),high=se::beltTension(200,centres,225,radius,0,151);
    const auto shaft=se::selectedFeedShaftSections();
    double max_rear=0,max_front=0,max_u=0,max_sigma=0;
    for(double bx:{-480.0,480.0})for(double rx:{-60.0,60.0})for(double rz:{-60.0,60.0}) {
      const std::array<se::BeamLoad,2> x{{{d::pulley_station,bx},{d::wire_contact_station,rx}}};
      const se::BeamLoad z{d::wire_contact_station,rz};
        const auto X=se::shaftBeam(shaft,d::steel_E_mpa,d::rear_support,d::front_support,x,5);
        const auto Z=se::shaftBeam(shaft,d::steel_E_mpa,d::rear_support,d::front_support,std::span(&z,1),5);
        const double rear=std::hypot(X.reaction_a_n,Z.reaction_a_n),front=std::hypot(X.reaction_b_n,Z.reaction_b_n);
        // Combining separate component maxima gives a safe envelope without
        // assuming that their governing stations coincide.
        const double u=std::hypot(X.max_displacement_mm,Z.max_displacement_mm);
        const double sigma=std::hypot(X.max_bending_mpa,Z.max_bending_mpa);
        max_rear=std::max(max_rear,rear);max_front=std::max(max_front,front);
        max_u=std::max(max_u,u);max_sigma=std::max(max_sigma,sigma);
        loads<<bx<<','<<rx<<','<<rz<<','<<rear<<','<<front<<','<<u<<','<<sigma<<','
             <<std::max(X.force_residual_n,Z.force_residual_n)<<','<<std::max(X.moment_residual_nmm,Z.moment_residual_nmm)<<'\n';
    }
    check("shaft_displacement_screen",max_u,0.05,"mm",max_u<=0.05,"Variable-section Euler-Bernoulli beam;does not include bearing,housing or polyurethane compliance");
    check("rear_bearing_static_ratio",c::rear_bearing_C0/max_rear,3,"ratio",c::rear_bearing_C0>=3*max_rear,"SKF6001-2Z/C3 radial rating;rotation and lubrication limits remain separate");
    check("front_bearing_static_ratio",c::front_bearing_C0/max_front,3,"ratio",c::front_bearing_C0>=3*max_front,"SKF6002-2Z/C3 radial rating;no invented preload");
    double max_tau_per_nm=0,twist_per_nm=0;
    for(const auto& s:shaft) {
        const double J=pi*(std::pow(s.diameter_mm,4)-std::pow(s.hole_mm,4))/32;
        max_tau_per_nm=std::max(max_tau_per_nm,1000*s.diameter_mm/(2*J));
        twist_per_nm+=1000*(s.to_mm-s.from_mm)/(d::steel_E_mpa/(2*1.29)*J);
    }
    const double notch=3; // Explicit sensitivity multiplier, not a resolved notch factor.
    const double running_vm=notch*std::hypot(max_sigma,std::sqrt(3.0)*max_tau_per_nm*d::running_torque_nm);
    const double fault_vm=notch*std::hypot(max_sigma,std::sqrt(3.0)*max_tau_per_nm*d::mechanical_fault_torque_nm);
    check("shaft_working_notch_sensitivity",running_vm,650/3.0,"MPa",running_vm<=650/3.0,"3x nominal combined stress sensitivity;actual keyway-root FEA still required");
    check("shaft_fault_notch_sensitivity",fault_vm,650/1.5,"MPa",fault_vm<=650/1.5,"5Nm fault and3x notch sensitivity;not a certified fatigue result");
    const auto pulley_key=se::keyStress(5,14,4.97,1.9,22.8);
    const auto roller_key=se::keyStress(5,9.991,2.975,2.065,14.8);
    check("pulley_key_shear",pulley_key.shear_mpa,100,"MPa",pulley_key.shear_mpa<=100,"C45 key;conservative effective width,length and height");
    check("roller_key_shear",roller_key.shear_mpa,100,"MPa",roller_key.shear_mpa<=100,"STARZ3x4x18 positive key,not the M3 set screws");
    check("roller_key_bearing",roller_key.bearing_mpa,65/1.5,"MPa",roller_key.bearing_mpa<=65/1.5,"Required aluminium collar minimum yield65MPa;hub ligament/notch not qualified by this bearing-area equation");
    // Bounded stack for nonlocating rear inner-ring retention. Positive end float
    // prevents a tolerance-driven axial preload of two deep-groove bearings.
    const double min_float=31.48-(18.52+1.14)-(2.72+8+1.02);
    const double max_float=31.52-(18.48+0.94)-(2.68+7.94+0.98);
    check("rear_inner_ring_min_endfloat",min_float,0,"mm",min_float>0,"Independent shaft,groove,ring,bearing and spacer bounds");
    check("rear_inner_ring_max_endfloat",max_float,0.6,"mm",max_float<0.6,"Nonlocating support;front6002 locates the shaft");
    report<<std::setprecision(8)<<"# Feed drivetrain calculation record\n\nRevision **"<<starz::feed::revision<<"**.\n\n"
          <<"Selected pulley **"<<c::pulley<<"**, belt **"<<c::belt<<"**; nominal centres **"<<centres<<"mm**. "
          <<"The12mm pulley-seat iteration failed the bending screen and was replaced by the14mm keyed catalogue bore.\n\n"
          <<"R3puts the wire lane atS108.75,5.25mm off the roller midplane,away from the twoMCurethane-access holes. "
          <<"With4.5mmwire,±0.20mmassembled lane position,4.2mm maximum incoming access diameter,±0.15mmaccess-centre tolerance and−0.2mmroller length: "
          <<"minimum pocket-edge clearance **"<<pocket_margin<<"mm**,tread-edge clearance **"<<edge_margin<<"mm**. Shaft loads use this actualoffset station.\n\n"
          <<"Nips are X75.5and138.0mm nominal;at maximumcentre setting,thefarnip isX140.0mm. "
          <<"A200mmpart with25mmleadingstrip and25mmtailbackfeed retains **"<<grip_margin<<"mm** insulated grip beyondthefarnip. "
          <<"The motor is on theouter/farshaft,awayfromthecutting stages.\n\n"
          <<"R5 adds machined shoulder/key-seat/groove radii and matching key/spacer relief. "
          <<"The blind M5 thread now has14mm full-profile depth after its0.2mm entry chamfer,18mm cylindrical drill depth and a118degree drill point. "
          <<"Worst specified clearance beyond a3-pitch cutting-tap lead is **"<<tap_clearance<<"mm**. "
          <<"R6 defines the external end/thread-lead chamfers and the washer-slot plan/intersection/floor blends. "
          <<"Reduced beam intervals now include the two end chamfers and the S33.30–61.70 key-lip envelope. "
          <<"The separate machined-section audit checks nominal principal bending inertias against these reduced sections. "
          <<"These geometry checks do not replace local contact/root stress analysis.\n\n"
          <<"## Belt and commissioning setting\n\nMinimum catalogue capacity across50–260rpm: **"<<min_capacity<<"Nm**. "
          <<"The1Nm operating envelope carries a2.5 reversing-service factor. The separate5Nm mechanical fault envelope has capacity ratio "<<min_capacity/5<<". "
          <<"These are design envelopes, not measured wire forces or a motor tolerance guarantee. No power-table extrapolation to standstill is made. "
          <<"Static holding/jam tooth integrity requires the5Nm bench test in the assembly record.\n\n"
          <<"Set initial strand tension190N, acceptance180–200N. At the62.5mm span, apply **1.000mm** transverse deflection. "
          <<"Corrected deflection force: **"<<low.deflection_force_n<<"–"<<high.deflection_force_n<<"N**, target **"<<target.deflection_force_n<<"N**. "
          <<"Use the actual measured span in the equation if the final tensioned centre distance differs. "
          <<"At180N initial tension and5Nm, the slack strand remains **"<<tension.slack_n<<"N**. "
          <<"Manufacturer225mm belt length tolerance±0.41mm corresponds to±0.205mm geometrical centre change before elastic tensioning.\n\n"
          <<"## Shaft and bearing screens\n\nEight independent load-sign combinations:±480N belt radialX;±60N rollerX and±60N rollerZ. "
          <<"These conservatively allow simultaneous components. Maximum displacement bound **"<<max_u<<"mm**; nominal bending bound **"<<max_sigma<<"MPa**. "
          <<"With an explicit3x notch sensitivity: working combined stress **"<<running_vm<<"MPa**,5Nm fault **"<<fault_vm<<"MPa**. "
          <<"Peak bearing resultants: rear **"<<max_rear<<"N**,front **"<<max_front<<"N**. "
          <<"Full-length constant-torque shaft wind-up screen **"<<twist_per_nm<<"rad/Nm**; actual torque distribution and encoder correction are separate.\n\n"
          <<"The model uses reduced circular sections, not resolved key-seat roots. The real housing, bearing contact/clearance, corner radii, pulley-belt dynamics, "
          <<"rubber contact and fatigue are not replaced by this beam calculation. No physical accuracy or machine-life result is asserted.\n\n"
          <<"## Axial retention\n\nRear inner-ring stack minimum end float **"<<min_float<<"mm**, maximum **"<<max_float<<"mm**. "
          <<"The rear outer ring floats axially in28.008–28.018mm. The front6002 is the locating bearing,in32.008–32.019mm,clamped at its inner ring byKM2/MB2. "
          <<"Matchedj5shaftseats use3–6µmactualinterference;see the separatebearing-fit/thermal-enveloperecord. "
          <<"Its outer ring has0.020–0.110mm specified end clearance in the9.02–9.05mm pocket; do not impose nut preload through both bearings.\n\n"
          <<"Generated **"<<count<<"** calculation checks; **"<<failures<<"** failures. Refer to the per-check basis. "
          <<"Fabrication release also requires completed housing/root analyses, pressure/tensioning assembly and fastener/tool-access drawings.\n";
    std::ofstream status(out/"status.json");
    status<<"{\"revision\":\""<<starz::feed::revision<<"\",\"run_complete\":true,\"checks\":"<<count<<",\"failed_checks\":"<<failures
          <<",\"physical_validation_complete\":false,\"fabrication_released\":false}\n";
    if(!checks||!speeds||!loads||!report||!status)throw std::runtime_error("Feed analysis write failure");
    std::cout<<"Feed calculation record: "<<count<<" checks, "<<failures<<" failures\n";
    return failures?2:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}