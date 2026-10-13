#include "head_geometry.hpp"
#include "head_fit.hpp"
#include "shared/cutting_head_spec.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GProp_GProps.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

namespace starz::engineering {
namespace c = head::catalogue;
namespace d = head::design;
namespace {
TopoDS_Shape box(double x, double y, double z, double a, double b, double h) {
    return BRepPrimAPI_MakeBox(gp_Pnt(x,y,z),a,b,h).Shape();
}
TopoDS_Shape cyl(double r, double l, double x, double y, double z, bool alongY=false) {
    return BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x,y,z),alongY?gp_Dir(0,1,0):gp_Dir(0,0,1)),r,l).Shape();
}
TopoDS_Shape cut(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepAlgoAPI_Cut op(a,b); op.Build();
    if (!op.IsDone()) throw std::runtime_error("CAD cut failed");
    return op.Shape();
}
TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepAlgoAPI_Fuse op(a,b); op.Build();
    if (!op.IsDone()) throw std::runtime_error("CAD fuse failed");
    return op.Shape();
}
TopoDS_Shape translate(const TopoDS_Shape& a,double x,double y,double z) {
    gp_Trsf t; t.SetTranslation(gp_Vec(x,y,z));
    return BRepBuilderAPI_Transform(a,t,true).Shape();
}
TopoDS_Shape halfTurn(const TopoDS_Shape& a) {
    gp_Trsf t; t.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),std::numbers::pi);
    return BRepBuilderAPI_Transform(a,t,true).Shape();
}
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
const TopoDS_Shape& actualCuttingMotor() {
    static const auto shape=[] {
        STEPControl_Reader reader;
        const auto path=std::filesystem::path(STARZ_SOURCE_DIR)/"engineering"/"sources"/"34E1K-85.step";
        require(reader.ReadFile(path.c_str())==IFSelect_RetDone && reader.TransferRoots()>0,"Original34E1K-85 STEP unavailable");
        auto s=reader.OneShape();
        require(BRepCheck_Analyzer(s).IsValid(),"Invalid original34E1K-85 STEP");
        // The supplied STEP includes the fitted5×5×25 parallel key fused into
        // the motor solid. MK2 is ordered with keyless clamp hubs: remove that
        // removable key, not shaft material. Coordinates/radii are from its
        // original cylindrical surfaces (R2.495,Y4..9,Z14.5/34.505).
        auto key=fuse(box(-2.495,4,14.5,4.99,5,20.005),cyl(2.495,5,0,4,14.5,true));
        key=fuse(key,cyl(2.495,5,0,4,34.505,true));
        const double before=volume(s); s=cut(s,key);
        require(before-volume(s)>590 && before-volume(s)<600,"Original motor key geometry changed");
        // Supplier native mounting planeZ=0; shaft axis+Z. Rotate to+Y,
        // preserving the complete original cable and connector geometry.
        gp_Trsf t; t.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(1,0,0)),-std::numbers::pi/2);
        return BRepBuilderAPI_Transform(s,t,true).Shape();
    }();
    return shape;
}
TopoDS_Shape knife() {
    // Purchase-to-drawing tooling, NOT a claimed standard supplier blade.
    BRepBuilderAPI_MakePolygon p;
    for(const auto& yz : std::array<std::array<double,2>,5>{{{-30,-19},{19,-19},{0,0},{19,19},{-30,19}}})
        p.Add(gp_Pnt(-3.01,yz[0],yz[1]));
    p.Close();
    auto s=BRepPrimAPI_MakePrism(BRepBuilderAPI_MakeFace(p.Wire()).Face(),gp_Vec(3,0,0)).Shape();
    for(double z : {-10.,10.}) {
        auto drill=BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(-4,-24,z),gp_Dir(1,0,0)),2.75,5).Shape();
        s=cut(s,drill);
    }
    return s;
}
}

