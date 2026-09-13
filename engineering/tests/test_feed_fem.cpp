#include "feed_fem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught,"Invalid shaft FEM input accepted");}
}
int main()try{
    const auto zero=se::feedShaftCoefficients(0,1,-1,1,0.7,-0.2);
    require(zero[0]==480&&zero[1]==0&&zero[2]==-60&&zero[3]==60,"Shaft phase-zero loads");
    const auto quarter=se::feedShaftCoefficients(std::numbers::pi/2,1,-1,1,0.7,-0.2);
    require(std::abs(quarter[0])<1e-10&&std::abs(quarter[1]-480)<1e-10&&std::abs(quarter[2]+60)<1e-10&&std::abs(quarter[3]+60)<1e-10,"Material-frame rotation sign");
    require(std::abs(quarter[6]-9.80665)<1e-12&&std::abs(quarter[7])<1e-12,"Gravity rotates with material frame");
    const auto vertices=se::feedShaftTorqueVertices(5);
    require(vertices.size()==6,"Torque polygon vertex count");
    for(const auto& v:vertices)require(std::abs(v[0])<=5&&std::abs(v[1])<=5&&std::abs(v[0]+v[1])<=5,"Torque vertex exceeds interface envelope");
    // Linear support-function maxima on a densely sampled feasible polygon
    // must not exceed the six selected vertices.
    for(int i=-6;i<=6;++i)for(int j=-6;j<=6;++j) {
        double maximum=-1e100;for(const auto& v:vertices)maximum=std::max(maximum,i*v[0]+j*v[1]);
        for(int r=-20;r<=20;++r)for(int p=-20;p<=20;++p)if(std::abs(r+p)<=20)
            require(i*(r/4.0)+j*(p/4.0)<=maximum+1e-12,"Torque polygon misses a feasible maximum");
    }
    se::StressTensor6 a,b,c;a<<13,-21,8,3,7,-5;b<<4,19,-11,-8,2,6;c<<-9,1,12,5,-4,3;
    for(std::size_t count:{8U,16U,48U,96U}) {
        double sampled=0,truth=0;
        for(std::size_t p=0;p<count;++p) {
            const double angle=2*std::numbers::pi*static_cast<double>(p)/static_cast<double>(count);
            sampled=std::max(sampled,se::vonMises(a+std::cos(angle)*b+std::sin(angle)*c));
        }
        for(int p=0;p<10001;++p) {
            const double angle=2*std::numbers::pi*(p+0.37)/10001;
            truth=std::max(truth,se::vonMises(a+std::cos(angle)*b+std::sin(angle)*c));
        }
        require(truth<=sampled+se::angularNormRemainder(se::vonMises(b),se::vonMises(c),count),"Angular stress bound misses unsampled maximum");
    }
    se::StressTensor6 hydro=se::StressTensor6::Zero();hydro.head<3>().setConstant(123);
    require(se::vonMises(hydro)==0,"Hydrostatic stress produces von Mises magnitude");
    require(se::feedShaftSupports().size()==6&&se::feedShaftBasisLoads().size()==8,"Shaft support/basis count");
    require(se::feedShaftRootDistance({5,92.2,0})<1e-9,"Roller root refinement field misses its tangent");
    require(se::feedShaftRootDistance({0,8,0})>5,"Root refinement incorrectly targets unfeatured interior");
    const std::array<std::string_view,8> focuses{"A12_groove","rear_shoulder_and_slot_entry","M15_relief","front_bearing_shoulder",
        "neck_shoulder","roller_shoulder","pulley_key_and_MB2_slot","roller_key_root"};
    for(double x:{-7.,0.,2.1,6.})for(double y:{0.,18.54,19.58,31.3,48.,61.55,72.8,81.8,87.5,92.2,100.,113.})for(double z:{-4.,0.,5.5}) {
        const Eigen::Vector3d p{x,y,z};double nearest=std::numeric_limits<double>::infinity();
        for(const auto focus:focuses)nearest=std::min(nearest,se::feedShaftRootDistance(p,focus));
        require(nearest==se::feedShaftRootDistance(p),"Feature guides do not reconstruct unchanged all-root field");
    }
    require(se::feedShaftRootDistance({5.725,18.54,0},"A12_groove")<1e-12,"Focused A12 guide misses actual minor radius");
    require(se::feedShaftRootDistance({5,92.2,0},"A12_groove")>70,"A12 focus refines unrelated roller root");
    rejects([]{se::feedShaftRootDistance({0,0,0},"unknown");});
    rejects([]{se::feedShaftRootDistance(Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN()));});
    rejects([]{se::feedShaftCoefficients(0,0,1,1,0,0);});
    rejects([]{se::feedShaftCoefficients(0,std::numeric_limits<int>::min(),1,1,0,0);});
    rejects([]{se::feedShaftTorqueVertices(0);});
    rejects([]{se::angularNormRemainder(1,1,0);});
    rejects([]{se::vonMises(se::StressTensor6::Constant(std::numeric_limits<double>::quiet_NaN()));});
    std::cout<<"Shaft rotation, torque vertices, angular bound and input checks passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}