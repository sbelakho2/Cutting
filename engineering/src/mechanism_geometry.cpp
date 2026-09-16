#include "mechanism_geometry.hpp"
#include "feed_geometry.hpp"
#include "measurement_geometry.hpp"
#include "shared/feed_drive_spec.hpp"
#include "shared/measurement_spec.hpp"
#include "shared/mechanism_spec.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopExp_Explorer.hxx>
#include <gp_Ax1.hxx>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <stdexcept>

namespace starz::engineering {
namespace {
namespace fd=starz::feed::design;
namespace fc=starz::feed::catalogue;
namespace mc=starz::mechanism::catalogue;
namespace md=starz::mechanism::design;
namespace msc=starz::measurement::catalogue;
constexpr double g_n=9.80665;
// Fitted purchased parts are modelled 0.01 mm under their nominal bores so
// that H7/g6, H7/m6 and K6 fits never produce coincident boolean faces.
constexpr double fit=0.01,thread=0.02;

TopoDS_Shape box(double x0,double y0,double z0,double dx,double dy,double dz) {
    return BRepPrimAPI_MakeBox(gp_Pnt(x0,y0,z0),dx,dy,dz).Shape();
}
TopoDS_Shape cylZ(double x,double y,double z0,double r,double len) {
    return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,y,z0),gp_Dir(0,0,1)),r,len).Shape();
}
TopoDS_Shape cylY(double x,double y0,double z,double r,double len) {
    return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,y0,z),gp_Dir(0,1,0)),r,len).Shape();
}
TopoDS_Shape cylX(double x0,double y,double z,double r,double len) {
    return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x0,y,z),gp_Dir(1,0,0)),r,len).Shape();
}
TopoDS_Shape cut(const TopoDS_Shape& a,const TopoDS_Shape& b) {
    BRepAlgoAPI_Cut op(a,b);op.Build();
    if(!op.IsDone())throw std::runtime_error("Mechanism cut failed");
    op.SimplifyResult();return op.Shape();
}
TopoDS_Shape fuse(const TopoDS_Shape& a,const TopoDS_Shape& b) {
    BRepAlgoAPI_Fuse op(a,b);op.Build();
    if(!op.IsDone())throw std::runtime_error("Mechanism fuse failed");
    op.SimplifyResult();return op.Shape();
}
TopoDS_Shape tubeZ(double x,double y,double z0,double ro,double ri,double len) {
    return cut(cylZ(x,y,z0,ro,len),cylZ(x,y,z0-0.01,ri,len+0.02));
}
struct Head { double d,h; };
constexpr Head head_M4{7,4},head_M5{8.5,5},head_M6{10,6},head_M8{13,8},head_M10{16,10};
// ISO4762 socket screw pointing -Z: head above z_under_head, shank below.
TopoDS_Shape screwDown(double x,double y,double z_under_head,double m,Head h,double len) {
    return fuse(cylZ(x,y,z_under_head,h.d/2,h.h),cylZ(x,y,z_under_head-len,(m-thread)/2,len));
}
// ISO4762 socket screw pointing +Z: head below z_under_head, shank above.
TopoDS_Shape screwUp(double x,double y,double z_under_head,double m,Head h,double len) {
    return fuse(cylZ(x,y,z_under_head-h.h,h.d/2,h.h),cylZ(x,y,z_under_head,(m-thread)/2,len));
}
// ISO4762 socket screw pointing +X: head at x<x_under_head.
TopoDS_Shape screwPlusX(double x_under_head,double y,double z,double m,Head h,double len) {
    return fuse(cylX(x_under_head-h.h,y,z,h.d/2,h.h),cylX(x_under_head,y,z,(m-thread)/2,len));
}
// ISO4762 socket screw pointing -X: head at x>x_under_head.
TopoDS_Shape screwMinusX(double x_under_head,double y,double z,double m,Head h,double len) {
    return fuse(cylX(x_under_head,y,z,h.d/2,h.h),cylX(x_under_head-len,y,z,(m-thread)/2,len));
}
// ISO4762 socket screw pointing -Y: head at y>y_under_head.
TopoDS_Shape screwMinusY(double x,double y_under_head,double z,double m,Head h,double len) {
    return fuse(cylY(x,y_under_head,z,h.d/2,h.h),cylY(x,y_under_head-len,z,(m-thread)/2,len));
}
void add(std::vector<HeadPart>& parts,std::string id,std::string description,TopoDS_Shape shape,bool purchased=false,bool moving=false) {
    parts.push_back({std::move(id),std::move(description),std::move(shape),purchased,moving});
}
double upperBaseZ(double d) { return fd::wire_axis_z+(fc::roller_d+d)/2+34; }
double wheelCentreZ(double d) { return fd::wire_axis_z+d/2+msc::wheel_od_mm/2; }
double counterRollerZ(double d) { return fd::wire_axis_z-d/2-msc::bearing_D_mm/2; }
const double nip_x[2]={fd::near_nip_x,fd::near_nip_x+fd::nominal_centres};
const double mount_y[2]={fd::rear_mount_station-fd::wire_contact_station,fd::front_mount_station-fd::wire_contact_station};
const double module_screw_y=fd::module_bolt_station-fd::wire_contact_station;
const double module_pin_y=fd::module_pin_station-fd::wire_contact_station;
double density(const std::string& description) {
    if(description.find("6082")!=std::string::npos||description.find("HTPA")!=std::string::npos)return 2.70e-6;
    if(description.find("UMCL")!=std::string::npos)return 1.80e-6;
    if(description.find("CuZn")!=std::string::npos)return 8.50e-6;
    return 7.85e-6;
}
}

CarrierSetting carrierSetting(double d,double carried_mass_kg,double p) {
    CarrierSetting s{};
    s.carrier_top_z=upperBaseZ(d)+md::carrier_t;s.carried_mass_kg=carried_mass_kg;
    s.adjuster_protrusion_mm=p;
    s.spring_length_mm=(md::bridge_z_min-p)-md::cap_t-s.carrier_top_z;
    s.spring_force_n=mc::carrier_spring_rate_n_mm*(mc::carrier_spring_L_mm-s.spring_length_mm);
    s.nip_force_n=(s.spring_force_n+carried_mass_kg*g_n)/2;
    return s;
}
double nominalCarrierProtrusion(double carried_mass_kg) {
    const double force=2*md::nip_force_target_n-carried_mass_kg*g_n;
    const double length=mc::carrier_spring_L_mm-force/mc::carrier_spring_rate_n_mm;
    return md::bridge_z_min-md::cap_t-length-(upperBaseZ(fd::wire_diameter_max)+md::carrier_t);
}
double carriedMassEstimate(const std::vector<HeadPart>& cassette,const std::vector<HeadPart>& carrier) {
    double mass=0;
    for(const auto& p:cassette)if(p.id.starts_with("U"))mass+=volume(p.shape)*density(p.description);
    for(const auto& p:carrier) {
        if(p.id=="F202"||p.id=="F208"||p.id=="DW01")mass+=volume(p.shape)*density(p.description);
        else if(p.id.starts_with("LB0"))mass+=mc::bushing_mass_kg;
        else if(p.id.starts_with("FB202")||p.id.starts_with("FW202")||p.id.starts_with("FB206"))mass+=volume(p.shape)*7.85e-6;
    }
    return mass;
}

