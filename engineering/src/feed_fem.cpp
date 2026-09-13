#include "feed_fem.hpp"
#include "shared/feed_drive_spec.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace starz::engineering {
namespace {
namespace d=starz::feed::design;
auto band(double y,double half,double radius) {
    return [=](const Eigen::Vector3d& p){return std::abs(p.y()-y)<half&&std::hypot(p.x(),p.z())>radius-0.12;};
}
LoadPatch forcePatch(double y,double half,double radius,int axis) {
    return {band(y,half,radius),Eigen::Vector3d::Unit(axis),Eigen::Vector3d{0,y,0}};
}
LoadPatch torquePatch(double y,double half,double radius,double sign) {
    return {band(y,half,radius),Eigen::Vector3d::Zero(),Eigen::Vector3d{0,y,0},{0,sign,0}};
}
double ringDistance(const Eigen::Vector3d& p,double station,double radius) {
    return std::hypot(p.y()-station,std::hypot(p.x(),p.z())-radius);
}
double capsulePerimeterDistance(const Eigen::Vector3d& p,double begin,double end,double radius,double z) {
    const double nearest=std::clamp(p.y(),begin+radius,end-radius);
    const double radial=std::hypot(p.x(),p.y()-nearest);
    return std::hypot(radial-radius,p.z()-z);
}
}
std::vector<DistributedConstraint> feedShaftSupports() {
    std::vector<DistributedConstraint> supports;
    for(int axis:{0,2})supports.push_back({forcePatch(d::rear_support,1.5,6,axis)});
    for(int axis:{0,2})supports.push_back({forcePatch(d::front_support,1.5,7.5,axis)});
    supports.push_back({forcePatch(d::front_support,1.5,7.5,1)});
    // Balanced transmission loads require zero spin reaction. Actual eccentric
    // mesh gravity additionally needs a drive holding moment at this interface.
    supports.push_back({torquePatch(9,5,6,1)});
    return supports;
}
std::vector<ElasticLoadCase> feedShaftBasisLoads() {
    std::vector<ElasticLoadCase> basis;
    for(int axis:{0,2})basis.push_back({{forcePatch(d::pulley_station,10,7,axis)}});
    for(int axis:{0,2}) {
        auto patch=forcePatch(105,6,5,axis);patch.resultant_point_mm=Eigen::Vector3d{0,d::wire_contact_station,0};
        basis.push_back({{patch}});
    }
    basis.push_back({{torquePatch(9,5,6,1),torquePatch(105,6,5,-1)}});
    basis.push_back({{torquePatch(9,5,6,1),torquePatch(d::pulley_station,10,7,-1)}});
    basis.push_back({{},Eigen::Vector3d{1,0,0}});basis.push_back({{},Eigen::Vector3d{0,0,1}});
    return basis;
}
std::array<double,8> feedShaftCoefficients(double phase,int belt,int rx,int rz,double tr,double tp) {
    const auto validSign=[](int sign){return sign==1||sign==-1;};
    if(!std::isfinite(phase)||!std::isfinite(tr)||!std::isfinite(tp)||!validSign(belt)||!validSign(rx)||!validSign(rz))
        throw std::invalid_argument("Invalid shaft phase/load sign");
    const double c=std::cos(phase),s=std::sin(phase),bx=belt*d::belt_radial_design_n,x=rx*d::roller_radial_design_n,z=rz*d::roller_radial_design_n;
    // Transform stationary global loads into the rotating material frame R_y(-phi).
    return {bx*c,bx*s,x*c-z*s,x*s+z*c,tr,tp,9.80665*s,-9.80665*c};
}
std::vector<std::array<double,2>> feedShaftTorqueVertices(double limit) {
    if(!std::isfinite(limit)||limit<=0)throw std::invalid_argument("Invalid interface torque limit");
    // |Tr|,|Tp|,|Tr+Tp| <= limit. Convex stress/displacement norms attain
    // their maxima at vertices, including opposing roller/pulley torque flow.
    return {{limit,0},{0,limit},{-limit,limit},{-limit,0},{0,-limit},{limit,-limit}};
}
double feedShaftRootDistance(const Eigen::Vector3d& p,std::string_view focus) {
    if(!p.allFinite())throw std::invalid_argument("Non-finite root-sizing coordinate");
    double distance=std::numeric_limits<double>::infinity();
    struct Ring {std::string_view name;double station,radius,fillet;};
    for(const auto& root:std::array<Ring,8>{{
        {"A12_groove",18.54,5.765,0.04},{"A12_groove",19.58,5.765,0.04},
        {"rear_shoulder_and_slot_entry",31.3,6.2,0.20},
        {"M15_relief",71.7,6.95,0.20},{"M15_relief",72.8,6.95,0.20},
        {"front_bearing_shoulder",81.8,7.7,0.20},{"neck_shoulder",87.5,6.5,0.50},
        {"roller_shoulder",92.2,5.2,0.20}}})
        if(focus.empty()||focus==root.name)
            distance=std::min(distance,std::abs(ringDistance(p,root.station,root.radius)-root.fillet));
    if(focus.empty()||focus=="pulley_key_and_MB2_slot")
        distance=std::min(distance,std::max(0.,capsulePerimeterDistance(p,33.45,61.55,2.52,4)-0.20));
    if(focus.empty()||focus=="roller_key_root")
        distance=std::min(distance,std::max(0.,capsulePerimeterDistance(p,94.45,112.55,1.52,-3.2)-0.20));
    if(focus.empty()||focus=="pulley_key_and_MB2_slot") {
        // Entire intersecting slot lips are included; no guessed STEP face IDs.
        const double dy=std::max({31.2-p.y(),0.,p.y()-73});
        distance=std::min(distance,std::sqrt(std::pow(std::abs(p.x())-2.1,2)+dy*dy+std::pow(p.z()-5.5,2)));
        for(double end:{31.2,33.45,61.55,73.0}) {
            const double dx=std::max(0.,std::abs(p.x())-2.65);
            distance=std::min(distance,std::sqrt(dx*dx+std::pow(p.y()-end,2)+std::pow(p.z()-5.5,2)));
        }
    }
    if(!std::isfinite(distance))throw std::invalid_argument("Unknown shaft mesh focus region");
    return distance;
}
std::vector<ShaftStressRegion> feedShaftStressRegions() {
    std::vector<ShaftStressRegion> regions{{"whole_nominal_shaft",[](const Eigen::Vector3d&){return true;}}};
    for(const auto& item:std::array<std::pair<const char*,double>,6>{{
        {"A12_groove",19.06},{"rear_shoulder_and_slot_entry",31.5},{"M15_relief",72.25},
        {"front_bearing_shoulder",82},{"neck_shoulder",87},{"roller_shoulder",92}}}) {
        const double station=item.second;
        regions.push_back({item.first,[=](const Eigen::Vector3d& p){return std::abs(p.y()-station)<1.3;}});
    }
    regions.push_back({"pulley_key_and_MB2_slot",[](const Eigen::Vector3d& p){return p.y()>32.8&&p.y()<64&&p.z()>3.5;}});
    regions.push_back({"roller_key_root",[](const Eigen::Vector3d& p){return p.y()>94.1&&p.y()<112.85&&p.z()<-2.85;}});
    regions.push_back({"blind_bore_and_cosmetic_thread",[](const Eigen::Vector3d& p){return p.y()>93.5&&std::hypot(p.x(),p.z())<2.9;}});
    return regions;
}
double vonMises(const StressTensor6& stress) {
    if(!stress.allFinite())throw std::invalid_argument("Non-finite stress tensor");
    return std::sqrt(0.5*(std::pow(stress[0]-stress[1],2)+std::pow(stress[1]-stress[2],2)+std::pow(stress[2]-stress[0],2))+
                     3*(stress[3]*stress[3]+stress[4]*stress[4]+stress[5]*stress[5]));
}
double angularNormRemainder(double c,double s,std::size_t phases) {
    if(!std::isfinite(c)||!std::isfinite(s)||c<0||s<0||phases<4)throw std::invalid_argument("Invalid angular norm bound");
    return std::hypot(c,s)*2*std::sin(std::numbers::pi/(2*static_cast<double>(phases)));
}
}