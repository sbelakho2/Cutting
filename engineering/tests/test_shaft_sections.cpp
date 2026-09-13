#include "shaft_sections.hpp"
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

using starz::engineering::shaftSection;
void close(double a,double b) {
    if(std::abs(a-b)>1e-7*std::max(1.0,std::abs(b)))throw std::runtime_error("Section benchmark mismatch");
}
template<class F>void rejects(F action) {
    bool rejected=false;try{action();}catch(const std::exception&){rejected=true;}
    if(!rejected)throw std::runtime_error("Bad section input accepted");
}
int main() try {
    constexpr double pi=std::numbers::pi;
    const auto solid=BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0,1,0)),6,20).Shape();
    const auto circle=shaftSection(solid,7);
    close(circle.area_mm2,36*pi);close(circle.minimum_I_mm4,pi*std::pow(6,4)/4);close(circle.maximum_I_mm4,circle.minimum_I_mm4);
    close(circle.centroid_x_mm,0);close(circle.centroid_z_mm,0);
    const auto bore=BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,-1,0),gp_Dir(0,1,0)),2,22).Shape();
    const auto tube=BRepAlgoAPI_Cut(solid,bore).Shape();const auto annulus=shaftSection(tube,12);
    close(annulus.area_mm2,32*pi);close(annulus.minimum_I_mm4,pi*(std::pow(6,4)-std::pow(2,4))/4);
    const auto rectangle=BRepPrimAPI_MakeBox(gp_Pnt(2,0,-4),4,20,6).Shape();
    const auto unrotated=shaftSection(rectangle,11);
    close(unrotated.area_mm2,24);close(unrotated.centroid_x_mm,4);close(unrotated.centroid_z_mm,-1);
    close(unrotated.Ixx_mm4,72);close(unrotated.Izz_mm4,32);close(unrotated.Ixz_mm4,0);
    gp_Trsf rotation;rotation.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,1,0)),0.37);
    const auto rotated=shaftSection(BRepBuilderAPI_Transform(rectangle,rotation,true).Shape(),11);
    close(rotated.minimum_I_mm4,32);close(rotated.maximum_I_mm4,72);
    close(rotated.Ixx_mm4+rotated.Izz_mm4,104);
    if(std::abs(rotated.Ixz_mm4)<1)throw std::runtime_error("Rotated section coupling not represented");
    // Asymmetric open keyway changes both the centroid and principal stiffness.
    const auto notch=BRepPrimAPI_MakeBox(gp_Pnt(-1,-1,3),2,22,5).Shape();
    const auto keyed=shaftSection(BRepAlgoAPI_Cut(solid,notch).Shape(),10);
    if(keyed.centroid_z_mm>=0||keyed.minimum_I_mm4>=circle.minimum_I_mm4)throw std::runtime_error("Keyed section incorrectly unchanged");
    rejects([&]{shaftSection(solid,0);});rejects([&]{shaftSection(solid,20);});
    rejects([&]{shaftSection(solid,std::numeric_limits<double>::quiet_NaN());});
    rejects([]{shaftSection(TopoDS_Shape(),1);});
    std::cout<<"B-rep solid/annular/translated/rotated/keyed section benchmarks passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}