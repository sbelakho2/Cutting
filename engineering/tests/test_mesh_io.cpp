#include "mesh_io.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs=std::filesystem;
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
void write(const fs::path& path,const std::string& text){std::ofstream f(path,std::ios::binary);f<<text;f.close();require(bool(f),"Fixture write failed");}
std::string replace(std::string input,const std::string& old,const std::string& value) {
    const auto at=input.find(old);require(at!=std::string::npos,"Fixture token missing");input.replace(at,old.size(),value);return input;
}
}
int main(int argc,char** argv)try {
    require(argc==2,"VTU reader test requires an output directory");const fs::path out=argv[1];fs::create_directories(out);const auto file=out/"fixture.vtu";
    const std::string valid=R"(<VTKFile type="UnstructuredGrid"><UnstructuredGrid><Piece NumberOfPoints="4" NumberOfCells="1">
<Points><DataArray type="Float64" NumberOfComponents="3" format="ascii">0 0 0 1 0 0 0 1 0 0 0 1</DataArray></Points>
<Cells><DataArray type="Int32" Name="connectivity" format="ascii">0 1 2 3</DataArray>
<DataArray type="Int32" Name="offsets" format="ascii">4</DataArray><DataArray type="UInt8" Name="types" format="ascii">10</DataArray></Cells>
</Piece></UnstructuredGrid></VTKFile>)";
    write(file,valid);const auto mesh=se::readLinearVtuGeometry(file);
    require(mesh.nodes_mm.size()==4&&mesh.tetrahedra.size()==1&&mesh.tetrahedra.front()==std::array<int,4>{0,1,2,3},"VTU geometry changed");
    require(mesh.nodes_mm[0].isZero()&&mesh.nodes_mm[1]==Eigen::Vector3d{1,0,0}&&mesh.nodes_mm[2]==Eigen::Vector3d{0,1,0}&&mesh.nodes_mm[3]==Eigen::Vector3d{0,0,1},"VTU coordinates changed");
    require(!mesh.geometry_audit,"Imported geometry must not invent CAD audit metadata");
    int rejected=0;const auto reject=[&](const std::string& text){write(file,text);bool threw=false;try{(void)se::readLinearVtuGeometry(file);}catch(const std::runtime_error&){threw=true;}require(threw,"Invalid VTU was accepted");++rejected;};
    for(const auto& [old,value]:std::vector<std::pair<std::string,std::string>>{
        {"NumberOfPoints=\"4\"","NumberOfPoints=\"150001\""},{"NumberOfPoints=\"4\"","NumberOfPoints=\"-4\""},
        {"NumberOfCells=\"1\"","NumberOfCells=\"0\""},{"NumberOfCells=\"1\"","NumberOfCells=\"2000001\""},
        {">0 1 2 3<",">0 1 2 4<"},{">0 1 2 3<",">0 1 2 -1<"},{">0 1 2 3<",">0 1 2 2<"},
        {">0 1 2 3<",">0 1 2.5 3<"},{">0 1 2 3<",">0 1 2<"},{">0 1 2 3<",">0 1 2 3 0<"},
        {">4<",">3<"},{">10<",">24<"},{"NumberOfComponents=\"3\"","NumberOfComponents=\"2\""},
        {"format=\"ascii\"","format=\"binary\""},{"type=\"Float64\"","type=\"Float32\""},
        {">0 0 0 1",">nan 0 0 1"},{">0 0 0 1",">1e999 0 0 1"},{">0 0 0 1",">0junk 0 0 1"},
        {"Name=\"offsets\"","Name=\"connectivity\""},{"</Piece>","<Points/></Piece>"},
        {"</UnstructuredGrid>","<Piece/></UnstructuredGrid>"},{"</VTKFile>",""}})reject(replace(valid,old,value));
    reject("<!DOCTYPE VTKFile [<!ENTITY bad SYSTEM 'file:///not-read'>]>"+valid);reject(std::string(1,'\0')+valid);
    se::ElasticResult result;result.displacement_m.resize(4,Eigen::Vector3d::Zero());result.von_mises_pa.resize(1,0);
    se::writeVtu(file,mesh,result);const auto roundtrip=se::readLinearVtuGeometry(file);
    require(roundtrip.tetrahedra==mesh.tetrahedra&&roundtrip.nodes_mm==mesh.nodes_mm,"Production VTU writer/readback mismatch");
    std::cout<<"ASCII linear VTU geometry, production writer/readback and "<<rejected<<" fail-closed cases passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}