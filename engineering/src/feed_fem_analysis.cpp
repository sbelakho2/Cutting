#include "feed_fem.hpp"
#include "feed_geometry.hpp"
#include "feed_refinement.hpp"
#include "head_geometry.hpp"
#include "shared/feed_drive_spec.hpp"
#include <BRepClass3d_SolidClassifier.hxx>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace d=starz::feed::design;
namespace {
struct Level {double bulk,root;};
struct Peak {double sampled=0,bound=0;std::size_t element=0,boundElement=0,phase=0;int signs=0,torque=0;};
struct Summary {
    double working=0,fault=0,displacement=0,volume=0;
    std::size_t sameSurface=0,outsideSameSurface=0,outsidePeaks=0;
};
double number(const std::string& s) {
    std::size_t n=0;const double value=std::stod(s,&n);
    if(n!=s.size()||!std::isfinite(value)||value<=0)throw std::invalid_argument("Expected a positive finite number");
    return value;
}
std::vector<Level> parseLevels(const std::string& input) {
    std::vector<Level> levels;std::stringstream list(input);std::string item;
    if(input.empty()||input.back()==',')throw std::invalid_argument("Empty mesh level");
    while(std::getline(list,item,',')) {
        const auto colon=item.find(':');if(colon==std::string::npos)throw std::invalid_argument("Use bulk:root mesh levels");
        const Level level{number(item.substr(0,colon)),number(item.substr(colon+1))};
        if(level.root>level.bulk||level.root<0.005||level.bulk>5)throw std::invalid_argument("Mesh level outside supported limits");
        if(!levels.empty()&&(level.bulk>=levels.back().bulk||level.root>=levels.back().root))
            throw std::invalid_argument("Both bulk and root sizes must decrease strictly");
        levels.push_back(level);
    }
    if(levels.empty()||levels.size()>6)throw std::invalid_argument("Require one to six mesh levels");
    return levels;
}
double change(double a,double b){return std::abs(a-b)/std::max(1e-30,std::abs(b));}
template<class Vector,class Access>
Vector combination(const std::vector<se::DistributedElasticResult>& basis,const std::array<double,8>& weights,Access access) {
    Vector out=Vector::Zero();for(std::size_t j=0;j<8;++j)out+=weights[j]*access(basis[j]);return out;
}
void close(std::ofstream& stream){stream.close();if(!stream)throw std::runtime_error("Shaft FEM report write failed");}
}
int main(int argc,char** argv)try{
    fs::path out;std::vector<Level> levels{{2.0,0.30},{1.6,0.20},{1.2,0.13}};std::size_t phases=48;bool requireConverged=false;
    std::string focus,feature;double backgroundRoot=0.20,focusGrowth=0.45,torusRoot=0;bool backgroundSpecified=false,growthSpecified=false,torusSpecified=false;
    se::RefinedMeshOptimizer optimizer=se::RefinedMeshOptimizer::gmsh;std::string optimizerName="gmsh";
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--require-converged")requireConverged=true;
        else if(arg=="--report"&&i+1<argc)out=argv[++i];
        else if(arg=="--meshes"&&i+1<argc)levels=parseLevels(argv[++i]);
        else if(arg=="--focus-region"&&i+1<argc) {
            focus=argv[++i];if(focus.empty())throw std::invalid_argument("Nonempty focus region required");
            (void)se::feedShaftRootDistance(Eigen::Vector3d::Zero(),focus);
        }
        else if(arg=="--background-root"&&i+1<argc){backgroundRoot=number(argv[++i]);backgroundSpecified=true;}
        else if(arg=="--focus-growth"&&i+1<argc){focusGrowth=number(argv[++i]);growthSpecified=true;}
        else if(arg=="--torus-root"&&i+1<argc){torusRoot=number(argv[++i]);torusSpecified=true;}
        else if(arg=="--focus-feature"&&i+1<argc){feature=argv[++i];if(feature.empty())throw std::invalid_argument("Nonempty CAD feature required");}
        else if(arg=="--mesh-optimizer"&&i+1<argc) {
            optimizerName=argv[++i];
            if(optimizerName=="netgen")optimizer=se::RefinedMeshOptimizer::netgen;
            else if(optimizerName=="gmsh")optimizer=se::RefinedMeshOptimizer::gmsh;
            else throw std::invalid_argument("Mesh optimizer must be gmsh or netgen");
        }
        else if(arg=="--phases"&&i+1<argc) {
            const double n=number(argv[++i]);if(n<8||n>720||n!=std::floor(n))throw std::invalid_argument("Phases must be an integer8–720");
            phases=static_cast<std::size_t>(n);
        } else throw std::invalid_argument("Use --report directory [--meshes bulk:root,...] [--focus-region region [--focus-feature CAD_feature] --background-root mm --focus-growth ratio] [--mesh-optimizer gmsh|netgen] [--phases integer] [--require-converged]");
    }
    if(out.empty())throw std::invalid_argument("Report directory required");
    std::optional<se::ShaftFeatureGuide> featureGuide;
    if(!feature.empty()) {
        featureGuide=se::feedShaftFeatureGuide(se::feedShaftShape(),feature);
        if(featureGuide->region.empty()) {
            if(!focus.empty())throw std::invalid_argument("Whole-shaft CAD feature requires all-root mesh mode");
        } else if(focus.empty()||focus!=featureGuide->region)
            throw std::invalid_argument("CAD feature requires its matching focus region");
    }
    if(focus.empty()&&(backgroundSpecified||(growthSpecified&&!featureGuide)))throw std::invalid_argument("Focused sizing options require a focus region");
    if(focusGrowth<0.05||focusGrowth>1)throw std::invalid_argument("Focused size growth must be0.05–1");
    if(torusSpecified&&!featureGuide)throw std::invalid_argument("Torus root requires a CAD focus feature");
    if(torusSpecified&&(torusRoot<0.005||torusRoot>0.2))throw std::invalid_argument("Torus root must be0.005–0.2 mm");
    for(const auto& level:levels) {
        if(focus.empty()&&level.root<0.02)throw std::invalid_argument("All-root minimum remains0.02 mm");
        if(!focus.empty()&&(backgroundRoot<level.root||backgroundRoot>level.bulk))
            throw std::invalid_argument("Focused root must not exceed background root; background must not exceed bulk");
        if(torusSpecified&&torusRoot>level.root)throw std::invalid_argument("Torus root must not exceed the level root");
    }
    fs::create_directories(out);
    {std::ofstream pending(out/"status.json");pending<<"{\"run_complete\":false,\"fabrication_released\":false}\n";close(pending);}
    const auto shaft=se::feedShaftShape();se::writeStep(shaft,out/"F101-R6-analysis.step");
    BRepClass3d_SolidClassifier classifier(shaft);
    const auto cadState=[&](const Eigen::Vector3d& p) {
        classifier.Perform(gp_Pnt(p.x(),p.y(),p.z()),1e-7);const auto state=classifier.State();
        if(state!=TopAbs_IN&&state!=TopAbs_ON&&state!=TopAbs_OUT)throw std::runtime_error("Unknown CAD centroid classification");
        return state==TopAbs_OUT?"outside":state==TopAbs_IN?"inside":"on";
    };
    if(featureGuide) {
        std::ofstream selected(out/"sizing-faces.csv");selected<<std::setprecision(12)
            <<"CAD_face,type,xmin,Smin,zmin,xmax,Smax,zmax,analytic_axis_defined,origin_x,origin_S,origin_z,axis_x,axis_S,axis_z,major_radius_mm,minor_radius_mm\n";
        for(const auto& f:featureGuide->faces) {
            selected<<f.cad_face<<','<<(f.kind==se::RefinementSurface::torus?"torus":f.kind==se::RefinementSurface::cylinder?"cylinder":"bounded_other")<<',';
            for(int j=0;j<3;++j)selected<<f.lower_mm[j]<<',';
            for(int j=0;j<3;++j)selected<<f.upper_mm[j]<<',';
            selected<<(f.kind!=se::RefinementSurface::bounded_other)<<',';
            for(int j=0;j<3;++j)selected<<f.origin_mm[j]<<',';
            for(int j=0;j<3;++j)selected<<f.axis[j]<<',';
            selected<<f.major_radius_mm<<','<<f.minor_radius_mm<<'\n';
        }
        close(selected);
    }
    const double exactVolume=se::volume(shaft)*1e-9;
    const auto regions=se::feedShaftStressRegions();const auto supports=se::feedShaftSupports();
    if(regions.size()!=10||supports.size()!=6)throw std::runtime_error("Expected ten shaft stress regions and six supports");
    std::size_t fieldRegion=0;
    if(!focus.empty()) {
        const auto selected=std::find_if(regions.begin(),regions.end(),[&](const auto& r){return r.name==focus;});
        if(selected==regions.end())throw std::runtime_error("Mesh focus has no reported stress region");
        fieldRegion=static_cast<std::size_t>(std::distance(regions.begin(),selected));
    }
    std::vector<std::array<double,2>> phaseDirections;
    for(std::size_t p=0;p<phases;++p) {
        const double angle=2*std::numbers::pi*static_cast<double>(p)/static_cast<double>(phases);
        phaseDirections.push_back({std::cos(angle),std::sin(angle)});
    }
    const se::ElasticMaterial material{d::steel_E_mpa*1e6,0.29,7850};
    std::ofstream table(out/"regions.csv"),basisTable(out/"basis.csv"),criteria(out/"criteria.csv"),report(out/"report.md"),
        meshTable(out/"mesh-quality.csv"),surfaceTable(out/"surface-audit.csv"),sameSurfaceTable(out/"same-surface-cells.csv"),peakCadTable(out/"peak-cad-states.csv");
    sameSurfaceTable<<std::setprecision(12)<<"bulk_mm,root_mm,element_zero_based,Gmsh_surface,x_mm,S_mm,z_mm,CAD_state\n";
    peakCadTable<<std::setprecision(12)<<"bulk_mm,root_mm,load_class,region,peak_kind,element_zero_based,x_mm,S_mm,z_mm,CAD_state\n";
    meshTable<<std::setprecision(12)<<"bulk_mm,root_mm,nodes,tets,min_SICN,CAD_volume_mm3,mesh_volume_mm3,relative_volume_error\n";
    surfaceTable<<std::setprecision(12)<<"bulk_mm,root_mm,face,type,nodes,triangles,max_node_CAD_distance_mm,max_centroid_CAD_distance_mm\n";
    table<<std::setprecision(12)<<"bulk_mm,root_mm,nodes,tets,load_class,region,sampled_vm_MPa,continuous_phase_upper_MPa,peak_x_mm,peak_S_mm,peak_z_mm,phase_deg,sign_index,torque_vertex\n";
    basisTable<<std::setprecision(12)<<"bulk_mm,root_mm,basis,volume_m3,energy_J,equilibrium_relative,constraint_relative,force_balance_relative,moment_balance_relative,rear_Fx_N,rear_Fz_N,front_Fx_N,front_Fz_N,axial_reaction_N,spin_reference_reaction_Nm\n";
    criteria<<std::setprecision(12)<<"check,value,limit,result\n";
    std::vector<Summary> summaries;std::vector<std::vector<double>> regionBounds,regionSampled;
    for(const auto& level:levels) {
        std::cout<<"Meshing F101 R6 bulk="<<level.bulk<<" root="<<level.root<<" mm focus="<<(focus.empty()?"all_roots":focus)<<" optimizer="<<optimizerName<<std::endl;
        const auto mesh=se::meshStepRefined(out/"F101-R6-analysis.step",level.bulk,level.root,[=](const Eigen::Vector3d& p){
            const double original=std::min(level.bulk,(focus.empty()?level.root:backgroundRoot)+0.45*se::feedShaftRootDistance(p));
            const double featureRoot=torusSpecified?torusRoot:level.root;
            if(focus.empty())
                return featureGuide?std::min(original,featureRoot+focusGrowth*featureGuide->distance_mm(p)):original;
            const double focusedDistance=featureGuide?featureGuide->distance_mm(p):se::feedShaftRootDistance(p,focus);
            return std::min(original,featureRoot+focusGrowth*focusedDistance);
        },optimizer);
        std::cout<<"Shaft mesh nodes="<<mesh.nodes_mm.size()<<" tets="<<mesh.tetrahedra.size()<<std::endl;
        if(!mesh.geometry_audit)throw std::runtime_error("Refined STEP geometry audit missing");
        const auto& audit=*mesh.geometry_audit;
        if(change(exactVolume,audit.cad_volume_mm3*1e-9)>1e-9)throw std::runtime_error("STEP import changed nominal shaft volume");
        meshTable<<level.bulk<<','<<level.root<<','<<mesh.nodes_mm.size()<<','<<mesh.tetrahedra.size()<<','<<audit.min_sicn<<','
                 <<audit.cad_volume_mm3<<','<<audit.element_volume_mm3<<','<<change(audit.cad_volume_mm3,audit.element_volume_mm3)<<'\n'<<std::flush;
        for(const auto& surface:audit.surfaces)
            surfaceTable<<level.bulk<<','<<level.root<<','<<surface.tag<<','<<surface.type<<','<<surface.nodes<<','<<surface.triangles<<','
                        <<surface.max_node_distance_mm<<','<<surface.max_facet_centroid_distance_mm<<'\n'<<std::flush;
        const auto basis=se::solveDistributedElasticCases(mesh,material,supports,se::feedShaftBasisLoads());
        if(basis.size()!=8)throw std::runtime_error("Expected eight shaft load bases");
        Summary summary;summary.volume=basis.front().volume_m3;
        summary.sameSurface=audit.same_surface_tetrahedra.size();
        if(change(summary.volume,audit.element_volume_mm3*1e-9)>1e-9)throw std::runtime_error("Gmsh and FEM element volumes disagree");
        for(std::size_t b=0;b<basis.size();++b) {
            const auto& r=basis[b];
            if(b<6&&std::abs(r.support_moment_nm[5].y())>1e-6)throw std::runtime_error("Balanced shaft load produced a spin-reference reaction");
            if(std::abs(r.support_moment_nm[5].y()+r.total_load_moment_nm.y())>1e-6)
                throw std::runtime_error("Drive holding reaction does not balance actual shaft gravity moment");
            if(b<4) {
                const double station=b<2?d::pulley_station:d::wire_contact_station;
                const double rear=-(d::front_support-station)/(d::front_support-d::rear_support),front=-1-rear;
                const std::size_t direction=b%2;const int axis=direction==0?0:2;
                if(std::abs(r.support_force_n[direction][axis]-rear)>1e-7||std::abs(r.support_force_n[2+direction][axis]-front)>1e-7)
                    throw std::runtime_error("Shaft bearing reactions disagree with independent force/moment statics");
            }
            basisTable<<level.bulk<<','<<level.root<<','<<b<<','<<r.volume_m3<<','<<r.energy_j<<','<<r.equilibrium_relative<<','<<r.constraint_relative<<','
                      <<r.force_balance_relative<<','<<r.moment_balance_relative<<','<<r.support_force_n[0].x()<<','<<r.support_force_n[1].z()<<','
                      <<r.support_force_n[2].x()<<','<<r.support_force_n[3].z()<<','<<r.support_force_n[4].y()<<','<<r.support_moment_nm[5].y()<<'\n'<<std::flush;
        }
        std::vector<std::vector<std::size_t>> memberships(mesh.tetrahedra.size());
        std::vector<Eigen::Vector3d> centres(mesh.tetrahedra.size(),Eigen::Vector3d::Zero());
        for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
            for(int id:mesh.tetrahedra[e])centres[e]+=mesh.nodes_mm[static_cast<std::size_t>(id)]/4;
            for(std::size_t r=0;r<regions.size();++r)if(regions[r].contains(centres[e]))memberships[e].push_back(r);
        }
        for(const auto& t:audit.same_surface_tetrahedra) {
            const auto& p=centres.at(t.element);const std::string state=cadState(p);if(state=="outside")++summary.outsideSameSurface;
            sameSurfaceTable<<level.bulk<<','<<level.root<<','<<t.element<<','<<t.surface<<','<<p.x()<<','<<p.y()<<','<<p.z()<<','<<state<<'\n'<<std::flush;
        }
        std::vector<double> levelBounds,levelSampled;
        for(double torqueLimit:{d::running_torque_nm,d::mechanical_fault_torque_nm}) {
            const auto vertices=se::feedShaftTorqueVertices(torqueLimit);std::vector<Peak> peaks(regions.size());double displacement=0;
            for(int signs=0;signs<8;++signs) {
                const int bx=(signs&1)?1:-1,rx=(signs&2)?1:-1,rz=(signs&4)?1:-1;
                const auto cosine=se::feedShaftCoefficients(0,bx,rx,rz,0,0);
                const auto sine=se::feedShaftCoefficients(std::numbers::pi/2,bx,rx,rz,0,0);
                for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
                    const auto access=[&](const auto& r)->const se::StressTensor6&{return r.stress_pa[e];};
                    const auto c=combination<se::StressTensor6>(basis,cosine,access),s=combination<se::StressTensor6>(basis,sine,access);
                    const double remainder=se::angularNormRemainder(se::vonMises(c),se::vonMises(s),phases);
                    for(std::size_t v=0;v<vertices.size();++v) {
                        const se::StressTensor6 constant=vertices[v][0]*basis[4].stress_pa[e]+vertices[v][1]*basis[5].stress_pa[e];
                        double maximum=0;std::size_t phaseAt=0;
                        for(std::size_t p=0;p<phases;++p) {
                            const double vm=se::vonMises(constant+phaseDirections[p][0]*c+phaseDirections[p][1]*s);
                            if(vm>maximum){maximum=vm;phaseAt=p;}
                        }
                        for(std::size_t r:memberships[e]) {
                            if(maximum>peaks[r].sampled){peaks[r].sampled=maximum;peaks[r].element=e;peaks[r].phase=phaseAt;peaks[r].signs=signs;peaks[r].torque=static_cast<int>(v);}
                            if(maximum+remainder>peaks[r].bound){peaks[r].bound=maximum+remainder;peaks[r].boundElement=e;}
                        }
                    }
                }
                if(torqueLimit==d::running_torque_nm)for(std::size_t n=0;n<mesh.nodes_mm.size();++n) {
                    const auto access=[&](const auto& r)->const Eigen::Vector3d&{return r.displacement_m[n];};
                    const auto c=combination<Eigen::Vector3d>(basis,cosine,access),s=combination<Eigen::Vector3d>(basis,sine,access);
                    const double remainder=se::angularNormRemainder(c.norm(),s.norm(),phases);
                    for(const auto& vertex:vertices) {
                        const Eigen::Vector3d constant=vertex[0]*basis[4].displacement_m[n]+vertex[1]*basis[5].displacement_m[n];
                        for(std::size_t p=0;p<phases;++p) {
                            displacement=std::max(displacement,(constant+phaseDirections[p][0]*c+phaseDirections[p][1]*s).norm()+remainder);
                        }
                    }
                }
            }
            for(std::size_t r=0;r<regions.size();++r) {
                const auto& peak=peaks[r];if(peak.sampled<=0)throw std::runtime_error("Empty/zero shaft stress region");
                const auto& p=centres[peak.element];levelBounds.push_back(peak.bound);levelSampled.push_back(peak.sampled);
                for(bool bound:{false,true}) {
                    const auto e=bound?peak.boundElement:peak.element;const auto& location=centres[e];
                    const std::string state=cadState(location);if(state=="outside")++summary.outsidePeaks;
                    peakCadTable<<level.bulk<<','<<level.root<<','<<(torqueLimit==d::running_torque_nm?"working":"fault")<<','<<regions[r].name<<','
                                <<(bound?"phase_bound":"sampled_raw")<<','<<e<<','<<location.x()<<','<<location.y()<<','<<location.z()<<','<<state<<'\n'<<std::flush;
                }
                table<<level.bulk<<','<<level.root<<','<<mesh.nodes_mm.size()<<','<<mesh.tetrahedra.size()<<','<<(torqueLimit==d::running_torque_nm?"working":"fault")<<','<<regions[r].name<<','
                     <<peak.sampled/1e6<<','<<peak.bound/1e6<<','<<p.x()<<','<<p.y()<<','<<p.z()<<','<<360.*static_cast<double>(peak.phase)/static_cast<double>(phases)<<','<<peak.signs<<','<<peak.torque<<'\n'<<std::flush;
            }
            if(torqueLimit==d::running_torque_nm){summary.working=peaks.front().bound;summary.displacement=displacement;}
            else summary.fault=peaks.front().bound;
            // Export the single load case governing the sampled raw stress,
            // not an envelope assembled from mutually incompatible states.
            const auto& governing=peaks[fieldRegion];
            const auto& torque=vertices[static_cast<std::size_t>(governing.torque)];
            const auto weights=se::feedShaftCoefficients(2*std::numbers::pi*static_cast<double>(governing.phase)/static_cast<double>(phases),
                (governing.signs&1)?1:-1,(governing.signs&2)?1:-1,(governing.signs&4)?1:-1,torque[0],torque[1]);
            se::ElasticResult field;
            for(std::size_t n=0;n<mesh.nodes_mm.size();++n)
                field.displacement_m.push_back(combination<Eigen::Vector3d>(basis,weights,[&](const auto& r)->const Eigen::Vector3d&{return r.displacement_m[n];}));
            for(std::size_t e=0;e<mesh.tetrahedra.size();++e)
                field.von_mises_pa.push_back(se::vonMises(combination<se::StressTensor6>(basis,weights,[&](const auto& r)->const se::StressTensor6&{return r.stress_pa[e];})));
            se::writeVtu(out/((torqueLimit==d::running_torque_nm?std::string("working-"):std::string("fault-"))+std::to_string(summaries.size()+1)+".vtu"),mesh,field);
        }
        summaries.push_back(summary);regionBounds.push_back(std::move(levelBounds));regionSampled.push_back(std::move(levelSampled));
        std::cout<<"F101 envelope working="<<summary.working/1e6<<" MPa fault="<<summary.fault/1e6<<" MPa displacement="<<summary.displacement*1000<<" mm"<<std::endl;
    }
    const auto& last=summaries.back();int failed=0;
    const auto gate=[&](const std::string& name,double value,double limit,bool pass){criteria<<name<<','<<value<<','<<limit<<','<<(pass?"PASS_SCOPED":"REFINE_OR_FAIL")<<'\n';if(!pass)++failed;};
    gate("working_peak_Pa",last.working,d::steel_yield_mpa*1e6/3,last.working<=d::steel_yield_mpa*1e6/3);
    gate("fault_peak_Pa",last.fault,d::steel_yield_mpa*1e6/1.5,last.fault<=d::steel_yield_mpa*1e6/1.5);
    gate("working_displacement_mm",last.displacement*1000,0.05,last.displacement*1000<=0.05);
    gate("Brep_volume_relative",change(exactVolume,last.volume),0.005,change(exactVolume,last.volume)<0.005);
    gate("no_exterior_same_surface_centroids",static_cast<double>(last.outsideSameSurface),0,last.outsideSameSurface==0);
    gate("all_reported_peak_centroids_in_CAD",static_cast<double>(last.outsidePeaks),0,last.outsidePeaks==0);
    gate("at_least_three_meshes",static_cast<double>(levels.size()),3,levels.size()>=3);
    if(!focus.empty())gate("focused_study_requires_background_and_all_root_verification",0,1,false);
    if(summaries.size()>=2) {
        gate("displacement_refinement",change(summaries[summaries.size()-2].displacement,last.displacement),0.03,change(summaries[summaries.size()-2].displacement,last.displacement)<0.03);
        for(std::size_t i=0;i<regionBounds.back().size();++i) {
            const double delta=change(regionBounds[regionBounds.size()-2][i],regionBounds.back()[i]);
            const std::string name=std::string(i<regions.size()?"working_":"fault_")+regions[i%regions.size()].name;
            gate(name+"_phase_bound_refinement",delta,0.05,delta<0.05);
            const double rawDelta=change(regionSampled[regionSampled.size()-2][i],regionSampled.back()[i]);
            gate(name+"_sampled_peak_refinement",rawDelta,0.05,rawDelta<0.05);
        }
    }
    report<<std::setprecision(10)<<"# F101 R6 distributed-support 3D elastic study\n\nRevision **"<<starz::feed::revision<<"**.\n\n"
          <<"Actual nominal R6 STEP; linear tetrahedra; E205 GPa,nu0.29,density7850 kg/m³. "<<levels.size()<<" independently generated bulk/root mesh levels. "
          <<"Eight unit solutions are superposed only at the displacement and full stress-tensor level; von Mises magnitudes are never added as though linear.\n\n"
            <<"## Mesh sizing scope\n\n";
        if(focus.empty()) {
            report<<"Original all-root sizing: min(bulk,root+0.45*root_distance).\n\n";
            if(featureGuide)report<<"Whole-shaft torus-focused augmentation with the **"<<feature<<"** guide: min(bulk,root+0.45*all_root_distance,"
                <<(torusSpecified?std::to_string(torusRoot):"root")<<"+"<<focusGrowth
                <<"*torus_guide_distance). The analytic trimmed-torus distance replaces the approximate ring/capsule offsets for the selected concave fillet tori. "
                <<"Every stress region, load, phase, torque vertex, gravity term, node cap and5% gate is unchanged.\n\n";
        }
        else report<<"Focused refinement of **"<<focus<<"** in the complete unchanged shaft. All other root guides retain a **"<<backgroundRoot<<" mm** background target. "
               <<"The size is min(bulk,background_root+0.45*all_root_distance,root+"<<focusGrowth<<"*focused_root_distance). "
               <<"No submodel cut boundary,transferred displacement,removed stress region or weakened5% peak criterion is introduced. "
               <<"The bulk size also decreases between levels; convergence of this focused region alone does not establish all-root convergence or background independence. "
               <<"A mandatory scope hold prevents any focused study from claiming full-shaft spatial convergence,even if all reported last-change checks happen to pass.\n\n";
    if(featureGuide)report<<"Selected CAD feature **"<<feature<<"**, "<<featureGuide->faces.size()<<" faces. The focused distance uses the maximum of padded face-bounds distance and untrimmed analytic-surface distance; "
        <<"BSpline junctions use only their padded bounds. These are lower-bound sizing guides to the selected nominal faces,not CAD geometry replacement or stress-error estimates. "
        <<"M15_slot_exit clips the selected upper-pocket face portions to S71.4–73.1. Individual A12-root studies do not independently qualify the other root. "
        <<"All original stress regions remain evaluated; see sizing-faces.csv for the complete selected set.\n\n";
        report<<"Volume optimizer **"<<optimizerName<<"**; netgen means Gmsh's Netgen volume optimization after the original linear Delaunay generation. "
            <<"No individual cell or stress is filtered. The finite elements remain four-node linear tetrahedra,not curved/high-order elements. "
            <<"The finest mesh has "<<last.sameSurface<<" tetrahedra whose four nodes share a CAD surface,including "<<last.outsideSameSurface<<" exterior centroids. "
              <<last.outsidePeaks<<" of the40 reported finest raw/phase-bound peak centroids are outside the exact nominal solid. "
            <<"These two exterior-centroid counts have separate zero-count gates; no stress is suppressed to meet them. See same-surface-cells.csv and peak-cad-states.csv. "
            <<"Neither check establishes all-cell or continuous curved-volume containment.\n\n";
        report<<"Every tetrahedron has finite positive signed inverse condition quality and volume. All CAD-face nodes project to the underlying original surface within1e-6 mm. "
          <<"Per-face facet-centroid distances and the mesh/CAD volume difference are reported,not represented as a continuous Hausdorff error bound or a stress error estimate. "
          <<"Independent STEP and element-volume checks agree within1e-9 relative. See mesh-quality.csv and surface-audit.csv.\n\n";
        report
          <<"## Loads and supports\n\nDistributed work-conjugate force constraints place radial resultants at S26.5 and77.5. Only the front bearing locates axially. "
          <<"A coupling spin reference removes the sixth rigid mode; its reaction is checked below1e-6 Nm for every balanced transmission basis. "
          <<"The shaft's eccentric mesh mass needs an additional drive holding torque under gravity; that reaction must equal minus the actual body-load moment and is retained in the solution. "
          <<"No torsional bearing clamp or guessed bearing stiffness is introduced. These ideal distributed supports are not ball/race/housing contact.\n\n"
          <<"Belt components±480 N and roller components±60 N inX/Z act at S47.5/S108.75, with actual shaft mesh gravity. "
          <<"All eight load-sign corners and the full rotating material-frame phase are covered. "
          <<"The imposed transmission-torque polygon |Tr|,|Tp|,|Tr+Tp|≤T has six vertices,including opposing interface torques. T=1 Nm working or5 Nm fault; the additional mesh-gravity holding moment is not omitted. "
          <<"Torque and radial loads are distributed over seat patches with preserved force/moment,not resolved key/hub contact pressure.\n\n"
          <<"## Phase and refinement\n\n"<<phases<<" equally spaced phases plus an analytic norm remainder bound per element/node enclose unsampled angles for each finite-element solution. "
          <<"The bound is2*sin(pi/(2N))*hypot(norm(cosine field),norm(sine field)). Torque and radial-load box extrema follow convexity of the stress/displacement norms. "
          <<"This closes angular sampling for the stated linear model,not geometric tolerances or contact states.\n\n"
          <<"Finest working/fault full-shaft upper stress **"<<last.working/1e6<<" / "<<last.fault/1e6<<" MPa**. Working displacement upper envelope **"<<last.displacement*1000<<" mm**. "
          <<"Failed/unmet numerical gates: **"<<failed<<"**. Every named region retains separate5% refinement gates for its sampled raw peak and continuous-phase upper bound; no percentile replaces a peak. "
          <<"Peak coordinates/signs/torques in regions.csv describe the sampled raw maximum,not necessarily the element producing the upper bound. "
          <<"Each working-N.vtu/fault-N.vtu contains the single sampled load case governing **"<<regions[fieldRegion].name<<"** raw stress at mesh level N,not an incompatible combined envelope. "
          <<"A single/coarse run is diagnostic only.\n\n"
          <<"## Release holds\n\nTrue helical threads,0.05–0.10 mm final edge breaks,dimensional tolerances,fit prestress,KM2/M5 preload,key/collar contact,elastic bearing/housing response and fatigue remain unqualified. "
          <<"Cosmetic major-thread transitions can contain artificial sharp edges; their peaks remain reported and gated rather than suppressed. "
          <<"This report does not release fabrication or establish physical accuracy/life.\n";
    close(table);close(basisTable);close(criteria);close(report);close(meshTable);close(surfaceTable);close(sameSurfaceTable);close(peakCadTable);
    std::ofstream status(out/"status.json");status<<"{\"revision\":\""<<starz::feed::revision<<"\",\"run_complete\":true,\"mesh_levels\":"<<levels.size()
            <<",\"mesh_focus_region\":\""<<(focus.empty()?"all_roots":focus)<<"\",\"background_root_mm\":"<<(focus.empty()?0:backgroundRoot)
              <<",\"focus_growth\":"<<focusGrowth<<",\"torus_root_mm\":"<<(torusSpecified?torusRoot:0)
              <<",\"mesh_focus_feature\":\""<<feature<<"\",\"sizing_face_count\":"<<(featureGuide?featureGuide->faces.size():0)
              <<",\"mesh_optimizer\":\""<<optimizerName<<"\",\"finest_same_surface_tets\":"<<last.sameSurface
              <<",\"finest_exterior_same_surface_centroids\":"<<last.outsideSameSurface<<",\"finest_exterior_peak_centroids\":"<<last.outsidePeaks
              <<",\"continuous_CAD_containment_verified\":false"
              <<",\"phases\":"<<phases<<",\"node_limit\":150000,\"mesh_geometry_audit_passed\":true"
            <<",\"field_governing_region\":\""<<regions[fieldRegion].name<<"\""
          <<",\"failed_gates\":"<<failed<<",\"phase_bound_proved_for_linear_model\":true,\"numerical_gates_passed\":"<<(failed?"false":"true")
          <<",\"contact_verified\":false,\"true_threads_verified\":false,\"fatigue_verified\":false,\"fabrication_released\":false}\n";close(status);
    return requireConverged&&failed?2:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}