TopoDS_Shape holderShape() {
    auto s=fuse(box(-30,-37.5,0,60,75,d::holder_foot_thickness),
                box(d::holder_arm_x,d::holder_arm_y,4,d::holder_arm_width,d::holder_arm_length,d::holder_arm_height));
    // Keep the main backbone centred on the screw/load line; machine the blade
    // seat only at the nose. The former full-length one-sided arm was eccentric.
    s=cut(s,box(d::blade_seat_x,d::blade_pocket_y,3,40,30,46));
    // Backing slot behind the blade seat (audit F22): carries the H007
    // backing plate that the blade shims against, so the cut reaction
    // never loads the blade in bending.
    s=cut(s,box(d::blade_seat_x-8.5,d::blade_pocket_y,3.5,8,30,45));
    TopTools_IndexedMapOfShape edges; TopExp::MapShapes(s,TopAbs_EDGE,edges);
    BRepFilletAPI_MakeFillet fillet(s); bool selected=false;
    for(int i=1;i<=edges.Extent();++i) {
        const auto b=bounds(edges(i));
        if(std::abs(b.xmin-d::blade_seat_x)<1e-5 && std::abs(b.xmax-d::blade_seat_x)<1e-5 &&
           std::abs(b.ymin-d::blade_pocket_y)<1e-5 && std::abs(b.ymax-d::blade_pocket_y)<1e-5 && b.zmax-b.zmin>40) {
            fillet.Add(d::blade_pocket_fillet,TopoDS::Edge(edges(i))); selected=true;
        }
    }
    require(selected,"Blade pocket fillet edge not found"); fillet.Build();
    require(fillet.IsDone(),"Blade pocket fillet failed"); s=fillet.Shape();
    for(double x : {-23.,23.}) for(double y : {-23.,23.}) {
        s=cut(s,cyl(3.3,50,x,y,-1));
        s=cut(s,cyl(5.5,10,x,y,40));
    }
    for(double z : {16.,36.}) {
        // M5 tapped tooling holes; minor-diameter representation, no invented threads.
        auto drill=BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(d::holder_arm_x-1,d::blade_mount_y,z),gp_Dir(1,0,0)),2.1,d::holder_arm_width+2).Shape();
        s=cut(s,drill);
    }
    return s;
}

TopoDS_Shape datumShape() {
    auto s=box(-90,-520,0,180,1040,d::datum_thickness);
    s=cut(s,box(-45,-25,-1,90,50,d::datum_thickness+2));
    for(double x : {-75.,75.}) for(double y : {-500.,-250.,250.,500.})
        s=cut(s,cyl(5.5,22,x,y,-1));
    for(double x : {-23.,23.}) for(int side:{-1,1}) for(int i=0;i<3;++i)
        s=cut(s,cyl(2.5,22,x,side*(d::rail_start_y+70+100*i),-1)); // M6 pilot, tap after facing.
    return s;
}

TopoDS_Shape adapterShape() {
    // Local motor shaft axis+Y. Motor face atY=-thickness; F0 faceY=0.
    auto s=box(-50,-d::adapter_thickness,-50,100,d::adapter_thickness,100);
    s=fuse(s,cyl(d::adapter_f0_register_diameter/2,d::adapter_f0_register_length,0,0,0,true));
    s=cut(s,cyl(21,d::adapter_thickness+4,0,-d::adapter_thickness-1,0,true));
    s=cut(s,cyl(d::adapter_motor_pilot_diameter/2,d::adapter_motor_pilot_depth,0,-d::adapter_thickness,0,true));
    const double q=35/std::sqrt(2.0);
    for(double x : {-q,q}) for(double z : {-q,q}) {
        s=cut(s,cyl(2.75,d::adapter_thickness+2,x,-d::adapter_thickness-1,z,true));
        s=cut(s,cyl(4.75,d::adapter_f0_counterbore_depth,x,-d::adapter_thickness,z,true));
    }
    for(double x : {-c::motor_pitch/2,c::motor_pitch/2})
        for(double z : {-c::motor_pitch/2,c::motor_pitch/2})
            s=cut(s,cyl(2.1,d::adapter_thickness+2,x,-d::adapter_thickness-1,z,true)); // M5 tapped through; accommodates actual ±0.25mm motor grid.
    return s;
}