std::vector<HeadPart> pressureCarrierGroup(double d,double p) {
    if(!std::isfinite(d)||d<fd::wire_diameter_min||d>fd::wire_diameter_max)throw std::invalid_argument("Wire diameter outside cassette envelope");
    std::vector<HeadPart> parts;
    const double post_y[2]={md::post_rear_y,md::post_front_y};
    // F201 lower cassette base: block on the bed plus a flange over H001/H002.
    auto base=fuse(box(md::base_block_x_min,md::base_y_min,0,md::base_x_max-md::base_block_x_min,md::base_y_max-md::base_y_min,md::base_top_z),
                   box(md::base_x_min,md::base_y_min,md::base_flange_bottom_z,md::base_block_x_min-md::base_x_min,md::base_y_max-md::base_y_min,md::base_top_z-md::base_flange_bottom_z));
    for(double nx:nip_x)for(double sx:{-fd::module_bolt_x,fd::module_bolt_x}) {
        const double x=nx+sx;
        for(double y:mount_y) {
            base=cut(base,cylZ(x,y,-0.01,2.75,md::base_top_z+0.02));
            // Head-side heads sit above the H002 riser top (Z40): counterbore from below.
            base=cut(base,cylZ(x,y,-0.01,4.75,md::lower_screw_cbore_z+0.01));
        }
        base=cut(base,cylZ(x,module_screw_y,-0.01,4.5,md::base_top_z+0.02));
    }
    for(double nx:nip_x)for(double sx:{-fd::module_pin_x,fd::module_pin_x})
        base=cut(base,cylZ(nx+sx,module_pin_y,-0.01,3,md::base_top_z+0.02));
    for(int i=0;i<2;++i) {
        const double y=post_y[i];
        base=cut(base,cylZ(md::post_x,y,md::base_top_z-md::post_bore_depth,(mc::post_d_mm+fit)/2,md::post_bore_depth+0.01));
        const double slit_y0=i==0?md::base_y_min-0.01:y;const double slit_y1=i==0?y:md::base_y_max+0.01;
        base=cut(base,box(md::post_x-1,slit_y0,20,2,slit_y1-slit_y0,md::base_top_z-20+0.01));
        const double cy=i==0?md::post_clamp_rear_y:md::post_clamp_front_y;
        base=cut(base,cylX(md::base_block_x_min-0.01,cy,md::post_clamp_z,5.5,6.51));
        base=cut(base,cylX(md::base_block_x_min-0.01,cy,md::post_clamp_z,3.3,md::post_x-md::base_block_x_min+0.02));
        base=cut(base,cylX(md::post_x,cy,md::post_clamp_z,3,20));
    }
    for(double x:{md::bed_screw_x1,md::bed_screw_x2})for(double y:{md::bed_screw_y1,md::bed_screw_y2}) {
        base=cut(base,cylZ(x,y,md::base_top_z-21,7,21.01));
        base=cut(base,cylZ(x,y,-0.01,4.5,md::base_top_z));
    }
    for(double y:{-md::guide_screw_y,md::guide_screw_y})base=cut(base,cylX(md::base_x_min-0.01,y,md::guide_screw_z,2.5,17.01));
    add(parts,"F201","6082-T6 lower cassette base;stepped block;post split clamps",base);
    // Lower housing screws from below, bed screws, posts, post clamps.
    int n=0;
    for(double nx:nip_x)for(double sx:{-fd::module_bolt_x,fd::module_bolt_x})for(double y:mount_y) {
        ++n;const double x=nx+sx;
        add(parts,"FB201-"+std::to_string(n),"ISO4762 M5x20 class8.8 lower housing screw",screwUp(x,y,md::lower_screw_cbore_z-1,5,head_M5,20),true);
        add(parts,"FW201-"+std::to_string(n),"ISO7092 M5 washer5.3x9x1 200HV",tubeZ(x,y,md::lower_screw_cbore_z-1,4.5,2.65,1),true);
    }
    n=0;
    for(double x:{md::bed_screw_x1,md::bed_screw_x2})for(double y:{md::bed_screw_y1,md::bed_screw_y2})
        add(parts,"FB203-"+std::to_string(++n),"ISO4762 M8x55 class8.8 base-to-bed screw",screwDown(x,y,md::base_top_z-21,8,head_M8,55),true);
    for(int i=0;i<2;++i) {
        add(parts,"PC0"+std::to_string(i+1),std::string(mc::post)+" linear shaft12g6 L240;EN1.3505 58HRC hard chrome",
            cylZ(md::post_x,post_y[i],md::base_top_z-md::post_bore_depth,(mc::post_d_mm-fit)/2,mc::post_L_mm),true);
        const double cy=i==0?md::post_clamp_rear_y:md::post_clamp_front_y;
        add(parts,"FB204-"+std::to_string(i+1),"ISO4762 M6x25 class8.8 post clamp screw",screwPlusX(md::base_block_x_min+6.5,cy,md::post_clamp_z,6,head_M6,25),true);
    }
    // F202 upper pressure carrier.
    const double zc=upperBaseZ(d),top=zc+md::carrier_t;
    auto carrier=box(md::base_x_min,md::base_y_min,zc,md::base_x_max-md::base_x_min,md::base_y_max-md::base_y_min,md::carrier_t);
    for(double nx:nip_x)for(double sx:{-fd::module_bolt_x,fd::module_bolt_x}) {
        const double x=nx+sx;
        for(double y:mount_y)carrier=cut(carrier,cylZ(x,y,zc-0.01,2.75,md::carrier_t+0.02));
        carrier=cut(carrier,cylZ(x,module_screw_y,zc-0.01,4.5,md::carrier_t+0.02));
    }
    for(double nx:nip_x)for(double sx:{-fd::module_pin_x,fd::module_pin_x})
        carrier=cut(carrier,cylZ(nx+sx,module_pin_y,zc-0.01,3,md::carrier_t+0.02));
    for(double y:post_y) {
        carrier=cut(carrier,cylZ(md::post_x,y,zc-0.01,(mc::bushing_D_mm+fit)/2,md::carrier_t+0.02));
        for(int k=0;k<4;++k) {
            const double a=(k+0.5)*std::numbers::pi/2;
            carrier=cut(carrier,cylZ(md::post_x+mc::bushing_pcd_mm/2*std::cos(a),y+mc::bushing_pcd_mm/2*std::sin(a),zc-0.01,2,md::carrier_t+0.02));
        }
    }
    carrier=cut(carrier,cylZ(md::spring_x,md::spring_y,zc-0.01,5,md::carrier_t+0.02));
    add(parts,"F202","6082-T6 upper pressure carrier plate10;carries both upper cartridges",carrier);
    n=0;
    for(double nx:nip_x)for(double sx:{-fd::module_bolt_x,fd::module_bolt_x})for(double y:mount_y) {
        ++n;const double x=nx+sx;
        add(parts,"FB202-"+std::to_string(n),"ISO4762 M5x20 class8.8 upper housing screw",screwDown(x,y,top+1,5,head_M5,20),true);
        add(parts,"FW202-"+std::to_string(n),"ISO7092 M5 washer5.3x9x1 200HV",tubeZ(x,y,top,4.5,2.65,1),true);
    }
    // Flanged bushings, flange on the carrier top, body hanging below.
    for(int i=0;i<2;++i) {
        const double y=post_y[i];
        auto flange=cut(cylZ(md::post_x,y,top,mc::bushing_flange_d_mm/2,mc::bushing_flange_t_mm),cylZ(md::post_x,y,top-0.01,(mc::bushing_dr_mm+fit)/2,mc::bushing_flange_t_mm+0.02));
        auto body=tubeZ(md::post_x,y,top-(mc::bushing_L_mm-mc::bushing_flange_t_mm),(mc::bushing_D_mm-fit)/2,(mc::bushing_dr_mm+fit)/2,mc::bushing_L_mm-mc::bushing_flange_t_mm);
        int k=0;
        for(int q=0;q<4;++q) {
            const double a=(q+0.5)*std::numbers::pi/2,hx=md::post_x+mc::bushing_pcd_mm/2*std::cos(a),hy=y+mc::bushing_pcd_mm/2*std::sin(a);
            flange=cut(flange,cylZ(hx,hy,top-0.01,mc::bushing_hole_mm/2,mc::bushing_flange_t_mm+0.02));
            flange=cut(flange,cylZ(hx,hy,top+mc::bushing_flange_t_mm-mc::bushing_cbore_t_mm,mc::bushing_cbore_mm/2,mc::bushing_cbore_t_mm+0.01));
            add(parts,"FB206-"+std::to_string(i*4+ ++k),"ISO4762 M4x10 class8.8 bushing flange screw",
                screwDown(hx,hy,top+mc::bushing_flange_t_mm-mc::bushing_cbore_t_mm,4,head_M4,10),true);
        }
        add(parts,"LB0"+std::to_string(i+1),std::string(mc::bushing)+" flanged double linear bushing;steel;D21 L57 flange42",fuse(flange,body),true);
    }
    // Spring column: dowel, spring, cap, adjuster, lock nut, bridge, clamps.
    const auto setting=carrierSetting(d,0,p);
    const double ls=setting.spring_length_mm,cap_top=top+ls+md::cap_t;
    add(parts,"DW01","ISO8734 dowel10m6x24 spring locator",cylZ(md::spring_x,md::spring_y,zc,(10-fit)/2,24),true);
    add(parts,"SP01",std::string(mc::carrier_spring)+" flat-wire compression spring D17 d10.5 L60 4.09N/mm;installed envelope",
        tubeZ(md::spring_x,md::spring_y,top,mc::carrier_spring_D_mm/2,mc::carrier_spring_d_mm/2,ls),true);
    auto cap=fuse(cylZ(md::spring_x,md::spring_y,top+ls,md::cap_d/2,md::cap_t),cylZ(md::spring_x,md::spring_y,top+ls-md::cap_spigot_L,md::cap_spigot_d/2,md::cap_spigot_L));
    cap=cut(cap,cylZ(md::spring_x,md::spring_y,cap_top-md::cap_recess_t,md::cap_recess_d/2,md::cap_recess_t+0.01));
    add(parts,"F208","C45 spring cap24x6;spigot10x10;recess9x2",cap);
    add(parts,"AJ01","DIN913 M8x40 45H flat-point adjuster",cylZ(md::spring_x,md::spring_y,cap_top-md::cap_recess_t,(8-thread)/2,40),true);
    add(parts,"NT01","ISO4032 M8 class8 lock nut;hex envelope",tubeZ(md::spring_x,md::spring_y,md::bridge_z_max,7.5,4,6.8),true);
    auto bridge=box(md::bridge_x_min,md::bridge_y_min,md::bridge_z_min,md::bridge_x_max-md::bridge_x_min,md::bridge_y_max-md::bridge_y_min,md::bridge_z_max-md::bridge_z_min);
    for(int i=0;i<2;++i) {
        const double y=post_y[i];
        bridge=cut(bridge,cylZ(md::post_x,y,md::bridge_z_min-0.01,(mc::post_d_mm+fit)/2,md::bridge_z_max-md::bridge_z_min+0.02));
        const double slit_y0=i==0?md::bridge_y_min-0.01:y;const double slit_y1=i==0?y:md::bridge_y_max+0.01;
        bridge=cut(bridge,box(md::post_x-1,slit_y0,md::bridge_z_min-0.01,2,slit_y1-slit_y0,md::bridge_z_max-md::bridge_z_min+0.02));
        const double cy=i==0?md::bridge_clamp_rear_y:md::bridge_clamp_front_y,cz=(md::bridge_z_min+md::bridge_z_max)/2;
        bridge=cut(bridge,cylX(md::bridge_x_max-5,cy,cz,4.5,5.01));
        bridge=cut(bridge,cylX(md::post_x+1,cy,cz,2.75,md::bridge_x_max-md::post_x));
        bridge=cut(bridge,cylX(md::bridge_x_min-0.01,cy,cz,2.5,md::post_x-1-md::bridge_x_min+0.02));
        add(parts,"FB205-"+std::to_string(i+1),"ISO4762 M5x35 class8.8 bridge clamp screw",screwMinusX(md::bridge_x_max-5,cy,cz,5,head_M5,34.5),true);
    }
    bridge=cut(bridge,cylZ(md::spring_x,md::spring_y,md::bridge_z_min-0.01,4,md::bridge_z_max-md::bridge_z_min+0.02));
    add(parts,"F203","C45 bridge flat40x12;split post clamps;M8 adjuster thread",bridge);
    return parts;
}

