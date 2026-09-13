#include "mechanism_geometry.hpp"
#include "feed_geometry.hpp"
#include "shared/cutting_head_spec.hpp"
#include "shared/feed_drive_spec.hpp"
#include "shared/measurement_spec.hpp"
#include "shared/mechanism_spec.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRep_Builder.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Ax1.hxx>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numbers>
#include <stdexcept>

namespace se=starz::engineering;
namespace fs=std::filesystem;
namespace md=starz::mechanism::design;
namespace mc=starz::mechanism::catalogue;
namespace {
using Group=std::vector<se::HeadPart>;
Group rotated(Group g,const std::string& prefix) {
    gp_Trsf turn;turn.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),std::numbers::pi);
    for(auto& p:g){p.id=prefix+p.id;p.shape=BRepBuilderAPI_Transform(p.shape,turn,true).Shape();}
    return g;
}
Group prefixed(Group g,const std::string& prefix){for(auto& p:g)p.id=prefix+p.id;return g;}
void append(Group& to,Group from){for(auto& p:from)to.push_back(std::move(p));}
std::size_t solidCount(const TopoDS_Shape& shape) {
    std::size_t count=0;
    for(TopExp_Explorer explorer(shape,TopAbs_SOLID);explorer.More();explorer.Next())++count;
    return count;
}
void readback(const TopoDS_Shape& shape,const fs::path& path) {
    se::writeStep(shape,path);STEPControl_Reader reader;
    if(reader.ReadFile(path.c_str())!=IFSelect_RetDone||reader.TransferRoots()<1)throw std::runtime_error("Mechanism STEP readback failed");
    const auto r=reader.OneShape();const auto a=se::bounds(shape),b=se::bounds(r);
    if(!BRepCheck_Analyzer(r).IsValid()||std::abs(se::volume(shape)-se::volume(r))>se::volume(shape)*1e-6||
        solidCount(shape)!=solidCount(r)||
        std::abs(a.xmin-b.xmin)+std::abs(a.xmax-b.xmax)+std::abs(a.ymin-b.ymin)+std::abs(a.ymax-b.ymax)+
        std::abs(a.zmin-b.zmin)+std::abs(a.zmax-b.zmax)>1e-4)throw std::runtime_error("Mechanism STEP dimension/topology mismatch");
}
double xGap(const se::BoxBounds& a,const se::BoxBounds& b){return std::max(b.xmin-a.xmax,a.xmin-b.xmax);}
struct Evaluation {
    double wire_od,carried_mass_kg,carrier_protrusion_mm,nip_force_n,wheel_force_n,head_gap_mm,wire_common_mm3;
    std::size_t occurrences;
};
Evaluation evaluate(double d,double carrier_p,double arm_p,bool shift_bushing,bool block_guard_wire_passage,const fs::path& out) {
    namespace fd=starz::feed::design;
    const auto head=se::headAssembly();
    const auto right_cassette=prefixed(se::feedCassette(d),"RIGHT_"),left_cassette=rotated(se::feedCassette(d),"LEFT_");
    // Carried mass is a CAD-volume estimate with assigned densities, not a weighed value.
    const double mass=se::carriedMassEstimate(se::feedCassette(d),se::pressureCarrierGroup(d,10));
    const double p=std::isfinite(carrier_p)?carrier_p:se::nominalCarrierProtrusion(mass);
    const double ap=std::isfinite(arm_p)?arm_p:se::nominalArmProtrusion();
    Group right_mech=se::pressureCarrierGroup(d,p);append(right_mech,se::wireGuideGroup(d));
    if(shift_bushing)for(auto& part:right_mech)if(part.id=="LB01") {
        gp_Trsf shift;shift.SetTranslation(gp_Vec(0,10,0));
        part.shape=BRepBuilderAPI_Transform(part.shape,shift,true).Shape();
    }
    const Group left_mech=rotated(right_mech,"LEFT_");right_mech=prefixed(right_mech,"RIGHT_");
    const Group measuring=prefixed(se::measuringModuleGroup(d,ap),"MM_"),frame=prefixed(se::frameGroup(),"FR_");
    const Group guard=prefixed(se::guardGroup(block_guard_wire_passage),"GD_"),payoff=prefixed(se::payoffGroup(),"PO_");
    for(const Group* g:std::initializer_list<const Group*>{&right_mech,&left_mech,&measuring,&frame,&guard,&payoff})se::checkMechanismSolids(*g);
    const Group* groups[]={&head,&right_cassette,&left_cassette,&right_mech,&left_mech,&measuring,&frame,&guard,&payoff};
    const char* names[]={"head","right cassette","left cassette","right mechanism","left mechanism","measuring","frame","guard","payoff"};
    for(int i=3;i<9;++i)se::checkNoInterference(*groups[i],*groups[i],names[i]);
    for(int i=0;i<9;++i)for(int j=std::max(i+1,3);j<9;++j) {
        const std::string label=std::string(names[i])+"/"+names[j];
        se::checkNoInterference(*groups[i],*groups[j],label.c_str());
    }
    const auto cs=se::carrierSetting(d,mass,p);
    if(cs.spring_length_mm>=mc::carrier_spring_L_mm||mc::carrier_spring_L_mm-cs.spring_length_mm>mc::carrier_spring_deflection_mm)
        throw std::runtime_error("Carrier spring outside its allowable deflection");
    if(cs.nip_force_n<md::nip_force_min_n||cs.nip_force_n>md::nip_force_max_n)
        throw std::runtime_error("Nip preload "+std::to_string(cs.nip_force_n)+" N outside the 50-75 N window");
    const auto as=se::armSetting(d,ap);
    if(as.spring_length_mm>=mc::arm_spring_L_mm||mc::arm_spring_L_mm-as.spring_length_mm>mc::arm_spring_deflection_mm)
        throw std::runtime_error("Arm spring outside its allowable deflection");
    if(as.wheel_force_n<md::wheel_force_min_n||as.wheel_force_n>md::wheel_force_max_n)
        throw std::runtime_error("Wheel contact force "+std::to_string(as.wheel_force_n)+" N outside the 6-19 N window");
    // Guide posts must cover both bushings with 10 mm of threading lift, and
    // must stand proud of the bridge clamps.
    const auto find=[&](const std::string& id){
        const auto it=std::find_if(right_mech.begin(),right_mech.end(),[&](const se::HeadPart& q){return q.id==id;});
        if(it==right_mech.end())throw std::runtime_error("Missing "+id);return se::bounds(it->shape);};
    for(int i=1;i<=2;++i) {
        const auto lb=find("RIGHT_LB0"+std::to_string(i)),post=find("RIGHT_PC0"+std::to_string(i));
        if(lb.zmax+10>post.zmax||lb.zmin-10<post.zmin)throw std::runtime_error("Guide post does not cover the bushing travel");
        if(post.zmax<find("RIGHT_F203").zmax+5)throw std::runtime_error("Post top below the bridge clamp");
    }
    // Moving head parts translate only in Y: a strict X separation certificate.
    double gap=std::numeric_limits<double>::infinity();
    for(const auto& h:head)if(h.moving){const auto hb=se::bounds(h.shape);
        for(const Group* g:std::initializer_list<const Group*>{&right_mech,&left_mech})for(const auto& q:*g){const double v=xGap(hb,se::bounds(q.shape));gap=std::min(gap,v);
            if(v<=1e-5)throw std::runtime_error("No continuous jaw/guide X-separation certificate");}}
    // Wire envelope through guides, wheel and counter roller: tangent only.
    const auto wire=BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(md::payoff_wire_x_min,0,fd::wire_axis_z),gp_Dir(1,0,0)),d/2,
        md::payoff_wire_x_max-md::payoff_wire_x_min).Shape();
    double wire_common=0;const auto wb=se::bounds(wire);
    for(const Group* g:std::initializer_list<const Group*>{&right_mech,&left_mech,&measuring,&frame,&guard,&payoff})for(const auto& q:*g){
        const auto qb=se::bounds(q.shape);
        if(qb.zmax<=wb.zmin||qb.zmin>=wb.zmax||qb.ymax<=wb.ymin||qb.ymin>=wb.ymax)continue;
        BRepAlgoAPI_Common c(wire,q.shape);c.Build();if(!c.IsDone())throw std::runtime_error("Wire boolean failed");
        const double v=se::volume(c.Shape());
        if(v>1e-3)throw std::runtime_error("Wire envelope intersects "+q.id+": "+std::to_string(v)+" mm3");
        wire_common+=v;}
    if(wire_common>1e-3)throw std::runtime_error("Wire envelope intersects the mechanism: "+std::to_string(wire_common)+" mm3");
    std::size_t count=0;for(const Group* g:groups)count+=g->size();
    if(!out.empty()) {
        fs::create_directories(out);
        std::ofstream pending(out/"status.json");
        pending<<"{\"mechanism_revision\":\""<<starz::mechanism::revision
             <<"\",\"run_complete\":false,\"guard_door_kinematics_validated\":false,\"guard_safety_function_validated\":false,\"payoff_selection_complete\":false,\"payoff_rotation_validated\":false,\"full_machine_integrated\":false,\"physical_results\":false,\"fabrication_released\":false}\n";
        pending.close();
         if(!pending)throw std::runtime_error("Mechanism pending-status output failed");
        BRep_Builder builder;TopoDS_Compound compound;builder.MakeCompound(compound);
        std::ofstream manifest(out/"manifest.csv");manifest<<std::setprecision(12)<<"id,description,purchased,xmin_mm,ymin_mm,zmin_mm,xmax_mm,ymax_mm,zmax_mm,volume_mm3\n";
        std::map<std::string,std::pair<int,bool>> bom;
        for(const Group* g:groups)for(const auto& part:*g){
            builder.Add(compound,part.shape);const auto b=se::bounds(part.shape);
            manifest<<part.id<<",\""<<part.description<<"\","<<(part.purchased?1:0)<<','<<b.xmin<<','<<b.ymin<<','<<b.zmin<<','<<b.xmax<<','<<b.ymax<<','<<b.zmax<<','<<se::volume(part.shape)<<'\n';
            if(g==&right_mech||g==&left_mech||g==&measuring||g==&frame||g==&guard||g==&payoff){auto& e=bom[part.description];++e.first;e.second=part.purchased;}}
        std::ofstream bomfile(out/"mechanism-bom.csv");bomfile<<"description,quantity,purchased\n";
        for(const auto& [k,v]:bom)bomfile<<'"'<<k<<"\","<<v.first<<','<<(v.second?1:0)<<'\n';
        manifest.close();bomfile.close();
        if(!manifest||!bomfile)throw std::runtime_error("Mechanism manifest/BOM output failed");
        readback(compound,out/"full-machine.step");
        std::ofstream status(out/"status.json");
        status<<std::setprecision(6)<<"{\n  \"mechanism_revision\":\""<<starz::mechanism::revision<<"\",\n  \"head_revision\":\""<<starz::head::revision<<"\",\n  \"feed_revision\":\""<<starz::feed::revision<<"\",\n  \"measurement_revision\":\""<<starz::measurement::revision<<"\",\n"
              <<"  \"run_complete\":true,\n  \"wire_od_mm\":"<<d<<",\n  \"occurrences\":"<<count<<",\n"
              <<"  \"carried_mass_estimate_kg\":"<<mass<<",\n  \"carrier_adjuster_protrusion_mm\":"<<p<<",\n  \"carrier_spring_length_mm\":"<<cs.spring_length_mm<<",\n  \"carrier_spring_force_n\":"<<cs.spring_force_n<<",\n  \"nip_preload_per_nip_n\":"<<cs.nip_force_n<<",\n"
              <<"  \"arm_adjuster_protrusion_mm\":"<<ap<<",\n  \"arm_spring_length_mm\":"<<as.spring_length_mm<<",\n  \"wheel_contact_force_n\":"<<as.wheel_force_n<<",\n"
              <<"  \"minimum_moving_head_to_mechanism_X_gap_mm\":"<<gap<<",\n  \"wire_envelope_common_mm3\":"<<wire_common<<",\n"
              <<"  \"scope\":\"head,two cassettes,two pressure carriers,two wire guides,measuring module,bed,welded frame,R2 guard/payoff selections with 6204 bearing pockets;cabinet and safety functions not modelled\",\n"
              <<"  \"cross_subsystem_static_intersections_checked\":true,\n  \"external_wire_route_checked\":true,\n  \"guard_door_kinematics_validated\":false,\n  \"guard_safety_function_validated\":false,\n  \"payoff_selection_complete\":false,\n  \"payoff_rotation_validated\":false,\n  \"step_readback\":true,\n  \"frame_integrated\":true,\n  \"full_machine_integrated\":false,\n  \"physical_results\":false,\n  \"fabrication_released\":false\n}\n";
        status.close();
        if(!status)throw std::runtime_error("Mechanism status output failed");
    }
    return {d,mass,p,cs.nip_force_n,as.wheel_force_n,gap,wire_common,count};
}
}
int main(int argc,char** argv)try {
    fs::path out;double d=4.5,carrier_p=std::numeric_limits<double>::quiet_NaN(),arm_p=carrier_p;
    std::string reject;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--out"&&i+1<argc)out=argv[++i];
        else if(arg=="--wire-od"&&i+1<argc)d=std::stod(argv[++i]);
        else if(arg=="--carrier-protrusion"&&i+1<argc)carrier_p=std::stod(argv[++i]);
        else if(arg=="--arm-protrusion"&&i+1<argc)arm_p=std::stod(argv[++i]);
        else if(arg.starts_with("--reject-"))reject=arg.substr(9);
        else if(arg!="--check")throw std::invalid_argument("Unknown mechanism argument");
    }
    if(!reject.empty()) {
        const char* expected=nullptr;
        if(reject=="bushing-clash")expected="Interference";
        else if(reject=="slack-spring"){carrier_p=0;expected="Nip preload";}
        else if(reject=="light-wheel"){arm_p=0;expected="Wheel contact force";}
        else if(reject=="guard-wire-route")expected="Wire envelope intersects";
        else throw std::invalid_argument("Unknown rejection case");
        try{evaluate(d,carrier_p,arm_p,reject=="bushing-clash",reject=="guard-wire-route",{});}
        catch(const std::runtime_error& e){
            if(std::string(e.what()).starts_with(expected)){std::cout<<"Rejected as required: "<<e.what()<<'\n';return 0;}
            throw;}
        throw std::runtime_error("Defective configuration "+reject+" was not rejected");
    }
    const auto r=evaluate(d,carrier_p,arm_p,false,false,out);
    std::cout<<r.occurrences<<"-part machine layout checked at wire "<<r.wire_od<<" mm;carried mass "<<r.carried_mass_kg<<" kg;adjuster "<<r.carrier_protrusion_mm
             <<" mm;nip preload "<<r.nip_force_n<<" N;wheel force "<<r.wheel_force_n<<" N;head/guide X-gap "<<r.head_gap_mm<<" mm\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