std::vector<HeadPart> headAssembly(double closure) {
    if(!std::isfinite(closure) || closure<0 || closure>d::blade_limit)
        throw std::invalid_argument("Closure outside the selected4.2mm jaw envelope");
    std::vector<HeadPart> out{{"H001","Ground head datum / S1-B",datumShape(),false,false}};
    std::vector<HeadPart> one;
    const auto add=[&](std::string id,std::string text,TopoDS_Shape s,bool purchased,bool moving=false) {
        one.push_back({std::move(id),std::move(text),std::move(s),purchased,moving});
    };
    auto riser=box(-55,d::rail_start_y-7,20,110,355,20);
    auto rail=box(-43,d::rail_start_y,40,86,340,19);
    // The complete unit's internal raceways are not reverse-engineered. This is
    // an external conservative envelope, not a machined replacement axis.
    for(double x : {-23.,23.}) for(int i=0;i<3;++i) {
        const double y=d::rail_start_y+70+100*i;
        rail=cut(rail,cyl(3.3,21,x,y,39));
        rail=cut(rail,cyl(5.5,6,x,y,53));
        riser=cut(riser,cyl(3.3,22,x,y,19));
    }
    add("H002","Stage riser /20mm clearance for true NEMA34 body",riser,false);
    add("AX01-R","Custom-axis HGR20 rail pair external envelope (docs/39)",rail,true);
    auto fixed=box(-42.5,d::stage_front_y,40,85,87,63);
    fixed=cut(fixed,cyl(23,66,0,d::stage_front_y-1,72,true));
    fixed=cut(fixed,cyl(25,3.5,0,d::stage_front_y,72,true));
    const double fq=35/std::sqrt(2.0);
    for(double x:{-fq,fq})for(double z:{-fq,fq})
        fixed=cut(fixed,cyl(2.5,10,x,d::stage_front_y,72+z,true)); // M5 nominal-major cosmetic thread.
    // Catalogue coupling service opening; unmeasured corner radii not represented.
    fixed=cut(fixed,box(-25,d::stage_front_y+14,72,50,50,40));
    add("AX01-F","Custom-axis BK12 fixed-end envelope / Ø46 coupling clearance (docs/39)",fixed,true);
    add("AX01-D","Custom-axis BF12 distal-end envelope",box(-43,d::rail_end_y,40,86,13,32),true);
    add("AX01-S","Custom-axis screw Ø8 x18 exposed input end (docs/39)",
        cyl(4,c::exposed_shaft,0,d::stage_front_y+d::stage_shaft_tip,72,true),true);
    auto carriage=box(-30,d::carriage_open_y-58,59,60,116,27);
    for(double x : {-23.,23.}) for(double y : {-23.,23.})
        carriage=cut(carriage,cyl(2.5,12,x,d::carriage_open_y+y,74));
    add("AX01-C","Custom-axis H006 carriage envelope / M6 46mm grid (docs/39)",carriage,true,true);
    add("H003","15mm motor adapter / full hub tolerance stack / M5 motor fixings",
        translate(adapterShape(),0,d::stage_front_y,72),false);
    add("DR01-M","34E1K-85 / original supplier STEP; removable shaft key removed",
        translate(actualCuttingMotor(),0,d::motor_face_y,72),true);
    const auto screw=[](double x,double bearingY,double z,double length) {
        return fuse(cyl(2.5,length,x,bearingY,z,true),cyl(4.25,5,x,bearingY-5,z,true));
    };
    const auto washer=[](double x,double y,double z) {
        return cut(cyl(4.5,1,x,y,z,true),cyl(2.65,1.2,x,y-0.1,z,true));
    };
    int boltIndex=0;
    for(double x:{-fq,fq})for(double z:{-fq,fq}) {
        const auto number=std::to_string(++boltIndex);
        const double seat=d::motor_face_y+d::adapter_f0_counterbore_depth;
        add("HF0-B"+number,"ISO4762 M5x16 class10.9;nominal-major cosmetic thread",screw(x,seat-1,72+z,16),true);
        add("HF0-W"+number,"ISO7092 M5 200HV reduced-OD washer9x5.3x1",washer(x,seat-1,72+z),true);
    }
    boltIndex=0;
    for(double x:{-c::motor_pitch/2,c::motor_pitch/2})for(double z:{-c::motor_pitch/2,c::motor_pitch/2}) {
        const auto number=std::to_string(++boltIndex);
        // Original supplier STEP has a9mm mounting flange. Washer is behind it.
        add("HM-B"+number,"ISO4762 M5x25 class10.9;nominal-major cosmetic thread",screw(x,d::motor_face_y-10,72+z,25),true);
        add("HM-W"+number,"ISO7092 M5 200HV motor washer9x5.3x1",washer(x,d::motor_face_y-10,72+z),true);
    }
    auto cp=cyl(c::coupling_diameter/2,c::coupling_length,0,d::stage_front_y+d::coupling_near,72,true);
    cp=cut(cp,cyl(7,c::coupling_fit+1,0,d::stage_front_y+d::coupling_near-1,72,true));
    cp=cut(cp,cyl(14,c::coupling_length-2*c::coupling_fit,0,d::stage_front_y+d::coupling_near+c::coupling_fit,72,true)); // Open bellows core, not vendor internals.
    cp=cut(cp,cyl(4,c::coupling_fit+1,0,d::stage_front_y+d::coupling_far-c::coupling_fit,72,true));
    add("CP01","R+W MK2/100/50/8/14 interference envelope",cp,true);
    add("H004","42CrMo4+QT single-piece blade holder",
        translate(holderShape(),0,d::carriage_open_y,d::carriage_z),false,true);
    // H007 backing plate inside the H004 blade pocket: the blade is shimmed
    // against it at assembly, so the 20 um face gap is set by the backing
    // (audit F22) and the 1200 N cut reaction is carried by the plate, not
    // by blade flexure. 0.5 mm nominal shim gap behind the blade.
    add("H007","42CrMo4+QT blade backing plate 30x46x8;shimmed blade seat;carries the 1200 N blade reaction;face gap set at assembly (docs/34)",
        translate(box(d::blade_seat_x-8.5,d::blade_pocket_y+0.5,3.5,8,29,45),0,d::carriage_open_y,d::carriage_z),false,true);
    add("T001","Ground V tooling purchase-to-drawing / qualification hold",
        translate(knife(),0,-4,d::wire_z),true,true);
    for(int side=0;side<2;++side) for(const auto& p:one) {
        auto s=p.moving?translate(p.shape,0,closure,0):p.shape;
        if(side==1) s=halfTurn(s);
        out.push_back({p.id+(side==0?"_F":"_R"),p.description,s,p.purchased,p.moving});
    }
    return out;
}