std::vector<HeadPart> wireGuideGroup(double d) {
    std::vector<HeadPart> parts;
    const double z=fd::wire_axis_z,L=md::guide_x_max-md::guide_x_min;
    auto holder=box(md::guide_x_min,-md::guide_half_y,md::guide_z_min,L,2*md::guide_half_y,md::guide_z_max-md::guide_z_min);
    holder=cut(holder,cylX(md::guide_x_min-0.01,0,z,md::insert_d/2,L+0.02));
    holder=cut(holder,cylZ((md::guide_x_min+md::guide_x_max)/2,0,z+md::insert_d/2-0.1,2,md::guide_z_max-z));
    for(double y:{-md::guide_screw_y,md::guide_screw_y}) {
        holder=cut(holder,cylX(md::guide_x_min-0.01,y,md::guide_screw_z,4.5,5.01));
        holder=cut(holder,cylX(md::guide_x_min+5,y,md::guide_screw_z,2.75,L-5+0.01));
        add(parts,"FB207-"+std::to_string(y>0?2:1),"ISO4762 M5x20 class8.8 guide holder screw",screwPlusX(md::guide_x_min+5,y,md::guide_screw_z,5,head_M5,20),true);
    }
    add(parts,"F205","6082-T6 wire guide holder9x24x83;replaceable insert",holder);
    const double bore=d+md::insert_bore_clearance,r=bore/2,flare=md::insert_cone_depth*std::tan(std::numbers::pi/6);
    auto insert=cylX(md::guide_x_min,0,z,(md::insert_d-fit)/2,L);
    insert=cut(insert,cylX(md::guide_x_min-0.01,0,z,r,L+0.02));
    insert=cut(insert,BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(md::guide_x_min-0.001,0,z),gp_Dir(1,0,0)),r+flare,r,md::insert_cone_depth).Shape());
    insert=cut(insert,BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(md::guide_x_max+0.001,0,z),gp_Dir(-1,0,0)),r+flare,r,md::insert_cone_depth).Shape());
    char label[96];std::snprintf(label,sizeof label,"CuZn39Pb3 guide insert8h6x9;bore%.2f for wire%.2f;60deg entry cones",bore,d);
    add(parts,"F206",label,insert);
    add(parts,"SS01","DIN913 M4x8 45H insert set screw",cylZ((md::guide_x_min+md::guide_x_max)/2,0,z+md::insert_d/2,(4-thread)/2,8),true);
    return parts;
}

