#include "bearing_fit.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>

namespace se=starz::engineering;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void near(double a,double b){require(std::abs(a-b)<1e-8,"Bearing-fit value mismatch");}
int main()try {
    const auto f=se::bearingFit({15,32,11,6,8,5,10,5,10});
    near(f.hot_shaft_interference_um,6.9);near(f.hot_housing_clearance_um,2.432);near(f.operating_clearance_um,1.28);
    const auto r=se::bearingFit({12,28,11,6,8,5,10,5,10});
    near(r.operating_clearance_um,1.88);
    // Full unselected j5 at+5µm andP6bore−7µm would give12µm
    // interference;C3label alone does not prevent thermal preload.
    require(se::bearingFit({15,32,11,12,8,5,10,5,10}).operating_clearance_um<0,"Unselected tight-fit regression");
    for(double nominal:{12.0,15.0})for(int i=0;i<=70;++i) {
        const double bore=nominal-i*0.0001;const auto m=se::matchedJ5Seat(nominal,bore);
        require(m.upper_mm-m.lower_mm>=0.002-1e-10,"Insufficient matched-journal manufacturing window");
        require(m.lower_mm-bore>=0.003-1e-10&&m.upper_mm-bore<=0.006+1e-10,"Match interference limits");
    }
    bool rejected=false;try{se::matchedJ5Seat(12,11.990);}catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"Out-of-spec bearing bore accepted");
    rejected=false;try{se::bearingFit({15,32,11,6,8,5,10,5,std::numeric_limits<double>::quiet_NaN()});}
    catch(const std::invalid_argument&){rejected=true;}require(rejected,"NaN accepted");
    std::cout<<"Matched j5/C3 limits,thermal-clearance bounds and rejected full-range fit passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}