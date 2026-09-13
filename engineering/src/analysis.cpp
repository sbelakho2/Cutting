#include "models.hpp"
#include "fem.hpp"
#include "head_geometry.hpp"
#include "field_checks.hpp"
#include "shared/cutting_head_spec.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace h=starz::head;
namespace c=h::catalogue;
namespace d=h::design;
namespace {
void studies(const fs::path& out,std::ostream& report) {
    std::ofstream f(out/"sensitivity.csv");
    f<<"study,input_a,input_b,result,unit,status\n"<<std::setprecision(12);
    const double inertia=c::rotor_inertia_kg_m2+c::coupling_inertia_kg_m2+
        7850*std::numbers::pi*std::pow(0.015,4)*0.340/32;
    for(double force:{500.,1000.,1500.,2000.,2500.,3000.}) for(double speed:{10.,12.5,15.,20.,25.}) {
        const double available=(se::interpolateTorque(speed/c::lead*60)-c::curve_reading_allowance_nm)*d::running_curve_fraction;
        const double demand=se::motorDemand(force,0.01,0.90,0.3,d::carriage_payload_limit_kg,inertia,0.150);
        const double margin=available/demand;
        f<<"motor_margin,"<<force<<','<<speed<<','<<margin<<",ratio,"<<(margin>=1.25?"within_screen":"outside_screen")<<'\n';
    }
    for(double shear:{160.,240.,320.}) for(double edge:{0.,0.01,0.03,0.05}) {
        const auto cut=se::cuttingScreen(2.5,4.5,shear,25,0.30,edge,350);
        f<<"cutting_force_screen,"<<shear<<','<<edge<<','<<cut.worn_n<<",N,uncalibrated_material_sweep\n";
        f<<"cutting_work_bound,"<<shear<<','<<edge<<','<<cut.energy_j<<",J,full_force_full_diameter_upper_bound\n";
    }
    for(double length:{0.2,1.,3.}) for(double tension:{1.,3.,5.,10.,20.}) {
        f<<"wire_0p5mm2_extension,"<<length<<','<<tension<<','<<se::copperExtension(tension,length,0.5)*1000<<",mm,solid_copper_elastic_lower_compliance\n";
    }
    for(double free:{0.01,0.02,0.05,0.1}) for(double strand:{0.1,0.2,0.3})
        f<<"wire_strand_buckling,"<<free<<','<<strand<<','<<se::wireBuckling(0.5,strand,free)<<",N,freely_sliding_strands_no_polymer_support\n";
    for(double normal:{5.,10.,20.,30.}) for(double mu:{0.15,0.3,0.5,0.65})
        f<<"two_driven_nip_traction,"<<normal<<','<<mu<<','<<2*normal*mu<<",N,not_four_driven_contacts\n";
    for(double normal:{5.,10.,20.,30.}) for(double modulus:{5e6,20e6,100e6}) {
        auto contact=se::sphereContact(normal,0.002,modulus);
        f<<"point_contact_indentation_screen,"<<normal<<','<<modulus<<','<<contact.indentation_m*1000<<",mm,NOT_crossed_cylinder_solution_or_damage_acceptance\n";
    }
    for(double temperature:{20.,25.,30.,35.,40.}) for(double thermalR:{0.5,0.8,1.2,1.6}) {
        const auto t=se::thermalSteady(c::curve_rms_a,0.54,temperature,thermalR,0.02,30);
        f<<"motor_hotspot_screen,"<<temperature<<','<<thermalR<<','<<t.motor_c<<",degC,thermal_resistance_sensitivity_not_measured\n";
        f<<"driver_cabinet_temperature,"<<temperature<<','<<thermalR<<','<<t.cabinet_c<<",degC,"<<(t.cabinet_c<=40?"within_manual":"outside_manual")<<'\n';
    }
    for(double phase=0;phase<6.3;phase+=0.3) for(double length:{25.,50.,100.,200.,333.,1000.})
        f<<"measuring_wheel_eccentricity,"<<phase<<','<<length<<','<<se::eccentricWheelError(100,0.2,length,phase)<<",mm,first_order_radialTIR_model\n";
    // Nonprobabilistic bounded sum. Unknown biases are not made Gaussian.
    for(double scalePpm:{50.,100.,500.,1000.}) for(double length:{200.,1000.,3000.})
        f<<"length_bounded_budget,"<<scalePpm<<','<<length<<','<<(scalePpm*length*1e-6+0.2+0.05+0.15+0.25)<<",mm,calibration_tension_slip_allocations_not_measured\n";
    // All bus values below are study inputs. No unspecified capacitor/dump part is accepted.
    for(double cap:{0.001,0.0022,0.0047,0.01}) for(double returned:{0.1,0.5,1.,5.}) {
        const auto bus=se::dcBus(0.001,1,cap,2200,60,returned,0.05);
        f<<"isolated_bus_regeneration,"<<cap<<','<<returned<<','<<bus.maximum_v<<",V,protective_hardware_not_yet_selected\n";
        f<<"bus_energy_balance,"<<cap<<','<<returned<<','<<bus.balance_error_j<<",J,numerical_balance\n";
    }
    for(double dt:{0.002,0.001,0.0005,0.00025}) {
        const auto dancer=se::dancerResponse(dt,1,0.4,12,60,0.5);
        double peak=0; for(const auto& s:dancer) peak=std::max(peak,std::abs(s.tension));
        f<<"dancer_peak_tension,"<<dt<<",0.4,"<<peak<<",N,linear_transfer_model_not_selected_payoff\n";
    }
    report<<"\n## Sensitivity studies\n\n"
      <<"The companion sensitivity table sweeps actual motor-curve operating points, imposed wire/shear/wear loads, "
      <<"strand buckling, tension relaxation, nip traction, point-contact screening, thermal resistance, wheel phase/runout, "
      <<"bounded calibration error, bus capacitance/returned energy and dancer time step. Sweeps are **not multiple machine designs**. "
      <<"They expose which physical properties must be controlled or measured for the single design.\n\n"
      <<"The contact sphere is a dimensional/nonlinearity screen only; it is not passed off as a crossed rubber-roller/insulated-wire "
      <<"Hertz solution. No no-damage conclusion follows from it. Archard wear is not used to invent a blade replacement interval.\n";
    if(!f) throw std::runtime_error("Sensitivity output failed");
}
int finiteElements(const fs::path& out,std::ostream& report,const std::string& selectedPart,
                   const std::vector<double>& meshSizes) {
    fs::create_directories(out/"fem");
    std::ofstream data(out/"fem"/"convergence.csv");
    std::ofstream criteria(out/"fem"/"criteria.csv");
    std::ofstream peaks(out/"fem"/"peak-locations.csv");
    peaks<<"part,load_case,h_mm,peak_vm_MPa,element_centroid_x_mm,element_centroid_y_mm,element_centroid_z_mm,nearest_constrained_node_mm\n";
    criteria<<"check,value,limit,result\n"<<std::flush; // No stale previous-revision criteria while solving.
        data<<"part,load_case,h_mm,nodes,tetrahedra,volume_m3,loaded_displacement_mm,max_displacement_mm,max_vm_MPa,volume_p99_vm_MPa,energy_J,force_balance,moment_balance,free_residual,mode1_Hz,mode1_residual\n";
    const se::ElasticMaterial material{d::structural_young_pa,d::poisson,d::density_kg_m3};
        const Eigen::Vector3d gravity{0,0,-9.80665};
    const auto peakLocation=[&](const se::TetMesh& mesh,const se::ElasticResult& result,const se::Constraints& bc) {
        const auto at=static_cast<std::size_t>(std::max_element(result.von_mises_pa.begin(),result.von_mises_pa.end())-result.von_mises_pa.begin());
        Eigen::Vector3d centre=Eigen::Vector3d::Zero();
        for(int id:mesh.tetrahedra.at(at))centre+=mesh.nodes_mm.at(static_cast<std::size_t>(id))/4;
        double nearest=std::numeric_limits<double>::infinity();
        for(const auto& node:mesh.nodes_mm) {
            const auto fixed=bc(node);
            if(std::any_of(fixed.begin(),fixed.end(),[](const auto& v){return v.has_value();}))
                nearest=std::min(nearest,(centre-node).norm());
        }
        return std::pair<Eigen::Vector3d,double>{centre,nearest};
    };
    const auto row=[&](const std::string& part,std::size_t loadCase,double size,const se::TetMesh& mesh,
              const se::ElasticResult& result,const se::Constraints& bc) {
            data<<std::setprecision(12)<<part<<','<<loadCase<<','<<size<<','<<mesh.nodes_mm.size()<<','<<mesh.tetrahedra.size()<<','<<result.volume_m3<<','
                <<result.load_average_displacement_m*1000<<','<<result.max_displacement_m*1000<<','<<result.max_stress_pa/1e6<<','<<result.p99_stress_pa/1e6<<','
                <<result.energy_j<<','<<result.force_balance_relative<<','<<result.moment_balance_relative<<','<<result.free_residual_relative<<','
                <<(result.modal_hz.empty()?0:result.modal_hz.front())<<','<<(result.modal_residual.empty()?0:result.modal_residual.front())<<'\n'<<std::flush;
            const auto [centre,nearest]=peakLocation(mesh,result,bc);
            peaks<<std::setprecision(12)<<part<<','<<loadCase<<','<<size<<','<<result.max_stress_pa/1e6<<','
                 <<centre.x()<<','<<centre.y()<<','<<centre.z()<<','<<nearest<<'\n'<<std::flush;
    };
    const auto refinement=[](const std::vector<double>& values) {return std::abs(values.back()-values[values.size()-2])/values.back();};
    int failures=0;
    std::ofstream loadTable(out/"fem"/"load-cases.csv");
    loadTable<<"part,load_case,Fx_N,Fy_N,Fz_N,note\n";
    if(selectedPart=="all"||selectedPart=="holder") {
    se::writeStep(se::holderShape(),out/"fem"/"holder.step");
    const auto clamp=[](const Eigen::Vector3d& p) {
        se::Constraint fixed{}; if(std::abs(p.z())<1e-6) fixed={0.,0.,0.}; return fixed;
    };
    std::vector<std::vector<se::LoadPatch>> holderLoads;
    for(double factor:{1.,d::proof_load_n/d::working_thrust_n}) for(int sx:{-1,1}) for(int sy:{-1,1}) for(int sz:{-1,1}) {
        const Eigen::Vector3d force=factor*Eigen::Vector3d{sx*d::transverse_load_n,sy*d::working_thrust_n,sz*d::vertical_load_n};
        loadTable<<"holder,"<<holderLoads.size()<<','<<force.x()<<','<<force.y()<<','<<force.z()<<",apex force plus actual mesh gravity\n";
        holderLoads.push_back({{[](const Eigen::Vector3d& p) {
            return std::abs(p.x()-d::blade_seat_x)<1e-5 && p.y()>d::blade_mount_y-10 && p.y()<d::blade_mount_y+10 && p.z()>10 && p.z()<43;
        },force,Eigen::Vector3d{d::blade_seat_x,d::blade_apex_y,26}}});
    }
    std::vector<double> displacement,volume,stress,frequency;
    se::BladeFaceMotion face{};
    double proofStress=0; std::size_t worstFaceCase=0;
    for(double size:meshSizes.empty()?std::vector<double>{4.5,3.,2.,1.6}:meshSizes) {
        const auto mesh=se::meshStep(out/"fem"/"holder.step",size);
        std::cout<<"Meshed holder h="<<size<<"mm nodes="<<mesh.nodes_mm.size()<<std::endl;
        const auto results=se::solveElasticCases(mesh,material,clamp,holderLoads,3,gravity);
        double maxU=0,maxStress=0; proofStress=0; face={};
        for(std::size_t i=0;i<results.size();++i) {
            const auto& result=results[i]; row("holder",i,size,mesh,result,clamp);
            if(i<8) {
                maxU=std::max(maxU,result.load_average_displacement_m); maxStress=std::max(maxStress,result.max_stress_pa);
                const auto fit=se::bladeFaceMotion(mesh,result,d::blade_seat_x,d::blade_apex_y,26);
                if(fit.worst_normal_mm>face.worst_normal_mm) {face=fit; worstFaceCase=i;}
            } else proofStress=std::max(proofStress,result.max_stress_pa);
        }
        displacement.push_back(maxU); volume.push_back(results.front().volume_m3); stress.push_back(maxStress); frequency.push_back(results.front().modal_hz.front());
        se::writeVtu(out/"fem"/("holder-h"+std::to_string(size)+".vtu"),mesh,results[worstFaceCase]);
        std::cout<<"FEM holder h="<<size<<"mm working envelope="<<maxStress/1e6<<"MPa, rigid-blade pair gap="<<d::blade_face_gap-2*face.worst_normal_mm<<"mm\n";
    }
    const double du=refinement(displacement),df=refinement(frequency),ds=refinement(stress);
        const double gapBound=d::blade_face_gap-2*face.worst_normal_mm;
        const bool stressPass=stress.back()<=d::specified_yield_pa/d::structural_factor;
        const bool proofPass=proofStress<=d::specified_yield_pa/1.5;
        failures+=static_cast<int>(du>=0.03)+static_cast<int>(df>=0.03)+static_cast<int>(ds>=0.05)+
                                 static_cast<int>(!stressPass)+static_cast<int>(!proofPass)+static_cast<int>(gapBound<=0);
        criteria<<"holder_displacement_refinement,"<<du<<",0.03,"<<(du<0.03?"PASS":"REFINE")<<'\n'
            <<"holder_frequency_refinement,"<<df<<",0.03,"<<(df<0.03?"PASS":"REFINE")<<'\n'
            <<"holder_peak_stress_refinement,"<<ds<<",0.05,"<<(ds<0.05?"PASS":"REFINE")<<'\n'
            <<"holder_peak_stress_Pa,"<<stress.back()<<','<<d::specified_yield_pa/d::structural_factor<<','<<(stressPass?"PASS":"FAIL")<<'\n'
            <<"holder_proof_stress_Pa,"<<proofStress<<','<<d::specified_yield_pa/1.5<<','<<(proofPass?"PASS_SCOPED":"FAIL")<<'\n'
            <<"isolated_rigid_blade_gap_bound_mm,"<<gapBound<<",0,"<<(gapBound>0?"PASS_SCOPED":"FAIL")<<'\n';
    report<<"\n## Three-dimensional finite elements\n\n"
      <<"Linear small-strain isotropic elasticity, actual custom holder STEP, Gmsh4-node tetrahedra, Eigen sparse LDLT. "
      <<"E="<<material.young_pa/1e9<<"GPa, Poisson="<<material.poisson<<", density="<<material.density_kg_m3<<"kg/m³. "
    <<"Holder transverse backbone width="<<d::holder_arm_width<<"mm, height="<<d::holder_arm_height<<"mm. "
        <<"The underside is ideally fixed. All eight sign combinations of (±750,±2500,±375)N are applied at the actual blade apex, "
            <<"with exact force/moment-preserving transfer to the blade seat and actual holder mesh self-weight. "
            <<"Eight additional proof cases double those imposed forces to5000N axial. These bound specified qualification loads, not measured cutting forces. "
            <<"The rear holder uses the same local-coordinate envelope under its180° assembly rotation.\n\n"
      <<"| Last refinement metric | Relative change | Criterion | Result |\n|---|---:|---:|---|\n"
      <<"| Load-patch displacement | "<<du*100<<"% |3% |"<<(du<0.03?"PASS":"REFINE")<<" |\n"
      <<"| First isolated-holder frequency | "<<df*100<<"% |3% |"<<(df<0.03?"PASS":"REFINE")<<" |\n\n"
    <<"Peak-stress last-refinement change="<<ds*100<<"%;5% criterion: **"<<(ds<0.05?"PASS":"REFINE")<<"**.\n\n"
      <<"Final maximum element von Mises stress="<<stress.back()/1e6<<"MPa, specified650MPa yield /3="<<650./3<<"MPa. "
      <<"This is not a whole-head stress qualification. Both raw maximum and volume-weighted99th percentile are exported; "
      <<"neither replaces a notch/convergence investigation. Sharp re-entrant corners, artificial clamp edges and threads can give "
    <<"mesh-dependent local stress. The actual3mm nose fillet is modelled; bolted contact, screw/guide compliance, preload loss and blade material are not included. "
    <<"Proof-load peak="<<proofStress/1e6<<"MPa, yield/1.5 screen: **"<<(proofPass?"PASS_SCOPED":"FAIL")<<"**.\n\n"
      <<"The first three modes use lumped consistent-total mass and mass-orthonormal inverse subspace/Rayleigh-Ritz iteration. "
      <<"Eigen residuals are reported. These are **isolated holder modes**, not machine modal frequencies.\n";
        report<<"\n### Loaded blade-face clearance\n\n"
            <<"A least-squares normal-motion plane is fitted to the actual loaded blade-seat node displacements. "
            <<"Governing working-load case="<<worstFaceCase<<" (see load-cases table). The fitted blade apex normal displacement is "<<face.apex_normal_mm<<"mm; worst absolute displacement across "
            <<"the full49×38mm rigid-blade envelope="<<face.worst_normal_mm<<"mm. Two-holder worst-direction bound: nominal"
            <<d::blade_face_gap<<" −2×"<<face.worst_normal_mm<<" = **"<<gapBound<<"mm**: **"<<(gapBound>0?"PASS_SCOPED":"FAIL")<<"**. "
            <<"Seat normal slopes are "<<face.normal_slope_y<<" and "<<face.normal_slope_z<<"rad to first order; fit RMS="<<face.residual_rms_mm<<"mm. "
            <<"This rigid-blade projection excludes blade flexure, screw/guide compliance, fixture slip and bearing error. "
            <<"A positive result is not an assembled clearance guarantee; a negative bound requires further design work.\n";
            }
    // Mesh refinement for the other custom load-bearing parts. Conditions remain
    // idealised bolted-region clamps, not a claimed full contact/assembly solve.
            if(selectedPart=="all"||selectedPart=="adapter")se::writeStep(se::adapterShape(),out/"fem"/"adapter.step");
            if(selectedPart=="all"||selectedPart=="datum")se::writeStep(se::datumShape(),out/"fem"/"datum.step");
    const double f0PcdRadius=35/std::sqrt(2.);
    // H003 support boundary: four ISO4762 M5x16 class10.9 screws with ISO7092
    // M5 washers (OD9mm) clamp the adapter to the stage F0 face. The F0 screw
    // passes through a Ø5.5mm clearance hole, so no shank bearing transfers
    // load; the clamp reaches the stage interface as seat compression bounded
    // by the washer footprint. The ideal fixed support is therefore the
    // stage-face material annulus r∈[2.75,4.5]mm around each PCD70 hole axis,
    // between the hole edge and the washer OD. This replaces the earlier
    // arbitrary square node patch; it remains an ideal clamp, not a
    // contact/preload/friction solve.
    const auto adapterClamp=[f0PcdRadius](const Eigen::Vector3d& p) {
        se::Constraint fixed{};
        if(std::abs(p.y())<1e-6) for(double x:{-f0PcdRadius,f0PcdRadius}) for(double z:{-f0PcdRadius,f0PcdRadius}) {
            const double r=std::hypot(p.x()-x,p.z()-z);
            if(r>2.75-1e-7 && r<4.5+1e-7) { fixed={0.,0.,0.}; return fixed; }
        }
        return fixed;
    };
    // Load boundary: the actual 85.85mm square motor-flange bearing footprint
    // on the motor-side face, outside the Ø73.10mm pilot bore (the Ø73mm pilot
    // is a0.10mm-diametral clearance fit and carries no load) and clear of the
    // four Ø6.5mm motor mounting holes. The unchanged resultant force/couple is
    // distributed over this footprint instead of arbitrary screw-hole boxes.
    std::vector<std::vector<se::LoadPatch>> adapterLoads;
    for(int sign:{-1,1}) adapterLoads.push_back({{[](const Eigen::Vector3d& p) {
        if(std::abs(p.y()+d::adapter_thickness)>1e-5) return false;
        if(std::abs(p.x())>c::motor_width/2+1e-7 || std::abs(p.z())>c::motor_width/2+1e-7) return false;
        if(std::hypot(p.x(),p.z())<d::adapter_motor_pilot_diameter/2-1e-7) return false;
        for(double x:{-c::motor_pitch/2,c::motor_pitch/2}) for(double z:{-c::motor_pitch/2,c::motor_pitch/2})
            if(std::hypot(p.x()-x,p.z()-z)<3.25-1e-7) return false;
        return true;
    },{0,0,-150},Eigen::Vector3d{0,-d::adapter_thickness-c::motor_rear,0},{0,sign*c::motor_holding_nm,0}}});
    struct RefineSummary {
        double deltaU=0,deltaS=0,u=0,s=0,p99=0; int levels=0; bool localAdded=false;
        double uniformDeltaU=0,uniformDeltaS=0;
        Eigen::Vector3d peakCentre=Eigen::Vector3d::Zero(); double peakNearest=0;
    };
    const auto refinePart=[&](const std::string& part,const std::vector<double>& sizes,const se::Constraints& bc,
                              const std::vector<std::vector<se::LoadPatch>>& loads,double localSize=0,
                              double localRadius=0) {
        std::vector<double> u,s; double finalP99=0; RefineSummary summary{};
        Eigen::Vector3d peakCentre=Eigen::Vector3d::Zero(); double peakNearest=0;
        const auto runLevel=[&](const se::TetMesh& mesh,double size,const std::string& vtuSuffix={}) {
            const auto results=se::solveElasticCases(mesh,material,bc,loads,0,gravity);
            double maxU=0,maxS=0; std::size_t governing=0,peakCase=0;
            for(std::size_t i=0;i<results.size();++i) {
                row(part,i,size,mesh,results[i],bc);
                if(results[i].max_displacement_m>maxU) {maxU=results[i].max_displacement_m; governing=i;}
                if(results[i].max_stress_pa>maxS) {maxS=results[i].max_stress_pa; peakCase=i;}
                finalP99=std::max(finalP99,results[i].p99_stress_pa);
            }
            const auto [centre,nearest]=peakLocation(mesh,results[peakCase],bc);
            peakCentre=centre; peakNearest=nearest;
            u.push_back(maxU); s.push_back(maxS); ++summary.levels;
            se::writeVtu(out/"fem"/(part+"-h"+std::to_string(size)+vtuSuffix+".vtu"),mesh,results[governing]);
        };
        for(double size:sizes) {
            const auto mesh=se::meshStep(out/"fem"/(part+".step"),size);
            std::cout<<"Meshed "<<part<<" h="<<size<<"mm nodes="<<mesh.nodes_mm.size()<<std::endl;
            runLevel(mesh,size);
        }
        summary.uniformDeltaU=refinement(u); summary.uniformDeltaS=refinement(s);
        // When the uniform series has not converged the raw peak, run one
        // justified local refinement as an isolated pair: a background level
        // meshed through the same refined-meshing path at the uniform bulk
        // size, and a focused level refining only the raw-peak region. The
        // reported last-refinement change is then computed over this pair,
        // which isolates the local-refinement variable; comparing unrelated
        // global meshes would not be a refinement comparison.
        if(localSize>0 && localRadius>0 && refinement(s)>=0.05) {
            const Eigen::Vector3d centre=peakCentre;
            const auto background=[&](const Eigen::Vector3d&) { return sizes.back(); };
            const auto focused=[&](const Eigen::Vector3d& p) {
                return (p-centre).norm()<localRadius?localSize:sizes.back();
            };
            const auto meshA=se::meshStepRefined(out/"fem"/(part+".step"),sizes.back(),localSize,background);
            std::cout<<"Meshed "<<part<<" local background h="<<sizes.back()<<"mm nodes="<<meshA.nodes_mm.size()<<std::endl;
            runLevel(meshA,sizes.back(),"-local-background");
            const auto meshB=se::meshStepRefined(out/"fem"/(part+".step"),sizes.back(),localSize,focused);
            summary.localAdded=true;
            std::cout<<"Meshed "<<part<<" local h="<<localSize<<"mm around peak nodes="<<meshB.nodes_mm.size()<<std::endl;
            runLevel(meshB,localSize,"-local");
        }
        const double deltaU=refinement(u),deltaS=refinement(s);
        const bool allowStress=s.back()<=d::specified_yield_pa/d::structural_factor;
        criteria<<part<<"_displacement_refinement,"<<deltaU<<",0.03,"<<(deltaU<0.03?"PASS":"REFINE")<<'\n'
                <<part<<"_peak_stress_refinement,"<<deltaS<<",0.05,"<<(deltaS<0.05?"PASS":"REFINE")<<'\n'
                <<part<<"_peak_stress_Pa,"<<s.back()<<','<<d::specified_yield_pa/d::structural_factor<<','<<(allowStress?"PASS_SCOPED":"FAIL")<<'\n';
        failures+=static_cast<int>(deltaU>=0.03)+static_cast<int>(deltaS>=0.05)+static_cast<int>(!allowStress);
        summary.deltaU=deltaU; summary.deltaS=deltaS; summary.u=u.back(); summary.s=s.back(); summary.p99=finalP99;
        summary.peakCentre=peakCentre; summary.peakNearest=peakNearest;
        report<<"\n**"<<part<<"**: "<<loads.size()<<" load cases at each of"<<sizes.size()<<" mesh sizes"
              <<(summary.localAdded?" plus one isolated local-refinement pair (background and peak-region levels)":"")<<". "
              <<"Final maximum displacement="<<u.back()*1e3<<"mm; stress="<<s.back()/1e6<<"MPa. "
              <<"Last displacement change="<<deltaU*100<<"% (3% criterion); peak stress change="<<deltaS*100<<"% (5% criterion). "
              <<(summary.localAdded?("Before the local pair the uniform-series last changes were displacement="+
                                     std::to_string(summary.uniformDeltaU*100)+"% and peak stress="+
                                     std::to_string(summary.uniformDeltaS*100)+"%. "):"")
              <<"Volume-weighted P99 stress envelope across refinements="<<finalP99/1e6<<"MPa; this is diagnostic, not a replacement for the raw peak gate.\n";
        return summary;
    };
    if(selectedPart=="all"||selectedPart=="adapter") {
    for(int sign:{-1,1})
        loadTable<<"adapter,"<<(sign<0?0:1)<<",0,0,-150,"
                 <<(sign<0?"-":"+")<<c::motor_holding_nm
                 <<"Nm shaft-axis reaction torque plus150N motor gravity qualification load at the rear-most motor plane, distributed over the actual motor-flange bearing footprint\n";
    report<<"\n### Adapter study\n\n"
      <<"Adapter: exact±8.5Nm shaft-axis reaction torque and150N motor gravity qualification load acting at the rear-most motor plane, "
      <<"not at the flange. The150N is a specified load bound (mass≤15.30kg within the actual134.1mm motor envelope), not a fabricated motor mass. "
      <<"Actual adapter self-weight is also included.\n\n"
      <<"**Support boundary (justified for this run):** the four ISO4762 M5x16 class10.9 F0 screws with ISO7092 M5 washers (OD9mm) clamp the adapter to the stage front. "
      <<"The screws pass through the Ø5.5mm F0 clearance holes, so no shank bearing transfers load; the clamp reaches the stage interface as seat compression bounded by the washer footprint. "
      <<"The ideal fixed support is therefore the stage-face material annulus r∈[2.75,4.5]mm around each PCD70 hole axis, between the hole edge and the washer OD. "
      <<"This replaces the earlier arbitrary square node patch; it remains an ideal clamp, not a contact/preload/friction solve.\n\n"
      <<"**Load boundary (justified for this run):** the motor-flange bearing footprint on the motor-side face is the actual85.85mm square flange contact area, "
      <<"excluding the Ø73.10mm pilot bore (the Ø73mm pilot is a0.10mm-diametral clearance fit and carries no load) and the four Ø6.5mm motor mounting holes. "
      <<"The unchanged resultant force/couple is distributed over this footprint instead of arbitrary screw-hole boxes; no load magnitude or direction changed.\n";
    const std::vector<double> adapterSizes=meshSizes.empty()?std::vector<double>{6.,4.,2.8,2.,1.4,1.0}:meshSizes;
    const double adapterLocalSize=0.7,adapterLocalRadius=8.;
    const auto adapter=refinePart("adapter",adapterSizes,adapterClamp,adapterLoads,adapterLocalSize,adapterLocalRadius);
    double seatAxis=std::numeric_limits<double>::infinity();
    for(double x:{-f0PcdRadius,f0PcdRadius}) for(double z:{-f0PcdRadius,f0PcdRadius})
        seatAxis=std::min(seatAxis,std::hypot(adapter.peakCentre.x()-x,adapter.peakCentre.z()-z));
    report<<"\nUniform-series last changes ("<<adapterSizes[adapterSizes.size()-2]<<"→"<<adapterSizes.back()<<"mm): displacement="
          <<adapter.uniformDeltaU*100<<"%, raw peak="<<adapter.uniformDeltaS*100<<"%."
          <<(adapter.localAdded?(" The isolated local-refinement pair ("+[](double v){std::ostringstream o;o<<v;return o.str();}(adapterSizes.back())+"mm background→"
                                 +[](double v){std::ostringstream o;o<<v;return o.str();}(adapterLocalSize)
                                 +"mm within"+[](double v){std::ostringstream o;o<<v;return o.str();}(adapterLocalRadius)+"mm of the raw peak) gives the final reported comparison."):"")
          <<" Finest-mesh peak element centroid ("<<adapter.peakCentre.x()<<','<<adapter.peakCentre.y()<<','<<adapter.peakCentre.z()
          <<") lies r="<<seatAxis<<"mm from the nearest F0 seat axis against the constrained annulus r∈[2.75,4.5]mm ("
          <<(seatAxis-4.5)<<"mm outside the washer-OD edge), "<<std::abs(adapter.peakCentre.y())<<"mm off the stage face; nearest constrained node="
          <<adapter.peakNearest<<"mm. "
          <<"Verdict: raw peak last-refinement change="<<adapter.deltaS*100<<"% ("<<(adapter.deltaS<0.05?"PASS":"**REFINE**")<<" vs5%), displacement change="
          <<adapter.deltaU*100<<"% ("<<(adapter.deltaU<0.03?"PASS":"REFINE")<<" vs3%)."
          <<(adapter.localAdded&&adapter.deltaS>=0.05?
             " The finest peak element straddles the ideal fixed/free transition at the washer-OD seat edge, and refining the peak region raised the raw peak instead of converging it — an ideal-clamp boundary singularity of the node-wise support model. The5% gate is retained; the boundary is reported as improved-but-still-held, not closed.":"")
          <<" These diagnostics locate the peak relative to the new boundary; they do not remove the peak from the convergence gate.\n";
    }
    const auto datumClamp=[](const Eigen::Vector3d& p) {
        se::Constraint fixed{};
        if(std::abs(p.z())<1e-6 && std::abs(p.x())>65 && (std::abs(std::abs(p.y())-500)<10 ||std::abs(std::abs(p.y())-250)<10)) fixed={0.,0.,0.};
        return fixed;
    };
    std::vector<std::vector<se::LoadPatch>> datumLoads;
    if(selectedPart=="all"||selectedPart=="datum") {
    for(int front=0;front<8;++front) for(int rear=0;rear<8;++rear) {
        std::vector<se::LoadPatch> stageLoads;
        for(int side:{-1,1}) {
            const int bits=side<0?front:rear;
            const auto selected=[side](const Eigen::Vector3d& p) {
                return std::abs(p.z()-d::datum_thickness)<1e-6 && p.y()*side>40 && p.y()*side<360 && std::abs(p.x())<45;
            };
            const Eigen::Vector3d force{(bits&1?1:-1)*d::transverse_load_n,(bits&2?1:-1)*d::working_thrust_n,(bits&4?1:-1)*d::vertical_load_n};
            stageLoads.push_back({selected,force,Eigen::Vector3d{-side*d::blade_seat_x,side*d::blade_open_apex/2,d::wire_z}});
            // Separate conservative mounted-hardware mass/CG envelope; the actual
            // datum mesh receives its own gravity independently of these loads.
            stageLoads.push_back({selected,{0,0,-300},Eigen::Vector3d{0,side*d::overall_head_half_y,72}});
            loadTable<<"datum_"<<(side<0?"front":"rear")<<','<<datumLoads.size()<<','<<force.x()<<','<<force.y()<<','<<force.z()
                     <<",plus300N mounted mass envelope per half and actual datum gravity\n";
        }
        datumLoads.push_back(std::move(stageLoads));
    }
    report<<"\nDatum: all64 combinations of the two independently signed jaw load envelopes; forces transferred from the actual112mm wire height. "
      <<"Separate300N mounted-mass qualification load at the outermost motor plane per half (≤30.59kg within that envelope); "
      <<"actual datum self-weight is integrated over its mesh. Eight ideal base attachment regions are fixed. "
      <<"This deliberately includes abnormal unbalanced loads; no frame, contact or purchased-stage compliance is fabricated.\n";
        refinePart("datum",meshSizes.empty()?std::vector<double>{8.,5.5,4.}:meshSizes,datumClamp,datumLoads);
        }
        report<<"\nPeak element centroids and distances to the nearest constrained node are exported in peak-locations.csv. "
            <<"These diagnostics locate possible support/load/geometry singularities; they do not remove peak stresses from the convergence gate. "
            <<"Only **"<<selectedPart<<"** was requested; omitted parts are not validated by this run.\n";
        if(!data||!criteria||!loadTable||!peaks) throw std::runtime_error("FEM report write failed");
    return failures;
}
}
int main(int argc,char** argv) try {
    fs::path out; bool fem=false,strict=false;
    std::string femPart="all";std::vector<double> meshSizes;
    for(int i=1;i<argc;++i) {
        const std::string a=argv[i];
        if(a=="--report" && i+1<argc) out=argv[++i];
        else if(a=="--fem") fem=true;
        else if(a=="--fem-part"&&i+1<argc) {fem=true;femPart=argv[++i];}
        else if(a=="--fem-mesh-sizes"&&i+1<argc) {
            const std::string specification=argv[++i];
            if(specification.empty()||specification.back()==',')throw std::invalid_argument("Empty FEM mesh-size entry");
            std::istringstream values(specification);std::string token;
            while(std::getline(values,token,',')) {
                if(token.empty())throw std::invalid_argument("Empty FEM mesh-size entry");
                std::size_t used=0;const double value=std::stod(token,&used);
                if(used!=token.size()||!std::isfinite(value)||value<0.1||value>50||
                   (!meshSizes.empty()&&value>=meshSizes.back()))
                    throw std::invalid_argument("FEM sizes must be finite,strictly decreasing and within0.1–50mm");
                meshSizes.push_back(value);
            }
            if(meshSizes.size()<2||meshSizes.size()>8)throw std::invalid_argument("Specify2–8 FEM mesh sizes");
        }
        else if(a=="--require-design-pass") strict=true;
        else throw std::invalid_argument("Unknown analysis argument: "+a);
    }
    if(out.empty()) throw std::invalid_argument("Explicit --report output directory required");
    if(femPart!="all"&&femPart!="holder"&&femPart!="adapter"&&femPart!="datum")
        throw std::invalid_argument("FEM part must be all,holder,adapter or datum");
    if(!meshSizes.empty()&&(!fem||femPart=="all"))
        throw std::invalid_argument("Mesh-size override requires one explicit --fem-part");
    fs::create_directories(out);
    // Invalidate an earlier status before running; a crash must not leave a stale
    // success marker beside incomplete new data.
    {
        std::ofstream pending(out/"status.json");
        pending<<"{\"revision\":\""<<h::revision<<"\",\"report_generated\":false,\"run_complete\":false,\"production_ready\":false}\n";
        if(!pending) throw std::runtime_error("Cannot initialise incomplete-run status");
    }
    std::ofstream table(out/"checks.csv"), report(out/"report.md");
    report<<std::unitbuf;
    table<<"id,category,value,limit,unit,result,provenance\n"<<std::setprecision(12);
    report<<"# STARZ parts-first engineering report\n\nRevision **"<<h::revision<<"**. Generated by C++20.\n\n"
      <<"**NOT A FABRICATION, ENERGISATION OR PRODUCTION RELEASE.** Exit0 means the report was produced, not that all design checks passed. "
      <<"Use the explicit design gate to obtain nonzero status for any failed check. Catalogue limits and imposed qualification loads are identified separately.\n\n"
      <<"## Selected cutting axis\n\n"<<c::stage<<"; "<<c::motor<<" /CL86T-V41 at60V,5.7Arms; "<<c::coupling<<". "
      <<"Direct10mm lead; "<<d::blade_speed_mm_s<<"mm/s;1000 command pulses/revolution.\n\n"
      <<"| Check | Value | Limit | Unit | Result |\n|---|---:|---:|---|---|\n";
    const auto checks=se::analyticalChecks(); int failures=0;
    for(const auto& check:checks) {
        table<<check.id<<','<<check.category<<','<<check.value<<','<<check.limit<<','<<check.unit<<','<<(check.passed?"PASS":"FAIL")<<",\""<<check.provenance<<"\"\n";
        report<<'|'<<check.id<<'|'<<check.value<<'|'<<check.limit<<'|'<<check.unit<<'|'<<(check.passed?"PASS":"**FAIL**")<<"|\n";
        if(!check.passed) ++failures;
    }
    report<<"\nPASS is scoped to the equation and its stated inputs. L10 is basic rolling-contact fatigue life, "
      <<"not machine availability, contamination endurance, short-stroke lubrication life or a replacement interval. "
      <<"The2500N working thrust and5000N structural proof loads are **specified acceptance envelopes**, not measured copper/PVC forces.\n";
    studies(out,report);
    int femFailures=0;
    if(fem) femFailures=finiteElements(out,report,femPart,meshSizes);
    else report<<"\n3D FE studies were not requested in this run.\n";
    report<<"\n## Remaining physical/design gates\n\n"
      <<"- Cutting/stripping load cell trials across actual wire constructions, temperatures and edge wear.\n"
      <<"- Hot-motor torque, short-stroke resonance, driver tune, following error and settled blade-position measurement.\n"
      <<"- Assembled screw/guide/bolted-joint compliance and worst loaded blade-face clearance; raw catalogue repeatability is not sufficient.\n"
      <<"- Ground tooling geometry, edge grind, conductor damage and stripping-force acceptance.\n"
      <<"- Loaded rolling-radius calibration, slip detection, actual-wire tension and relaxation.\n"
      <<"- Selected electrical protection, dump/discharge hardware, cabinet temperature limit and measured stopping time.\n"
      <<"- Transport/payoff/enclosure selection and full assembly collision/stability/guard tests.\n"
      <<"- Safety risk assessment and hardware validation; software reference tests do not establish a PL/SIL or safety certification.\n";
    std::ofstream json(out/"status.json");
    json<<"{\n  \"revision\": \""<<h::revision<<"\",\n  \"report_generated\": true,\n  \"run_complete\": true,\n  \"analytical_checks\": "<<checks.size()<<",\n  \"failed_checks\": "<<failures
    <<",\n  \"fem_failed_or_unconverged_checks\": "<<femFailures<<",\n  \"finite_elements_run\": "<<(fem?"true":"false")
    <<",\n  \"finite_elements_scope\": \""<<(fem?femPart:"not_run")<<"\",\n  \"full_machine_modelled\": false,\n"
       <<"  \"physical_validation_complete\": false,\n  \"production_ready\": false\n}\n";
    if(!table||!report||!json) throw std::runtime_error("Engineering output write failure");
    std::cout<<"Generated "<<checks.size()<<" checks; "<<failures<<" design failures remain. Not a machine release.\n";
    return strict&&(failures||femFailures)?2:0;
} catch(const std::exception& e) { std::cerr<<"Engineering error: "<<e.what()<<'\n'; return 1; }