ArmSetting armSetting(double d,double p) {
    ArmSetting s{};
    s.arm_top_z=wheelCentreZ(d)+md::housing_half+md::arm_t;
    s.adjuster_protrusion_mm=p;
    s.spring_length_mm=(md::lug_z_min-p)-5-s.arm_top_z;
    s.spring_force_n=mc::arm_spring_rate_n_mm*(mc::arm_spring_L_mm-s.spring_length_mm);
    s.wheel_force_n=s.spring_force_n*(md::arm_spring_x-md::pivot_x)/(md::meas_x-md::pivot_x);
    return s;
}
double nominalArmProtrusion() {
    const double force=md::wheel_force_target_n*(md::meas_x-md::pivot_x)/(md::arm_spring_x-md::pivot_x);
    const double length=mc::arm_spring_L_mm-force/mc::arm_spring_rate_n_mm;
    return md::lug_z_min-5-length-(wheelCentreZ(fd::wire_diameter_max)+md::housing_half+md::arm_t);
}

std::vector<HeadPart> measuringModuleGroup(double d,double p) {
    std::vector<HeadPart> parts;
    const double x=md::meas_x,zw=wheelCentreZ(d);
    gp_Trsf to_wheel;to_wheel.SetTranslation(gp_Vec(x,md::wheel_front_y,zw));
    add(parts,"MW01",std::string(msc::wheel)+" measuring wheel catalogue envelope",BRepBuilderAPI_Transform(measuringWheelInterface(),to_wheel,true).Shape(),true);
    auto shaft=fuse(cylY(x,md::shaft_y_min,zw,(msc::wheel_bore_mm-fit)/2,md::shaft_y_max-md::shaft_y_min),cylY(x,md::wheel_front_y+msc::wheel_overall_mm,zw,3.75,md::front_bearing_y-(md::wheel_front_y+msc::wheel_overall_mm)));
    add(parts,"M202","C45 measuring shaft6h6 L57.75;collar7.5x2.25;bearing seats6j5",shaft);
    for(int i=0;i<2;++i) {
        gp_Trsf t;t.SetTranslation(gp_Vec(x,i?md::rear_bearing_y:md::front_bearing_y,zw));
        add(parts,"MB0"+std::to_string(i+1),std::string(msc::bearing)+" external interface",BRepBuilderAPI_Transform(measuringBearingInterface(),t,true).Shape(),true);
    }
    const double hx=md::housing_x_min,hy=md::housing_y_min,hl=md::housing_y_max-md::housing_y_min;
    auto housing=box(hx,hy,zw-md::housing_half,md::housing_x_max-hx,hl,2*md::housing_half);
    housing=cut(housing,cylY(x,hy-0.01,zw,(msc::bearing_D_mm+fit)/2,md::front_bearing_y+msc::bearing_width_mm-hy+0.01));
    housing=cut(housing,cylY(x,md::front_bearing_y+msc::bearing_width_mm-0.01,zw,8,md::rear_bearing_y-md::front_bearing_y-msc::bearing_width_mm+0.02));
    housing=cut(housing,cylY(x,md::rear_bearing_y,zw,(msc::bearing_D_mm+fit)/2,msc::bearing_width_mm+2));
    housing=cut(housing,cylY(x,md::rear_bearing_y+msc::bearing_width_mm+2-0.01,zw,11,md::housing_y_max-(md::rear_bearing_y+msc::bearing_width_mm+2)+0.02));
    housing=cut(housing,box(hx-0.01,md::coupling_y-2,zw-10,x-11-hx+0.02,msc::coupling_length_mm+4,20));
    // Rear-face counterbores: three for the M4x12 flange-screw heads on
    // PCD42 and four for the M4x6 plate support-screw heads at (+-18,+-18).
    for(int i=0;i<3;++i) {
        const double a=i*2*std::numbers::pi/3;
        housing=cut(housing,cylY(x-21*std::sin(a),md::housing_y_max-4,zw+21*std::cos(a),3.75,4.01));
    }
    for(double sx:{-18.0,18.0})for(double sz:{-18.0,18.0})housing=cut(housing,cylY(x+sx,md::housing_y_max-4,zw+sz,3.75,4.01));
    const double arm_sx[2]={md::housing_x_min+10,md::housing_x_max-10},arm_sy[2]={md::arm_y_min+8,md::arm_y_max-8};
    for(double sx:arm_sx)for(double sy:arm_sy)housing=cut(housing,cylZ(sx,sy,zw+md::housing_half-12,2.5,12.01));
    add(parts,"M203","6082-T6 measuring bearing housing46x46x59.5;626 seats K6;coupling window",housing);
    gp_Trsf to_coupling;to_coupling.SetTranslation(gp_Vec(x,md::coupling_y,zw));
    add(parts,"CP01","Supplied6/6 flexible coupling15x22 interference envelope",BRepBuilderAPI_Transform(measuringCouplingInterface(),to_coupling,true).Shape(),true);
    gp_Trsf turn;turn.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),std::numbers::pi);
    gp_Trsf to_plate;to_plate.SetTranslation(gp_Vec(x,md::plate_y+6,zw));
    for(auto part:measuringEncoderMount()) {
        part.shape=BRepBuilderAPI_Transform(part.shape,to_plate*turn,true).Shape();
        parts.push_back(std::move(part));
    }
    int n=0;
    for(double sx:{-18.0,18.0})for(double sz:{-18.0,18.0})
        add(parts,"FB301-"+std::to_string(++n),"ISO4762 M4x6 class8.8 encoder plate support screw",
            fuse(cylY(x+sx,md::housing_y_max-4,zw+sz,3.5,4),cylY(x+sx,md::housing_y_max,zw+sz,1.99,6)),true);
    // Pivot arm on top of the housing.
    const double arm_z=zw+md::housing_half,pivot_z=arm_z+md::arm_t/2;
    auto arm=fuse(box(md::arm_x_min,md::arm_y_min,arm_z,md::arm_x_max-md::arm_x_min,md::arm_y_max-md::arm_y_min,md::arm_t),
                  cylY(md::pivot_x,md::arm_y_min,pivot_z,md::arm_hub_d/2,md::arm_y_max-md::arm_y_min));
    arm=cut(arm,cylY(md::pivot_x,md::arm_y_min-0.01,pivot_z,(msc::bearing_D_mm+fit)/2,md::arm_y_max-md::arm_y_min+0.02));
    n=0;
    for(double sx:arm_sx)for(double sy:arm_sy) {
        arm=cut(arm,cylZ(sx,sy,arm_z-0.01,2.75,md::arm_t+0.02));
        arm=cut(arm,cylZ(sx,sy,arm_z+md::arm_t-5,4.5,5.01));
        add(parts,"FB302-"+std::to_string(++n),"ISO4762 M5x16 class8.8 arm-to-housing screw",screwDown(sx,sy,arm_z+md::arm_t-5,5,head_M5,16),true);
    }
    arm=cut(arm,cylZ(md::arm_spring_x,(md::arm_y_min+md::arm_y_max)/2,arm_z+md::arm_t-8,3,8.01));
    add(parts,"M204","6082-T6 pivot arm100x40x12;hub30;626 seats K6",arm);
    for(int i=0;i<2;++i) {
        gp_Trsf t;t.SetTranslation(gp_Vec(md::pivot_x,i?md::arm_y_max-msc::bearing_width_mm:md::arm_y_min,pivot_z));
        add(parts,"AB0"+std::to_string(i+1),std::string(msc::bearing)+" pivot bearing external interface",BRepBuilderAPI_Transform(measuringBearingInterface(),t,true).Shape(),true);
    }
    add(parts,"DW02","ISO8734 dowel6m6x70 pivot pin",cylY(md::pivot_x,md::column_y_min,pivot_z,(6-fit)/2,md::column_y_max-md::column_y_min),true);
    // Bracket: base, slotted column, top lug with the contact-force adjuster.
    auto bracket=fuse(box(md::bracket_base_x_min,md::bracket_base_y_min,0,md::bracket_base_x_max-md::bracket_base_x_min,md::bracket_base_y_max-md::bracket_base_y_min,md::bracket_base_t),
                      box(md::column_x_min,md::column_y_min,md::bracket_base_t,md::column_x_max-md::column_x_min,md::column_y_max-md::column_y_min,md::lug_z_max-md::bracket_base_t));
    bracket=fuse(bracket,box(md::column_x_max,md::column_y_min,md::lug_z_min,md::lug_x_max-md::column_x_max,md::column_y_max-md::column_y_min,md::lug_z_max-md::lug_z_min));
    bracket=cut(bracket,box(md::column_x_min-0.01,md::column_slot_y_min,md::column_slot_z,md::column_x_max-md::column_x_min+0.02,md::column_slot_y_max-md::column_slot_y_min,md::lug_z_min-md::column_slot_z));
    bracket=cut(bracket,cylY(md::pivot_x,md::column_y_min-0.01,pivot_z,3,md::column_y_max-md::column_y_min+0.02));
    const double ay=(md::arm_y_min+md::arm_y_max)/2;
    bracket=cut(bracket,cylZ(md::arm_spring_x,ay,md::lug_z_min-0.01,3,md::lug_z_max-md::lug_z_min+0.02));
    const double bsx[2]={md::bracket_base_x_min+md::bracket_screw_inset,md::bracket_base_x_max-md::bracket_screw_inset},bsy[2]={md::bracket_base_y_min+md::bracket_screw_inset,md::bracket_base_y_max-md::bracket_screw_inset};
    n=0;
    for(double sx:bsx)for(double sy:bsy) {
        bracket=cut(bracket,cylZ(sx,sy,-0.01,4.5,md::bracket_base_t+0.02));
        bracket=cut(bracket,cylZ(sx,sy,md::bracket_base_t-8,7,8.01));
        add(parts,"FB303-"+std::to_string(++n),"ISO4762 M8x25 class8.8 bracket-to-bed screw",screwDown(sx,sy,md::bracket_base_t-8,8,head_M8,25),true);
    }
    add(parts,"M206","6082-T6 measuring bracket;base71x112x12;slotted column40x70;adjuster lug",bracket);
    const auto s=armSetting(d,p);
    const double la=s.spring_length_mm,arm_top=s.arm_top_z,cap_top=arm_top+la+5;
    add(parts,"DW03","ISO8734 dowel6m6x20 arm spring locator",cylZ(md::arm_spring_x,ay,arm_top-8,(6-fit)/2,20),true);
    add(parts,"SP02",std::string(mc::arm_spring)+" flat-wire compression spring D10.5 d6 L35 3.27N/mm;installed envelope",
        tubeZ(md::arm_spring_x,ay,arm_top,mc::arm_spring_D_mm/2,mc::arm_spring_d_mm/2,la),true);
    auto cap=fuse(cylZ(md::arm_spring_x,ay,arm_top+la,7,5),cylZ(md::arm_spring_x,ay,arm_top+la-6,2.9,6));
    cap=cut(cap,cylZ(md::arm_spring_x,ay,cap_top-1.5,3.5,1.51));
    add(parts,"M209","C45 arm spring cap14x5;spigot5.8x6;recess7x1.5",cap);
    add(parts,"AJ02","DIN913 M6x25 45H flat-point contact-force adjuster",cylZ(md::arm_spring_x,ay,cap_top-1.5,(6-thread)/2,25),true);
    add(parts,"NT02","ISO4032 M6 class8 lock nut;hex envelope",tubeZ(md::arm_spring_x,ay,md::lug_z_max,5.77,3,5.2),true);
    // Counter roller under the wire.
    const double zc=counterRollerZ(d),fork_z0=zc-20;
    auto column=fuse(box(md::roller_base_x_min,-md::roller_base_half_y,0,md::roller_base_x_max-md::roller_base_x_min,2*md::roller_base_half_y,md::roller_base_t),
                     box(md::roller_column_x_min,-md::roller_column_half_y,md::roller_base_t,md::roller_column_x_max-md::roller_column_x_min,2*md::roller_column_half_y,md::roller_column_z_max-md::roller_base_t));
    for(double y:{-md::fork_screw_y,md::fork_screw_y})column=cut(column,cylX(md::roller_column_x_max-12,y,md::fork_screw_z,2.5,12.01));
    const double csx[2]={md::roller_screw_x1,md::roller_screw_x2};
    n=0;
    for(double sx:csx)for(double sy:{-md::roller_screw_y,md::roller_screw_y}) {
        column=cut(column,cylZ(sx,sy,-0.01,3.5,md::roller_base_t+0.02));
        column=cut(column,cylZ(sx,sy,md::roller_base_t-6,5.5,6.01));
        add(parts,"FB304-"+std::to_string(++n),"ISO4762 M6x20 class8.8 roller column screw",screwDown(sx,sy,md::roller_base_t-6,6,head_M6,20),true);
    }
    add(parts,"M207","6082-T6 counter-roller column;base36x48x10;column16x20x90",column);
    auto fork=box(md::fork_x_min,-md::fork_half_y,fork_z0,md::fork_x_max-md::fork_x_min,2*md::fork_half_y,md::fork_z_max-md::fork_z_min);
    fork=cut(fork,box(md::fork_x_min-0.01,-md::fork_slot_half,fork_z0+6,md::fork_x_max-md::fork_x_min+0.02,2*md::fork_slot_half,md::fork_z_max-md::fork_z_min));
    fork=cut(fork,cylY(x,-md::fork_half_y-0.01,zc,3,2*md::fork_half_y+0.02));
    for(double y:{-md::fork_screw_y,md::fork_screw_y}) {
        const double z0=md::fork_screw_z-md::fork_slot_travel/2,z1=md::fork_screw_z+md::fork_slot_travel/2;
        fork=cut(fork,cylX(md::fork_x_min-0.01,y,z0,2.75,md::fork_x_max-md::fork_x_min+0.02));
        fork=cut(fork,cylX(md::fork_x_min-0.01,y,z1,2.75,md::fork_x_max-md::fork_x_min+0.02));
        fork=cut(fork,box(md::fork_x_min-0.01,y-2.75,z0,md::fork_x_max-md::fork_x_min+0.02,5.5,z1-z0));
        add(parts,"FB305-"+std::to_string(y>0?2:1),"ISO4762 M5x25 class8.8 fork clamp screw;vertical slot",screwMinusX(md::fork_x_max,y,md::fork_screw_z,5,head_M5,25),true);
    }
    add(parts,"M210","6082-T6 counter-roller fork18x19x28;slot6.4;4mm vertical adjustment",fork);
    add(parts,"DW04","ISO8734 dowel6m6x22 counter-roller pin",cylY(x,-md::fork_half_y,zc,(6-fit)/2,2*md::fork_half_y),true);
    gp_Trsf t;t.SetTranslation(gp_Vec(x,-msc::bearing_width_mm/2,zc));
    add(parts,"RB03",std::string(msc::bearing)+" counter roller;outer ring is the running surface",BRepBuilderAPI_Transform(measuringBearingInterface(),t,true).Shape(),true);
    return parts;
}

