#include "fem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught,"Invalid distributed model accepted");}
std::vector<se::DistributedConstraint> endSupport() {
    const auto face=[](const Eigen::Vector3d& p){return std::abs(p.y())<1e-7;};
    std::vector<se::DistributedConstraint> constraints;
    for(int axis=0;axis<3;++axis)constraints.push_back({{face,Eigen::Vector3d::Unit(axis),Eigen::Vector3d::Zero()}});
    for(int axis=0;axis<3;++axis)constraints.push_back({{face,Eigen::Vector3d::Zero(),Eigen::Vector3d::Zero(),Eigen::Vector3d::Unit(axis)}});
    return constraints;
}
}
int main()try{
    const se::ElasticMaterial material{205e9,0.29,7850};
    const auto constraints=endSupport();
    const auto end=[](const Eigen::Vector3d& p){return std::abs(p.y()-40)<1e-7;};
    const se::LoadPatch axial{end,{0,1000,0},Eigen::Vector3d{0,40,0}};
    const se::LoadPatch torque{end,Eigen::Vector3d::Zero(),Eigen::Vector3d{0,40,0},{0,1,0}};
    double previous=1;
    for(double h:{1.8,0.9,0.6,0.4,0.28}) {
        const auto mesh=se::meshCylinder(5,40,h);
        auto doubled=torque;doubled.couple_nm*=2;
        const auto results=se::solveDistributedElasticCases(mesh,material,constraints,{{{axial}},{{torque}},{{doubled}}});
        const double area=std::numbers::pi*25e-6,J=std::numbers::pi*std::pow(0.005,4)/2;
        const double G=material.young_pa/(2*(1+material.poisson)),exactAngle=0.040/(G*J);
        const double axialEnergy=0.5*1000*1000*0.040/(material.young_pa*area);
        require(std::abs(results[0].energy_j/axialEnergy-1)<0.035,"Distributed axial energy versus circular bar");
        const double angle=2*results[1].energy_j; // Work = T*angle/2 for unit torque.
        const double error=std::abs(angle/exactAngle-1);
        require(error<previous,"Circular-shaft torsion failed mesh refinement");previous=error;
        require((results[0].total_load_n-Eigen::Vector3d{0,1000,0}).norm()<1e-8,"Axial resultant changed");
        require((results[1].total_load_moment_nm-Eigen::Vector3d{0,1,0}).norm()<1e-9,"Pure torque resultant changed");
        require(std::abs(results[2].energy_j-4*results[1].energy_j)<1e-11,"Distributed cases accumulated loads");
        require(std::abs(results[2].max_displacement_m-2*results[1].max_displacement_m)<1e-12,"Distributed displacement scaling");
        // The interior circular-shaft solution has sigma_xy=G*twist_rate*z and
        // sigma_yz=-G*twist_rate*x. No arbitrary torsion/notch factor is used.
        double numerator=0,denominator=0;
        for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
            Eigen::Vector3d p=Eigen::Vector3d::Zero();for(int node:mesh.tetrahedra[e])p+=mesh.nodes_mm[static_cast<std::size_t>(node)]/4;
            if(p.y()<10||p.y()>30)continue;
            se::StressTensor6 expected=se::StressTensor6::Zero();expected[3]=p.z()*1e-3/J;expected[4]=-p.x()*1e-3/J;
            numerator+=(results[1].stress_pa[e]-expected).squaredNorm();denominator+=expected.squaredNorm();
        }
        std::cout<<"Distributed circular bar h="<<h<<" nodes="<<mesh.nodes_mm.size()<<" torsion_relative_error="<<error
             <<" stress_tensor_relative_error="<<std::sqrt(numerator/denominator)<<'\n';
        if(h<0.3) {
            require(error<0.04,"Fine torsional compliance error exceeds4percent");
            require(std::sqrt(numerator/denominator)<0.12,"Interior torsion stress tensor error");
        } else if(h>=0.9) {
            const auto changed=se::solveDistributedElasticCases(mesh,material,constraints,{{{torque}}},10);
            require(std::abs(changed[0].energy_j/results[1].energy_j-1)<1e-7,"Stabilisation contaminated physical stiffness");
            auto moved=constraints;moved[0].value=1e-7;
            const auto translated=se::solveDistributedElasticCases(mesh,material,moved,{{{axial}}});
            double maxDifference=0;
            for(std::size_t i=0;i<mesh.nodes_mm.size();++i)
                maxDifference=std::max(maxDifference,(translated[0].displacement_m[i]-results[0].displacement_m[i]-Eigen::Vector3d{1e-7,0,0}).norm());
            require(maxDifference<1e-10,"Nonzero distributed constraint lifting");
            auto duplicate=constraints;duplicate[5]=duplicate[4];
            rejects([&]{se::solveDistributedElasticCases(mesh,material,duplicate,{{{axial}}});});
            auto empty=constraints;empty[0].virtual_load.select=[](const Eigen::Vector3d&){return false;};
            rejects([&]{se::solveDistributedElasticCases(mesh,material,empty,{{{axial}}});});
            rejects([&]{se::solveDistributedElasticCases(mesh,material,constraints,{{{axial}}},0);});
        }
    }
    std::cout<<"Distributed support, axial, torsion, scaling and rejection benchmarks passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}