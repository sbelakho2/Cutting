#include "models.hpp"
#include "measurement_models.hpp"
#include "shared/cutting_head_spec.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace starz::engineering {
namespace {
constexpr double pi=std::numbers::pi;
void positive(double x,const char* what) { if(!std::isfinite(x)||x<=0) throw std::invalid_argument(what); }
void nonnegative(double x,const char* what) { if(!std::isfinite(x)||x<0) throw std::invalid_argument(what); }
}
double interpolateTorque(double rpm) {
    const auto& p=head::catalogue::torque_curve;
    if(!std::isfinite(rpm)||rpm<p.front().rpm||rpm>p.back().rpm)
        throw std::out_of_range("Motor curve extrapolation is prohibited");
    for(std::size_t i=1;i<p.size();++i) if(rpm<=p[i].rpm) {
        const double t=(rpm-p[i-1].rpm)/(p[i].rpm-p[i-1].rpm);
        return p[i-1].nm+t*(p[i].nm-p[i-1].nm);
    }
    return p.back().nm;
}
double motorDemand(double force,double lead,double eta,double accel,double mass,double inertia,double drag) {
    nonnegative(force,"force"); positive(lead,"lead"); positive(eta,"efficiency");
    if(eta>1) throw std::invalid_argument("Efficiency exceeds unity");
    nonnegative(accel,"acceleration"); nonnegative(mass,"mass"); nonnegative(inertia,"inertia"); nonnegative(drag,"drag");
    return (force+mass*accel)*lead/(2*pi*eta)+inertia*(2*pi*accel/lead)+drag;
}
double equivalentLoad(const std::vector<double>& loads,const std::vector<double>& fractions,double exponent) {
    positive(exponent,"exponent");
    if(loads.empty()||loads.size()!=fractions.size()) throw std::invalid_argument("Load spectrum dimensions");
    double sum=0,total=0;
    for(std::size_t i=0;i<loads.size();++i) {
        nonnegative(loads[i],"load"); nonnegative(fractions[i],"fraction");
        sum+=fractions[i]*std::pow(loads[i],exponent); total+=fractions[i];
    }
    positive(total,"total fraction"); return std::pow(sum/total,1/exponent);
}
double lifeRevolutions(double c,double p,double exponent) {
    positive(c,"dynamic capacity"); positive(p,"equivalent load"); positive(exponent,"life exponent");
    return 1e6*std::pow(c/p,exponent);
}
double eulerLoad(double e,double i,double l,double k) {
    positive(e,"modulus"); positive(i,"inertia"); positive(l,"length"); positive(k,"effective length");
    return pi*pi*e*i/std::pow(k*l,2);
}
double shaftCriticalRpm(double e,double rho,double diameter,double length) {
    positive(e,"modulus"); positive(rho,"density"); positive(diameter,"diameter"); positive(length,"length");
    // Simply supported UNIFORM beam, no axial force or attachments. rad/s -> rpm.
    const double area=pi*diameter*diameter/4, inertia=pi*std::pow(diameter,4)/64;
    const double omega=pi*pi/(length*length)*std::sqrt(e*inertia/(rho*area));
    return omega*60/(2*pi);
}
double cantileverDeflection(double f,double l,double e,double b,double h,double nu) {
    positive(l,"length"); positive(e,"modulus"); positive(b,"width"); positive(h,"height");
    if(!std::isfinite(f)||nu<=-1||nu>=0.5) throw std::invalid_argument("beam material/load");
    const double i=b*std::pow(h,3)/12, g=e/(2*(1+nu));
    return f*std::pow(l,3)/(3*e*i)+f*l/((5.0/6)*g*b*h);
}
BoltResult boltedJoint(int n,double preload,double axial,double fraction,double mu,double area,double proof) {
    if(n<1 || fraction<0 || fraction>1 || !std::isfinite(fraction)) throw std::invalid_argument("joint parameters");
    positive(preload,"preload"); nonnegative(axial,"separating force"); nonnegative(mu,"friction");
    positive(area,"tensile area"); positive(proof,"proof stress");
    const double total=preload*n-(1-fraction)*axial;
    const double tension=preload+fraction*axial/n;
    return {total,tension,std::max(0.,total)*mu,tension/(area*proof)};
}
double copperExtension(double tension,double l,double area,double e) {
    nonnegative(tension,"tension"); nonnegative(l,"wire length"); positive(area,"copper area"); positive(e,"modulus");
    return tension*l/(e*area*1e-6);
}
double wireBuckling(double area,double strand,double length,double e) {
    positive(area,"copper area"); positive(strand,"strand diameter"); positive(length,"length"); positive(e,"modulus");
    // Freely sliding individual strands: not the falsely stiff equivalent solid rod.
    const double count=area/(pi*strand*strand/4);
    const double inertia=count*pi*std::pow(strand*1e-3,4)/64;
    return eulerLoad(e,inertia,length,1);
}
double eccentricWheelError(double circumference,double tir,double travel,double phase) {
    positive(circumference,"circumference"); nonnegative(tir,"runout"); nonnegative(travel,"travel");
    if(!std::isfinite(phase)) throw std::invalid_argument("phase");
    // First-order eccentric rolling-radius model; TIR is twice eccentricity.
    return tir/2*(std::sin(phase+2*pi*travel/circumference)-std::sin(phase));
}
HertzResult sphereContact(double f,double r,double e) {
    positive(f,"force"); positive(r,"radius"); positive(e,"modulus");
    const double a=std::cbrt(3*f*r/(4*e));
    return {a,a*a/r,3*f/(2*pi*a*a)};
}
std::vector<DancerState> dancerResponse(double dt,double duration,double m,double c,double k,double speed) {
    positive(dt,"dt"); positive(duration,"duration"); positive(m,"mass"); nonnegative(c,"damping"); positive(k,"stiffness");
    if(!std::isfinite(speed)||duration/dt>1e7) throw std::invalid_argument("dancer input");
    // Linear displacement-storage model, drive mismatch applied for first0.1s.
    std::vector<DancerState> out; double x=0,v=0;
    const auto derivative=[&](double t,double q,double w) {
        const double disturbance=t<0.1?speed:0;
        return std::array<double,2>{w,(-c*w-k*q+c*disturbance)/m};
    };
    double t=0;
    while(t<duration) {
        const double h=std::min(dt,duration-t);
        const auto a=derivative(t,x,v),b=derivative(t+h/2,x+h*a[0]/2,v+h*a[1]/2);
        const auto q=derivative(t+h/2,x+h*b[0]/2,v+h*b[1]/2),r=derivative(t+h,x+h*q[0],v+h*q[1]);
        x+=h*(a[0]+2*b[0]+2*q[0]+r[0])/6; v+=h*(a[1]+2*b[1]+2*q[1]+r[1])/6;
        out.push_back({x,v,k*x+c*v}); t+=h;
    }
    return out;
}
DecayResult dcBus(double dt,double duration,double cap,double resistance,double initial,double returned,double interval) {
    positive(dt,"dt"); positive(duration,"duration"); positive(cap,"capacitance"); positive(resistance,"resistance");
    nonnegative(initial,"initial voltage"); nonnegative(returned,"regenerated energy"); positive(interval,"return time");
    if(duration/dt>1e7) throw std::invalid_argument("Too many bus steps");
    double energy=0.5*cap*initial*initial,t=0,loss=0,input=0,peak=0,minV=initial,maxV=initial;
    while(t<duration) {
        double h=std::min(dt,duration-t);
        if(t<interval && t+h>interval) h=interval-t; // Resolve discontinuity exactly.
        const double power=t<interval?returned/interval:0;
        const double rate=2/(resistance*cap),steady=power/rate;
        const double next=steady+(energy-steady)*std::exp(-rate*h);
        loss+=power*h-(next-energy); input+=power*h;
        peak=std::max({peak,2*energy/(cap*resistance),2*next/(cap*resistance)});
        energy=std::max(0.,next); const double v=std::sqrt(2*energy/cap);
        minV=std::min(minV,v); maxV=std::max(maxV,v); t+=h;
    }
    return {minV,maxV,std::sqrt(2*energy/cap),loss,peak,0.5*cap*initial*initial+input-loss-energy};
}
DynamicResult freeOscillator(double dt,double duration,double m,double k,double c,double x,double v) {
    positive(dt,"dt"); positive(duration,"duration"); positive(m,"mass"); positive(k,"stiffness"); nonnegative(c,"damping");
    if(!std::isfinite(x)||!std::isfinite(v)||duration/dt>1e7) throw std::invalid_argument("oscillator state");
    // Newmark average acceleration (beta1/4,gamma1/2), equilibrium at both ends.
    double a=(-c*v-k*x)/m,t=0,loss=0,peak=std::abs(x);
    const double initial=0.5*(m*v*v+k*x*x);
    while(t<duration) {
        const double h=std::min(dt,duration-t),xp=x+h*v+h*h*a/4,vp=v+h*a/2;
        const double an=(-c*vp-k*xp)/(m+c*h/2+k*h*h/4);
        const double xn=xp+h*h*an/4,vn=vp+h*an/2;
        loss+=c*std::pow((v+vn)/2,2)*h;
        x=xn; v=vn; a=an; peak=std::max(peak,std::abs(x)); t+=h;
    }
    const double energy=0.5*(m*v*v+k*x*x);
    return {x,peak,initial,energy,loss,initial-energy-loss};
}
ThermalResult thermalSteady(double i,double r,double ambient,double motorR,double airflow,double other) {
    nonnegative(i,"current"); positive(r,"resistance"); positive(motorR,"thermal resistance"); positive(airflow,"airflow"); nonnegative(other,"other heat");
    const double airR=1/(1.18*1006*airflow),alpha=0.00393;
    // Two phases RMS, copper resistance feedback, coupled motor/cabinet nodes.
    const double base=2*i*i*r;
    const double denom=1-base*alpha*(motorR+airR);
    if(denom<=0) throw std::domain_error("Linear thermal model predicts runaway; reject operating point");
    const double loss=base*(1+alpha*(ambient+airR*other-20))/denom;
    const double cabinet=ambient+airR*(other+loss);
    return {cabinet+motorR*loss,cabinet,loss};
}
double requiredAirflow(double heat,double rise,double density,double cp) {
    nonnegative(heat,"heat"); positive(rise,"temperature rise"); positive(density,"air density"); positive(cp,"heat capacity");
    return heat/(density*cp*rise);
}
CutEstimate cuttingScreen(double copper,double od,double shear,double polymer,double mu,double edge,double indent) {
    positive(copper,"copper area"); positive(od,"wire diameter"); positive(shear,"copper shear"); nonnegative(polymer,"polymer shear");
    nonnegative(mu,"friction"); nonnegative(edge,"edge radius"); nonnegative(indent,"indentation pressure");
    const double area=pi*od*od/4;
    if(area<copper) throw std::invalid_argument("Conductor exceeds whole wire area");
    // Upper-bound full-area shearing screen. No progressive V-edge benefit claimed.
    const double ideal=(copper*shear+(area-copper)*polymer)*(1+mu);
    const double worn=ideal+2*edge*od*indent;
    return {ideal,worn,worn*od*1e-3};
}
std::vector<Check> analyticalChecks() {
    namespace c=head::catalogue; namespace d=head::design;
    std::vector<Check> out;
    const auto check=[&](std::string id,std::string category,double value,double limit,std::string unit,std::string source,bool below=true) {
        if(!std::isfinite(value)||!std::isfinite(limit)) throw std::runtime_error("Non-finite engineering result");
        out.push_back({std::move(id),std::move(category),std::move(unit),std::move(source),value,limit,below?value<=limit:value>=limit});
    };
    const double rpm=d::blade_speed_mm_s/c::lead*60;
    const double solidScrewInertia=7850*pi*std::pow(0.015,4)*0.340/32; // Upper inertia, not root stiffness.
    const double inertia=c::rotor_inertia_kg_m2+c::coupling_inertia_kg_m2+solidScrewInertia;
    const double demand=motorDemand(d::working_thrust_n,0.01,d::efficiency_lower,0.3,d::carriage_payload_limit_kg,inertia,c::start_torque_nm);
    const double available=(interpolateTorque(rpm)-c::curve_reading_allowance_nm)*d::running_curve_fraction;
    check("motor_running_margin","conditional_design",available/demand,d::minimum_running_margin,"ratio","60V/5.7Arms catalogue curve;80% qualification reserve",false);
    check("coupling_working_torque","catalogue_screen",demand,c::coupling_torque_nm,"Nm","MK2/100 catalogue");
    const double stall=2*pi*c::motor_holding_nm/0.01; // eta=1 worst possible thrust conversion.
    check("fixed_bearing_stall_thrust","catalogue_screen",stall,c::custom_fixed_end_max_n,"N","BK12-class supplier-stated axial rating, verify at receipt");
    check("ballscrew_static_factor","catalogue_screen",c::custom_screw_static_n/stall,2,"ratio","SFU1610-class supplier-stated static capacity / calculated stall thrust",false);
    const double momentPitch=d::working_thrust_n*0.040+d::vertical_load_n*0.123;
    const double momentYaw=d::transverse_load_n*0.123;
    const double momentRoll=d::transverse_load_n*0.040;
    // Conservative linear utilisation screen, not a manufacturer combined-rating equation.
    const double usage=momentPitch/c::custom_block_moment_pitch_nm+momentYaw/c::custom_block_moment_yaw_nm+momentRoll/c::custom_block_moment_roll_nm+
        std::hypot(d::transverse_load_n,d::vertical_load_n)/c::custom_guide_static_n;
    check("guide_combined_static_screen","conditional_design",usage,0.5,"utilisation","Linear sum; imposed2500/750/375N load case; required factor2");
    const double lifeLoad=equivalentLoad({d::working_thrust_n,200,50},{0.20,0.30,0.50});
    check("screw_basic_l10_revolutions","informational",lifeRevolutions(c::custom_screw_dynamic_n,lifeLoad),1e6,"rev","SFU1610-class supplier-stated dynamic capacity; illustrative spectrum; NOT machine life",false);
    check("coupling_elastic_position","catalogue_screen",demand/c::coupling_stiffness_nm_rad*10/(2*pi),0.005,"mm","Catalogue torsional stiffness; one axis");
    check("pulse_frequency","catalogue_screen",d::blade_speed_mm_s*d::steps_per_mm,40000,"Hz","Shared controller pulse ceiling");
    check("holder_axial_average_stress","conditional_design",d::working_thrust_n/(d::holder_arm_width*d::holder_arm_height),d::specified_yield_pa/1e6/d::structural_factor,"MPa","Certified650MPa steel requirement; local stresses require FEA");
    const auto joint=boltedJoint(4,9000,d::vertical_load_n,0.25,0.15,20.1e-6,970e6);
    check("holder_joint_slip","conditional_design",d::working_thrust_n,joint.shear_capacity_n,"N","M6 class12.9; required measured9kN preload; qualify mu>=0.15");
    check("holder_joint_bolt_proof","conditional_design",joint.proof_utilisation,0.75,"utilisation","ISO898 class12.9 proof970MPa; required preload, not torque prediction");
    check("wire_tension_error_0p5mm2_1m","sensitivity",copperExtension(5,1,0.5)*1000,0.70,"mm","110GPa copper, imposed5N tension; strand/layer compliance excluded");
    check("encoder_runout_error_bound","catalogue_screen",0.2,0.2,"mm","Kuebler0.2mm radialTIR => first-order arbitrary-phase error<=0.2mm");
    // Deliberately exposes failure: catalogue pulse spacing cannot justify strip precision.
    const double angleRad=2*pi/4000;
    check("motor_encoder_count_spacing","informational",angleRad/(2*pi)*10,0.2,"mm","4000CPR; resolution only");
    const double power=d::working_thrust_n*d::blade_speed_mm_s*1e-3/d::efficiency_lower+
                       2*c::curve_rms_a*c::curve_rms_a*c::motor_resistance_ohm*1.4+20;
    check("axis_power_screen","conditional_design",power,c::supply_voltage*c::supply_current_a*0.70,"W","70% electrical budget; winding+driver loss factors are qualification loads");
    check("driver_at_40C_room_with_5C_rise","design_failure",45,c::driver_max_ambient_c,"degC","Driver manual maximum40C; ventilation alone cannot cool below room");
    check("uncalibrated_length_budget_1m","design_failure",0.2+0.05+factoryWheelLengthError(1000)+0.15+0.25,0.70,"mm",
          "Bounded sum:0.2runout/0.05quantisation/1.0factory_circumference/0.15tension/0.25slip;Kuebler100plusminus0.1mm;other terms are allocations,not measured errors");
    return out;
}
}