std::vector<HeadPart> frameGroup() {
    std::vector<HeadPart> parts;
    auto bed=box(-md::bed_half_x,-md::bed_half_y,-md::bed_t,2*md::bed_half_x,2*md::bed_half_y,md::bed_t);
    const auto tap=[&](double x,double y,double m,double depth){bed=cut(bed,cylZ(x,y,-depth,m/2,depth+0.01));};
    for(double x:{-75.0,75.0})for(double y:{-500.0,-250.0,250.0,500.0})tap(x,y,10,20);
    for(double sgn:{-1.0,1.0})for(double x:{md::bed_screw_x1,md::bed_screw_x2})for(double y:{md::bed_screw_y1,md::bed_screw_y2})tap(sgn*x,sgn*y,8,22);
    for(double x:{md::bracket_base_x_min+md::bracket_screw_inset,md::bracket_base_x_max-md::bracket_screw_inset})
        for(double y:{md::bracket_base_y_min+md::bracket_screw_inset,md::bracket_base_y_max-md::bracket_screw_inset})tap(x,y,8,22);
    for(double x:{md::roller_screw_x1,md::roller_screw_x2})for(double y:{-md::roller_screw_y,md::roller_screw_y})tap(x,y,6,18);
    const double bolt_y[4]={-md::rail_y,-170,170,md::rail_y};
    for(double x:{-md::rail_x,md::rail_x})for(double y:bolt_y) {
        bed=cut(bed,cylZ(x,y,-md::bed_t-0.01,5.5,md::bed_t+0.02));
        bed=cut(bed,cylZ(x,y,-11,8.5,11.01));
    }
    add(parts,"F301","EN AW-6082 T651 tooling plate bed640x1120x25;tapped H001/F201/M206/M207 patterns",bed);
    const double t=md::tube,w=md::tube_wall,z1=-md::bed_t,z0=z1-t;
    const auto tubeBox=[&](double x0,double y0,double zz0,double dx,double dy,double dz,bool alongZ){
        auto outer=box(x0,y0,zz0,dx,dy,dz);
        if(alongZ)return cut(outer,box(x0+w,y0+w,zz0-0.01,dx-2*w,dy-2*w,dz+0.02));
        if(dx>dy)return cut(outer,box(x0-0.01,y0+w,zz0+w,dx+0.02,dy-2*w,dz-2*w));
        return cut(outer,box(x0+w,y0-0.01,zz0+w,dx-2*w,dy+0.02,dz-2*w));
    };
    int n=0;
    for(double x:{-md::rail_x,md::rail_x}) {
        auto rail=tubeBox(x-t/2,-md::rail_y-t/2,z0,t,2*md::rail_y+t,t,false);
        for(double y:bolt_y)rail=cut(rail,cylZ(x,y,z1-w-0.01,5.5,w+0.02));
        add(parts,"F302-"+std::to_string(++n),"S235JR square tube60x60x3 L1080 long rail;M10 weld nuts under top wall",rail);
    }
    n=0;
    for(double y:{-md::rail_y,md::rail_y})
        add(parts,"F303-"+std::to_string(++n),"S235JR square tube60x60x3 L440 cross rail",tubeBox(-md::rail_x+t/2,y-t/2,z0,2*md::rail_x-t,t,t,false));
    n=0;
    for(double x:{-md::rail_x,md::rail_x})for(double y:{-md::rail_y,md::rail_y}) {
        ++n;
        add(parts,"F304-"+std::to_string(n),"S235JR square tube60x60x3 L760 leg",tubeBox(x-t/2,y-t/2,z0-md::leg_length,t,t,md::leg_length,true));
        auto foot=box(x-md::foot/2,y-md::foot/2,z0-md::leg_length-md::foot_t,md::foot,md::foot,md::foot_t);
        for(double ax:{-35.0,35.0})for(double ay:{-35.0,35.0})foot=cut(foot,cylZ(x+ax,y+ay,z0-md::leg_length-md::foot_t-0.01,6.5,md::foot_t+0.02));
        add(parts,"F305-"+std::to_string(n),"S235JR foot plate100x100x8;4x13 anchor holes",foot);
    }
    n=0;
    for(double x:{-md::rail_x,md::rail_x})for(double y:bolt_y) {
        ++n;
        add(parts,"FB306-"+std::to_string(n),"ISO4762 M10x35 class8.8 bed-to-frame screw",screwDown(x,y,-11,10,head_M10,35),true);
        add(parts,"NT03-"+std::to_string(n),"DIN929 M10 hexagon weld nut;envelope",tubeZ(x,y,z1-w-8,8.5,5,8),true);
    }
    return parts;
}

