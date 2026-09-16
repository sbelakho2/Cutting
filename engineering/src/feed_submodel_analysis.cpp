#include "feed_fem.hpp"
#include "feed_geometry.hpp"
#include "feed_submodel.hpp"
#include "head_geometry.hpp"
#include "mesh_io.hpp"
#include "shared/feed_drive_spec.hpp"
#include <BRepClass3d_SolidClassifier.hxx>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace d=starz::feed::design;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
double change(double a,double b){return std::abs(a-b)/std::max(1e-30,std::abs(b));}
double number(const std::string& s) {
    std::size_t used=0;const double value=std::stod(s,&used);
    require(used==s.size()&&std::isfinite(value),"Expected a finite number");return value;
}
std::vector<std::string> split(const std::string& text,char separator) {
    std::vector<std::string> out;std::stringstream stream(text);std::string item;
    while(std::getline(stream,item,separator)){require(!item.empty(),"Empty argument field");out.push_back(item);}
    return out;
}
void close(std::ofstream& stream){stream.close();require(bool(stream),"Submodel report write failed");}
struct CaseSpec {
    std::string load;
    double phase_deg=0;int signs=0,torque=0;
    double global_sampled_mpa=0,global_bound_mpa=0;
};
struct WindowSpec {
    std::string dir,region;double station=0,half=0;
    std::optional<std::array<double,3>> kt;
    std::optional<std::array<double,2>> band; // nominal band station,half
    std::optional<std::array<double,2>> sector; // peak azimuth centre,half (degrees)
    std::optional<std::array<double,2>> refineBand; // axial S-window that gets the level root
    std::optional<std::array<double,5>> capsule; // key-floor capsule guide begin,end,radius,z,fillet
    std::vector<CaseSpec> cases;
};
struct CasePeak {
    double sampled=0,bound=0;std::size_t element=0,phase=0;
    Eigen::Vector3d location=Eigen::Vector3d::Zero();
    int cad_state=0; // 0 inside, 1 on, 2 outside
};
struct LevelSpec {double root=0;bool sector=false;};
struct LevelRecord {
    double root=0;bool sector=false;std::size_t nodes=0,tets=0;
    std::vector<CasePeak> cases;
};
Eigen::Vector3d centroid(const se::TetMesh& mesh,std::size_t e) {
    Eigen::Vector3d c=Eigen::Vector3d::Zero();
    for(int n:mesh.tetrahedra[e])c+=mesh.nodes_mm.at(static_cast<std::size_t>(n))/4;
    return c;
}
double azimuthDeg(const Eigen::Vector3d& p) {
    double a=std::atan2(p.x(),p.z())*180/std::numbers::pi;
    if(a<0)a+=360;
    return a;
}
double angularDistance(double a,double b) {
    double d=std::abs(a-b);return std::min(d,360-d);
}
template<class Vector,class Access>
Vector combination(const std::vector<se::DistributedElasticResult>& basis,const std::array<double,8>& weights,Access access) {
    Vector out=Vector::Zero();for(std::size_t j=0;j<8;++j)out+=weights[j]*access(basis[j]);return out;
}
}
int main(int argc,char** argv)try{
    fs::path vtu,out;std::string optimizerName="gmsh";double bulk=0.5;
    std::vector<LevelSpec> levels;std::optional<std::array<double,4>> vtuCase;
    std::vector<WindowSpec> windows;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--vtu"&&i+1<argc)vtu=argv[++i];
        else if(arg=="--out"&&i+1<argc)out=argv[++i];
        else if(arg=="--optimizer"&&i+1<argc){optimizerName=argv[++i];require(optimizerName=="gmsh"||optimizerName=="netgen","Optimizer must be gmsh or netgen");}
        else if(arg=="--bulk"&&i+1<argc){bulk=number(argv[++i]);require(bulk>0&&bulk<=2,"Bulk size must be positive and at most 2 mm");}
        else if(arg=="--roots"&&i+1<argc) {
            for(const auto& item:split(argv[++i],',')) {
                const auto fields=split(item,':');require(fields.size()==2,"Root must be size:full|sector");
                const double r=number(fields[0]);require(r>=0.005&&r<=0.2,"Root size must be 0.005-0.2 mm");
                require(fields[1]=="full"||fields[1]=="sector","Root mode must be full or sector");
                levels.push_back({r,fields[1]=="sector"});
            }
            require(levels.size()>=2&&levels.size()<=3,"Two or three levels required");
            for(std::size_t j=1;j<levels.size();++j)require(levels[j-1].root>=levels[j].root,"Roots must not increase");
            require(levels.front().root>levels.back().root,"Roots must decrease overall");
        }
        else if(arg=="--vtu-case"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==4,"VTU case must be phase:signs:torque:limit");
            vtuCase={number(fields[0]),number(fields[1]),number(fields[2]),number(fields[3])};
        }
        else if(arg=="--window"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==4||fields.size()==7,"Window must be dir:region:station:half[:D:d:r]");
            WindowSpec window;window.dir=fields[0];window.region=fields[1];window.station=number(fields[2]);window.half=number(fields[3]);
            require(window.half>=2&&window.half<=10,"Window half-length must be 2-10 mm");
            if(fields.size()==7)window.kt={number(fields[4]),number(fields[5]),number(fields[6])};
            windows.push_back(std::move(window));
        }
        else if(arg=="--case"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==7,"Case must be region:working|fault:phase:signs:torque:sampled:bound");
            CaseSpec spec{fields[1],number(fields[2]),static_cast<int>(number(fields[3])),static_cast<int>(number(fields[4])),
                          number(fields[5]),number(fields[6])};
            require(fields[0]==windows.back().region,"Case region must match its window");
            require(spec.load=="working"||spec.load=="fault","Case load class must be working or fault");
            require(spec.signs>=0&&spec.signs<8&&spec.torque>=0&&spec.torque<6,"Invalid case sign/torque index");
            windows.back().cases.push_back(std::move(spec));
        }
        else if(arg=="--band"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==2,"Band must be station:half");
            require(!windows.empty(),"Band requires a preceding --window");
            const double station=number(fields[0]),half=number(fields[1]);
            require(half>0&&half<=2,"Band half-length must be at most 2 mm");
            require(station>windows.back().station-windows.back().half&&station<windows.back().station+windows.back().half,
                    "Nominal band must lie inside the submodel window");
            windows.back().band={station,half};
        }
        else if(arg=="--sector"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==2,"Sector must be centre:half in degrees");
            require(!windows.empty(),"Sector requires a preceding --window");
            const double centre=number(fields[0]),half=number(fields[1]);
            require(half>0&&half<=90,"Sector half-angle must be at most 90 degrees");
            windows.back().sector={centre,half};
        }
        else if(arg=="--refine-band"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==2,"Refine band must be Smin:Smax in mm");
            require(!windows.empty(),"Refine band requires a preceding --window");
            const double lo=number(fields[0]),hi=number(fields[1]);
            require(hi>lo,"Refine band must be ordered");
            windows.back().refineBand={lo,hi};
        }
        else if(arg=="--guide-capsule"&&i+1<argc) {
            const auto fields=split(argv[++i],':');require(fields.size()==5,"Capsule guide must be begin:end:radius:z:fillet");
            require(!windows.empty(),"Capsule guide requires a preceding --window");
            const auto capsule=std::array<double,5>{number(fields[0]),number(fields[1]),number(fields[2]),number(fields[3]),number(fields[4])};
            require(capsule[1]>capsule[0]&&capsule[2]>0,"Invalid capsule guide");
            windows.back().capsule=capsule;
        }
        else throw std::invalid_argument("Unknown submodel argument");
    }
    require(!vtu.empty()&&!out.empty()&&vtuCase&&!windows.empty()&&!levels.empty(),
            "Require --vtu --out --roots --vtu-case and at least one --window");
    for(const auto& window:windows)require(!window.cases.empty(),"Every window requires at least one case");
    if(std::any_of(levels.begin(),levels.end(),[](const auto& l){return l.sector;}))
        for(const auto& window:windows)require(window.sector.has_value(),"Sector levels require a --sector specification on every window");
    const auto optimizer=optimizerName=="netgen"?se::RefinedMeshOptimizer::netgen:se::RefinedMeshOptimizer::gmsh;
    fs::create_directories(out);
    {std::ofstream state(out/"status.json");state<<"{\"run_complete\":false,\"fabrication_released\":false}\n";close(state);}
    const se::ElasticMaterial material{d::steel_E_mpa*1e6,0.29,7850};
    std::cout<<"Reading global VTU "<<vtu<<std::endl;
    const se::VtuField field=se::readLinearVtuField(vtu);
    const auto& global=field.mesh;
    require(global.nodes_mm.size()<=150000,"Global VTU exceeds the retained node cap");
    std::cout<<"Solving the eight shaft bases on the global finest mesh ("<<global.nodes_mm.size()<<" nodes, "
             <<global.tetrahedra.size()<<" tets)"<<std::endl;
    const auto basis=se::solveDistributedElasticCases(global,material,se::feedShaftSupports(),se::feedShaftBasisLoads());
    require(basis.size()==8,"Expected eight shaft load bases");
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
    }
    const double vtuPhase=2*std::numbers::pi*(*vtuCase)[0]/360;
    const int vtuSigns=static_cast<int>((*vtuCase)[1]),vtuTorque=static_cast<int>((*vtuCase)[2]);
    const double vtuLimit=(*vtuCase)[3];
    const auto vtuTorqueVertex=se::feedShaftTorqueVertices(vtuLimit)[static_cast<std::size_t>(vtuTorque)];
    const auto vtuWeights=se::feedShaftCoefficients(vtuPhase,(vtuSigns&1)?1:-1,(vtuSigns&2)?1:-1,(vtuSigns&4)?1:-1,
                                                    vtuTorqueVertex[0],vtuTorqueVertex[1]);
    double maxDispDiff=0,maxVmDiff=0;
    for(std::size_t n=0;n<global.nodes_mm.size();++n) {
        const Eigen::Vector3d combined=combination<Eigen::Vector3d>(basis,vtuWeights,[&](const auto& r)->const Eigen::Vector3d&{return r.displacement_m[n];});
        maxDispDiff=std::max(maxDispDiff,(combined*1000-field.displacement_mm[n]).norm());
    }
    for(std::size_t e=0;e<global.tetrahedra.size();++e) {
        const auto combined=combination<se::StressTensor6>(basis,vtuWeights,[&](const auto& r)->const se::StressTensor6&{return r.stress_pa[e];});
        maxVmDiff=std::max(maxVmDiff,std::abs(se::vonMises(combined)*1e-6-field.von_mises_mpa[e]));
    }
    require(maxDispDiff<1e-4&&maxVmDiff<1e-3,"Basis reconstruction disagrees with the stored VTU field");
    std::cout<<"VTU field reconstruction verified: max displacement diff "<<maxDispDiff<<" mm, max stress diff "<<maxVmDiff<<" MPa"<<std::endl;
    std::vector<std::vector<Eigen::Vector3d>> basisFieldsMm;
    for(std::size_t b=0;b<8;++b) {
        basisFieldsMm.emplace_back();
        for(std::size_t n=0;n<global.nodes_mm.size();++n)basisFieldsMm.back().push_back(basis[b].displacement_m[n]*1000);
    }
    se::DisplacementFieldInterpolator interpolator;interpolator.build(global,basisFieldsMm,0.05);
    const auto regions=se::feedShaftStressRegions();
    const auto shaft=se::feedShaftShape();
    std::size_t convergedCases=0;
    for(const auto& window:windows) {
        std::cout<<"Window "<<window.region<<" S="<<window.station<<" +-"<<window.half<<" mm"<<std::endl;
        const auto region=std::find_if(regions.begin(),regions.end(),[&](const auto& r){return r.name==window.region;});
        require(region!=regions.end(),"Unknown submodel stress region");
        const fs::path windowOut=out/window.dir;
        if(fs::is_regular_file(windowOut/"report.md")) {std::cout<<"  (existing report retained)"<<std::endl;continue;}
        fs::create_directories(windowOut);
        const auto slab=se::cutShaftSlab(shaft,window.station,window.half);
        se::writeStep(slab.shape,windowOut/"submodel.step");
        std::vector<std::array<double,8>> weightsC,weightsS,weightsK;
        std::vector<double> phaseRad;
        for(const auto& spec:window.cases) {
            const int bx=(spec.signs&1)?1:-1,rx=(spec.signs&2)?1:-1,rz=(spec.signs&4)?1:-1;
            const auto vertices=se::feedShaftTorqueVertices(spec.load=="working"?d::running_torque_nm:d::mechanical_fault_torque_nm);
            const auto vertex=vertices[static_cast<std::size_t>(spec.torque)];
            weightsC.push_back(se::feedShaftCoefficients(0,bx,rx,rz,0,0));
            weightsS.push_back(se::feedShaftCoefficients(std::numbers::pi/2,bx,rx,rz,0,0));
            std::array<double,8> kw{};kw[4]=vertex[0];kw[5]=vertex[1];
            weightsK.push_back(kw);
            phaseRad.push_back(2*std::numbers::pi*spec.phase_deg/360);
        }
        std::vector<LevelRecord> records;
        std::optional<se::TetMesh> finestMesh;
        std::vector<se::SubmodelSolveResult> finestSolved;
        BRepClass3d_SolidClassifier classifier(shaft);
        const auto peakState=[&](const Eigen::Vector3d& p) {
            classifier.Perform(gp_Pnt(p.x(),p.y(),p.z()),1e-7);
            const auto state=classifier.State();
            require(state==TopAbs_IN||state==TopAbs_ON||state==TopAbs_OUT,"Unknown submodel peak classification");
            return state==TopAbs_IN?0:state==TopAbs_ON?1:2;
        };
        for(const auto& level:levels) {
            std::cout<<"  Meshing submodel root="<<level.root<<" mm mode="<<(level.sector?"sector":"full")
                     <<" optimizer="<<optimizerName<<std::endl;
            const auto submesh=se::meshStepRefined(windowOut/"submodel.step",bulk,level.root,[&](const Eigen::Vector3d& p){
                double guideRoot=level.root;
                if(window.refineBand&&(p.y()<(*window.refineBand)[0]||p.y()>(*window.refineBand)[1]))
                    guideRoot=std::max(level.root,0.1);
                if(level.sector) {
                    require(window.sector.has_value(),"Sector level without a sector specification");
                    if(angularDistance(azimuthDeg(p),(*window.sector)[0])>(*window.sector)[1])guideRoot=std::max(level.root,0.1);
                }
                double distance;
                if(window.capsule) {
                    distance=std::abs(se::capsulePerimeterDistance(p,(*window.capsule)[0],(*window.capsule)[1],(*window.capsule)[2],(*window.capsule)[3])-(*window.capsule)[4]);
                    if(window.region=="pulley_key_and_MB2_slot") {
                        // Published slot-lip terms of the region guide: a 1D lip
                        // curve (the slot walls at z=5.5 plus the end corners),
                        // restored here so the competing slot-lip peak is
                        // resolved alongside the key-floor perimeter.
                        const double dy=std::max({31.2-p.y(),0.,p.y()-73.0});
                        double lip=std::sqrt(std::pow(std::abs(p.x())-2.1,2)+dy*dy+std::pow(p.z()-5.5,2));
                        for(const double end:{31.2,33.45,61.55,73.0}) {
                            const double dx=std::max(0.,std::abs(p.x())-2.65);
                            lip=std::min(lip,std::sqrt(dx*dx+std::pow(p.y()-end,2)+std::pow(p.z()-5.5,2)));
                        }
                        distance=std::min(distance,lip);
                    }
                } else {
                    distance=se::feedShaftRootDistance(p,window.region);
                }
                return std::min(bulk,guideRoot+0.45*distance);
            },optimizer);
            require(submesh.geometry_audit.has_value(),"Submodel geometry audit missing");
            const auto classification=se::classifySlabCutNodes(submesh,slab);
            std::cout<<"  Submodel nodes="<<submesh.nodes_mm.size()<<" tets="<<submesh.tetrahedra.size()
                     <<" cut_nodes="<<classification.cut_nodes<<" boundary_nodes="<<classification.boundary_nodes<<std::endl;
            LevelRecord record{level.root,level.sector,submesh.nodes_mm.size(),submesh.tetrahedra.size()};
            std::vector<se::SubmodelCase> solves;
            for(std::size_t c=0;c<window.cases.size();++c) {
                for(const auto& weights:{weightsC[c],weightsS[c],weightsK[c]}) {
                    se::SubmodelCase subcase;subcase.prescribed_m.resize(submesh.nodes_mm.size());
                    subcase.body_acceleration_m_s2={weights[6],0,weights[7]};
                    for(std::size_t n=0;n<submesh.nodes_mm.size();++n)if(classification.constrained[n]) {
                        const Eigen::Vector3d u=interpolator.combine(weights,submesh.nodes_mm[n])*1e-3;
                        subcase.prescribed_m[n]={u.x(),u.y(),u.z()};
                    }
                    solves.push_back(std::move(subcase));
                }
            }
            const auto solved=se::solveSubmodelCases(submesh,material,solves);
            require(solved.size()==3*window.cases.size(),"Submodel solve count mismatch");
            const bool isFinest=level.root==levels.back().root&&level.sector==levels.back().sector;
            if(isFinest){finestMesh=submesh;finestSolved=solved;}
            std::vector<std::size_t> membership;
            for(std::size_t e=0;e<submesh.tetrahedra.size();++e)if(region->contains(centroid(submesh,e)))membership.push_back(e);
            require(!membership.empty(),"Submodel region has no elements");
            for(std::size_t c=0;c<window.cases.size();++c) {
                const auto& cosine=solved[3*c],sine=solved[3*c+1],constant=solved[3*c+2];
                CasePeak peak;
                for(std::size_t e:membership) {
                    const se::StressTensor6 cT=cosine.stress_pa[e],sT=sine.stress_pa[e],kT=constant.stress_pa[e];
                    const double remainder=se::angularNormRemainder(se::vonMises(cT),se::vonMises(sT),48);
                    double maximum=0;std::size_t phaseAt=0;
                    for(std::size_t p=0;p<48;++p) {
                        const double angle=2*std::numbers::pi*static_cast<double>(p)/48;
                        const double vm=se::vonMises(kT+std::cos(angle)*cT+std::sin(angle)*sT);
                        if(vm>maximum){maximum=vm;phaseAt=p;}
                    }
                    if(maximum>peak.sampled){peak.sampled=maximum;peak.element=e;peak.phase=phaseAt;}
                    if(maximum+remainder>peak.bound)peak.bound=maximum+remainder;
                }
                peak.location=centroid(submesh,peak.element);
                peak.cad_state=peakState(peak.location);
                if(level.sector&&level.root==levels.back().root&&level.sector==levels.back().sector) {
                    const double peakAzimuth=azimuthDeg(peak.location);
                    const double margin=(*window.sector)[1]-angularDistance(peakAzimuth,(*window.sector)[0]);
                    require(margin>=-1,"Submodel finest peak outside the refined sector");
                }
                record.cases.push_back(peak);
                if(level.root==levels.back().root&&level.sector==levels.back().sector) {
                    const double angle=2*std::numbers::pi*static_cast<double>(peak.phase)/48;
                    se::ElasticResult exportField;
                    for(std::size_t n=0;n<submesh.nodes_mm.size();++n)
                        exportField.displacement_m.push_back(constant.field.displacement_m[n]+std::cos(angle)*cosine.field.displacement_m[n]
                                                             +std::sin(angle)*sine.field.displacement_m[n]);
                    for(std::size_t e=0;e<submesh.tetrahedra.size();++e)
                        exportField.von_mises_pa.push_back(se::vonMises(constant.stress_pa[e]+std::cos(angle)*cosine.stress_pa[e]
                                                                       +std::sin(angle)*sine.stress_pa[e]));
                    se::writeVtu(windowOut/(window.cases[c].load+"-submodel.vtu"),submesh,exportField);
                }
            }
            records.push_back(std::move(record));
        }
        std::ofstream table(windowOut/"results.csv");
        table<<std::setprecision(12)<<"root_mm,mode,nodes,tets,case,sampled_vm_MPa,phase_upper_MPa,peak_x_mm,peak_S_mm,peak_z_mm,phase_deg,CAD_state\n";
        for(const auto& record:records)for(std::size_t c=0;c<window.cases.size();++c) {
            const auto& peak=record.cases[c];
            table<<record.root<<','<<(record.sector?"sector":"full")<<','<<record.nodes<<','<<record.tets<<','<<window.cases[c].load<<','
                 <<peak.sampled/1e6<<','<<peak.bound/1e6<<','<<peak.location.x()<<','<<peak.location.y()<<','<<peak.location.z()<<','
                 <<360.*static_cast<double>(peak.phase)/48<<','<<(peak.cad_state==0?"inside":peak.cad_state==1?"on":"outside")<<'\n';
        }
        close(table);
        // Azimuthal profile of the region raw peak at the finest level: 24 bins
        // of the peak-phase von Mises per element, evidence for the peak plateau
        // and the sector guard.
        if(finestMesh) {
            std::ofstream profile(windowOut/"azimuth-profile.csv");
            profile<<std::setprecision(12)<<"case,azimuth_centre_deg,peak_vm_MPa\n";
            for(std::size_t c=0;c<window.cases.size();++c) {
                const double angle=2*std::numbers::pi*static_cast<double>(records.back().cases[c].phase)/48;
                std::array<double,24> bins{};std::array<int,24> counts{};
                for(std::size_t e=0;e<finestMesh->tetrahedra.size();++e) {
                    if(!region->contains(centroid(*finestMesh,e)))continue;
                    const auto& cosine=finestSolved[3*c],sine=finestSolved[3*c+1],constant=finestSolved[3*c+2];
                    const double vm=se::vonMises(constant.stress_pa[e]+std::cos(angle)*cosine.stress_pa[e]
                                                 +std::sin(angle)*sine.stress_pa[e]);
                    const double az=azimuthDeg(centroid(*finestMesh,e));
                    const int bin=std::clamp(static_cast<int>(std::floor(az/15)),0,23);
                    bins[static_cast<std::size_t>(bin)]=std::max(bins[static_cast<std::size_t>(bin)],vm);
                    ++counts[static_cast<std::size_t>(bin)];
                }
                for(int bin=0;bin<24;++bin)if(counts[static_cast<std::size_t>(bin)]>0)
                    profile<<window.cases[c].load<<','<<(bin*15+7.5)<<','<<bins[static_cast<std::size_t>(bin)]/1e6<<'\n';
            }
            close(profile);
        }
        std::ofstream report(windowOut/"report.md");
        report<<std::setprecision(10)<<"# F101 R6 feed-shaft submodel — "<<window.region<<" ("<<optimizerName<<")\n\n"
              <<"Slab window S="<<window.station<<" +-"<<window.half<<" mm cut from the exact nominal shaft B-rep. "
              <<"Global field transferred from the torus-focused "<<optimizerName<<" 1/0.09 mm finest solution (barycentric "
              <<"element-local interpolation, nearest-hull projection within 0.05 mm on the faceted boundary). "
              <<"Submodel levels: ";
        for(std::size_t j=0;j<levels.size();++j)report<<(j?", ":"")<<levels[j].root<<" mm "<<(levels[j].sector?"sector":"full-ring");
        report<<" on the region guide faces (growth 0.45, bulk "<<bulk<<" mm), documented node budget 150000 (unchanged cap, solver parity).";
        if(window.sector)report<<" Sector refinement is confined to the peak azimuth "<<(*window.sector)[0]<<" +-"<<(*window.sector)[1]
              <<" degrees with a 15-degree finest-peak-margin guard; the full-ring level cross-checks the sector restriction.";
        if(window.refineBand)report<<" Root refinement is confined to S"<<(*window.refineBand)[0]<<"-"<<(*window.refineBand)[1]
              <<" mm; other guide features inside the window retain a 0.1 mm background root floor.";
        report<<"\n\n";
        report<<"| Case | Root mm | Mode | Nodes | Tets | Sampled MPa | Bound MPa | Peak S mm | Phase deg |\n|---|---:|---|---:|---:|---:|---:|---:|---:|\n";
        for(const auto& record:records)for(std::size_t c=0;c<window.cases.size();++c) {
            const auto& peak=record.cases[c];
            report<<'|'<<window.cases[c].load<<" ("<<window.cases[c].phase_deg<<" deg, signs "<<window.cases[c].signs<<", torque "<<window.cases[c].torque<<") | "
                  <<record.root<<" | "<<(record.sector?"sector":"full")<<" | "<<record.nodes<<" | "<<record.tets<<" | "<<peak.sampled/1e6<<" | "<<peak.bound/1e6<<" | "
                  <<peak.location.y()<<" | "<<360.*static_cast<double>(peak.phase)/48<<" |\n";
        }
        const auto& coarse=records[records.size()-2],finest=records.back();
        report<<"\nFour-mesh (global) values and submodel last-change (final pair "
              <<coarse.root<<" mm "<<(coarse.sector?"sector":"full")<<" -> "<<finest.root<<" mm "<<(finest.sector?"sector":"full")<<"):\n\n"
              <<"| Case | Global sampled MPa | Global bound MPa | Submodel sampled change % | Submodel bound change % | Peak dS mm | Peak dr mm | Peak daz deg | Peak CAD state |\n|---|---:|---:|---:|---:|---:|---:|---:|---|\n";
        for(std::size_t c=0;c<window.cases.size();++c) {
            const double sampledDelta=change(coarse.cases[c].sampled,finest.cases[c].sampled);
            const double boundDelta=change(coarse.cases[c].bound,finest.cases[c].bound);
            const double dS=std::abs(coarse.cases[c].location.y()-finest.cases[c].location.y());
            const double r0=std::hypot(coarse.cases[c].location.x(),coarse.cases[c].location.z());
            const double r1=std::hypot(finest.cases[c].location.x(),finest.cases[c].location.z());
            const double dAzimuth=angularDistance(azimuthDeg(coarse.cases[c].location),azimuthDeg(finest.cases[c].location));
            const char* stateName=finest.cases[c].cad_state==0?"inside":finest.cases[c].cad_state==1?"on":"OUTSIDE";
            const bool converged=sampledDelta<0.05&&boundDelta<0.05&&finest.cases[c].cad_state!=2;
            report<<'|'<<window.cases[c].load<<" | "<<window.cases[c].global_sampled_mpa<<" | "<<window.cases[c].global_bound_mpa<<" | "
                  <<sampledDelta*100<<" | "<<boundDelta*100<<" | "<<dS<<" | "<<std::abs(r0-r1)<<" | "<<dAzimuth<<" | "<<stateName<<" |\n";
            if(converged)++convergedCases;
        }
        if(window.kt) {
            const double D=(*window.kt)[0],d=(*window.kt)[1],r=(*window.kt)[2];
            const double h=(D-d)/2,ratio=h/r;
            const auto ktBending=se::pilkeyShoulderBendingKt(D,d,r);
            report<<"\nPeterson/Pilkey shoulder-fillet cross-check (W.D. Pilkey, Peterson's Stress Concentration Factors, "
                  <<"3rd ed., Wiley 2008, Table 3.3, bending, 2<h/r<=20; Kt=C1+C2(2h/D)+C3(2h/D)^2+C4(2h/D)^3): "
                  <<"D="<<D<<" mm, d="<<d<<" mm, r="<<r<<" mm, h/r="<<ratio<<"\n\n";
            for(std::size_t c=0;c<window.cases.size();++c) {
                const double angle=phaseRad[c];
                std::vector<se::StressTensor6> governing(global.tetrahedra.size());
                for(std::size_t e=0;e<global.tetrahedra.size();++e) {
                    const auto kT=combination<se::StressTensor6>(basis,weightsK[c],[&](const auto& b)->const se::StressTensor6&{return b.stress_pa[e];});
                    const auto cT=combination<se::StressTensor6>(basis,weightsC[c],[&](const auto& b)->const se::StressTensor6&{return b.stress_pa[e];});
                    const auto sT=combination<se::StressTensor6>(basis,weightsS[c],[&](const auto& b)->const se::StressTensor6&{return b.stress_pa[e];});
                    governing[e]=kT+std::cos(angle)*cT+std::sin(angle)*sT;
                }
                const double bandStation=window.band?(*window.band)[0]:window.station-window.half;
                const double bandHalf=window.band?(*window.band)[1]:1.5;
                const auto rearBand=se::nominalBandStress(global,governing,window.station-window.half,1.5);
                const auto frontBand=se::nominalBandStress(global,governing,window.station+window.half,1.5);
                const auto band=se::nominalBandStress(global,governing,bandStation,bandHalf);
                const double nominalRadius=d/2;
                const double sigmaNominalBending=band.bending_surface_pa*std::pow(band.section_radius_mm/nominalRadius,3);
                const double measuredKt=finest.cases[c].sampled/std::max(1e-30,sigmaNominalBending);
                const double measuredVsBandVm=finest.cases[c].sampled/std::max(1e-30,band.von_mises_mean_pa);
                report<<"Case "<<window.cases[c].load<<" (phase "<<window.cases[c].phase_deg<<" deg): nominal band at S="<<bandStation
                      <<" (section radius "<<band.section_radius_mm<<" mm), nominal bending surface stress "<<sigmaNominalBending/1e6
                      <<" MPa, band mean von Mises "<<band.von_mises_mean_pa/1e6<<" MPa, torsion surface shear "<<band.torsion_surface_pa/1e6<<" MPa. ";
                report<<"Cut-plane reference bands: rear S="<<window.station-window.half<<" (bending "<<rearBand.bending_surface_pa/1e6
                      <<" MPa at r "<<rearBand.section_radius_mm<<"), front S="<<window.station+window.half<<" (bending "<<frontBand.bending_surface_pa/1e6
                      <<" MPa at r "<<frontBand.section_radius_mm<<"). ";
                if(ktBending) {
                    const double combined=std::sqrt(std::pow(*ktBending*sigmaNominalBending,2)+3*std::pow(band.torsion_surface_pa,2));
                    report<<"Handbook bending Kt="<<*ktBending<<"; Kt-based peak "<<(*ktBending)*sigmaNominalBending/1e6
                          <<" MPa vs submodel sampled peak "<<finest.cases[c].sampled/1e6<<" MPa (measured Kt "
                          <<measuredKt<<", submodel/band-vm ratio "<<measuredVsBandVm<<"). ";
                    report<<"Combined bending+torsion estimate with the band torsion shear but no separate torsional Kt: "<<combined/1e6<<" MPa. ";
                } else {
                    report<<"Handbook bending Kt not asserted (h/r outside the published 2<h/r<=20 fit range); submodel peak "
                          <<finest.cases[c].sampled/1e6<<" MPa, measured peak/nominal-bending ratio "<<measuredKt
                          <<", submodel/band-vm ratio "<<measuredVsBandVm<<". ";
                }
                report<<"The torsion fit (Pilkey Table 4.1, 0.25<=h/r<=4) is not asserted; the band torsion shear is FE-derived.\n";
            }
        } else {
            report<<"\nNo citable closed-form Kt exists for the milled key-floor perimeter blend of this region; "
                  <<"the submodel convergence is the primary evidence and no handbook factor is asserted.\n";
        }
        report<<"\nInterpolation audit: exact hits "<<interpolator.audit().exact_hits
              <<", boundary projections "<<interpolator.audit().boundary_projections
              <<", unsupported "<<interpolator.audit().unsupported
              <<", maximum projection gap "<<interpolator.audit().max_gap_mm<<" mm. "
              <<"This is a submodel study of the regional raw peaks only; it does not release fabrication or re-certify "
              <<"whole-shaft equilibrium, and no four-mesh gate value is altered or removed.\n";
        close(report);
        std::cout<<"  Interpolation audit: exact="<<interpolator.audit().exact_hits
                 <<" boundary_projections="<<interpolator.audit().boundary_projections
                 <<" unsupported="<<interpolator.audit().unsupported
                 <<" max_gap_mm="<<interpolator.audit().max_gap_mm<<std::endl;
    }
    {std::ofstream state(out/"status.json");
     state<<"{\"run_complete\":true,\"optimizer\":\""<<optimizerName<<"\",\"node_limit\":150000,"
          <<"\"mesh_levels\":"<<levels.size()<<",\"submodel_converged_cases\":"<<convergedCases
          <<",\"fabrication_released\":false,\"numerical_gates_passed\":false}\n";close(state);}
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
