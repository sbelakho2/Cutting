#include "feed_models.hpp"
#include "shared/feed_drive_spec.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace f=starz::feed;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void close(double actual,double expected,double relative=1e-6) {
    require(std::abs(actual-expected)<=relative*std::max(1.0,std::abs(expected)),"Numeric mismatch");
}
template<class F>void rejects(F action){bool caught=false;try{action();}catch(const std::exception&){caught=true;}require(caught,"Invalid input accepted");}
int main() try {
    constexpr double pi=std::numbers::pi;
    close(se::equalPulleyCentres(225,20,5),62.5);
    close(se::equalPulleyCentres(224.59,20,5),62.295);
    close(se::equalPulleyCentres(225.41,20,5),62.705);
    close(se::s5mPowerCapacity(200),238.56);
    close(se::s5mPowerCapacity(260),296.496);
    require(se::s5mTorqueCapacity(260)>2*f::design::mechanical_fault_torque_nm,"S5M fault torque margin");
    const double rpm=0.5*60/(pi*0.04);
    const double old_power=se::interpolateTable(f::catalogue::rejected_rpm,f::catalogue::rejected_power_w,rpm)*1.79;
    require(old_power/(rpm*2*pi/60)<4.0,"Undersized S3M regression not rejected");
    const auto tension=se::beltTension(190,62.5,225,100/(2*pi),5,151);
    close(tension.deflection_mm,1);close(tension.deflection_force_n,14.4965277777778);
    close(tension.tight_n-tension.slack_n,100*pi);close(tension.radial_n,380);
    require(se::beltTension(180,62.5,225,100/(2*pi),5,151).slack_n>22,"Slack strand under fault torque");
    const auto key=se::keyStress(5,10,3,2.0,15);
    close(key.shear_mpa,1000.0/45);close(key.bearing_mpa,1000.0/30);

    // Independent closed-form beam benchmark: central point load on simple supports.
    const se::ShaftSegment section{0,100,10,0};const se::BeamLoad centre{50,-100};
    const auto beam=se::shaftBeam(std::span(&section,1),200000,0,100,std::span(&centre,1));
    close(beam.reaction_a_n,50);close(beam.reaction_b_n,50);
    const double I=pi*std::pow(10,4)/64;
    close(beam.max_displacement_mm,100*std::pow(100,3)/(48*200000*I),1e-8);
    close(beam.max_bending_mpa,2500*5/I);
    require(beam.force_residual_n<1e-7&&beam.moment_residual_nmm<1e-5,"Beam balance");

    // The same uniform beam split around a short machining-feature interval.
    // Redundant requested subdivisions must not change the exact solution.
    const std::array<se::ShaftSegment,3> split{{{0,49.8,10,0},{49.8,50.2,10,0},{50.2,100,10,0}}};
    for(int divisions:{2,5,10}) {
        const auto small=se::shaftBeam(split,200000,0,100,std::span(&centre,1),divisions);
        close(small.max_displacement_mm,beam.max_displacement_mm,1e-8);
        close(small.reaction_a_n,50);close(small.reaction_b_n,50);
        require(small.force_residual_n<1e-5&&small.moment_residual_nmm<1e-3,"Short-feature beam balance");
    }

    // Overhang benchmark: right end P beyond simple supports separated by L.
    const se::ShaftSegment overhang{0,150,10,0};const se::BeamLoad end{150,-100};
    const auto cantilever=se::shaftBeam(std::span(&overhang,1),200000,0,100,std::span(&end,1));
    close(cantilever.reaction_a_n,-50);close(cantilever.reaction_b_n,150);
    close(std::abs(cantilever.displacements_mm.back()),100*50*50*150/(3*200000*I),1e-8);
    const auto sections=se::selectedFeedShaftSections();
    for(double belt:{-480.0,480.0})for(double roller:{-60.0,60.0}) {
        const std::array<se::BeamLoad,2> forces{{{f::design::pulley_station,belt},{f::design::wire_contact_station,roller}}};
        const auto coarse=se::shaftBeam(sections,205000,26.5,77.5,forces,2);
        const auto fine=se::shaftBeam(sections,205000,26.5,77.5,forces,5);
        close(coarse.reaction_a_n,fine.reaction_a_n);close(coarse.reaction_b_n,fine.reaction_b_n);
        close(coarse.displacements_mm.back(),fine.displacements_mm.back(),1e-7);
        close(coarse.max_displacement_mm,fine.max_displacement_mm,1e-7);
        std::cout<<"Shaft signs "<<belt<<'/'<<roller<<": max deflection "<<fine.max_displacement_mm
             <<" mm; nominal bending "<<fine.max_bending_mpa<<" MPa\n";
        require(fine.max_displacement_mm<0.05,"Feed shaft displacement screen");
        require(3*fine.max_bending_mpa<650/3.0,"Feed shaft bending screen");
    }
    rejects([]{se::s5mPowerCapacity(0);});rejects([]{se::s5mPowerCapacity(49.9);});
    rejects([]{se::s5mPowerCapacity(801);});rejects([]{se::s5mPowerCapacity(std::numeric_limits<double>::quiet_NaN());});
    rejects([]{se::equalPulleyCentres(100,20,5);});rejects([]{se::equalPulleyCentres(225,20.5,5);});
    rejects([]{se::keyStress(5,10,0,1,15);});
    rejects([&]{se::shaftBeam(std::span(&section,1),200000,0,0,std::span(&centre,1));});
    const se::BeamLoad outside{101,10};
    rejects([&]{se::shaftBeam(std::span(&section,1),200000,0,100,std::span(&outside,1));});
    std::cout<<"Feed belt tables, fault tension, positive keys, beam benchmarks and rejected inputs passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}