#include "feed_geometry.hpp"
#include "shared/feed_drive_spec.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace starz::engineering {
namespace {
namespace c=feed::catalogue;
namespace d=feed::design;
TopoDS_Shape cylinder(double radius,double y,double length) {
    return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,y,0),gp_Dir(0,1,0)),radius,length).Shape();
}
TopoDS_Shape box(double x,double y,double z,double a,double b,double h) {
    return BRepPrimAPI_MakeBox(gp_Pnt(x,y,z),a,b,h).Shape();
}
TopoDS_Shape cut(const TopoDS_Shape& a,const TopoDS_Shape& b) {
    BRepAlgoAPI_Cut operation(a,b);operation.Build();
    if(!operation.IsDone())throw std::runtime_error("Feed boolean cut failed");
    operation.SimplifyResult();return operation.Shape();
}
TopoDS_Shape fuse(const TopoDS_Shape& a,const TopoDS_Shape& b) {
    BRepAlgoAPI_Fuse operation(a,b);operation.Build();
    if(!operation.IsDone())throw std::runtime_error("Feed boolean union failed");
    operation.SimplifyResult();return operation.Shape();
}
TopoDS_Shape tube(double outer,double inner,double y,double length) {
    return cut(cylinder(outer,y,length),cylinder(inner,y-0.01,length+0.02));
}
TopoDS_Shape capsule(double width,double length,double y,double z,double height) {
    const double r=width/2;
    auto shape=box(-r,y+r,z,width,length-width,height);
    for(double end:{y+r,y+length-r})shape=fuse(shape,
        BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,end,z),gp_Dir(0,0,1)),r,height).Shape());
    return shape;
}
TopoDS_Shape roundCircularEdge(const TopoDS_Shape& shape,double station,double edgeRadius,double rootRadius) {
    BRepFilletAPI_MakeFillet fillet(shape);int found=0;
    TopTools_IndexedMapOfShape edges;TopExp::MapShapes(shape,TopAbs_EDGE,edges);
    for(int i=1;i<=edges.Extent();++i) {
        const auto edge=TopoDS::Edge(edges(i));const BRepAdaptor_Curve curve(edge);
        if(curve.GetType()!=GeomAbs_Circle)continue;
        const auto circle=curve.Circle();const auto centre=circle.Location();
        if(std::abs(centre.X())<1e-6&&std::abs(centre.Z())<1e-6&&std::abs(centre.Y()-station)<1e-6&&
           std::abs(circle.Radius()-edgeRadius)<1e-6) {fillet.Add(rootRadius,edge);++found;}
    }
    if(found!=1)throw std::runtime_error("Expected one circular machining edge atS"+std::to_string(station));
    fillet.Build();
    if(!fillet.IsDone()||!BRepCheck_Analyzer(fillet.Shape()).IsValid())throw std::runtime_error("Shaft/bearing circular fillet failed");
    return fillet.Shape();
}
TopoDS_Shape roundKeyFloor(const TopoDS_Shape& shape,double floorZ) {
    BRepFilletAPI_MakeFillet fillet(shape);int found=0;
    TopTools_IndexedMapOfShape edges;TopExp::MapShapes(shape,TopAbs_EDGE,edges);
    for(int i=1;i<=edges.Extent();++i) {
        const auto b=bounds(edges(i));
        if(std::abs(b.zmin-floorZ)<1e-6&&std::abs(b.zmax-floorZ)<1e-6) {
            fillet.Add(d::key_floor_radius,TopoDS::Edge(edges(i)));++found;
        }
    }
    if(found<3)throw std::runtime_error("Key-seat floor boundary missing");
    fillet.Build();if(!fillet.IsDone()||!BRepCheck_Analyzer(fillet.Shape()).IsValid())throw std::runtime_error("Key-seat floor fillet failed");
    return fillet.Shape();
}
TopoDS_Shape turnedChamfer(const TopoDS_Shape& shape,double station,double firstRadius,double lastRadius,double length) {
    // An explicit conical turning envelope handles the external thread lead
    // which consumes the full 0.5mm radial step. Generic edge chamfer builders
    // cannot reliably remove that entire supporting planar annulus.
    const auto kept=BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,station,0),gp_Dir(0,1,0)),
                                        firstRadius,lastRadius,length).Shape();
    const auto tool=cut(cylinder(std::max(firstRadius,lastRadius)+0.1,station,length),kept);
    const auto result=cut(shape,tool);
    if(!BRepCheck_Analyzer(result).IsValid())throw std::runtime_error("Turned chamfer failed atS"+std::to_string(station));
    return result;
}
TopoDS_Shape washerSlotTool() {
    const double half=d::washer_slot_width/2;
    const auto raw=box(-half,d::washer_slot_start,d::washer_slot_floor,d::washer_slot_width,
                       d::washer_slot_end-d::washer_slot_start,5);
    BRepFilletAPI_MakeFillet fillet(raw);int found=0;
    TopTools_IndexedMapOfShape edges;TopExp::MapShapes(raw,TopAbs_EDGE,edges);
    for(int i=1;i<=edges.Extent();++i) {
        const auto b=bounds(edges(i));
        if(b.xmax-b.xmin<1e-6&&b.ymax-b.ymin<1e-6&&b.zmax-b.zmin>4.9) {
            fillet.Add(d::washer_slot_plan_radius,TopoDS::Edge(edges(i)));++found;
        }
    }
    if(found!=4)throw std::runtime_error("Washer slot plan corner count mismatch");
    fillet.Build();
    if(!fillet.IsDone()||!BRepCheck_Analyzer(fillet.Shape()).IsValid())throw std::runtime_error("Washer slot plan radii failed");
    return fillet.Shape();
}
TopoDS_Shape pulleyAndWasherTool() {
    const auto tool=fuse(washerSlotTool(),capsule(5.04,28.10,33.45,4,8));
    // Build all meeting radii together. Sequential vertical-then-floor
    // filleting splits the lip at their common vertices and produces invalid
    // contours in OCC7.9.3. The cutter also handles the interrupted rear entry
    // without requiring a fillet around a vanishing shaft wall.
    BRepFilletAPI_MakeFillet fillet(tool);int vertical=0,slotFloor=0,keyFloor=0;
    TopTools_IndexedMapOfShape edges;TopExp::MapShapes(tool,TopAbs_EDGE,edges);
    for(int i=1;i<=edges.Extent();++i) {
        const auto edge=TopoDS::Edge(edges(i));const auto b=bounds(edge);
        if(BRepAdaptor_Curve(edge).GetType()==GeomAbs_Line&&b.xmax-b.xmin<1e-6&&b.ymax-b.ymin<1e-6&&
           std::abs(std::abs(b.xmin)-d::washer_slot_width/2)<1e-6&&
           b.ymin>33.45&&b.ymax<61.55&&b.zmax-b.zmin>0.5) {
            fillet.Add(d::washer_slot_intersection_radius,edge);++vertical;
        }
        if(std::abs(b.zmin-d::washer_slot_floor)<1e-6&&std::abs(b.zmax-d::washer_slot_floor)<1e-6&&
           b.ymin>=d::washer_slot_start-1e-6&&b.ymax<=d::washer_slot_end+1e-6) {
            fillet.Add(d::washer_slot_floor_radius,edge);++slotFloor;
        }
        if(std::abs(b.zmin-4)<1e-6&&std::abs(b.zmax-4)<1e-6) {
            fillet.Add(d::key_floor_radius,edge);++keyFloor;
        }
    }
    if(vertical!=4||slotFloor!=12||keyFloor!=4)throw std::runtime_error("Combined pocket machining edge count mismatch");
    fillet.Build();
    if(!fillet.IsDone()||!BRepCheck_Analyzer(fillet.Shape()).IsValid())throw std::runtime_error("Combined pocket cutter blends failed");
    return fillet.Shape();
}
TopoDS_Shape chamferKeyInside(const TopoDS_Shape& shape,double insideZ) {
    BRepFilletAPI_MakeChamfer chamfer(shape);int found=0;
    TopTools_IndexedMapOfShape edges;TopExp::MapShapes(shape,TopAbs_EDGE,edges);
    for(int i=1;i<=edges.Extent();++i) {
        const auto b=bounds(edges(i));
        if(std::abs(b.zmin-insideZ)<1e-6&&std::abs(b.zmax-insideZ)<1e-6) {
            chamfer.Add(d::key_inside_chamfer,TopoDS::Edge(edges(i)));++found;
        }
    }
    if(found<3)throw std::runtime_error("Key inner-face boundary missing");
    chamfer.Build();if(!chamfer.IsDone()||!BRepCheck_Analyzer(chamfer.Shape()).IsValid())throw std::runtime_error("Key inner-edge chamfer failed");
    return chamfer.Shape();
}
TopoDS_Shape originalFeedMotor() {
    static const TopoDS_Shape shape=[] {
        STEPControl_Reader reader;
        const std::filesystem::path path=std::filesystem::path(STARZ_SOURCE_DIR)/"engineering/sources/24HS40-5004D-E1000.step";
        if(reader.ReadFile(path.c_str())!=IFSelect_RetDone||reader.TransferRoots()<1)
            throw std::runtime_error("Original feed motor STEP import failed");
        gp_Trsf turn;turn.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(1,0,0)),-std::numbers::pi/2);
        gp_Trsf shift;shift.SetTranslation(gp_Vec(0,d::motor_face,0));
        return BRepBuilderAPI_Transform(reader.OneShape(),shift*turn,true).Shape();
    }();
    return shape;
}
TopoDS_Shape pulley() {
    auto p=cylinder(c::pulley_od/2,d::pulley_start,c::pulley_width);
    // Full flange envelopes include the swaged end regions. Tooth grooves are
    // purchased features, not a tooth-cutting drawing or a generated OEM STEP.
    p=fuse(p,cylinder(c::pulley_flange_d/2,d::pulley_start,2.5));
    p=fuse(p,cylinder(c::pulley_flange_d/2,d::pulley_start+c::pulley_width-2.5,2.5));
    p=cut(p,cylinder(c::pulley_bore/2,d::pulley_start-0.1,c::pulley_width+0.2));
    return cut(p,box(-2.5,d::pulley_start-0.1,0,5,c::pulley_width+0.2,9.3));
}
TopoDS_Shape bearingCover() {
    auto p=cut(box(-23,70.98,-18.25,46,2,37.25),cylinder(14.9,70.9,2.2));
    for(double x:{-19.0,19.0})for(double z:{-12.5,12.5})
        p=cut(p,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,70.9,z),gp_Dir(0,1,0)),1.65,2.2).Shape());
    return p;
}
TopoDS_Shape motorPlate() {
    auto p=box(-40,d::motor_face,-40,80,10,80);
    p=cut(p,cylinder(19.09,d::motor_face-0.01,10.02));
    p=cut(p,cylinder(21.25,d::motor_face+1.95,8.06));
    for(double x:{-25.0,25.0})for(double z:{-25.0,25.0})
        p=cut(p,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::motor_face-0.1,z),gp_Dir(0,1,0)),2.25,10.2).Shape());
    return p;
}
TopoDS_Shape coupling() {
    auto p=cylinder(20,d::coupling_start,50);
    p=cut(p,cylinder(5,d::coupling_start-0.01,16.01));
    p=cut(p,cylinder(14,d::coupling_start+16,18));
    return cut(p,cylinder(6,d::coupling_end-16,16.01));
}
TopoDS_Shape nut() {
    auto p=tube(12.5,7.5,66,5);
    for(int i=0;i<4;++i) {
        gp_Trsf rotation;rotation.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,1,0)),i*std::numbers::pi/2);
        p=cut(p,BRepBuilderAPI_Transform(box(-2,65.9,10.5,4,5.2,4),rotation,true).Shape());
    }
    return p;
}
TopoDS_Shape lockWasher(double y=71,double length=1) {
    // MB2 d=15,d4=13.5 => inner tang tip at Z=+6,not+5.5.
    // Outer bend-up tabs remain an installed radial envelope.
    return fuse(tube(14,7.5,y,length),box(-2,y,6,4,length,3));
}
TopoDS_Shape frontModule() {
    auto p=box(-23,d::front_module_start,d::front_module_bottom,46,92-d::front_module_start,37.5);
    p=cut(p,cylinder(d::front_housing_bore/2,72.97,9.03));p=cut(p,cylinder(14.9,82,10.1));
    for(double x:{-19.0,19.0})for(double z:{-12.5,12.5})
        p=cut(p,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,72.97,z),gp_Dir(0,1,0)),1.5,6.02).Shape());
    for(double x:{-d::module_bolt_x,d::module_bolt_x})
        p=cut(p,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::module_bolt_station,-18.51),gp_Dir(0,0,1)),2,10.02).Shape());
    for(double x:{-d::module_pin_x,d::module_pin_x})
        p=cut(p,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::module_pin_station,-18.51),gp_Dir(0,0,1)),1.505,37.52).Shape());
    return p;
}
void requireNoOverlap(const TopoDS_Shape& moving,const TopoDS_Shape& fixed,const std::string& label) {
    BRepAlgoAPI_Common common(moving,fixed);common.Build();
    if(!common.IsDone())throw std::runtime_error("Assembly-path boolean failed: "+label);
    const double v=volume(common.Shape());
    if(v>1e-4)throw std::runtime_error("Blocked assembly path: "+label+" = "+std::to_string(v)+" mm3");
}
const TopoDS_Shape& partShape(const std::vector<HeadPart>& p,const std::string& id) {
    const auto it=std::find_if(p.begin(),p.end(),[&](const auto& item){return item.id==id;});
    if(it==p.end())throw std::runtime_error("Assembly-path part absent: "+id);
    return it->shape;
}
TopoDS_Shape makeFeedShaft(bool finished_edges) {
    // Nominal thread major diameters: drawing callouts govern turning/tapping.
    auto shaft=cylinder(6,0,31.5);
    shaft=fuse(shaft,cylinder(7,31.5,33));
    shaft=fuse(shaft,cylinder(7.5,64.5,17.5));
    shaft=fuse(shaft,cylinder(9.5,82,5));
    shaft=fuse(shaft,cylinder(6,87,5));
    shaft=fuse(shaft,cylinder(5,92,21));
    shaft=roundCircularEdge(shaft,31.5,6,d::bearing_shoulder_radius);
    shaft=roundCircularEdge(shaft,82,7.5,d::bearing_shoulder_radius);
    shaft=roundCircularEdge(shaft,87,6,d::neck_shoulder_radius);
    shaft=roundCircularEdge(shaft,92,5,d::bearing_shoulder_radius);
    if(finished_edges) {
        shaft=turnedChamfer(shaft,0,6-d::shaft_end_chamfer,6,d::shaft_end_chamfer);
        shaft=turnedChamfer(shaft,d::shaft_end-d::shaft_end_chamfer,5,5-d::shaft_end_chamfer,d::shaft_end_chamfer);
        shaft=turnedChamfer(shaft,d::thread_start,7.5-d::thread_lead_chamfer,7.5,d::thread_lead_chamfer);
    }
    shaft=cut(shaft,tube(7,d::groove_d/2,d::groove_start,d::groove_end-d::groove_start));
    shaft=roundCircularEdge(shaft,d::groove_start,d::groove_d/2,d::groove_root_radius);
    shaft=roundCircularEdge(shaft,d::groove_end,d::groove_d/2,d::groove_root_radius);
    shaft=cut(shaft,tube(8,d::thread_relief_d/2,d::thread_relief_start,d::thread_relief_end-d::thread_relief_start));
    shaft=roundCircularEdge(shaft,d::thread_relief_start,d::thread_relief_d/2,d::bearing_shoulder_radius);
    shaft=roundCircularEdge(shaft,d::thread_relief_end,d::thread_relief_d/2,d::bearing_shoulder_radius);
    // MB2 inward tang:4mm wide,13.5mm opposing-wall-to-tip dimension.
    // Open an insertion path for MB2 across the14mm pulley seat. It intersects
    // the deeper/wider pulley key seat;washer must be installed BEFORE that key.
    if(finished_edges) {
        shaft=cut(shaft,pulleyAndWasherTool());
    }else {
        // Previous R5 shape retained only for a rejection regression.
        shaft=cut(shaft,capsule(5.04,28.10,33.45,4,8));
        shaft=roundKeyFloor(shaft,4);
        shaft=cut(shaft,box(-2.1,d::washer_slot_start,5.5,4.2,73-d::washer_slot_start,5));
    }
    shaft=cut(shaft,capsule(3.04,18.10,94.45,-10,6.8));
    shaft=roundKeyFloor(shaft,-3.2);
    // Full-profile thread starts after the entry chamfer. The minor-diameter
    // drill and actual118degree point extend beyond the thread/tap lead.
    const double threadBottom=d::shaft_end-d::retaining_entry_chamfer-d::retaining_full_thread_depth;
    const double drillBottom=d::shaft_end-d::retaining_drill_full_depth;
    const double drillRadius=d::retaining_drill_d/2;
    const double pointDepth=drillRadius/std::tan(d::retaining_drill_point_degrees*std::numbers::pi/360);
    shaft=cut(shaft,cylinder(2.5,threadBottom,d::shaft_end-threadBottom+0.1));
    shaft=cut(shaft,cylinder(drillRadius,drillBottom,d::shaft_end-drillBottom+0.1));
    shaft=cut(shaft,BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,drillBottom,0),gp_Dir(0,-1,0)),drillRadius,0,pointDepth).Shape());
    shaft=cut(shaft,BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,d::shaft_end-d::retaining_entry_chamfer,0),gp_Dir(0,1,0)),
        2.5,2.5+d::retaining_entry_chamfer,d::retaining_entry_chamfer).Shape());
    return shaft;
}
void checkShaftEdges(const TopoDS_Shape& shaft) {
    const auto expect=[&](double x,double s,double z,TopAbs_State state,const char* feature) {
        BRepClass3d_SolidClassifier classifier(shaft,gp_Pnt(x,s,z),1e-7);
        if(classifier.State()!=state)throw std::runtime_error(std::string("Unfinished shaft edge: ")+feature);
    };
    expect(5.95,0.05,0,TopAbs_OUT,"rear end chamfer");
    expect(5.70,0.05,0,TopAbs_IN,"rear end retained face");
    expect(4.95,112.95,0,TopAbs_OUT,"roller end chamfer");
    expect(4.70,112.95,0,TopAbs_IN,"roller end retained face");
    expect(7.40,64.60,0,TopAbs_OUT,"M15 external lead chamfer");
    expect(7.40,65.10,0,TopAbs_IN,"M15 full major envelope after lead");
    expect(2.05,72.95,6.10,TopAbs_IN,"slot R0.5 plan corner");
    expect(1.50,72.80,6.10,TopAbs_OUT,"slot end access retained");
    expect(2.05,70.00,5.52,TopAbs_IN,"slot floor R0.15");
    expect(1.80,70.00,5.60,TopAbs_OUT,"slot floor clear interior");
    expect(0,33.43,5.45,TopAbs_OUT,"slot-to-key floor lip blend");
    expect(0,33.20,5.40,TopAbs_IN,"slot-to-key retained core");
}
}
TopoDS_Shape feedShaftShape() {
    static const TopoDS_Shape finished=makeFeedShaft(true);
    return finished;
}
void rejectUnfinishedFeedMachining() {
    const auto legacy=makeFeedShaft(false);
    try {checkShaftEdges(legacy);}
    catch(const std::runtime_error& e) {
        if(std::string(e.what())=="Unfinished shaft edge: rear end chamfer")return;
        throw;
    }
    throw std::runtime_error("Unfinished R5 shaft edges were not rejected");
}
void checkFeedMachiningFeatures() {
    const auto shaft=feedShaftShape();
    checkShaftEdges(shaft);
    const auto expect=[&](double x,double s,double z,TopAbs_State state,const char* feature) {
        BRepClass3d_SolidClassifier classifier(shaft,gp_Pnt(x,s,z),1e-7);
        if(classifier.State()!=state)throw std::runtime_error(std::string("Machining feature failed: ")+feature);
    };
    expect(6.05,31.46,0,TopAbs_IN,"rear-bearing shoulder R0.2");
    expect(7.55,81.96,0,TopAbs_IN,"front-bearing shoulder R0.2");
    expect(6.10,87.10,0,TopAbs_IN,"neck shoulder R0.5");
    expect(5.05,92.04,0,TopAbs_IN,"roller shoulder R0.2");
    expect(5.735,18.51,0,TopAbs_IN,"A12 groove rounded root");
    expect(7.4,72.2,0,TopAbs_OUT,"M15 thread runout relief");
    expect(6.7,72.2,0,TopAbs_IN,"thread relief retained core");
    expect(2.49,47.5,4.02,TopAbs_IN,"pulley-key floor R0.15");
    expect(1.49,103.5,-3.22,TopAbs_IN,"roller-key floor R0.15");
    expect(0,96,0,TopAbs_OUT,"blind drill cylinder");
    expect(0,94,0,TopAbs_OUT,"118-degree drill point");
    expect(0,93.5,0,TopAbs_IN,"material behind drill point");
    expect(2.3,97,0,TopAbs_IN,"minor-diameter drill not cosmetic major bore");
    expect(2.3,100,0,TopAbs_OUT,"cosmetic full-thread envelope");
    expect(2.59,112.9,0,TopAbs_OUT,"thread entry chamfer");
    if(c::seeger_a12_groove_width_min-2*0.05+1e-12<c::seeger_a12_width||0.05>0.1*(c::seeger_a12_width-c::seeger_a12_width_minus))
        throw std::runtime_error("Retaining groove worst-case radius/width does not fit A12");
}
TopoDS_Shape feedHousingShape() {
    auto housing=box(-23,19,-34,46,73,53);
    housing=cut(housing,cylinder(10.5,18.9,73.2));
    housing=cut(housing,cylinder(d::rear_housing_bore/2,18.9,12.6));
    // Open pulley bay;15.5mm bottom bridge remains below the flange envelope.
    housing=cut(housing,box(-23.1,31.5,-18.5,46.2,41.48,37.6));
    // The25mm belt backing extends toR18.845,beyond theR18flanges.
    // Relief includes the±0.6mm belt-width bound and leaves14mm bridge aluminium.
    housing=cut(housing,box(-23.1,34.5,-20,46.2,26,39.1));
    // Removable front block lets the pre-keyed pulley/shaft enter from+S.
    housing=cut(housing,box(-23.1,d::front_module_start,d::front_module_bottom,46.2,19.12,37.6));
    for(double x:{-d::module_bolt_x,d::module_bolt_x}) {
        housing=cut(housing,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::module_bolt_station,-34.01),gp_Dir(0,0,1)),2.25,15.52).Shape());
        housing=cut(housing,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::module_bolt_station,-34.01),gp_Dir(0,0,1)),4.5,5.01).Shape());
    }
    for(double x:{-d::module_pin_x,d::module_pin_x})
        housing=cut(housing,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::module_pin_station,-34.01),gp_Dir(0,0,1)),1.5,15.52).Shape());
    for(double x:{-16.0,16.0})for(double y:{d::rear_mount_station,d::front_mount_station})
        housing=cut(housing,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,y,-34.01),gp_Dir(0,0,1)),2.5,12.02).Shape());
    return housing;
}
TopoDS_Shape feedRollerShape() {
    auto roller=tube(20,5,d::roller_start,20);
    // STARZ secondary operation on the18mm aluminium collar, opposite the MC holes.
    roller=cut(roller,box(-1.52,d::roller_start-0.1,-7.35,3.04,20.2,7.35));
    for(const gp_Dir axis:{gp_Dir(1,0,0),gp_Dir(0,0,1)}) {
        roller=cut(roller,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,d::roller_station,0),axis),1.5,20.1).Shape());
        // Original MC alteration drawing: M+1 clearance in the urethane,not
        // an M3 threaded hole through the elastomer. Collar radius is9mm.
        roller=cut(roller,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(9*axis.X(),d::roller_station,9*axis.Z()),axis),2,11.1).Shape());
    }
    return roller;
}
std::vector<HeadPart> feedCartridge(FeedRole role) {
    std::vector<HeadPart> p;
    const auto add=[&](std::string id,std::string description,TopoDS_Shape shape,bool bought=false) {
        p.push_back({std::move(id),std::move(description),std::move(shape),bought,false});
    };
    add("F101","42CrMo4+QT rotor;matched j5 bearing seats;nominal-major cosmetic threads",feedShaftShape());
    add("F102","6082-T6 rear bearing/base;front module removable for assembly",feedHousingShape());
    add("F112","6082-T6 removable locating-bearing module;32.008–32.019 bore;matched to F102",frontModule());
    add("F103","Front bearing rear cover;M3 clearance holes",bearingCover());
    add("F104","16ODx12.1IDx2.7 rear inner-ring abutment with positive end float",tube(8,6.05,19.8,2.7));
    add("F105","16ODx12.1IDx1 rear-bearing abutment;shaft-shoulder side C0.30",
        cut(tube(8,6.05,30.5,1),BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,31.2,0),gp_Dir(0,1,0)),6.05,6.35,0.3).Shape()));
    add("F106","19.5ODx15.1IDx1 front inner-ring abutment",tube(9.75,7.55,72,1));
    add("F107","17.5ODx10.1IDx1.5 roller spacer;shaft-shoulder side C0.30",
        cut(tube(8.75,5.05,92,1.5),BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,92,0),gp_Dir(0,1,0)),5.35,5.05,0.3).Shape()));
    add("F108","17.5ODx5.5IDx3 positive roller end plate",tube(8.75,2.75,113.5,3));
    add("F109","C45 roller key3x4x18;inside perimeter C0.20",chamferKeyInside(capsule(3,18,94.5,-7.2,4),-3.2));
    add("F110","Secondary-keyed UMCL40-10-20-MC;18mm collar;MC4mm urethane-access holes",feedRollerShape());
    add("RB01","SKF6001-2Z/C3 external interface",tube(14,6,22.5,8),true);
    add("RB02","SKF6002-2Z/C3 interface;minimum R0.3 inner shoulder corner",
        roundCircularEdge(tube(16,7.5,73,9),82,7.5,0.3),true);
    // The retaining ring model represents its installed radial envelope. Gap/lugs
    // are purchased details; the groove is the exact selected mating interface.
    add("RR01","SEEGER A12 installed envelope;centred between rounded groove roots",tube(9.1,5.725,d::ring_installed_start,1),true);
    add("RN01","SKF KM2 M15x1 nut",nut(),true);
    add("RW01","SKF MB2;actual4mm inward tang in open shaft slot",lockWasher(),true);
    auto bolt=fuse(cylinder(2.5,100.5,16),cylinder(4.25,116.5,5));
    add("FB01","ISO4762 M5x16 class10.9 roller retention;socket omitted",bolt,true);
    for(int i=0;i<2;++i) {
        const double x=(i==0?-1:1)*d::module_bolt_x;
        gp_Trsf t;t.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(1,0,0)),std::numbers::pi/2);
        gp_Trsf shift;shift.SetTranslation(gp_Vec(x,d::module_bolt_station,0));
        const auto upright=[&](const TopoDS_Shape& shape){return BRepBuilderAPI_Transform(shape,shift*t,true).Shape();};
        add("FMB"+std::to_string(i+1),"ISO4762 M4x20 class10.9 front-module screw",upright(fuse(cylinder(2,-29.5,20),cylinder(3.5,-33.5,4))),true);
        add("FMW"+std::to_string(i+1),"ISO7092 M4 washer4.3x8x0.5 200HV",upright(tube(4,2.15,-29.5,0.5)),true);
        add("FDP"+std::to_string(i+1),"ISO8734 dowel3m6x20;recessed1.5mm from underside",
            BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt((i==0?-1:1)*d::module_pin_x,d::module_pin_station,-32.5),gp_Dir(0,0,1)),1.5,20).Shape(),true);
    }
    int coverIndex=0;
    for(double x:{-19.0,19.0})for(double z:{-12.5,12.5}) {
        gp_Trsf t;t.SetTranslation(gp_Vec(x,0,z));
        add("FCB"+std::to_string(++coverIndex),"ISO4762 M3x8 class8.8 locating-bearing cover screw",
            BRepBuilderAPI_Transform(fuse(cylinder(1.5,70.98,8),cylinder(2.75,67.98,3)),t,true).Shape(),true);
    }
    if(role!=FeedRole::passive) {
        add("TP01",std::string(c::pulley)+" purchased external interface",pulley(),true);
        add("TK01","DIN6885-A5x5x28 C45 key;inside perimeter C0.20 finishing operation",
            chamferKeyInside(capsule(5,28,33.5,4,5),4),true);
    }
    if(role==FeedRole::motor_driven) {
        add("CP02",std::string(c::coupling)+" catalogue interference envelope",coupling(),true);
        add("FM01","Original24HS40-5004D-E1000 STEP;D-flat retained",originalFeedMotor(),true);
        add("F111","80x80x10 feed-motor plate;50mm mounting grid",motorPlate());
    }
    return p;
}
void checkFeedCartridge(const std::vector<HeadPart>& parts) {
    std::vector<BoxBounds> boxes;boxes.reserve(parts.size());
    for(const auto& p:parts) {
        if(!BRepCheck_Analyzer(p.shape).IsValid()||volume(p.shape)<=0)
            throw std::runtime_error("Invalid feed solid: "+p.id);
        int n=0;for(TopExp_Explorer e(p.shape,TopAbs_SOLID);e.More();e.Next())++n;
        if(n==0||(!p.purchased&&n!=1))throw std::runtime_error("Disconnected feed part: "+p.id);
        boxes.push_back(bounds(p.shape));
    }
    for(std::size_t i=0;i<parts.size();++i)for(std::size_t j=i+1;j<parts.size();++j) {
        const auto& a=boxes[i];const auto& b=boxes[j];
        if(a.xmax<=b.xmin+1e-7||b.xmax<=a.xmin+1e-7||a.ymax<=b.ymin+1e-7||b.ymax<=a.ymin+1e-7||
           a.zmax<=b.zmin+1e-7||b.zmax<=a.zmin+1e-7)continue;
        BRepAlgoAPI_Common common(parts[i].shape,parts[j].shape);common.Build();
        if(!common.IsDone())throw std::runtime_error("Feed intersection operation failed");
        const double overlap=volume(common.Shape());
        if(overlap>1e-4)throw std::runtime_error("Feed interference: "+parts[i].id+" / "+parts[j].id+" = "+std::to_string(overlap)+" mm3");
    }
    for(const auto& p:parts)if(p.id=="F101") {
        const auto b=bounds(p.shape);
        if(std::abs(b.ymin)>1e-5||std::abs(b.ymax-d::shaft_end)>1e-5)
            throw std::runtime_error("Feed shaft axial chain mismatch");
    }
}
void checkFeedAssemblyPaths(const std::vector<HeadPart>& parts) {
    const auto& shaft=partShape(parts,"F101");const auto& base=partShape(parts,"F102");
    // Exact continuous sweep of the MB2 cross-section from outside the rear end
    // to its finalS71..72 position. Pulley key is deliberately not installed yet.
    requireNoOverlap(lockWasher(-2,74),shaft,"MB2 inward tang / shaft insertion");
    // Conservative continuous swept envelopes for100mm axial insertion from+S.
    // Final pulley rear edge never crossesS31.5;front module and pins are removed.
    requireNoOverlap(cylinder(9.5,0,213),base,"shaft / removable base");
    requireNoOverlap(cylinder(18,31.5,132),base,"pulley / removable base");
    requireNoOverlap(box(-23,70.98,-18.25,46,102,37.25),base,"bearing cover / removable base");
    requireNoOverlap(box(-23,72.98,-18.5,46,119.02,37.5),base,"front module / removable base");
    requireNoOverlap(cylinder(16,66,116),base,"bearing and retention / removable base");
    // The rear bearing slides over the12mm journal only after this insertion.
    requireNoOverlap(tube(14,6,-10,40.5),shaft,"rear bearing / shaft");
    for(double x:{-d::module_pin_x,d::module_pin_x}) {
        const auto punch=BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,d::module_pin_station,-40),gp_Dir(0,0,1)),1.4,100).Shape();
        requireNoOverlap(punch,shaft,"dowel service punch / shaft");
        requireNoOverlap(punch,base,"dowel service punch / base");
        requireNoOverlap(punch,partShape(parts,"F112"),"dowel service punch / front module");
    }
}
void rejectOldFeedAssemblyPaths(bool closed_washer_slot) {
    if(closed_washer_slot) {
        // Restore the actual obstructing14mm land between31.5and33.4.
        const auto legacy=fuse(feedShaftShape(),cylinder(7,31.5,1.9));
        try {requireNoOverlap(lockWasher(-2,74),legacy,"closed MB2 insertion land");}
        catch(const std::runtime_error& e) {
            if(std::string(e.what()).starts_with("Blocked assembly path:"))return;
            throw;
        }
    } else {
        const auto legacy=fuse(feedHousingShape(),frontModule());
        gp_Trsf pose;pose.SetTranslation(gp_Vec(0,40,0));
        try {requireNoOverlap(BRepBuilderAPI_Transform(pulley(),pose,true).Shape(),legacy,"pulley through monolithic front support");}
        catch(const std::runtime_error& e) {
            if(std::string(e.what()).starts_with("Blocked assembly path:"))return;
            throw;
        }
    }
    throw std::runtime_error("Old blocked assembly path was not rejected");
}
std::vector<HeadPart> feedCassette(double wire_diameter_mm) {
    if(!std::isfinite(wire_diameter_mm)||wire_diameter_mm<d::wire_diameter_min||wire_diameter_mm>d::wire_diameter_max)
        throw std::invalid_argument("Wire diameter outside1.2–4.5mm cassette envelope");
    std::vector<HeadPart> parts;
    for(int upper=0;upper<2;++upper)for(int far=0;far<2;++far) {
        const FeedRole role=upper?FeedRole::passive:(far?FeedRole::motor_driven:FeedRole::belt_follower);
        auto cartridge=feedCartridge(role);
        gp_Trsf rotation;if(upper)rotation.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,1,0)),std::numbers::pi);
        const double z=d::wire_axis_z+(upper?1:-1)*(c::roller_d+wire_diameter_mm)/2;
        gp_Trsf position;position.SetTranslation(gp_Vec(d::near_nip_x+far*d::nominal_centres,-d::wire_contact_station,z));
        const std::string prefix=std::string(upper?"U":"L")+(far?"2_":"1_");
        for(auto& p:cartridge) {
            p.id=prefix+p.id;p.shape=BRepBuilderAPI_Transform(p.shape,position*rotation,true).Shape();
            parts.push_back(std::move(p));
        }
    }
    const double pitch_r=c::pulley_teeth*c::pitch_mm/(2*std::numbers::pi);
    const double inner=pitch_r-0.480,outer=inner+3.41;
    const double y=d::pulley_station-d::wire_contact_station-c::belt_width/2;
    const double z=d::wire_axis_z-(c::roller_d+wire_diameter_mm)/2;
    const auto loop=[&](double r,double from,double width) {
        auto p=box(d::near_nip_x,from,z-r,d::nominal_centres,width,2*r);
        for(double x:{d::near_nip_x,d::near_nip_x+d::nominal_centres})
            p=fuse(p,BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,from,z),gp_Dir(0,1,0)),r,width).Shape());
        return p;
    };
    // Smooth external interference envelope. Purchased teeth inside the pulley
    // OD are intentionally not generated as a false tooth-manufacturing model.
    auto belt=cut(loop(outer,y,c::belt_width),loop(inner,y-0.01,c::belt_width+0.02));
    parts.push_back({"TB01","HTBN225S5M-250 external backing envelope;tooth engagement excluded",belt,true,false});
    return parts;
}
}