std::vector<HeadPart> guardGroup(bool block_wire_passage) {
    std::vector<HeadPart> parts;
    const double side_length=md::guard_side_y_max-md::guard_side_y_min;
    const double wall_height=md::guard_wall_z_max-md::guard_wall_z_min;
    const double panel_x_min=-md::guard_inner_half_x-md::guard_wall_t;
    auto side_panel=[&](double x) {
        auto panel=box(x,md::guard_side_y_min,md::guard_wall_z_min,md::guard_wall_t,side_length,wall_height);
        if(!block_wire_passage)
            panel=cut(panel,cylX(x-0.01,0,fd::wire_axis_z,md::guard_wire_passage_d/2,md::guard_wall_t+0.02));
        return panel;
    };
    add(parts,"G101","R2 selected DC01 2mm powder-coated left guard panel;30mm wire pass-through;not a safety-gap design",side_panel(panel_x_min));
    add(parts,"G102","R2 selected DC01 2mm powder-coated right guard panel;30mm wire pass-through;not a safety-gap design",side_panel(md::guard_inner_half_x));
    add(parts,"G103","R2 selected DC01 2mm powder-coated fixed rear guard panel",box(-md::guard_inner_half_x,md::guard_rear_y_min,md::guard_wall_z_min,
        2*md::guard_inner_half_x,md::guard_rear_y_max-md::guard_rear_y_min,wall_height));
    add(parts,"G104","R2 selected DC01 2mm powder-coated guard roof",box(-md::guard_inner_half_x,md::guard_side_y_min,md::guard_roof_z_min,
        2*md::guard_inner_half_x,side_length,md::guard_roof_z_max-md::guard_roof_z_min));
    auto door=box(-md::guard_inner_half_x,md::guard_front_y_min,md::guard_wall_z_min,2*md::guard_inner_half_x,
        md::guard_front_y_max-md::guard_front_y_min,wall_height);
    // This aperture reserves only the listed actuator envelope. It is not a
    // manufacturer mounting drawing and no door/actuator engagement is modeled.
    door=cut(door,box(md::guard_actuator_x_min-md::guard_lock_pocket_clearance,md::guard_front_y_min-0.01,
        md::guard_actuator_z_min-md::guard_lock_pocket_clearance,md::guard_actuator_x+2*md::guard_lock_pocket_clearance,
        md::guard_front_y_max-md::guard_front_y_min+0.02,md::guard_actuator_z+2*md::guard_lock_pocket_clearance));
    add(parts,"G105","R2 selected 5mm polycarbonate closed front door;actuator aperture only;no kinematic or safety validation",door);
    auto lock_plate=box(md::guard_lock_plate_x_min,md::guard_lock_plate_y_min,md::guard_lock_plate_z_min,
        md::guard_lock_plate_x_max-md::guard_lock_plate_x_min,md::guard_lock_plate_y_max-md::guard_lock_plate_y_min,
        md::guard_lock_plate_z_max-md::guard_lock_plate_z_min);
    lock_plate=cut(lock_plate,box(md::guard_lock_body_x_min-md::guard_lock_pocket_clearance,
        md::guard_lock_body_y_min-md::guard_lock_pocket_clearance,md::guard_lock_body_z_min-md::guard_lock_pocket_clearance,
        md::guard_lock_body_x+2*md::guard_lock_pocket_clearance,md::guard_lock_body_y+md::guard_lock_pocket_clearance+5,
        md::guard_lock_body_z+2*md::guard_lock_pocket_clearance));
    add(parts,"G106","R2 selected S235JR 5mm guard-lock mounting plate;body pocket only;SLC drilling pattern requires manufacturer drawing",lock_plate);
    add(parts,"G107","Bernstein SLC-F-024-20/20-R4 30x192x44 catalogue envelope;static fit only",box(md::guard_lock_body_x_min,
        md::guard_lock_body_y_min,md::guard_lock_body_z_min,md::guard_lock_body_x,md::guard_lock_body_y,md::guard_lock_body_z),true);
    add(parts,"G108","Bernstein ACF-1 60x40x52.6 catalogue envelope;static fit only",box(md::guard_actuator_x_min,
        md::guard_actuator_y_min,md::guard_actuator_z_min,md::guard_actuator_x,md::guard_actuator_y,md::guard_actuator_z),true);
    return parts;
}

