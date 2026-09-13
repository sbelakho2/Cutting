#include "bearing_fit.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

int main(int argc,char** argv)try {
    namespace fs=std::filesystem;namespace se=starz::engineering;
    if(argc!=3||std::string(argv[1])!="--report")throw std::invalid_argument("Use --report directory");
    const fs::path out=argv[2];fs::create_directories(out);
    {std::ofstream f(out/"status.json");f<<"{\"run_complete\":false,\"thermal_field_verified\":false}\n";if(!f)throw std::runtime_error("Fit pending output failed");}
    std::ofstream corners(out/"corners.csv"),matches(out/"matching-limits.csv"),report(out/"report.md");
    corners<<std::setprecision(12)<<"bearing,initial_clearance_um,interference_um,housing_clearance_um,shaft_inner_K,inner_outer_K,outer_housing_K,housing_C,hot_interference_um,hot_housing_clearance_um,operating_clearance_um\n";
    matches<<std::setprecision(12)<<"nominal_mm,actual_bearing_bore_mm,shaft_lower_mm,shaft_upper_mm\n";
    double min_clear=1e100,min_housing=1e100;int cases=0;
    for(int bearing=0;bearing<2;++bearing) {
        const double d=bearing?15:12,D=bearing?32:28;
        for(int i=0;i<=70;++i) {
            const double bore=d-i*0.0001;const auto m=se::matchedJ5Seat(d,bore);
            matches<<d<<','<<bore<<','<<m.lower_mm<<','<<m.upper_mm<<'\n';
        }
        for(double c:{11.0,25.0})for(double fit:{3.0,6.0})for(double h:{8.0,28.0})
        for(double si:{0.0,5.0})for(double io:{0.0,10.0})for(double oh:{0.0,5.0})for(double t:{10.0,55.0}) {
            const auto r=se::bearingFit({d,D,c,fit,h,si,io,oh,t});
            min_clear=std::min(min_clear,r.operating_clearance_um);min_housing=std::min(min_housing,r.hot_housing_clearance_um);++cases;
            corners<<(bearing?"6002-2Z/C3":"6001-2Z/C3")<<','<<c<<','<<fit<<','<<h<<','<<si<<','<<io<<','<<oh<<','<<t<<','
                   <<r.hot_shaft_interference_um<<','<<r.hot_housing_clearance_um<<','<<r.operating_clearance_um<<'\n';
        }
    }
    const bool pass=min_clear>=1&&min_housing>=2;
    report<<std::setprecision(9)<<"# Selected feed bearing fit envelope\n\n"
          <<"SKF6001-2Z/C3and6002-2Z/C3,unmountedC3=11–25µm. Use **matched j5bearing seats**,not unselectedh6. "
          <<"For10<d≤18mm,j5is−3/+5µm;each actualbearing bore ismatched to **3–6µm diametral interference** withinthatzone. "
          <<"The matchingtable lists actual-size limits,not gaugeuncertainty. F102bore28.008–28.018mm;F112bore32.008–32.019mm. "
          <<"No retentionadhesive oradditionalbearingpreload is specified.\n\n"
          <<"Enumerated **"<<cases<<"independent corners**. Minimum residualradialclearance **"<<min_clear<<"µm**;minimum outerring/housingdiametralclearance **"<<min_housing<<"µm**. "
          <<"The maximumcatalogue/C3andj5ranges cannot simplybe combined:anunselected12µminterferencecase fails andis testedasaregression.\n\n"
          <<"## Thermal envelope used\n\nHousing10–55°C;outerring0–5Kabovehousing;innerring0–10Kaboveouter;shaft0–5Kaboveinner. "
          <<"These areexplicitdesign/acceptancelimits,NOTmeasuredtemperatures or a completedheat-transferprediction. "
          <<"The thermalnetwork andinstrumentedcold-start/hot-run testmust demonstrate these limitsbeforethefit isreleased. "
          <<"Steel12µm/m/K,6082aluminium23.4µm/m/K;ringthermalreduction0.012ΔT(d+D)/2inµm. "
          <<"Fitreductionfactor1.0 isusedasaconservativebound;no favourablethick-steelhousingfactor isappliedtoaluminium.\n\n"
          <<"## Inspection and assembly\n\nFinish matchedjournalswithcertifiedcylindricalfinishing;Ra≤0.4µm. Markeachbearing/shaft/cartridgepair. "
          <<"Gaugeuncertaintymustbeincluded:combineddiameteruncertainty≤0.5µm requiresmeasuredinterference3.5–5.5µmandguardedj5limits. "
          <<"Measureat20°C;pressonlytherelevantring. Rearouterringremainsaxiallyfree;do notturnthe6001innerringendfloat intoaxialbearingpreload. "
          <<"Matchreplacementbearings beforeinstallation. Precisionseatrunout/rootgeometryandhousingstiffness remainseparatechecks.\n";
    std::ofstream status(out/"status.json");
    status<<"{\"run_complete\":true,\"corners\":"<<cases<<",\"conditional_fit_envelope_passed\":"<<(pass?"true":"false")
          <<",\"thermal_field_verified\":false,\"physical_validation_complete\":false,\"fabrication_released\":false}\n";
    if(!corners||!matches||!report||!status)throw std::runtime_error("Bearing-fit output failed");
    std::cout<<cases<<" bearing-fit corners;minimum operating clearance "<<min_clear<<"um;thermal-field validation still required\n";
    return pass?0:2;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}