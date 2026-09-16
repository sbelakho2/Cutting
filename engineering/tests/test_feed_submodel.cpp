#include "feed_submodel.hpp"
#include "feed_geometry.hpp"
#include "head_geometry.hpp"
#include "mesh_io.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught,"Invalid submodel input accepted");}
Eigen::Vector3d linear(const Eigen::Vector3d& p) {
    return {1e-5*(p.x()+p.y()),2e-5*(p.y()+p.z()),3e-5*(p.x()+p.z())};
}
}
int main(int argc,char** argv)try{
    require(argc>=2,"Artifact directory argument required");
    const fs::path out=argv[1];fs::create_directories(out);
    const se::ElasticMaterial material{205e9,0.29,7850};

    // 1. VTU field write/read round-trip and fail-closed field rejections.
    {
        const auto box=se::meshBox(2,2,2,0.5);
        const se::Constraint fixed{std::optional<double>(0.),std::optional<double>(0.),std::optional<double>(0.)};
        const se::Constraint free{std::nullopt,std::nullopt,std::nullopt};
        const auto solved=se::solveElastic(box,material,[&](const Eigen::Vector3d& p)->se::Constraint{
            if(std::abs(p.x())<1e-6)return fixed;
            return free;
        },std::vector<se::LoadPatch>{},0,Eigen::Vector3d{0,0,-9.80665});
        se::writeVtu(out/"roundtrip.vtu",box,solved);
        const auto field=se::readLinearVtuField(out/"roundtrip.vtu");
        require(field.mesh.nodes_mm.size()==box.nodes_mm.size(),"VTU field node count mismatch");
        require(field.displacement_mm.size()==box.nodes_mm.size()&&field.von_mises_mpa.size()==box.tetrahedra.size(),"VTU field array size mismatch");
        double maxDisp=0,maxVm=0;
        for(std::size_t n=0;n<box.nodes_mm.size();++n)maxDisp=std::max(maxDisp,(field.displacement_mm[n]-solved.displacement_m[n]*1000).norm());
        for(std::size_t e=0;e<box.tetrahedra.size();++e)maxVm=std::max(maxVm,std::abs(field.von_mises_mpa[e]-solved.von_mises_pa[e]*1e-6));
        require(maxDisp<1e-9&&maxVm<1e-9,"VTU field round-trip disagrees");
        std::ifstream source(out/"roundtrip.vtu");const std::string content{std::istreambuf_iterator<char>(source),std::istreambuf_iterator<char>()};
        {
            std::string broken=content;const std::string token="Name=\"displacement_mm\" NumberOfComponents=\"3\" format=\"ascii\"";
            const auto at=broken.find(token);require(at!=std::string::npos,"Displacement array token missing");
            broken.replace(at+token.size()-10,10,"format=\"binary\"");
            std::ofstream file(out/"broken-binary.vtu");file<<broken;file.close();
            rejects([&]{se::readLinearVtuField(out/"broken-binary.vtu");});
        }
        {
            std::string broken=content;const std::string token="Name=\"von_mises_MPa\" format=\"ascii\"";
            const auto at=broken.find(token);require(at!=std::string::npos,"Stress array token missing");
            broken.erase(at+token.size()-15,15); // remove the array name
            std::ofstream file(out/"broken-stress.vtu");file<<broken;file.close();
            rejects([&]{se::readLinearVtuField(out/"broken-stress.vtu");});
        }
    }

    // 2. Barycentric interpolation, outside-support error path, hull projection.
    {
        const auto box=se::meshBox(10,10,10,1.0);
        std::vector<std::vector<Eigen::Vector3d>> fields(2);
        for(std::size_t n=0;n<box.nodes_mm.size();++n) {
            fields[0].push_back(linear(box.nodes_mm[n]));
            fields[1].push_back(-2*linear(box.nodes_mm[n]));
        }
        se::DisplacementFieldInterpolator interpolator;interpolator.build(box,fields,0.02);
        std::mt19937 engine(7);std::uniform_real_distribution<double> inside(1,9);
        for(int i=0;i<200;++i) {
            const Eigen::Vector3d p{inside(engine),inside(engine),inside(engine)};
            require((interpolator.interpolate(0,p)-linear(p)).norm()<1e-6,"Interior barycentric interpolation error");
            const Eigen::Vector3d combined=interpolator.combine({1,-1,0,0,0,0,0,0},p);
            require((combined-3*linear(p)).norm()<1e-6,"Weighted combination interpolation error");
        }
        require(interpolator.audit().exact_hits>=200,"Interpolator did not record exact hits");
        rejects([&]{interpolator.interpolate(0,{10.5,5,5});});
        const Eigen::Vector3d nearValue=interpolator.interpolate(0,{10.001,5,5});
        require((nearValue-linear(Eigen::Vector3d{10,5,5})).norm()<1e-6,"Hull projection value error");
        require(interpolator.audit().boundary_projections>=1,"Hull projection not recorded");
        rejects([&]{interpolator.interpolate(0,{10.03,5,5});});
    }

    // 3. Slab boolean cut of the actual shaft and cut-node classification.
    {
        const auto shaft=se::feedShaftShape();
        const auto slab=se::cutShaftSlab(shaft,92.1,6.0);
        require(se::volume(slab.shape)>0&&se::volume(slab.shape)<se::volume(shaft),"Slab volume out of range");
        se::writeStep(slab.shape,out/"slab.step");
        const auto submesh=se::meshStepRefined(out/"slab.step",0.8,0.8,[](const Eigen::Vector3d&){return 0.8;});
        const auto classification=se::classifySlabCutNodes(submesh,slab);
        require(classification.cut_nodes>0&&classification.cut_nodes<classification.boundary_nodes,"Slab cut classification empty or total");
        for(std::size_t n=0;n<submesh.nodes_mm.size();++n)if(classification.constrained[n]) {
            const double y=submesh.nodes_mm[n].y();
            require(std::abs(y-(92.1-6.0))<1e-3||std::abs(y-(92.1+6.0))<1e-3,"Constrained node off the cut planes");
        }
    }

    // 4. Submodel solver patch test: prescribed linear field reproduced with
    //    zero body acceleration and exact equilibrium/balance checks.
    {
        const auto box=se::meshBox(6,6,6,0.6);
        se::SubmodelCase load;load.prescribed_m.resize(box.nodes_mm.size());
        const auto hull=[&](const Eigen::Vector3d& p) {
            const double tol=1e-4;
            return p.x()<tol||p.y()<tol||p.z()<tol||p.x()>6-tol||p.y()>6-tol||p.z()>6-tol;
        };
        for(std::size_t n=0;n<box.nodes_mm.size();++n)if(hull(box.nodes_mm[n])) {
            const Eigen::Vector3d u=linear(box.nodes_mm[n]);
            load.prescribed_m[n]={u.x(),u.y(),u.z()};
        }
        const auto solved=se::solveSubmodelCases(box,material,{load});
        require(solved.size()==1,"Submodel solve count mismatch");
        double maxError=0,scale=0;
        for(std::size_t n=0;n<box.nodes_mm.size();++n) {
            const Eigen::Vector3d exact=linear(box.nodes_mm[n]);
            scale=std::max(scale,exact.norm());
            maxError=std::max(maxError,(solved[0].field.displacement_m[n]-exact).norm());
        }
        require(maxError<1e-6*scale,"Submodel patch test does not reproduce the exact linear field");
        require(solved[0].field.free_residual_relative<1e-7&&solved[0].field.force_balance_relative<1e-7,"Submodel residual checks failed");
        require(solved[0].stress_pa.size()==box.tetrahedra.size(),"Submodel stress tensor count mismatch");
    }

    // 5. Pilkey shoulder-fillet bending fit.
    {
        const auto anchor=se::pilkeyShoulderBendingKt(1.5,1.0,0.1);
        require(anchor&&std::abs(*anchor-1.7433)<0.01,"Pilkey bending fit disagrees with the Shigley A-15-9 anchor 1.74");
        const auto sharp=se::pilkeyShoulderBendingKt(2.0,1.0,0.15),rounder=se::pilkeyShoulderBendingKt(2.0,1.0,0.2);
        require(sharp&&rounder&&*sharp>*rounder&&*rounder>1,"Pilkey fit not monotonic in fillet radius");
        require(se::pilkeyShoulderBendingKt(12.4,10.0,0.2)&&se::pilkeyShoulderBendingKt(15.0,13.5,0.2),"Shaft shoulder geometries outside fit range");
        require(!se::pilkeyShoulderBendingKt(1.2,1.0,0.5),"Fit asserted below h/r<=2");
        require(!se::pilkeyShoulderBendingKt(3.0,1.0,0.04),"Fit asserted above h/r>20");
        rejects([]{se::pilkeyShoulderBendingKt(1.0,2.0,0.2);});
    }

    std::cout<<"Submodel VTU field, interpolation, slab cut, solver patch test and Kt checks passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
