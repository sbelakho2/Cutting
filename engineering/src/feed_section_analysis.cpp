#include "feed_geometry.hpp"
#include "feed_models.hpp"
#include "shaft_sections.hpp"
#include "shared/feed_drive_spec.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

int main(int argc,char** argv) try {
    namespace fs=std::filesystem;namespace se=starz::engineering;
    if(argc!=3||std::string(argv[1])!="--report")throw std::invalid_argument("Use --report output_directory");
    const fs::path out=argv[2];fs::create_directories(out);
    {std::ofstream pending(out/"status.json");pending<<"{\"run_complete\":false,\"fabrication_released\":false}\n";
     pending.close();if(!pending)throw std::runtime_error("Cannot initialize section report");}
    const auto shaft=se::feedShaftShape();const auto sections=se::selectedFeedShaftSections();
    std::vector<double> stations;
    // Interior checks on both sides of each feature and one-mm regular spacing.
    // Sampling is explicitly recorded; it is not continuous interval proof.
    for(const auto& s:sections) {
        stations.push_back(s.from_mm+0.001);stations.push_back((s.from_mm+s.to_mm)/2);stations.push_back(s.to_mm-0.001);
    }
    for(int s=1;s<113;++s)stations.push_back(s+0.017);
    for(double s:{31.22,31.48,33.43,33.47,34.4336,34.577,34.688,35.97,59.03,60.312,60.423,60.5664,61.53,61.57,
                  64.6,64.99,65.01,71.49,71.51,72.49,72.51,72.79,72.99,73.01,81.99,82.01,87.01,92.01,93.738,94.47,112.57,112.90})stations.push_back(s);
    std::sort(stations.begin(),stations.end());
    stations.erase(std::unique(stations.begin(),stations.end(),[](double a,double b){return std::abs(a-b)<1e-8;}),stations.end());
    std::ofstream csv(out/"sections.csv");csv<<std::setprecision(12)
        <<"S_mm,area_mm2,centroid_X_mm,centroid_Z_mm,Ixx_mm4,Izz_mm4,Ixz_mm4,Imin_mm4,Imax_mm4,beam_I_mm4,stiffness_ratio,integration_estimate,result\n";
    double minimum=std::numeric_limits<double>::infinity(),governing=0;int failures=0;
    for(double station:stations) {
        const auto result=se::shaftSection(shaft,station);
        const auto it=std::find_if(sections.begin(),sections.end(),[&](const auto& s){return station>=s.from_mm&&station<=s.to_mm;});
        if(it==sections.end())throw std::logic_error("Missing reduced shaft segment");
        const double I=std::numbers::pi*(std::pow(it->diameter_mm,4)-std::pow(it->hole_mm,4))/64;
        const double ratio=result.minimum_I_mm4/I;
        const bool pass=ratio>=1-1e-7;
        if(!pass)++failures;
        if(ratio<minimum){minimum=ratio;governing=station;}
        csv<<station<<','<<result.area_mm2<<','<<result.centroid_x_mm<<','<<result.centroid_z_mm<<','
           <<result.Ixx_mm4<<','<<result.Izz_mm4<<','<<result.Ixz_mm4<<','<<result.minimum_I_mm4<<','<<result.maximum_I_mm4<<','
           <<I<<','<<ratio<<','<<result.integration_error_estimate<<','<<(pass?"PASS_SAMPLED":"FAIL")<<'\n';
    }
    std::ofstream report(out/"report.md");report<<std::setprecision(10)
        <<"# Feed shaft machined-section audit\n\nRevision **"<<starz::feed::revision<<"**.\n\n"
        <<"B-rep plane intersections and adaptive surface integration at **"<<stations.size()<<"** axial stations, including both sides of reduced-section boundaries and the washer/key blends. "
        <<"Each section includes the actual nominal keyways, slot intersections, radii, chamfers and blind drill. Threads retain their documented cosmetic-major envelopes.\n\n"
        <<"The area centroid and full centroidal X/Z inertia tensor are calculated. Its **smaller eigenvalue** is compared with the beam surrogate's circular-section I, so a favourable shaft rotational angle is not assumed. "
        <<"Minimum nominal stiffness ratio **"<<minimum<<"**, at S**"<<governing<<"**; **"<<failures<<"** sampled failures. A1e−7 numerical relative comparison tolerance covers integration round-off only.\n\n"
        <<"R6 extends the reduced pulley-key interval to S33.30–61.70 around the floor-lip blends and includes the two0.20 end chamfers in the end-section diameters. "
        <<"The end bore and collar-side keyway remain conservatively reduced.\n\n"
        <<"## Scope\n\nThis is a **sampled nominal bending-stiffness audit**, not a continuous tolerance proof. "
        <<"It does not validate local peak stress, fatigue, Saint-Venant torsion of an open keyway, true helical thread roots, bearing/housing compliance, preload or physical accuracy. "
        <<"A minimum principal I does not alone bound nominal bending stress about a shifted centroid. Existing3× stress sensitivity is still a screen, not an established notch factor. "
        <<"No force, material or release gate is relaxed by this report.\n";
    csv.close();report.close();if(!csv||!report)throw std::runtime_error("Section report output failed");
    std::ofstream status(out/"status.json");status<<std::setprecision(12)
        <<"{\"revision\":\""<<starz::feed::revision<<"\",\"run_complete\":true,\"sampled_sections\":"<<stations.size()
        <<",\"failed_sections\":"<<failures<<",\"minimum_stiffness_ratio\":"<<minimum
        <<",\"continuous_bound_proved\":false,\"true_thread_geometry\":false,\"local_stress_verified\":false,\"fabrication_released\":false}\n";
    status.close();if(!status)throw std::runtime_error("Section status output failed");
    std::cout<<"Machined section audit: "<<stations.size()<<" stations, "<<failures<<" failures, minimum stiffness ratio "<<minimum<<'\n';
    return failures?2:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}