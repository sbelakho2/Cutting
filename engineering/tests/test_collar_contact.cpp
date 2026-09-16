#include "collar_contact.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace se=starz::engineering;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
bool near(double a,double b,double tol){return std::abs(a-b)<=tol;}

int main()try {
    const se::CollarKeyGeometry din;
    const auto r=se::runCollarContactStudy(din,se::CollarStudyRun{0.9,0.45});
    require(r.solved,"DIN study did not solve");
    require(r.nodes>1000&&r.nodes<=150000,"DIN node budget outside range");
    require(r.tetrahedra>r.nodes,"DIN tetrahedron count implausible");
    require(near(r.key_force_n,1000,1e-6),"Key force is not 5 Nm / 5 mm");
    require(near(r.key_shear_average_mpa,16.6667,0.02),"Key shear average mismatch");
    require(near(r.seat_bearing_average_mpa,27.7778,0.02),"Seat bearing average mismatch");
    require(near(r.combined.applied_face_pressure_mpa,41.6667,4.0),"Applied face pressure outside mesh-resolution band");
    require(r.combined.peak_key_vm_mpa>5&&r.combined.peak_key_vm_mpa<300,"DIN key peak stress outside plausible band");
    require(r.combined.peak_key_shear_mpa>5&&r.combined.peak_key_shear_mpa<180,"DIN key peak shear outside plausible band");
    require(r.combined.peak_seat_wall_vm_mpa>5&&r.combined.peak_seat_wall_vm_mpa<650,"Seat-wall peak outside plausible band");
    require(r.combined.peak_shaft_vm_mpa>5&&r.combined.peak_shaft_vm_mpa<650,"Shaft peak outside plausible band");
    require(r.clamp_only.peak_key_vm_mpa<0.5*r.torque_only.peak_key_vm_mpa,"Clamp case must be a minor contributor");
    require(r.combined.max_displacement_mm<0.1,"DIN displacement implausible");
    require(r.verdict=="PASS","DIN verdict not PASS");
    se::CollarKeyGeometry f109=din; f109.key_height_mm=4.0; f109.key_length_mm=18.0;
    const auto f=se::runCollarContactStudy(f109,se::CollarStudyRun{0.9,0.45});
    require(f.solved&&f.verdict=="PASS","F109 study failed");
    require(near(f.key_shear_average_mpa,18.5185,0.02),"F109 shear average mismatch");
    require(near(f.combined.applied_face_pressure_mpa,25.2525,3.0),"F109 face pressure outside band");
    require(f.combined.applied_face_pressure_mpa<r.combined.applied_face_pressure_mpa,"Taller key must reduce contact pressure");
    bool rejected=false;
    try{ se::CollarKeyGeometry bad=din; bad.seat_depth_mm=6.0; se::runCollarContactStudy(bad,se::CollarStudyRun{1.2,0.6}); }
    catch(const std::runtime_error&){rejected=true;}
    require(rejected,"Over-deep seat geometry accepted");
    rejected=false;
    try{ se::runCollarContactStudy(din,se::CollarStudyRun{0.2,0.6}); }
    catch(const std::runtime_error&){rejected=true;}
    require(rejected,"Inverted mesh-size request accepted");
    std::cout<<"Collar contact study: analytic bearing/shear values, FEM peaks, clamp negligibility and rejections passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
catch(...){std::cerr<<"Collar contact test: non-standard failure\n";return 1;}
