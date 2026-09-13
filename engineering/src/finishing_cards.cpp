// Generates finishing cards (grind/bore targets) from measured incoming parts.
// Usage:
//   starz_finishing_cards --template <dir>
//   starz_finishing_cards --seats <csv> --bores <csv> --out <dir>
// Records with blank fields are rejected: the shipped templates cannot be run.
#include "finishing.hpp"
#include "shared/feed_drive_spec.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fin=starz::engineering::finishing;
namespace fs=std::filesystem;
namespace {
std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> out;std::string cell;std::stringstream ss(line);
    while(std::getline(ss,cell,','))out.push_back(cell);
    if(!line.empty()&&line.back()==',')out.emplace_back();
    return out;
}
double num(const std::string& s,const char* what) {
    if(s.empty())throw std::invalid_argument(std::string("Blank field: ")+what);
    std::size_t n=0;const double v=std::stod(s,&n);
    if(n!=s.size()||!std::isfinite(v))throw std::invalid_argument(std::string("Bad numeric field: ")+what);
    return v;
}
std::vector<std::vector<std::string>> table(const fs::path& p,const std::string& header) {
    std::ifstream in(p);if(!in)throw std::runtime_error("Cannot open "+p.string());
    std::string line;if(!std::getline(in,line)||line!=header)throw std::runtime_error("Header mismatch in "+p.string());
    std::vector<std::vector<std::string>> rows;const auto n=split(header).size();
    while(std::getline(in,line)){if(line.empty())continue;auto c=split(line);if(c.size()!=n)throw std::runtime_error("Column count in "+p.string());rows.push_back(std::move(c));}
    if(rows.empty())throw std::runtime_error("No records in "+p.string());
    return rows;
}
const std::string seatHeader="shaft_serial,seat,nominal_mm,bearing_serial,bearing_bore_mm,bearing_temperature_c,gauge_id,gauge_u_um";
const std::string boreHeader="part_serial,part,lower_mm,upper_mm,gauge_id,gauge_u_um";
}
int main(int argc,char** argv)try {
    fs::path tmpl,seats,bores,out;
    for(int i=1;i<argc;++i) {
        const std::string a=argv[i];
        if(a=="--template"&&i+1<argc)tmpl=argv[++i];
        else if(a=="--seats"&&i+1<argc)seats=argv[++i];
        else if(a=="--bores"&&i+1<argc)bores=argv[++i];
        else if(a=="--out"&&i+1<argc)out=argv[++i];
        else throw std::invalid_argument("Unknown finishing argument "+a);
    }
    if(!tmpl.empty()) {
        fs::create_directories(tmpl);
        std::ofstream s(tmpl/"seat-records-template.csv"),b(tmpl/"bore-records-template.csv");
        s<<seatHeader<<"\n,S22.5-30.5_d12,12,,,,,\n,S73-82_d15,15,,,,,\n";
        b<<boreHeader<<"\n,F102,"<<std::setprecision(6)<<starz::feed::design::rear_housing_bore-0.005<<','<<starz::feed::design::rear_housing_bore+0.005<<",,\n"
         <<",F112,"<<starz::feed::design::front_housing_bore-0.0055<<','<<starz::feed::design::front_housing_bore+0.0055<<",,\n";
        if(!s||!b)throw std::runtime_error("Template write failed");
        std::cout<<"Blank finishing templates written\n";return 0;
    }
    if(seats.empty()||bores.empty()||out.empty())throw std::invalid_argument("Need --seats, --bores and --out");
    fs::create_directories(out);
    {std::ofstream pending(out/"status.json");pending<<"{\"run_complete\":false}\n";}
    std::vector<fin::GrindCard> cards;
    for(const auto& c:table(seats,seatHeader))
        cards.push_back(fin::grindCard({c[0],c[1],num(c[2],"nominal_mm"),c[3],num(c[4],"bearing_bore_mm"),
                                        num(c[5],"bearing_temperature_c"),c[6],num(c[7],"gauge_u_um")}));
    std::vector<fin::BoreCard> bcards;
    for(const auto& c:table(bores,boreHeader))
        bcards.push_back(fin::boreCard({c[0],c[1],num(c[2],"lower_mm"),num(c[3],"upper_mm"),c[4],num(c[5],"gauge_u_um")}));
    std::ofstream g(out/"grind-cards.csv");g<<std::setprecision(7)<<std::fixed
     <<"shaft_serial,seat,bearing_serial,bearing_bore_20c_mm,pre_grind_lower_mm,pre_grind_upper_mm,target_mm,accept_lower_mm,accept_upper_mm,guarded_lower_mm,guarded_upper_mm,rough_passes_0.020,finish_passes_0.005,final_passes_0.002,spark_out,interference_lower_um,interference_upper_um\n";
    for(const auto& c:cards)g<<c.in.shaft_serial<<','<<c.in.seat<<','<<c.in.bearing_serial<<','<<c.bearing_bore_20c_mm<<','<<c.pre_grind_lower_mm<<','<<c.pre_grind_upper_mm<<','
        <<c.target_mm<<','<<c.accept_lower_mm<<','<<c.accept_upper_mm<<','<<c.guarded_lower_mm<<','<<c.guarded_upper_mm<<','<<c.rough_passes<<','<<c.finish_passes<<','<<c.final_passes<<','
        <<fin::spark_out_passes<<','<<std::setprecision(3)<<c.interference_lower_um<<','<<c.interference_upper_um<<std::setprecision(7)<<'\n';
    std::ofstream b(out/"bore-cards.csv");b<<std::setprecision(7)<<std::fixed
     <<"part_serial,part,drawing_lower_mm,drawing_upper_mm,semi_finish_target_mm,finish_target_mm,guarded_lower_mm,guarded_upper_mm\n";
    for(const auto& c:bcards)b<<c.in.part_serial<<','<<c.in.part<<','<<c.in.lower_mm<<','<<c.in.upper_mm<<','<<c.semi_finish_target_mm<<','<<c.target_mm<<','<<c.guarded_lower_mm<<','<<c.guarded_upper_mm<<'\n';
    std::ofstream r(out/"report.md");
    r<<"# Finishing cards — "<<fin::revision<<"\n\nGenerated from the supplied incoming-inspection records. Targets are at 20°C; guard bands equal each gauge's stated U(k=2).\n\n"
     <<"## Cylindrical grinding, F101 bearing seats\n\nWheel ISO525 A60K5V, between centres, coolant on. Rough "<<fin::rough_pass_diametral_mm<<" mm/pass to "<<fin::rough_stop_remaining_mm
     <<" mm remaining; finish "<<fin::finish_pass_diametral_mm<<" mm/pass to "<<fin::finish_stop_remaining_mm<<" mm; measure; final "<<fin::final_pass_diametral_mm<<" mm/pass plus "<<fin::spark_out_passes
     <<" spark-out passes. Acceptance per seat: cylindricity "<<fin::seat_cylindricity_um<<" µm, roundness "<<fin::seat_roundness_um<<" µm, Ra "<<fin::seat_ra_um<<" µm, runout to centres "<<fin::seat_runout_to_centres_um<<" µm.\n\n| Shaft | Seat | Bearing | Target mm | Guarded window mm |\n|---|---|---|---|---|\n";
    for(const auto& c:cards)r<<std::setprecision(4)<<std::fixed<<"| "<<c.in.shaft_serial<<" | "<<c.in.seat<<" | "<<c.in.bearing_serial<<" | "<<c.target_mm<<" | "<<c.guarded_lower_mm<<"–"<<c.guarded_upper_mm<<" |\n";
    r<<"\n## Fine boring, F102/F112\n\nSemi-finish "<<fin::bore_semi_finish_undersize_mm<<" mm under target, gauge, then one "<<fin::bore_finish_pass_diametral_mm
     <<" mm diametral finishing pass. Cylindricity "<<fin::bore_cylindricity_um<<" µm, Ra "<<fin::bore_ra_um<<" µm; assembled coaxiality F102/F112 "<<fin::bore_coaxiality_F102_F112_um<<" µm.\n\n| Part | Serial | Finish target mm | Guarded window mm |\n|---|---|---|---|\n";
    for(const auto& c:bcards)r<<"| "<<c.in.part<<" | "<<c.in.part_serial<<" | "<<c.target_mm<<" | "<<c.guarded_lower_mm<<"–"<<c.guarded_upper_mm<<" |\n";
    r<<"\nThese cards are process instructions. Finished sizes must be measured and accepted with `acceptFinishedSeat`/`acceptFinishedBore` logic before assembly.\n";
    std::ofstream status(out/"status.json");
    status<<"{\"revision\":\""<<fin::revision<<"\",\"run_complete\":true,\"grind_cards\":"<<cards.size()<<",\"bore_cards\":"<<bcards.size()<<",\"physical_results\":false}\n";
    if(!g||!b||!r||!status)throw std::runtime_error("Finishing output failed");
    std::cout<<cards.size()<<" grind cards, "<<bcards.size()<<" bore cards\n";return 0;
}catch(const std::invalid_argument& e){std::cerr<<e.what()<<'\n';return 1;}
 catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