BoxBounds bounds(const TopoDS_Shape& s) {
    Bnd_Box b; BRepBndLib::AddOptimal(s,b,false,false);
    BoxBounds v{}; b.Get(v.xmin,v.ymin,v.zmin,v.xmax,v.ymax,v.zmax); return v;
}
double volume(const TopoDS_Shape& s) {
    GProp_GProps v; BRepGProp::VolumeProperties(s,v); return std::abs(v.Mass());
}
void writeStep(const TopoDS_Shape& s,const std::filesystem::path& p) {
    STEPControl_Writer writer;
    require(writer.Transfer(s,STEPControl_AsIs)==IFSelect_RetDone,"STEP transfer failed");
    require(writer.Write(p.string().c_str())==IFSelect_RetDone,"STEP write failed");
}
void checkHead(const std::vector<HeadPart>& parts,double closure) {
    std::vector<BoxBounds> boxes;boxes.reserve(parts.size());
    for(const auto& p:parts) {
        require(BRepCheck_Analyzer(p.shape).IsValid(),p.id+": invalid solid");
        int solids=0;
        for(TopExp_Explorer e(p.shape,TopAbs_SOLID);e.More();e.Next()) ++solids;
        require((p.purchased?solids>=1:solids==1) && volume(p.shape)>0,p.id+": disconnected/empty part");
        boxes.push_back(bounds(p.shape));
    }
    for(std::size_t i=0;i<parts.size();++i) for(std::size_t j=i+1;j<parts.size();++j) {
        const auto& a=boxes[i];const auto& b=boxes[j];
        if(a.xmax<=b.xmin+1e-7 || b.xmax<=a.xmin+1e-7 ||
           a.ymax<=b.ymin+1e-7 || b.ymax<=a.ymin+1e-7 ||
           a.zmax<=b.zmin+1e-7 || b.zmax<=a.zmin+1e-7) continue;
        BRepAlgoAPI_Common common(parts[i].shape,parts[j].shape); common.Build();
        require(common.IsDone(),"Intersection operation failed");
        auto unintended=common.Shape();
        // Only motor screw / adapter thread engagement is intentionally
        // overlapping: the adapter stores tapping minor diameter and fasteners
        // store nominal major diameter. Nothing outside these exact cylinders
        // is exempted (in particular neither screw heads nor motor pilot).
        const auto& ai=parts[i].id;const auto& bi=parts[j].id;
        const bool threadPair=(ai.starts_with("H003_")&&bi.starts_with("HM-B"))||
                              (bi.starts_with("H003_")&&ai.starts_with("HM-B"));
        if(threadPair&&ai.ends_with("_F")==bi.ends_with("_F")) {
            for(double x:{-c::motor_pitch/2,c::motor_pitch/2})for(double z:{-c::motor_pitch/2,c::motor_pitch/2}) {
                auto zone=cyl(2.501,d::adapter_thickness+0.002,x,d::motor_face_y-0.001,72+z,true);
                if(ai.ends_with("_R"))zone=halfTurn(zone);
                if(!unintended.IsNull()&&volume(unintended)>1e-7)unintended=cut(unintended,zone);
            }
        }
        if(volume(unintended)>=1e-5) {
            const auto collision=bounds(unintended);
            throw std::runtime_error(parts[i].id+" interferes with "+parts[j].id+"; intersection="+std::to_string(volume(unintended))+
                "mm3, min=("+std::to_string(collision.xmin)+","+std::to_string(collision.ymin)+","+std::to_string(collision.zmin)+
                "), max=("+std::to_string(collision.xmax)+","+std::to_string(collision.ymax)+","+std::to_string(collision.zmax)+")");
        }
    }
    const auto find=[&](const std::string& id)->const HeadPart& {
        const auto it=std::find_if(parts.begin(),parts.end(),[&](const auto& p){return p.id==id;});
        require(it!=parts.end(),"Missing required part "+id); return *it;
    };
    auto b=bounds(find("DR01-M_F").shape);
    require(std::abs(b.ymin-(d::motor_face_y-c::motor_rear))<1e-5,"Wrong actual motor rear dimension");
    require(std::abs((b.xmax-b.xmin)-c::motor_width)<1e-5,"Wrong actual motor frame width");
    BRepAlgoAPI_Common shaftSection(find("DR01-M_F").shape,cyl(7.01,38,0,d::motor_face_y,72,true)); shaftSection.Build();
    require(shaftSection.IsDone()&&volume(shaftSection.Shape())>0,"Motor shaft not at required datum");
    require(std::abs(bounds(shaftSection.Shape()).ymax-(d::motor_face_y+c::motor_shaft))<1e-5,"Wrong actual motor shaft length");
    b=bounds(find("CP01_F").shape);
    require(std::abs((b.xmax-b.xmin)-c::coupling_diameter)<1e-6,"Wrong coupling diameter");
    require(c::coupling_diameter< c::cavity_diameter,"Coupling too large");
    require(d::coupling_far-d::stage_shaft_tip>=c::coupling_fit,"Incomplete stage hub engagement");
        require(d::stage_shaft_shoulder-d::coupling_far>=d::stage_hub_shoulder_gap,"Hub touches locknut");
        const auto fit=selectedHeadFitEnvelope();
        require(fit.passed && fit.minimum_motor_projection_mm>0 && fit.minimum_stage_projection_mm>0,
            "Incomplete cutting hub engagement in the bounded tolerance stack");
    require(d::carriage_open_y+closure+58<=d::rail_end_y-c::travel_reserve,"Carriage enters manufacturer reserve");
    const auto f=bounds(find("T001_F").shape),r=bounds(find("T001_R").shape);
    require(std::abs((r.xmin-f.xmax)-d::blade_face_gap)<1e-6,"Wrong nominal blade face gap");
}

