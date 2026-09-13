#include "feed_refinement.hpp"
#include "head_geometry.hpp"
#include <BRepAdaptor_Surface.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace starz::engineering {
namespace {
constexpr double tolerance=1e-6;
Eigen::Vector3d coordinates(const gp_Pnt& p){return {p.X(),p.Y(),p.Z()};}
Eigen::Vector3d direction(const gp_Dir& p){return {p.X(),p.Y(),p.Z()};}
}
double ShaftFeatureGuide::distance_mm(const Eigen::Vector3d& p) const {
    if(!p.allFinite()||faces.empty())throw std::invalid_argument("Invalid CAD feature sizing request");
    double nearest=std::numeric_limits<double>::infinity();
    for(const auto& face:faces) {
        const Eigen::Vector3d outside=(face.lower_mm-p).cwiseMax(p-face.upper_mm).cwiseMax(0.);
        double distance=outside.norm();
        if(face.kind!=RefinementSurface::bounded_other) {
            const Eigen::Vector3d relative=p-face.origin_mm;
            const double axial=relative.dot(face.axis);
            const double radial=(relative-axial*face.axis).norm();
            const double surface=face.kind==RefinementSurface::cylinder?std::abs(radial-face.major_radius_mm):
                std::abs(std::hypot(radial-face.major_radius_mm,axial)-face.minor_radius_mm);
            // Both untrimmed analytic-surface distance and bounding-box distance
            // are lower bounds to the trimmed face; their maximum remains one.
            distance=std::max(distance,std::max(0.,surface-tolerance));
        }
        nearest=std::min(nearest,distance);
    }
    if(!std::isfinite(nearest))throw std::runtime_error("Non-finite CAD feature guide");
    return nearest;
}
ShaftFeatureGuide feedShaftFeatureGuide(const TopoDS_Shape& shaft,std::string_view feature) {
    ShaftFeatureGuide guide;guide.feature=feature;
    const bool rear=feature=="A12_S18_5",front=feature=="A12_S19_62",slot=feature=="M15_slot_exit",pulley=feature=="pulley_slot_blends",
        floor=feature=="pulley_key_floor",shoulder=feature=="rear_shoulder_blends",roller=feature=="roller_key_floor",
        allTori=feature=="concave_fillet_tori";
    if(!(rear||front||slot||pulley||floor||shoulder||roller||allTori))throw std::invalid_argument("Unknown CAD refinement feature");
    guide.region=allTori?"":rear||front?"A12_groove":slot?"M15_relief":shoulder?"rear_shoulder_and_slot_entry":roller?"roller_key_root":"pulley_key_and_MB2_slot";
    TopTools_IndexedMapOfShape faces;TopExp::MapShapes(shaft,TopAbs_FACE,faces);
    for(int i=1;i<=faces.Extent();++i) {
        const auto face=TopoDS::Face(faces(i));const BRepAdaptor_Surface surface(face);const auto b=bounds(face);
        ShaftRefinementFace selected;selected.cad_face=i;selected.shape=face;
        selected.lower_mm={b.xmin-tolerance,b.ymin-tolerance,b.zmin-tolerance};
        selected.upper_mm={b.xmax+tolerance,b.ymax+tolerance,b.zmax+tolerance};
        if(surface.GetType()==GeomAbs_Torus) {
            const auto t=surface.Torus();selected.kind=RefinementSurface::torus;
            selected.origin_mm=coordinates(t.Location());selected.axis=direction(t.Axis().Direction());
            selected.major_radius_mm=t.MajorRadius();selected.minor_radius_mm=t.MinorRadius();
        } else if(surface.GetType()==GeomAbs_Cylinder) {
            const auto c=surface.Cylinder();selected.kind=RefinementSurface::cylinder;
            selected.origin_mm=coordinates(c.Location());selected.axis=direction(c.Axis().Direction());selected.major_radius_mm=c.Radius();
        }
        bool include=false;
        if(rear||front) {
            const double station=rear?18.54:19.58;
            include=selected.kind==RefinementSurface::torus&&std::abs(selected.origin_mm.y()-station)<tolerance&&
                std::abs(selected.major_radius_mm-5.765)<tolerance&&std::abs(selected.minor_radius_mm-0.04)<tolerance&&
                std::abs(std::abs(selected.axis.y())-1)<1e-10;
        } else if(allTori) {
            // Whole-shaft sizing only (empty region): all concave fillet tori
            // diagnosed in the exterior-centroid study (rear-shoulder R0.20,
            // both MB2 washer-entry R0.15 tori, both pulley key-floor end
            // R0.15 tori, both roller key-floor end R0.15 tori) plus both
            // A12 ring-groove roots, selected by torus geometry.
            const bool nearRearShoulder=selected.kind==RefinementSurface::torus&&selected.minor_radius_mm<=0.200001&&b.ymin>=31.199999&&b.ymax<=31.700001;
            const bool pulleyFloorEnd=selected.kind==RefinementSurface::torus&&selected.minor_radius_mm<=0.200001&&b.ymin>=33.449999&&b.ymax<=61.550001&&b.zmax<=4.150001;
            const bool rollerFloorEnd=selected.kind==RefinementSurface::torus&&selected.minor_radius_mm<=0.200001&&b.ymin>=94.449999&&b.ymax<=112.550001&&b.zmax<=-3.199999;
            const bool a12Root=selected.kind==RefinementSurface::torus&&std::abs(selected.major_radius_mm-5.765)<tolerance&&
                std::abs(selected.minor_radius_mm-0.04)<tolerance&&std::abs(std::abs(selected.axis.y())-1)<1e-10&&
                (std::abs(selected.origin_mm.y()-18.54)<tolerance||std::abs(selected.origin_mm.y()-19.58)<tolerance);
            include=nearRearShoulder||pulleyFloorEnd||rollerFloorEnd||a12Root;
        } else if(shoulder) {
            // Rear bearing-shoulder R0.20 blend (S31.3-31.5) and the two R0.15
            // MB2 washer-entry floor tori (S31.2-31.7); all diagnosed exterior
            // centroid tori of this station, selected by torus geometry.
            include=selected.kind==RefinementSurface::torus&&selected.minor_radius_mm<=0.200001&&b.ymin>=31.199999&&b.ymax<=31.700001;
        } else if(roller) {
            // Roller key R0.15 floor-end blends below the Z-3.2 floor (S94.45-112.55).
            const bool small=selected.kind==RefinementSurface::torus&&selected.minor_radius_mm<=0.200001;
            include=small&&b.ymin>=94.449999&&b.ymax<=112.550001&&b.zmax<=-3.199999;
        } else {
            // All small upper-pocket blends, selected geometrically rather than
            // by transient STEP face IDs. Large journal/key-wall cylinders are
            // not mistaken for blend faces. BSpline junctions use their bounds.
            const bool small=selected.kind==RefinementSurface::cylinder?selected.major_radius_mm<=0.500001:
                selected.kind==RefinementSurface::torus?selected.minor_radius_mm<=0.500001:
                surface.GetType()==GeomAbs_BSplineSurface;
            include=small&&b.ymin>=31.199999&&b.ymax<=73.000001&&b.zmin>=3.999999;
            if(floor)include=include&&b.ymin>=33.449999&&b.ymax<=61.550001&&b.zmax<=4.150001;
            if(slot) {
                include=include&&b.ymax>=71.4&&b.ymin<=73.1;
                selected.lower_mm.y()=std::max(selected.lower_mm.y(),71.4-tolerance);
                selected.upper_mm.y()=std::min(selected.upper_mm.y(),73.1+tolerance);
            }
        }
        if(include)guide.faces.push_back(std::move(selected));
    }
    if(guide.faces.empty()||((rear||front)&&guide.faces.size()!=1)||(shoulder&&guide.faces.size()!=3)||(roller&&guide.faces.size()!=2)||
        (allTori&&guide.faces.size()!=9))
        throw std::runtime_error("CAD feature selection is empty or ambiguous");
    return guide;
}
}