std::vector<HeadPart> payoffGroup() {
    std::vector<HeadPart> parts;
    const double flange_y_lo=md::payoff_spool_y_min;
    const double flange_y_hi=md::payoff_spool_y_min+md::payoff_spool_y_length;
    const double bore_r=md::payoff_shaft_r+md::payoff_shaft_clearance;
    auto flange=[&](double y0) {
        auto disc=cylY(md::payoff_spool_x,y0,md::payoff_spool_z,md::payoff_spool_r,md::payoff_flange_t);
        return cut(disc,cylY(md::payoff_spool_x,y0-0.01,md::payoff_spool_z,bore_r,md::payoff_flange_t+0.02));
    };
    // The hub tube passes through both flanges (welded joint): real volume
    // overlap keeps the fused spool a single connected solid.
    auto hub=cut(cylY(md::payoff_spool_x,flange_y_lo,md::payoff_spool_z,md::payoff_hub_r,md::payoff_spool_y_length),
                 cylY(md::payoff_spool_x,flange_y_lo-0.01,md::payoff_spool_z,md::payoff_hub_r-md::payoff_hub_wall,
                  md::payoff_spool_y_length+0.02));
    BRepAlgoAPI_Fuse spool_fuse(flange(flange_y_lo),hub);spool_fuse.Build();
    if(!spool_fuse.IsDone())throw std::runtime_error("Payoff spool hub fuse failed");
    BRepAlgoAPI_Fuse spool_all(spool_fuse.Shape(),flange(flange_y_hi));spool_all.Build();
    if(!spool_all.IsDone())throw std::runtime_error("Payoff spool flange fuse failed");
    add(parts,"P001","R2 selected welded payoff spool OD400x160;3mm S235JR flanges with 20.5 bores;60x2.5 ERW hub;not load-qualified",spool_all.Shape());
    add(parts,"P002","R2 selected C45 20h6 payoff spindle L660;M20x1.5 ends not modeled;bearing pockets in P004/P005",cylY(md::payoff_spool_x,
        md::payoff_shaft_y_min,md::payoff_spool_z,md::payoff_shaft_r,md::payoff_shaft_y_max-md::payoff_shaft_y_min));
    add(parts,"P003","R2 selected S235JR payoff base 430x560x8;4x13 anchor holes;selected not load-qualified",box(md::payoff_base_x_min,
        md::payoff_base_y_min,md::payoff_base_z_min,md::payoff_base_x_max-md::payoff_base_x_min,
        md::payoff_base_y_max-md::payoff_base_y_min,md::payoff_base_z_max-md::payoff_base_z_min));
    auto post=[&](double inner_face,double outer_face) {
        const double y0=std::min(inner_face,outer_face);
        auto p=box(md::payoff_post_x_min,y0,md::payoff_post_z_min,
                   md::payoff_post_x_max-md::payoff_post_x_min,md::payoff_post_t,
                   md::payoff_post_z_max-md::payoff_post_z_min);
        p=cut(p,cylY(md::payoff_spool_x,y0-0.01,md::payoff_spool_z,bore_r,md::payoff_post_t+0.02));
        const double dir=(inner_face<0)?-1.0:1.0;
        const double pocket_lo=std::min(inner_face,inner_face+dir*md::payoff_bearing_w);
        p=cut(p,cylY(md::payoff_spool_x,pocket_lo,md::payoff_spool_z,md::payoff_bearing_r,md::payoff_bearing_w));
        return p;
    };
    add(parts,"P004","R2 selected S235JR 18mm left payoff post;47H7 6204 pocket and 20.5 spindle bore;weld and load path not validated",
        post(md::payoff_post_y1_inner,md::payoff_post_y1_outer));
    add(parts,"P005","R2 selected S235JR 18mm right payoff post;47H7 6204 pocket and 20.5 spindle bore;weld and load path not validated",
        post(md::payoff_post_y2_inner,md::payoff_post_y2_outer));
    auto disc=cut(cylY(md::payoff_spool_x,md::payoff_disc_y_min,md::payoff_spool_z,md::payoff_disc_r,md::payoff_disc_t),
                  cylY(md::payoff_spool_x,md::payoff_disc_y_min-0.01,md::payoff_spool_z,md::payoff_shaft_r+0.02,md::payoff_disc_t+0.02));
    add(parts,"P006","R2 selected S235JR brake disc 180x6;20H7 bore;keyway and setscrew are fabrication details;friction brake not safety brake",disc);
    add(parts,"P007","R2 selected brake caliper bracket envelope;pads and SWU-class adjuster spring not modeled;tension brake not safety brake",
        box(md::payoff_caliper_x_min,md::payoff_caliper_y_min,md::payoff_caliper_z_min,
            md::payoff_caliper_x_max-md::payoff_caliper_x_min,md::payoff_caliper_y_max-md::payoff_caliper_y_min,
            md::payoff_caliper_z_max-md::payoff_caliper_z_min));
    return parts;
}