void checkHeadSweep(const std::vector<HeadPart>& parts) {
    checkHead(parts,0);
    std::vector<BoxBounds> boxes;boxes.reserve(parts.size());
    for(const auto& p:parts)boxes.push_back(bounds(p.shape));
    const auto group=[](const HeadPart& p) {
        if(!p.moving) return 0;
        if(p.id.ends_with("_F")) return 1;
        if(p.id.ends_with("_R")) return -1;
        throw std::runtime_error("Unknown motion group: "+p.id);
    };
    for(std::size_t i=0;i<parts.size();++i) for(std::size_t j=i+1;j<parts.size();++j) {
        const auto& a=parts[i]; const auto& b=parts[j];
        const int ga=group(a),gb=group(b);
        if(ga==gb) continue; // Rigidly related; the open-assembly exact check applies throughout.
        // The relative motion of two independent, parallel translating jaws is
        // a one-dimensional interval. This covers every pair of jaw positions,
        // not merely a synchronous sampled trajectory.
        const double lo=std::min(0.,ga*d::blade_limit)-std::max(0.,gb*d::blade_limit);
        const double hi=std::max(0.,ga*d::blade_limit)-std::min(0.,gb*d::blade_limit);
        const auto& ab=boxes[i];const auto& bb=boxes[j];
        if(ab.xmax<=bb.xmin+1e-7 || bb.xmax<=ab.xmin+1e-7 ||
           ab.zmax<=bb.zmin+1e-7 || bb.zmax<=ab.zmin+1e-7 ||
           ab.ymax+hi<=bb.ymin+1e-7 || bb.ymax<=ab.ymin+lo+1e-7) continue;
        std::function<void(double,double,int)> certify;
        certify=[&](double left,double right,int depth) {
            const double mid=(left+right)/2;
            const auto moved=translate(a.shape,0,mid,0);
            BRepExtrema_DistShapeShape distance(moved,b.shape); distance.Perform();
            require(distance.IsDone(),"Swept distance failed: "+a.id+" / "+b.id);
            // Euclidean set distance changes by no more than translation length.
            // A lower bound above zero therefore certifies the whole interval.
            if(distance.Value()>(right-left)/2+1e-6) return;
            if(distance.Value()<1e-6) {
                BRepAlgoAPI_Common intersection(moved,b.shape); intersection.Build();
                require(intersection.IsDone(),"Swept intersection failed");
                require(volume(intersection.Shape())<1e-7,"Swept collision: "+a.id+" / "+b.id);
            }
            require(depth<18 && right-left>1e-5,"Swept clearance unresolved (not a pass): "+a.id+" / "+b.id);
            certify(left,mid,depth+1); certify(mid,right,depth+1);
        };
        certify(lo,hi,0);
    }
}
}