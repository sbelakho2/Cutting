#include "measurement_geometry.hpp"
#include "shared/measurement_spec.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopoDS.hxx>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace starz::engineering {
namespace {
namespace c=measurement::catalogue;
namespace d=measurement::design;
TopoDS_Shape cylinder(double x,double y,double z,double radius,double length) {
    return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,y,z),gp_Dir(0,1,0)),radius,length).Shape();
}
TopoDS_Shape cut(const TopoDS_Shape& a,const TopoDS_Shape& b) {
    BRepAlgoAPI_Cut operation(a,b);operation.Build();
    if(!operation.IsDone())throw std::runtime_error("Measuring interface cut failed");
    operation.SimplifyResult();return operation.Shape();
}
TopoDS_Shape fuse(const TopoDS_Shape& a,const TopoDS_Shape& b) {
    BRepAlgoAPI_Fuse operation(a,b);operation.Build();
    if(!operation.IsDone())throw std::runtime_error("Measuring interface union failed");
    operation.SimplifyResult();return operation.Shape();
}
void solid(const TopoDS_Shape& shape,const char* label) {
    if(shape.IsNull()||!BRepCheck_Analyzer(shape).IsValid()||volume(shape)<=0)throw std::runtime_error(label);
    int count=0;for(TopExp_Explorer explorer(shape,TopAbs_SOLID);explorer.More();explorer.Next())++count;
    if(count!=1)throw std::runtime_error("Measuring interface must be a single connected solid");
}
void near(double actual,double expected,const char* message) {
    if(std::abs(actual-expected)>1e-5)throw std::runtime_error(message);
}
void state(const TopoDS_Shape& shape,const gp_Pnt& point,TopAbs_State expected,const char* message) {
    BRepClass3d_SolidClassifier classifier(shape,point,1e-7);
    if(classifier.State()!=expected)throw std::runtime_error(message);
}
}
TopoDS_Shape measuringWheelInterface(double radial_thread_d_mm) {
    if(!std::isfinite(radial_thread_d_mm)||radial_thread_d_mm<=0||radial_thread_d_mm>5)
        throw std::invalid_argument("Invalid wheel thread diameter");
    auto wheel=cylinder(0,0,0,c::wheel_od_mm/2,c::wheel_tread_mm);
    wheel=fuse(wheel,cylinder(0,c::wheel_tread_mm,0,c::wheel_hub_od_mm/2,c::wheel_overall_mm-c::wheel_tread_mm-c::wheel_end_land_mm));
    wheel=fuse(wheel,cylinder(0,c::wheel_overall_mm-c::wheel_end_land_mm,0,c::wheel_end_hub_od_mm/2,c::wheel_end_land_mm));
    wheel=cut(wheel,cylinder(0,-0.01,0,c::wheel_bore_mm/2,c::wheel_overall_mm+0.02));
    // Four radial openings are depicted in the front/section drawing. The
    // No.123 text calls out M4x6 but does not separately give screw quantity.
    // Threads are represented at major diameter; no rubber bond/core geometry
    // or unspecified edge chamfers are fabricated from the drawing artwork.
    for(int i=0;i<4;++i) {
        const double angle=i*std::numbers::pi/2;
        wheel=cut(wheel,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,c::wheel_overall_mm-c::wheel_screw_from_end_mm,0),
            gp_Dir(std::cos(angle),0,std::sin(angle))),radial_thread_d_mm/2,c::wheel_hub_od_mm/2+0.01).Shape());
    }
    return wheel;
}
TopoDS_Shape measuringEncoderInterface(double face_to_tip_mm) {
    if(!std::isfinite(face_to_tip_mm)||face_to_tip_mm<=c::encoder_pilot_length_mm)
        throw std::invalid_argument("Invalid encoder shaft tip station");
    auto encoder=cylinder(0,-c::encoder_body_length_mm,0,c::encoder_body_od_mm/2,c::encoder_body_length_mm);
    encoder=fuse(encoder,cylinder(0,0,0,c::encoder_pilot_od_mm/2,c::encoder_pilot_length_mm));
    const double shaft_radius=(c::encoder_shaft_min_mm+c::encoder_shaft_max_mm)/4;
    encoder=fuse(encoder,cylinder(0,c::encoder_pilot_length_mm,0,shaft_radius,face_to_tip_mm-c::encoder_pilot_length_mm));
    const double flat_x=c::encoder_flat_across_mm-shaft_radius;
    encoder=cut(encoder,BRepPrimAPI_MakeBox(gp_Pnt(flat_x,face_to_tip_mm-c::encoder_flat_length_mm,-4),4,c::encoder_flat_length_mm+0.01,8).Shape());
    for(int i=0;i<3;++i) {
        const double angle=i*2*std::numbers::pi/3;
        encoder=cut(encoder,cylinder(15*std::sin(angle),-c::encoder_mount_thread_depth_mm,15*std::cos(angle),1.5,c::encoder_mount_thread_depth_mm+0.01));
    }
    return encoder;
}
TopoDS_Shape measuringCouplingInterface() {
    auto shape=cut(cylinder(0,0,0,c::coupling_od_mm/2,c::coupling_length_mm),cylinder(0,-0.01,0,3,c::coupling_length_mm+0.02));
    for(double y:{c::coupling_screw_from_end_mm,c::coupling_length_mm-c::coupling_screw_from_end_mm})for(int i=0;i<2;++i) {
        const double angle=i*std::numbers::pi/2;
        shape=cut(shape,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,y,0),gp_Dir(std::cos(angle),0,std::sin(angle))),1.5,7.6).Shape());
    }
    // The cylinder is an OUTER/BORE envelope, not a solid replacement coupling.
    // Flexure cuts/inner transitions are not dimensioned by this source.
    return shape;
}
TopoDS_Shape measuringBearingInterface() {
    return cut(cylinder(0,0,0,c::bearing_D_mm/2,c::bearing_width_mm),cylinder(0,-0.01,0,c::bearing_d_mm/2,c::bearing_width_mm+0.02));
}
TopoDS_Shape measuringEncoderPlate() {
    // M201: mounting faceA atY0; purchased encoder pilot axisB atX0/Z0.
    const double half=d::encoder_plate_width_mm/2,thickness=d::encoder_plate_thickness_mm;
    auto plate=BRepPrimAPI_MakeBox(gp_Pnt(-half,0,-half),2*half,thickness,2*half).Shape();
    plate=cut(plate,cylinder(0,-0.01,0,d::encoder_plate_pilot_mm/2,thickness+0.02));
    for(int i=0;i<3;++i) {
        const double angle=i*2*std::numbers::pi/3;
        plate=cut(plate,cylinder(c::encoder_mount_pcd_mm/2*std::sin(angle),-0.01,c::encoder_mount_pcd_mm/2*std::cos(angle),
            d::encoder_plate_encoder_clearance_mm/2,thickness+0.02));
    }
    for(double x:{-d::encoder_plate_outer_grid_mm/2,d::encoder_plate_outer_grid_mm/2})
        for(double z:{-d::encoder_plate_outer_grid_mm/2,d::encoder_plate_outer_grid_mm/2})
            plate=cut(plate,cylinder(x,-0.01,z,d::encoder_plate_support_clearance_mm/2,thickness+0.02));
    BRepFilletAPI_MakeChamfer chamfer(plate);
    TopTools_IndexedDataMapOfShapeListOfShape faces;
    TopExp::MapShapesAndAncestors(plate,TopAbs_EDGE,TopAbs_FACE,faces);
    for(int i=1;i<=faces.Extent();++i) {
        const auto edge=TopoDS::Edge(faces.FindKey(i));const auto& adjacent=faces.FindFromIndex(i);
        if(adjacent.Extent()!=2||adjacent.First().IsSame(adjacent.Last()))continue;
        if(BRep_Tool::IsClosed(edge,TopoDS::Face(adjacent.First()))||BRep_Tool::IsClosed(edge,TopoDS::Face(adjacent.Last())))continue;
        chamfer.Add(d::encoder_plate_edge_chamfer_mm,edge);
    }
    chamfer.Build();
    if(!chamfer.IsDone()||!BRepCheck_Analyzer(chamfer.Shape()).IsValid())throw std::runtime_error("Encoder plate edge chamfers failed");
    return chamfer.Shape();
}
std::vector<HeadPart> measuringEncoderMount() {
    std::vector<HeadPart> parts{{"ME-E01","E40S6-1024-6-L-5 catalogue face/shaft envelope;not original OEM CAD",measuringEncoderInterface(),true,false},
                              {"M201","6082-T6 encoder mounting plate46x46x6;shaft axisY",measuringEncoderPlate(),false,false}};
    for(int i=0;i<3;++i) {
        const double angle=i*2*std::numbers::pi/3,x=15*std::sin(angle),z=15*std::cos(angle);
        const auto washer=cut(cylinder(x,6,z,3,0.5),cylinder(x,5.99,z,1.6,0.52));
        auto screw=fuse(cylinder(x,-3.5,z,1.5,10),cylinder(x,6.5,z,2.75,3));
        // Socket/threads are not a fastener manufacturing definition. The
        // 3.5mm nominal engagement and full external head/washer are checked.
        parts.push_back({"ME-W"+std::to_string(i+1),"ISO7092 M3 200HV washer3.2x6x0.5",washer,true,false});
        parts.push_back({"ME-S"+std::to_string(i+1),"ISO4762 M3x10 class8.8;nominal3.5mm encoder engagement",screw,true,false});
    }
    return parts;
}
void checkMeasuringInterfaces(const TopoDS_Shape& wheel,const TopoDS_Shape& encoder) {
    solid(wheel,"Invalid measuring wheel interface");solid(encoder,"Invalid encoder interface");
    const auto w=bounds(wheel),e=bounds(encoder);
    near(w.ymin,0,"Wheel front datum mismatch");near(w.ymax,14.5,"Wheel overall width mismatch");
    near(w.xmin,-15.915,"Wheel outside diameter mismatch");near(w.xmax,15.915,"Wheel outside diameter mismatch");
    near(e.ymin,-31,"Encoder body length mismatch");near(e.ymax,20,"Encoder tip must be20mm from mounting face");
    state(wheel,gp_Pnt(0,4,0),TopAbs_OUT,"Wheel6mm bore missing");
    state(wheel,gp_Pnt(3.1,4,0),TopAbs_IN,"Wheel bore oversized");
    state(wheel,gp_Pnt(5,14.2,0),TopAbs_OUT,"Wheel9mm end land missing");
    state(wheel,gp_Pnt(1.75,12,7),TopAbs_OUT,"No123 wheel requiresM4 notM3 radial holes");
    state(wheel,gp_Pnt(2.05,12,7),TopAbs_IN,"Wheel radial thread envelope oversized");
    state(encoder,gp_Pnt(9.9,4.9,0),TopAbs_IN,"Encoder5mm pilot missing");
    state(encoder,gp_Pnt(9.9,5.1,0),TopAbs_OUT,"Encoder pilot too long");
    state(encoder,gp_Pnt(2.5,9,0),TopAbs_IN,"Encoder full round shaft section missing");
    state(encoder,gp_Pnt(2.5,11,0),TopAbs_OUT,"Encoder single shaft flat missing");
    state(encoder,gp_Pnt(-2.5,11,0),TopAbs_IN,"Encoder shaft incorrectly double flattened");
    solid(measuringCouplingInterface(),"Invalid encoder coupling interface");
    solid(measuringBearingInterface(),"Invalid626 bearing interface");
}
void checkMeasuringEncoderMount(const std::vector<HeadPart>& parts) {
    if(parts.size()!=8)throw std::runtime_error("Encoder mount requires8 occurrences");
    for(const auto& part:parts)solid(part.shape,"Invalid encoder mounting part");
    for(std::size_t i=0;i<parts.size();++i)for(std::size_t j=i+1;j<parts.size();++j) {
        const auto a=bounds(parts[i].shape),b=bounds(parts[j].shape);
        if(a.xmax<=b.xmin+1e-7||b.xmax<=a.xmin+1e-7||a.ymax<=b.ymin+1e-7||b.ymax<=a.ymin+1e-7||
           a.zmax<=b.zmin+1e-7||b.zmax<=a.zmin+1e-7)continue;
        BRepAlgoAPI_Common common(parts[i].shape,parts[j].shape);common.Build();
        if(!common.IsDone())throw std::runtime_error("Encoder mount interference check failed");
        if(volume(common.Shape())>1e-4)throw std::runtime_error("Encoder mount interference: "+parts[i].id+" / "+parts[j].id);
    }
}
}