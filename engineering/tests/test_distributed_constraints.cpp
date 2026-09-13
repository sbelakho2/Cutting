#include "fem.hpp"
#include <Eigen/Geometry>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::vector<se::DistributedConstraint> endCoordinates(double y) {
    const auto face=[=](const Eigen::Vector3d& p){return std::abs(p.y()-y)<1e-7;};
    const Eigen::Vector3d centre{5,y,5};std::vector<se::DistributedConstraint> out;
    for(int a=0;a<3;++a)out.push_back({{face,Eigen::Vector3d::Unit(a),centre}});
    for(int a=0;a<3;++a)out.push_back({{face,Eigen::Vector3d::Zero(),centre,Eigen::Vector3d::Unit(a)}});
    return out;
}
}
int main()try{
    const auto mesh=se::meshBox(10,40,10,4);
    const se::ElasticMaterial material{205e9,0.29,7850};
    constexpr double extension=1e-6,length=0.040,area=1e-4;
    const double axialStress=material.young_pa*extension/length,reaction=axialStress*area,energy=reaction*extension/2;
    const auto six=endCoordinates(0);auto moved=six;
    const Eigen::Vector3d translation{1e-6,-2e-6,3e-6},rotation{2e-5,-3e-5,4e-5};
    for(int a=0;a<3;++a){moved[static_cast<std::size_t>(a)].value=translation[a];moved[static_cast<std::size_t>(a+3)].value=rotation[a];}
    const auto tip=endCoordinates(40);
    se::ElasticLoadCase load;load.patches={tip[0].virtual_load,tip[1].virtual_load,tip[4].virtual_load};
    load.patches[0].force_n*=17;load.patches[1].force_n*=31;load.patches[2].couple_nm*=0.2;
    load.body_acceleration_m_s2={1.2,-3.4,5.6};
    const auto reference=se::solveDistributedElasticCases(mesh,material,six,{load}).front();
    double referenceStress=0;for(const auto& stress:reference.stress_pa)referenceStress=std::max(referenceStress,stress.norm());
    for(double scale:{0.1,1.0,10.0}) {
        const auto result=se::solveDistributedElasticCases(mesh,material,moved,{load},scale).front();
        require(std::abs(result.energy_j/reference.energy_j-1)<1e-7,"Six-coordinate rigid lifting changes elastic energy");
        for(std::size_t n=0;n<mesh.nodes_mm.size();++n) {
            const Eigen::Vector3d rigid=translation+rotation.cross(mesh.nodes_mm[n]*1e-3-Eigen::Vector3d{0.005,0,0.005});
            require((result.displacement_m[n]-reference.displacement_m[n]-rigid).norm()<1e-12,"Simultaneous six-coordinate lifting mismatch");
        }
        for(std::size_t e=0;e<mesh.tetrahedra.size();++e)
            require((result.stress_pa[e]-reference.stress_pa[e]).norm()<referenceStress*1e-7,"Rigid lifting changes stress tensor");
        for(std::size_t j=0;j<six.size();++j) {
            require((result.support_force_n[j]-reference.support_force_n[j]).norm()<1e-8,"Rigid lifting changes support force");
            require((result.support_moment_nm[j]-reference.support_moment_nm[j]).norm()<1e-10,"Rigid lifting changes support moment");
        }
    }
    for(int count:{7,12}) {
        auto constraints=endCoordinates(0);auto front=endCoordinates(40);front[1].value=extension;
        if(count==7)constraints.push_back(front[1]);else constraints.insert(constraints.end(),front.begin(),front.end());
        for(double scale:{0.1,1.0,10.0}) {
            const auto result=se::solveDistributedElasticCases(mesh,material,constraints,{se::ElasticLoadCase{}},scale).front();
            require(std::abs(result.energy_j/energy-1)<1e-7,"Prescribed axial-strain energy");
            require(std::abs(result.support_force_n[1].y()+reaction)<reaction*1e-7,"Rear imposed-strain reaction");
            require(std::abs(result.support_force_n[count==7?6U:7U].y()-reaction)<reaction*1e-7,"Front imposed-strain reaction");
            for(std::size_t n=0;n<mesh.nodes_mm.size();++n) {
                const Eigen::Vector3d p=mesh.nodes_mm[n]*1e-3;
                const Eigen::Vector3d exact{-material.poisson*extension/length*(p.x()-0.005),extension*p.y()/length,
                                            -material.poisson*extension/length*(p.z()-0.005)};
                require((result.displacement_m[n]-exact).norm()<extension*1e-7,"Affine axial displacement / Poisson contraction");
            }
            se::StressTensor6 exact=se::StressTensor6::Zero();exact[1]=axialStress;
            for(const auto& stress:result.stress_pa)require((stress-exact).norm()<axialStress*1e-7,"Affine axial full stress tensor");
        }
    }
    auto dependent=endCoordinates(0);dependent.push_back(dependent[0]);bool rejected=false;
    try{se::solveDistributedElasticCases(mesh,material,dependent,{se::ElasticLoadCase{}});}catch(const std::exception&){rejected=true;}
    require(rejected,"Dependent seventh constraint accepted");
    std::cout<<"Six-coordinate rigid lifting; seven/twelve-coordinate strain; reactions, tensor and scale invariance passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}