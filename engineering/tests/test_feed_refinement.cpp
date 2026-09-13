#include "feed_refinement.hpp"
#include "feed_geometry.hpp"
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace se=starz::engineering;
namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}require(rejected,"Invalid feature request accepted");}
}
int main()try {
    const auto shape=se::feedShaftShape();std::size_t checked=0;
    for(const char* feature:{"A12_S18_5","A12_S19_62","M15_slot_exit","pulley_slot_blends","pulley_key_floor","rear_shoulder_blends","roller_key_floor","concave_fillet_tori"}) {
        const auto guide=se::feedShaftFeatureGuide(shape,feature);
        std::cout<<feature<<" selected "<<guide.faces.size()<<" faces\n";
        for(const auto& face:guide.faces) {
            require(face.lower_mm.allFinite()&&face.upper_mm.allFinite()&&(face.upper_mm-face.lower_mm).minCoeff()>0,"Invalid bounded feature");
            if(face.kind!=se::RefinementSurface::bounded_other) {
                require(std::abs(face.axis.norm()-1)<1e-12&&face.major_radius_mm>0,"Invalid analytic feature");
            }
            const BRepAdaptor_Surface surface(face.shape);
            for(int u=1;u<=5;++u)for(int v=1;v<=5;++v) {
                const auto point=surface.Value(surface.FirstUParameter()+(surface.LastUParameter()-surface.FirstUParameter())*u/6,
                                               surface.FirstVParameter()+(surface.LastVParameter()-surface.FirstVParameter())*v/6);
                const Eigen::Vector3d p{point.X(),point.Y(),point.Z()};
                if(guide.feature=="M15_slot_exit"&&(p.y()<71.5||p.y()>72.9))continue;
                for(double offset:{-0.013,0.,0.017}) {
                    const Eigen::Vector3d q=p+Eigen::Vector3d(offset,-offset/3,offset/2);
                    BRepExtrema_DistShapeShape closest(BRepBuilderAPI_MakeVertex(gp_Pnt(q.x(),q.y(),q.z())).Shape(),face.shape);
                    closest.Perform();require(closest.IsDone()&&closest.NbSolution()>0,"Trimmed face projection failed");
                    require(guide.distance_mm(q)>=0&&guide.distance_mm(q)<=closest.Value()+1e-6,"Sizing guide exceeded trimmed-face distance");++checked;
                }
            }
        }
        rejects([&]{(void)guide.distance_mm({0,std::numeric_limits<double>::quiet_NaN(),0});});
        rejects([&]{(void)guide.distance_mm({0,0,std::numeric_limits<double>::infinity()});});
    }
    const auto rear=se::feedShaftFeatureGuide(shape,"A12_S18_5"),front=se::feedShaftFeatureGuide(shape,"A12_S19_62");
    require(rear.faces.size()==1&&front.faces.size()==1,"A12 root selection not unique");
    require(rear.faces[0].cad_face!=front.faces[0].cad_face,"Separate groove roots selected the same face");
    require(rear.distance_mm({5.725,18.54,0})<1e-9&&front.distance_mm({5.725,19.58,0})<1e-9,"A12 root missing");
    require(rear.distance_mm({5.725,19.58,0})>1&&front.distance_mm({5.725,18.54,0})>1,"Opposite root unexpectedly refined");
    require(se::feedShaftFeatureGuide(shape,"M15_slot_exit").distance_mm({2.02832634681,72.7671416783,6.43073599306})<0.005,"Observed M15 peak not covered");
    const auto pulley=se::feedShaftFeatureGuide(shape,"pulley_slot_blends");
    require(pulley.distance_mm({2.4462782772,40.502250772,3.99053857997})<0.027,"Pulley floor-side peak not covered");
    require(pulley.distance_mm({-1.1899121593,61.2066569583,4.01600761551})<0.024,"Pulley floor-end peak not covered");
    const auto floor=se::feedShaftFeatureGuide(shape,"pulley_key_floor");
    require(floor.faces.size()==4,"Pulley floor must include both side blends and both end blends");
    require(floor.distance_mm({2.4462782772,40.502250772,3.99053857997})<0.027&&
        floor.distance_mm({-1.1899121593,61.2066569583,4.01600761551})<0.024,"Pulley floor peak coverage missing");
    require(floor.distance_mm({2.6,59.3,6.4})>2,"Floor guide unexpectedly refines distant upper lip");
    // Diagnosed exterior centroids of the finest Netgen front mesh (cells
    // 284882, 261908, 74727, 175303): each must be covered by its feature at
    // below its recorded 1.99-5.45 um CAD gap and not by the wrong feature.
    const auto shoulder=se::feedShaftFeatureGuide(shape,"rear_shoulder_blends"),roller=se::feedShaftFeatureGuide(shape,"roller_key_floor");
    require(shoulder.region=="rear_shoulder_and_slot_entry"&&roller.region=="roller_key_root","New feature regions changed");
    std::size_t shoulderTori=0,washerTori=0;
    for(const auto& f:shoulder.faces) {
        require(f.kind==se::RefinementSurface::torus,"Rear shoulder feature selected a non-torus");
        if(std::abs(f.minor_radius_mm-0.2)<1e-9&&std::abs(f.major_radius_mm-6.2)<1e-9&&std::abs(f.origin_mm.y()-31.3)<1e-9)++shoulderTori;
        if(std::abs(f.minor_radius_mm-0.15)<1e-9&&std::abs(f.major_radius_mm-0.35)<1e-9&&std::abs(std::abs(f.origin_mm.x())-1.6)<1e-9)++washerTori;
    }
    require(shoulderTori==1&&washerTori==2,"Rear shoulder feature must be the R0.20 shoulder torus plus both R0.15 washer-entry tori");
    for(const auto& f:roller.faces)require(f.kind==se::RefinementSurface::torus&&std::abs(f.minor_radius_mm-0.15)<1e-9&&std::abs(f.major_radius_mm-1.37)<1e-9,"Roller floor feature is not the R0.15 end tori");
    require(shoulder.distance_mm({2.1500360313575,31.482279229125,5.7427204929})<0.0054519451894011,"Rear shoulder exterior centroid not covered");
    require(shoulder.distance_mm({1.943253295785,31.3608177503,5.588812085715})<0.0039965317834536,"Washer-entry exterior centroid not covered");
    require(floor.distance_mm({2.2522127044775,34.9647271979,4.0384359335425})<0.0025691779313421,"Pulley floor exterior centroid not covered");
    require(roller.distance_mm({-1.3956415395325,95.506457439025,-3.24143672586})<0.0019868446352496,"Roller floor exterior centroid not covered");
    require(shoulder.distance_mm({-1.3956415395325,95.506457439025,-3.24143672586})>60&&roller.distance_mm({2.1500360313575,31.482279229125,5.7427204929})>60,"Features overlap distant stations");
    // Whole-shaft concave-fillet torus guide: the six diagnosed concave fillet
    // tori (rear-shoulder, washer-entry pair, pulley key-floor pair, roller
    // key-floor pair) plus both A12 ring-groove roots; empty region (all-root
    // sizing only) and no non-torus member.
    const auto tori=se::feedShaftFeatureGuide(shape,"concave_fillet_tori");
    require(tori.region.empty()&&tori.faces.size()==9,"Concave fillet tori feature must select nine faces in all-root mode");
    std::size_t tShoulder=0,tWasher=0,tPulley=0,tRoller=0,tA12=0;
    for(const auto& f:tori.faces) {
        require(f.kind==se::RefinementSurface::torus,"Concave fillet tori feature selected a non-torus");
        if(std::abs(f.minor_radius_mm-0.2)<1e-9&&std::abs(f.major_radius_mm-6.2)<1e-9&&std::abs(f.origin_mm.y()-31.3)<1e-9)++tShoulder;
        if(std::abs(f.minor_radius_mm-0.15)<1e-9&&std::abs(f.major_radius_mm-0.35)<1e-9&&std::abs(std::abs(f.origin_mm.x())-1.6)<1e-9)++tWasher;
        if(std::abs(f.minor_radius_mm-0.15)<1e-9&&std::abs(f.major_radius_mm-2.37)<1e-9&&std::abs(f.origin_mm.z()-4.15)<1e-9)++tPulley;
        if(std::abs(f.minor_radius_mm-0.15)<1e-9&&std::abs(f.major_radius_mm-1.37)<1e-9)++tRoller;
        if(std::abs(f.minor_radius_mm-0.04)<1e-9&&std::abs(f.major_radius_mm-5.765)<1e-9)++tA12;
    }
    require(tShoulder==1&&tWasher==2&&tPulley==2&&tRoller==2&&tA12==2,"Concave fillet tori selection incomplete or ambiguous");
    require(tori.distance_mm({2.1500360313575,31.482279229125,5.7427204929})<0.0054519451894011&&
        tori.distance_mm({1.943253295785,31.3608177503,5.588812085715})<0.0039965317834536&&
        tori.distance_mm({2.2522127044775,34.9647271979,4.0384359335425})<0.0025691779313421&&
        tori.distance_mm({-1.3956415395325,95.506457439025,-3.24143672586})<0.0019868446352496,
        "Concave fillet tori guide misses a diagnosed exterior centroid");
    require(tori.distance_mm({5.725,18.54,0})<1e-9&&tori.distance_mm({5.725,19.58,0})<1e-9,"A12 roots missing from concave fillet tori guide");
    require(tori.distance_mm({0,0,0})>1,"Concave fillet tori guide unexpectedly refines the distant axis");
    rejects([&]{(void)se::feedShaftFeatureGuide(shape,"unknown");});
    rejects([&]{(void)se::feedShaftFeatureGuide(BRepPrimAPI_MakeBox(1,2,3).Shape(),"A12_S18_5");});
    rejects([]{(void)se::ShaftFeatureGuide{}.distance_mm(Eigen::Vector3d::Zero());});
    require(checked>=300,"Insufficient geometry checks");
    std::cout<<"CAD feature selection and "<<checked<<" trimmed-face lower-bound samples passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}