void checkMechanismSolids(const std::vector<HeadPart>& parts) {
    for(const auto& p:parts) {
        if(p.shape.IsNull()||!BRepCheck_Analyzer(p.shape).IsValid()||volume(p.shape)<=0)throw std::runtime_error("Invalid mechanism solid: "+p.id);
        int n=0;for(TopExp_Explorer e(p.shape,TopAbs_SOLID);e.More();e.Next())++n;
        if(n==0||(!p.purchased&&n!=1))throw std::runtime_error("Disconnected mechanism part: "+p.id);
    }
}
void checkNoInterference(const std::vector<HeadPart>& a,const std::vector<HeadPart>& b,const char* label) {
    const bool self=&a==&b;
    std::vector<BoxBounds> A,B;A.reserve(a.size());
    for(const auto& p:a)A.push_back(bounds(p.shape));
    if(self)B=A;else{B.reserve(b.size());for(const auto& p:b)B.push_back(bounds(p.shape));}
    for(std::size_t i=0;i<a.size();++i)for(std::size_t j=self?i+1:0;j<b.size();++j) {
        const auto& x=A[i];const auto& y=B[j];
        if(x.xmax<=y.xmin+1e-7||y.xmax<=x.xmin+1e-7||x.ymax<=y.ymin+1e-7||y.ymax<=x.ymin+1e-7||x.zmax<=y.zmin+1e-7||y.zmax<=x.zmin+1e-7)continue;
        BRepAlgoAPI_Common common(a[i].shape,b[j].shape);common.Build();
        if(!common.IsDone())throw std::runtime_error(std::string("Interference boolean failed: ")+label+" "+a[i].id+" / "+b[j].id);
        const double v=volume(common.Shape());
        if(v>1e-4)throw std::runtime_error(std::string("Interference ")+label+" "+a[i].id+" / "+b[j].id+": "+std::to_string(v)+" mm3");
    }
}
}
