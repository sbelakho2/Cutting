#include "feed_geometry.hpp"
#include "feed_refinement.hpp"
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <Eigen/Geometry>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace se=starz::engineering;
namespace {
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
gp_Pnt point(const Eigen::Vector3d& p){return {p.x(),p.y(),p.z()};}
double distance(const Eigen::Vector3d& p,const TopoDS_Shape& face) {
    BRepExtrema_DistShapeShape projection(BRepBuilderAPI_MakeVertex(point(p)).Shape(),face);
    projection.Perform();require(projection.IsDone()&&projection.NbSolution()>0,"A12 trimmed-face projection failed");
    const double value=projection.Value();require(std::isfinite(value)&&value>=0,"Invalid A12 projection distance");return value;
}
}
int main()try {
    // Printed coordinates of zero-based cell489924 in the published unchanged
    // A12-front/working-4.vtu (CAD-feature increment, not a new mesh fixture).
    // Its four vertices lie on one saddle-shaped R0.04 torus; two cell faces
    // are exterior mesh faces. Positive volume and surface nodes are insufficient
    // to establish that its constant-strain stress represents a material point.
    const std::array<Eigen::Vector3d,4> vertices{{
        {3.99299694256,19.6132718947,4.12743149891},
        {4.00256898574,19.6156406177,4.12378756916},
        {3.99096699514,19.6082212623,4.12084560232},
        {3.99969032266,19.6105712017,4.11593816692}}};
    const auto shaft=se::feedShaftShape();const auto guide=se::feedShaftFeatureGuide(shaft,"A12_S19_62");
    require(guide.faces.size()==1,"Expected one geometrically selected A12 front torus");
    const auto& torus=guide.faces.front();require(torus.kind==se::RefinementSurface::torus,"A12 root is not a torus");
    require(std::abs(torus.major_radius_mm-5.765)<1e-10&&std::abs(torus.minor_radius_mm-0.04)<1e-10,"A12 physical CAD changed");
    BRepClass3d_SolidClassifier classifier(shaft);Eigen::Vector3d centroid=Eigen::Vector3d::Zero();double nodeDistance=0;
    for(const auto& p:vertices) {
        nodeDistance=std::max(nodeDistance,distance(p,torus.shape));centroid+=p/4;
        classifier.Perform(point(p),1e-7);require(classifier.State()==TopAbs_ON,"Fixture vertex left nominal CAD boundary");
    }
    require(nodeDistance<1e-8,"Fixture vertices no longer reproduce exact-surface nodes");
    const double volume=std::abs((vertices[1]-vertices[0]).dot((vertices[2]-vertices[0]).cross(vertices[3]-vertices[0])))/6;
    require(std::isfinite(volume)&&volume>1e-9,"Fixture does not have a finite nonzero linear volume");
    classifier.Perform(point(centroid),1e-7);require(classifier.State()==TopAbs_OUT,"Regression no longer reproduces exterior material-point discrepancy");
    const double gap=distance(centroid,torus.shape);
    require(std::abs(gap-0.00028117705629338)<1e-9,"Exterior distance changed or millimetres/micrometres confused");
    // Independent analytic torus equation, with the quarter-torus trim checked
    // through the actual CAD projection above. The centroid is in its void side.
    const Eigen::Vector3d delta=centroid-torus.origin_mm;
    const double axial=delta.dot(torus.axis),radial=(delta-axial*torus.axis).norm();
    const double analyticGap=torus.minor_radius_mm-std::hypot(radial-torus.major_radius_mm,axial);
    require(analyticGap>0&&std::abs(analyticGap-gap)<1e-10,"Analytic torus and trimmed CAD disagree");
    const Eigen::Vector3d interior=centroid*0.99+Eigen::Vector3d{0,centroid.y(),0}*0.01;
    classifier.Perform(point(interior),1e-7);require(classifier.State()==TopAbs_IN,"Independent inward control point is not material");
    std::cout<<std::setprecision(14)<<"A12 regression: node gap="<<nodeDistance<<" mm, linear volume="<<volume
             <<" mm^3, exterior centroid gap="<<gap<<" mm ("<<gap*1000<<" micrometres). No cell or stress was filtered.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}