#include "bearing_contact_thermal.hpp"
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
bool near(double a,double b,double tol){return std::abs(a-b)<=tol;}

int main()try {
    // --- Lamé shrink-fit arithmetic (study 1 analytic basis) ---
    const double p6=se::laméPressure(15.0,19.74,3.0,205e9,0.29,210e9,0.30);
    const double p3=se::laméPressure(15.0,19.74,1.5,205e9,0.29,210e9,0.30);
    require(near(p6,17.65e6,0.15e6),"Lamé pressure at 6 um diametral interference outside analytic band");
    require(near(p3,p6/2,1e-6),"Lamé pressure is not linear in interference");
    const double hoop=se::laméSleeveHoop(15.0,19.74,p6);
    require(near(hoop,17.65e6*3.7328,0.6e6),"Lamé ring hoop stress outside analytic band");
    require(p3>0&&p6>p3,"Lamé ordering wrong");
    bool rejected=false;
    try{ se::laméPressure(0,20,3,205e9,0.29,210e9,0.30); }catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"Invalid Lamé geometry accepted");
    rejected=false;
    try{ se::laméPressure(15,19.74,-1,205e9,0.29,210e9,0.30); }catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"Negative Lamé interference accepted");

    // --- Thermal solver benchmarks on a meshed cylinder (axis +Y) ---
    // 1. Pure axial conduction: ends pinned 0 / 100 degC, sides insulated.
    {
        const se::TetMesh mesh=se::meshCylinder(10.0,20.0,1.2);
        std::vector<se::ThermalBoundary> bc;
        bc.push_back({se::ThermalBoundary::Kind::prescribed,
            [&](const Eigen::Vector3d& c){ return c.y()<0.6&&std::hypot(c.x(),c.z())<9.9; }, 0.0, 0.0});
        bc.push_back({se::ThermalBoundary::Kind::prescribed,
            [&](const Eigen::Vector3d& c){ return c.y()>19.4&&std::hypot(c.x(),c.z())<9.9; }, 100.0, 0.0});
        // A non-negligible mid-band flux must travel axially to the pinned
        // caps: this catches any operator that fails to conduct along Y.
        bc.push_back({se::ThermalBoundary::Kind::flux,
            [&](const Eigen::Vector3d& c){
                return std::hypot(c.x(),c.z())>9.9&&std::abs(c.y()-10.0)<0.6; },
            0.5/(2*std::numbers::pi*10.0*1.2*1e-6), 25.0});
        const auto r=se::solveThermal(mesh,42.7,bc);
        require(r.nodes>0&&r.nodes<=150000,"Conduction benchmark node budget");
        double mid=0; int n=0; double tmax=-1e9,tmin=1e9;
        for(std::size_t i=0;i<mesh.nodes_mm.size();++i) {
            tmax=std::max(tmax,r.temperature_c[i]); tmin=std::min(tmin,r.temperature_c[i]);
            if(std::abs(mesh.nodes_mm[i].y()-10.0)<0.5) { mid+=r.temperature_c[i]; ++n; }
        }
        require(n>0&&near(mid/n,50.0,0.6),"Linear conduction profile not recovered at the midplane");
        require(tmin>=0&&tmax<=100.5,"Conduction benchmark temperatures outside the pinned band");
        require(r.balance_relative<1e-6,"Conduction benchmark power balance failed");
    }
    // 2. Uniform convection everywhere with an end-face flux: conservation.
    {
        const se::TetMesh mesh=se::meshCylinder(10.0,20.0,1.2);
        const double q=12.5;
        std::vector<se::ThermalBoundary> bc;
        bc.push_back({se::ThermalBoundary::Kind::flux,
            [&](const Eigen::Vector3d& c){ return c.y()>19.4&&std::hypot(c.x(),c.z())<9.9; },
            q/(std::numbers::pi*100.0*1e-6), 25.0});
        bc.push_back({se::ThermalBoundary::Kind::convection,
            [&](const Eigen::Vector3d& c){ return c.y()<=19.4||std::hypot(c.x(),c.z())>=9.9; }, 10.0, 25.0});
        const auto r=se::solveThermal(mesh,42.7,bc);
        require(r.balance_relative<1e-6,"Convection benchmark power balance failed");
        require(near(r.power_in_w,q,0.05),"Convection benchmark source power mismatch");
        double tmax=25,tmin=1e9;
        for(double t:r.temperature_c){ tmax=std::max(tmax,t); tmin=std::min(tmin,t); }
        require(tmin>=25-1e-6,"Convection benchmark temperature below the sink temperature");
        require(tmax>500&&tmax<1200,"Convection benchmark temperature outside the analytic band");
    }
    // 3. Invalid requests are rejected fail-closed.
    rejected=false;
    try{
        const se::TetMesh mesh=se::meshCylinder(10.0,20.0,1.2);
        std::vector<se::ThermalBoundary> bc;
        bc.push_back({se::ThermalBoundary::Kind::prescribed,
            [&](const Eigen::Vector3d&){ return true; }, 25.0, 0.0});
        se::solveThermal(mesh,42.7,bc); // every node pinned -> no free system
    }catch(const std::runtime_error&){rejected=true;}
    require(rejected,"Fully prescribed thermal model accepted");

    // --- Envelope and input consistency (study 2 contract) ---
    require(near(se::ThermalStudyOutput{}.envelope_shaft_above_housing_k,20.0,1e-12),
        "Temperature envelope is not the fit record's 5+10+5 = 20 K");
    const se::ThermalInput in;
    require(near(in.source_w,30.0,1e-12)&&near(in.convection_h_w_m2k,10.0,1e-12)
        &&near(in.conductivity_w_mk,42.7,1e-12)&&near(in.ambient_c,25.0,1e-12),
        "Thermal input defaults drifted from the stated values");

    // --- Study 1 contract values ---
    const se::BearingContactGeometry g;
    require(near(g.shaft_interference_diametral_um,6.0,1e-12),"Shaft interference default drifted");
    require(near(g.housing_bore_diameter_mm,32.3,1e-12),"F112 bore meshed stand-in default drifted");
    require(near(g.housing_clearance_diametral_um,8.0,1e-12),"Record housing clearance default drifted");

    std::cout<<"Bearing-contact-thermal: Lamé arithmetic, thermal conduction/conservation benchmarks, fail-closed rejections and stated inputs passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
catch(...){std::cerr<<"Bearing-contact-thermal test: non-standard failure\n";return 1;}
