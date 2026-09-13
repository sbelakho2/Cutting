#include "cad_boundary.hpp"
#include "feed_geometry.hpp"
#include <BRepAdaptor_Surface.hxx>
#include <Eigen/Geometry>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

// Fixed regression for the 25 retained non-A12 exterior-centroid tetrahedra of
// the finest published Netgen front mesh (working-4.vtu, 542803 cells). Four
// representative cells, one per governing torus family, are pinned exactly:
// three vertices on the CAD, one inside, one mesh-boundary face, centroid
// outside by 1.99-5.45 um. The nearest CAD faces are identified by their torus
// geometry, not by transient face IDs, and OCC's trimmed projection is checked
// against an independent analytic torus distance.
namespace se=starz::engineering;
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Torus {Eigen::Vector3d origin,axis;double major,minor;};
// Unsigned distance from p to the complete torus surface.
double torusDistance(const Torus& t,const Eigen::Vector3d& p) {
    const Eigen::Vector3d d=p-t.origin;const double h=d.dot(t.axis);const double rho=(d-h*t.axis).norm();
    return std::abs(std::sqrt((rho-t.major)*(rho-t.major)+h*h)-t.minor);
}
struct Fixture {
    const char* name;std::array<Eigen::Vector3d,4> vertices;Torus torus;double recorded_gap_mm;int expected_inside_vertex;
};
}
int main()try {
    const auto shaft=se::feedShaftShape();se::CadBoundaryInspector inspector(shaft);
    require(inspector.faceCount()==85,"F101-R6 analysis shaft face count changed");
    bool threw=false;try{(void)inspector.inspect({std::numeric_limits<double>::quiet_NaN(),0,0});}catch(const std::runtime_error&){threw=true;}
    require(threw,"Non-finite inspection point accepted");
    const std::vector<Fixture> fixtures{
        {"cell 284882 rear bearing-shoulder R0.20 torus (maximum gap)",
         {{{2.0582575695,31.5,5.84838232143},{2.19318298377,31.4351335452,5.64122466394},{2.10310841699,31.4939833713,5.70223506605},{2.24559515517,31.5,5.77903992018}}},
         {{0,31.3,0},{0,1,0},6.2,.2},0.0054519451894011,2},
        {"cell 74727 rear pulley key-floor R0.15 end torus",
         {{{2.24533644095,34.8259439407,4.15},{2.17038278417,35.0179818436,4},{2.32817642193,35.0056377504,4.15},{2.26495517086,35.0093452569,3.85374373417}}},
         {{0,35.97,4.15},{0,0,-1},2.37,.15},0.0025691779313421,3},
        {"cell 175303 rear roller key-floor R0.15 end torus",
         {{{-1.46820725596,95.5765950514,-3.35},{-1.40429688942,95.3883211828,-3.35},{-1.31450537385,95.5840263971,-3.2},{-1.3955566389,95.4768871248,-3.06574690344}}},
         {{0,95.97,-3.35},{0,0,1},1.37,.15},0.0019868446352496,3},
        {"cell 261908 washer-entry floor R0.15 torus",
         {{{1.96769604619,31.4438689795,5.53653534175},{1.9906277566,31.3191851625,5.63086744248},{1.95272877178,31.3456239094,5.65},{1.86196060857,31.3345929498,5.53784555863}}},
         {{1.6,31.7,5.65},{0,0,-1},.35,.15},0.0039965317834536,1}};
    for(const auto& f:fixtures) {
        const auto& p=f.vertices;const double volume=std::abs((p[1]-p[0]).dot((p[2]-p[0]).cross(p[3]-p[0])))/6;
        require(volume>1e-4&&volume<2e-3,"Fixture tetrahedron volume outside the published 1.58e-4..1.21e-3 mm^3 band");
        int on=0,inside=0;
        for(int i=0;i<4;++i) {
            const auto r=inspector.inspect(p[static_cast<std::size_t>(i)]);
            if(r.state==TopAbs_ON){++on;require(r.distance_mm<1e-9,"Boundary vertex is not on its nearest trimmed face");}
            else if(r.state==TopAbs_IN){++inside;require(i==f.expected_inside_vertex&&r.distance_mm>.02,"Interior vertex position/depth changed");}
            else throw std::runtime_error("Fixture vertex classified outside the CAD");
        }
        require(on==3&&inside==1,"Fixture must have exactly three boundary vertices and one interior vertex");
        Eigen::Vector3d centroid=Eigen::Vector3d::Zero();for(const auto& v:p)centroid+=v/4;
        const auto c=inspector.inspect(centroid);
        require(c.state==TopAbs_OUT,"Fixture centroid is no longer exterior");
        require(c.type==GeomAbs_Torus&&c.support_dimension==2&&c.curvature.has_value(),"Centroid nearest face is not a torus interior");
        const BRepAdaptor_Surface surface(inspector.face(static_cast<std::size_t>(c.face)));require(surface.GetType()==GeomAbs_Torus,"Reported face type disagrees with CAD");
        const auto torus=surface.Torus();const Eigen::Vector3d origin{torus.Location().X(),torus.Location().Y(),torus.Location().Z()};
        const Eigen::Vector3d axis{torus.Axis().Direction().X(),torus.Axis().Direction().Y(),torus.Axis().Direction().Z()};
        require((origin-f.torus.origin).norm()<1e-9&&std::abs(std::abs(axis.dot(f.torus.axis))-1)<1e-12,"Governing torus origin/axis changed");
        require(std::abs(torus.MajorRadius()-f.torus.major)<1e-9&&std::abs(torus.MinorRadius()-f.torus.minor)<1e-9,"Governing torus radii changed");
        require(std::abs(c.distance_mm-f.recorded_gap_mm)<1e-9,"Exterior centroid gap changed from the published audit");
        require(std::abs(torusDistance(f.torus,centroid)-c.distance_mm)<1e-9,"OCC trimmed projection disagrees with analytic torus distance");
        require(std::abs(torusDistance(f.torus,c.projected_mm))<1e-9,"Projected point is not on the governing torus");
        require(std::abs(std::abs(c.curvature->maximum_per_mm)-1/f.torus.minor)<1e-9||std::abs(std::abs(c.curvature->minimum_per_mm)-1/f.torus.minor)<1e-9,"Minor-circle curvature missing from principal curvatures");
        std::cout<<f.name<<": gap "<<c.distance_mm*1e3<<" um verified\n";
    }
    std::cout<<"CAD boundary inspector: "<<fixtures.size()<<" exterior-centroid torus fixtures verified against analytic distances\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
