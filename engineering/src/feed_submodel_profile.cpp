#include "feed_fem.hpp"
#include "feed_submodel.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
double azimuthDeg(const Eigen::Vector3d& p) {
    double a=std::atan2(p.x(),p.z())*180/std::numbers::pi;
    if(a<0)a+=360;
    return a;
}
}
// Post-processor: azimuthal raw-peak profile of one region from the finest
// submodel VTU (the exported single governing sampled phase), 24 bins of 15
// degrees. Read-only; does not alter any submodel or four-mesh artifact.
int main(int argc,char** argv)try{
    fs::path vtu,out;std::string regionName;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--vtu"&&i+1<argc)vtu=argv[++i];
        else if(arg=="--region"&&i+1<argc)regionName=argv[++i];
        else if(arg=="--out"&&i+1<argc)out=argv[++i];
        else throw std::invalid_argument("Use --vtu PATH --region NAME --out CSV");
    }
    require(!vtu.empty()&&!regionName.empty()&&!out.empty(),"Missing profile argument");
    const se::VtuField field=se::readLinearVtuField(vtu);
    const auto regions=se::feedShaftStressRegions();
    const auto region=std::find_if(regions.begin(),regions.end(),[&](const auto& r){return r.name==regionName;});
    require(region!=regions.end(),"Unknown stress region");
    std::array<double,24> bins{};std::array<int,24> counts{};
    for(std::size_t e=0;e<field.mesh.tetrahedra.size();++e) {
        Eigen::Vector3d c=Eigen::Vector3d::Zero();
        for(int n:field.mesh.tetrahedra[e])c+=field.mesh.nodes_mm[static_cast<std::size_t>(n)]/4;
        if(!region->contains(c))continue;
        const int bin=std::clamp(static_cast<int>(std::floor(azimuthDeg(c)/15)),0,23);
        bins[static_cast<std::size_t>(bin)]=std::max(bins[static_cast<std::size_t>(bin)],field.von_mises_mpa[e]);
        ++counts[static_cast<std::size_t>(bin)];
    }
    std::ofstream table(out);table<<std::setprecision(12)<<"azimuth_centre_deg,peak_vm_MPa,elements\n";
    for(int bin=0;bin<24;++bin)if(counts[static_cast<std::size_t>(bin)]>0)
        table<<(bin*15+7.5)<<','<<bins[static_cast<std::size_t>(bin)]<<','<<counts[static_cast<std::size_t>(bin)]<<'\n';
    table.close();require(bool(table),"Profile write failed");
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
