// Evaluates physical qualification records of an assembled S1-B machine.
// Usage:
//   starz_qualification --template <dir>
//   starz_qualification --lengths <csv> --forces <csv> --out <dir>
// Blank templates are rejected (exit 1); a pass requires complete coverage,
// in-range gauges and Cpk >= 1.33 in every required cell. R2 adds duplicate-
// sample rejection, per-record provenance, per-station force records and a
// declared-coverage statement with its physical remainder.
#include "qualification.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

namespace q=starz::engineering::qualification;
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
std::string blanks(int n){return std::string(std::size_t(n),',');}
const std::string lengthHeader="sample_id,wire_od_mm,commanded_mm,measured_mm,gauge_id,gauge_u_mm,temperature_c,machine_id,build_id,recipe_id,calibration_id,operator_slot";
const std::string forceHeader="station,wire_od_mm,force_n,gauge_id,gauge_u_n,temperature_c,machine_id,build_id,recipe_id,calibration_id,operator_slot";
}
int main(int argc,char** argv)try {
    fs::path tmpl,lengths,forces,out;
    for(int i=1;i<argc;++i) {
        const std::string a=argv[i];
        if(a=="--template"&&i+1<argc)tmpl=argv[++i];
        else if(a=="--lengths"&&i+1<argc)lengths=argv[++i];
        else if(a=="--forces"&&i+1<argc)forces=argv[++i];
        else if(a=="--out"&&i+1<argc)out=argv[++i];
        else throw std::invalid_argument("Unknown qualification argument "+a);
    }
    if(!tmpl.empty()) {
        fs::create_directories(tmpl);
        std::ofstream l(tmpl/"length-records-template.csv"),f(tmpl/"force-records-template.csv");
        l<<lengthHeader<<'\n';
        for(double od:q::required_wire_od_mm)for(double L:q::required_length_mm)for(int i=1;i<=q::min_samples_per_cell;++i)
            l<<','<<od<<','<<L<<blanks(9)<<'\n';
        f<<forceHeader<<'\n';
        for(const char* st:q::required_force_stations)for(double od:q::required_wire_od_mm)f<<st<<','<<od<<blanks(9)<<'\n';
        if(!l||!f)throw std::runtime_error("Template write failed");
        std::cout<<"Blank qualification templates written\n";return 0;
    }
    if(lengths.empty()||forces.empty()||out.empty())throw std::invalid_argument("Need --lengths, --forces and --out");
    fs::create_directories(out);
    {std::ofstream pending(out/"status.json");pending<<"{\"run_complete\":false,\"physical_results\":false,\"qualification_passed\":false}\n";}
    std::vector<q::LengthRecord> lr;
    for(const auto& c:table(lengths,lengthHeader))
        lr.push_back({c[0],num(c[1],"wire_od_mm"),num(c[2],"commanded_mm"),num(c[3],"measured_mm"),c[4],num(c[5],"gauge_u_mm"),num(c[6],"temperature_c"),c[7],c[8],c[9],c[10],c[11]});
    std::vector<q::ForceRecord> fr;
    for(const auto& c:table(forces,forceHeader))
        fr.push_back({c[0],num(c[1],"wire_od_mm"),num(c[2],"force_n"),c[3],num(c[4],"gauge_u_n"),num(c[5],"temperature_c"),c[6],c[7],c[8],c[9],c[10]});
    const auto rep=q::evaluate(lr,fr);
    std::ofstream cells(out/"length-cells.csv");cells<<std::setprecision(6)<<std::fixed
        <<"wire_od_mm,commanded_mm,n,mean_error_mm,sd_mm,tolerance_mm,cpk,max_abs_error_mm,pass\n";
    for(const auto& c:rep.cells)cells<<c.wire_od_mm<<','<<c.commanded_mm<<','<<c.n<<','<<c.mean_error_mm<<','<<c.sd_mm<<','<<c.tolerance_mm<<','<<c.cpk<<','<<c.max_abs_error_mm<<','<<(c.pass?1:0)<<'\n';
    std::ofstream fo(out/"force-checks.csv");fo<<std::setprecision(3)<<std::fixed<<"station,wire_od_mm,force_n,lower_n,upper_n,pass\n";
    for(const auto& f:rep.forces)fo<<f.station<<','<<f.wire_od_mm<<','<<f.force_n<<','<<f.lower_n<<','<<f.upper_n<<','<<(f.pass?1:0)<<'\n';
    std::ofstream r(out/"report.md");
    r<<"# Physical qualification — "<<q::revision<<"\n\nResult: **"<<(rep.all_pass?"PASS":"FAIL")<<"** on the supplied records. Length acceptance ±("<<q::length_rel*100<<" % + "<<q::length_abs_mm<<" mm) guard-banded by each gauge's U(k=2); Cpk ≥ "<<q::min_cpk<<" with n ≥ "<<q::min_samples_per_cell<<" **distinct** pieces per cell.\n\n| Wire OD | Length | n | Mean error mm | SD mm | Cpk | Pass |\n|---|---|---|---|---|---|---|\n";
    for(const auto& c:rep.cells)r<<std::setprecision(3)<<std::fixed<<"| "<<c.wire_od_mm<<" | "<<c.commanded_mm<<" | "<<c.n<<" | "<<c.mean_error_mm<<" | "<<c.sd_mm<<" | "<<c.cpk<<" | "<<(c.pass?"yes":"no")<<" |\n";
    r<<"\n| Station | Wire OD | Force N | Window N | Pass |\n|---|---|---|---|---|\n";
    for(const auto& f:rep.forces)r<<"| "<<f.station<<" | "<<f.wire_od_mm<<" | "<<f.force_n<<" | "<<f.lower_n<<"–"<<f.upper_n<<" | "<<(f.pass?"yes":"no")<<" |\n";
    if(!rep.failures.empty()){r<<"\n## Failures\n\n";for(const auto& s:rep.failures)r<<"- "<<s<<'\n';}
    std::set<std::string> gauges,samples,machines,builds,recipes,calibrations,operators;bool synthetic=false;
    for(const auto& x:lr){gauges.insert(x.gauge_id);samples.insert(x.sample_id);machines.insert(x.machine_id);builds.insert(x.build_id);recipes.insert(x.recipe_id);calibrations.insert(x.calibration_id);operators.insert(x.operator_slot);synthetic|=x.gauge_id.rfind("SYN-",0)==0||x.sample_id.rfind("SYN-",0)==0;}
    for(const auto& x:fr){gauges.insert(x.gauge_id);machines.insert(x.machine_id);builds.insert(x.build_id);recipes.insert(x.recipe_id);calibrations.insert(x.calibration_id);operators.insert(x.operator_slot);synthetic|=x.gauge_id.rfind("SYN-",0)==0;}
    auto list=[&](const char* what,const std::set<std::string>& s){r<<" "<<what<<":";for(const auto& v:s)r<<" `"<<v<<"`";r<<".";};
    r<<"\n## Provenance\n\nInput files: `"<<lengths.filename().string()<<"`, `"<<forces.filename().string()<<"`; "<<lr.size()<<" length records, "<<samples.size()<<" distinct sample ids, "<<rep.duplicate_records<<" duplicate-sample records rejected (a reused id never counts as a distinct piece); "<<fr.size()<<" force records across the required stations:";
    for(const char* st:q::required_force_stations)r<<" `"<<st<<"`";
    r<<".\n";list("Gauges",gauges);list("Machines",machines);list("Builds",builds);list("Recipes",recipes);list("Calibrations",calibrations);list("Operator slots",operators);
    r<<"\n";
    if(synthetic)r<<"**SYN- identities present: these are synthetic records for arithmetic verification, not measurements of a machine.**\n";
    r<<"\n## Declared coverage\n\nDemonstrated by the supplied records: the mandatory length cells (OD 1.2/4.5 mm at 200/1000 mm commanded length) and every required nip/wheel force station. The declaration is wider than the record formats; the following scope remains **physical** (required before any wider claim):\n\n";
    for(const char* s:q::declared_physical_remainder)r<<"- "<<s<<"\n";
    std::ofstream status(out/"status.json");
    status<<"{\"revision\":\""<<q::revision<<"\",\"run_complete\":true,\"physical_results\":"<<(synthetic?"false":"true")<<",\"synthetic_identities\":"<<(synthetic?"true":"false")<<",\"length_records\":"<<lr.size()<<",\"force_records\":"<<fr.size()
          <<",\"duplicate_sample_records\":"<<rep.duplicate_records<<",\"coverage_complete\":"<<(rep.coverage_complete?"true":"false")<<",\"declared_physical_remainder\":"<<std::size(q::declared_physical_remainder)<<",\"qualification_passed\":"<<(rep.all_pass?"true":"false")<<"}\n";
    if(!cells||!fo||!r||!status)throw std::runtime_error("Qualification output failed");
    std::cout<<(rep.all_pass?"PASS":"FAIL")<<": "<<rep.cells.size()<<" length cells, "<<rep.forces.size()<<" force records, "<<rep.failures.size()<<" failures\n";
    return rep.all_pass?0:3;
}catch(const std::invalid_argument& e){std::cerr<<e.what()<<'\n';return 1;}
 catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
