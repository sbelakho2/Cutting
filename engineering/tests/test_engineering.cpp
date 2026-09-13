#include "models.hpp"
#include "fem.hpp"
#include "field_checks.hpp"
#include "shared/cutting_head_spec.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>

using namespace starz::engineering;
namespace {
int checks=0;
void expect(bool ok,const char* message) { ++checks; if(!ok) throw std::runtime_error(message); }
void near(double a,double b,double tolerance,const char* message) { expect(std::abs(a-b)<=tolerance,message); }
void throws(const std::function<void()>& f) {
    bool caught=false; try {f();} catch(const std::exception&) {caught=true;} expect(caught,"Invalid model input was accepted");
}
}
int main() try {
    near(interpolateTorque(75),7.475,1e-12,"Catalogue curve interpolation");
    throws([]{interpolateTorque(0);}); throws([]{interpolateTorque(1600);});
    near(motorDemand(1000,0.01,1,0,1,0,0),10/(2*std::numbers::pi),1e-12,"Thrust/lead SI conversion");
    throws([]{motorDemand(1000,0.01,1.1,0,1,0,0);});
    near(equivalentLoad({100,100},{0.1,0.9}),100,1e-10,"Constant equivalent load");
    near(lifeRevolutions(100,100),1e6,1e-6,"Basic life definition");
    near(eulerLoad(1,1,1,1),std::numbers::pi*std::numbers::pi,1e-12,"Euler dimension check");
    expect(shaftCriticalRpm(205e9,7850,0.01,0.2)<40000,"Critical speed unit regression: millions of RPM");
    near(shaftCriticalRpm(205e9,7850,0.01,0.4)*4,shaftCriticalRpm(205e9,7850,0.01,0.2),1e-8,"Critical speed L squared scaling");
    const auto contact=sphereContact(10,0.01,1e8);
    near(2*std::numbers::pi*contact.radius_m*contact.radius_m*contact.pressure_pa/3,10,1e-12,"Hertz integrated load");
    near(copperExtension(5,1,0.5),5/(110e9*0.5e-6),1e-15,"Copper extension units");
    expect(wireBuckling(0.5,0.2,0.1)<1,"Strand bundle incorrectly modelled as solid rod");
    near(eccentricWheelError(100,0.2,100,0.5),0,1e-15,"Full wheel revolution cancels eccentric error");
    for(int i=0;i<101;++i) expect(std::abs(eccentricWheelError(100,0.2,i*2.71,i))<=0.2,"Runout error bound");
    auto bus=dcBus(0.01,1,0.01,100,60,0,0.1);
    near(bus.final_v,60*std::exp(-1),1e-9,"RC analytical decay");
    near(bus.balance_error_j,0,1e-10,"DC bus energy balance");
    auto busFine=dcBus(0.0007,1,0.01,100,60,4,0.05);
    auto busCoarse=dcBus(0.013,1,0.01,100,60,4,0.05);
    near(busFine.final_v,busCoarse.final_v,1e-8,"DC bus discontinuity integration");
    auto oscillator=freeOscillator(1e-4,0.2,1,1000,0,0.01,0);
    near(oscillator.final_energy,oscillator.initial_energy,1e-10,"Newmark undamped energy conservation");
    auto damped=freeOscillator(1e-4,0.2,1,1000,2,0.01,0);
    near(damped.balance_error,0,1e-9,"Damped oscillator energy balance");
    expect(damped.final_energy<damped.initial_energy,"Damping creates energy");
    auto coarse=freeOscillator(0.01,0.2,1,1000,0,0.01,0);
    auto fine=freeOscillator(0.005,0.2,1,1000,0,0.01,0);
    const double exact=0.01*std::cos(std::sqrt(1000.)*0.2);
    expect(std::abs(fine.final_x-exact)<std::abs(coarse.final_x-exact),"Time-step refinement must reduce phase error");
    const auto thermal=thermalSteady(5.7,0.54,25,0.8,0.02,20);
    expect(thermal.motor_c>thermal.cabinet_c && thermal.cabinet_c>25,"Thermal node ordering");
    throws([]{thermalSteady(20,1,25,10,0.001,100);});
    auto spectrum=analyticalChecks();
    expect(!spectrum.empty(),"Engineering report empty");
    expect(std::any_of(spectrum.begin(),spectrum.end(),[](const auto& c){return !c.passed;}),"Unqualified design must not claim all checks pass");
    // Constant-strain3D patch test: all four components of affine displacement
    // follow uniaxial stress and Poisson contraction, irrespective of tetrahedron size.
    const auto mesh=meshBox(40,20,12,6);
    ElasticMaterial material{205e9,0.29,7850};
    const auto bc=[](const Eigen::Vector3d& p) {
        Constraint c{}; if(std::abs(p.x())<1e-7)c[0]=0;
        if(std::abs(p.y())<1e-7)c[1]=0; if(std::abs(p.z())<1e-7)c[2]=0; return c;
    };
    const LoadPatch load{[](const Eigen::Vector3d& p){return std::abs(p.x()-40)<1e-7;},{1000,0,0},std::nullopt};
    const auto result=solveElastic(mesh,material,bc,{load});
    const double stress=1000/(0.020*0.012),strain=stress/material.young_pa;
    near(result.volume_m3,0.040*0.020*0.012,1e-12,"Tet integration volume");
    double maxError=0;
    for(std::size_t i=0;i<mesh.nodes_mm.size();++i) {
        const Eigen::Vector3d p=mesh.nodes_mm[i]*1e-3;
        const Eigen::Vector3d target{strain*p.x(),-0.29*strain*p.y(),-0.29*strain*p.z()};
        maxError=std::max(maxError,(result.displacement_m[i]-target).norm());
    }
    expect(maxError<1e-11,"Constant-strain patch test failed");
    near(result.max_stress_pa,stress,stress*1e-7,"FEM stress recovery");
    expect(result.force_balance_relative<1e-9,"FEM reaction balance");
    expect(result.moment_balance_relative<1e-9,"FEM moment balance");
    near(result.energy_j,0.5*1000*strain*0.040,1e-9,"FEM strain energy");
    const Eigen::Vector3d applied{10,100,-50},application{40,30,20};
    const LoadPatch momentLoad{[](const Eigen::Vector3d& p){return std::abs(p.x()-40)<1e-7;},applied,application};
    const auto momentResult=solveElastic(mesh,material,bc,{momentLoad});
    expect((momentResult.total_load_n-applied).norm()<1e-8,"Load patch force preservation");
    expect((momentResult.total_load_moment_nm-(application*1e-3).cross(applied)).norm()<1e-9,"Load patch moment preservation");
        const Eigen::Vector3d couple{0,8.5,0};
        const LoadPatch pureTorque{load.select,Eigen::Vector3d::Zero(),std::nullopt,couple};
        const auto torqueResult=solveElastic(mesh,material,bc,{pureTorque});
        expect(torqueResult.total_load_n.norm()<1e-9,"Pure torque creates no net force");
        expect((torqueResult.total_load_moment_nm-couple).norm()<1e-9,"Exact patch torque preservation");
        const Eigen::Vector3d gravity{0,0,-9.80665};
        const auto gravityResult=solveElastic(mesh,material,bc,{},0,gravity);
        const Eigen::Vector3d weight=gravity*(material.density_kg_m3*0.040*0.020*0.012);
        expect((gravityResult.total_load_n-weight).norm()<1e-10,"Body-force integration preserves actual mesh mass");
        expect((gravityResult.total_load_moment_nm-Eigen::Vector3d{0.020,0.010,0.006}.cross(weight)).norm()<1e-11,
            "Body-force integration preserves centre of gravity");
        expect((gravityResult.total_reaction_n+weight).norm()<1e-9,"Gravity support reactions");
        auto twice=load; twice.force_n*=2;
        const auto cases=solveElasticCases(mesh,material,bc,{{load},{twice}});
        expect(cases.size()==2,"Multiple-load-case result count");
        near(cases[0].energy_j,result.energy_j,1e-12,"Factor reuse leaves original solution unchanged");
        near(cases[1].energy_j,4*cases[0].energy_j,1e-11,"Double force gives four times strain energy");
        near(cases[1].max_displacement_m,2*cases[0].max_displacement_m,1e-12,"Load cases must not accumulate forces");
    TetMesh faceMesh; ElasticResult faceResult;
    for(double y:{91.,99.,107.}) for(double z:{12.,26.,40.}) {
        faceMesh.nodes_mm.emplace_back(-3.01,y,z);
        faceResult.displacement_m.emplace_back((0.003+0.0001*(y-123)-0.0002*(z-26))*1e-3,0,0);
    }
    const auto faceMotion=bladeFaceMotion(faceMesh,faceResult,-3.01,123,26);
    near(faceMotion.apex_normal_mm,0.003,1e-12,"Blade face displacement extrapolation");
    near(faceMotion.normal_slope_y,0.0001,1e-12,"Blade yaw fit");
    near(faceMotion.normal_slope_z,-0.0002,1e-12,"Blade roll fit");
    near(faceMotion.residual_rms_mm,0,1e-12,"Rigid blade seat fit residual");
    // Independent bending/modal benchmark, not just a constant-strain patch.
    const auto cantileverClamp=[](const Eigen::Vector3d& p) {
        Constraint c{}; if(std::abs(p.x())<1e-7)c={0.,0.,0.}; return c;
    };
    const LoadPatch tipLoad{[](const Eigen::Vector3d& p){return std::abs(p.x()-120)<1e-7;},{0,10,0},std::nullopt};
    const double beamExact=cantileverDeflection(10,0.120,205e9,0.012,0.012,0.29);
    double previousError=1;
    for(double size:{3.,1.5}) {
        const auto beamMesh=meshBox(120,12,12,size);
        const auto beam=solveElastic(beamMesh,material,cantileverClamp,{tipLoad},2);
        const double error=std::abs(beam.load_average_displacement_m-beamExact)/beamExact;
        expect(error<previousError,"Tet bending must improve with mesh refinement"); previousError=error;
        if(size==1.5) {
            expect(error<0.06,"Fine cantilever versus Timoshenko reference");
            const double area=0.012*0.012,inertia=std::pow(0.012,4)/12;
            const double frequency=std::pow(1.875104,2)/(2*std::numbers::pi*std::pow(0.120,2))*std::sqrt(205e9*inertia/(7850*area));
            expect(std::abs(beam.modal_hz[0]-frequency)/frequency<0.06,"Fine FEM fundamental frequency versus Euler-Bernoulli limit");
            expect(beam.modal_residual[0]<1e-6,"Modal eigen residual");
        }
    }
    std::cout<<checks<<" numerical/physical-consistency checks passed. No hardware qualification implied.\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }